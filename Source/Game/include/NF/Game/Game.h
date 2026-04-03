#pragma once
// NF::Game — Game layer: voxels, interaction loop, R.I.G. system, inventory
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"

namespace NF {

// ── Voxel types ──────────────────────────────────────────────────

enum class VoxelType : uint8_t {
    Air = 0,
    Stone,
    Dirt,
    Grass,
    Metal,
    Glass,
    Water,
    Ore_Iron,
    Ore_Gold,
    Ore_Crystal,
    Count
};

inline const char* voxelTypeName(VoxelType type) {
    switch (type) {
        case VoxelType::Air:         return "Air";
        case VoxelType::Stone:       return "Stone";
        case VoxelType::Dirt:        return "Dirt";
        case VoxelType::Grass:       return "Grass";
        case VoxelType::Metal:       return "Metal";
        case VoxelType::Glass:       return "Glass";
        case VoxelType::Water:       return "Water";
        case VoxelType::Ore_Iron:    return "Ore_Iron";
        case VoxelType::Ore_Gold:    return "Ore_Gold";
        case VoxelType::Ore_Crystal: return "Ore_Crystal";
        default:                     return "Unknown";
    }
}

inline VoxelType voxelTypeFromName(std::string_view name) {
    if (name == "Stone")       return VoxelType::Stone;
    if (name == "Dirt")        return VoxelType::Dirt;
    if (name == "Grass")       return VoxelType::Grass;
    if (name == "Metal")       return VoxelType::Metal;
    if (name == "Glass")       return VoxelType::Glass;
    if (name == "Water")       return VoxelType::Water;
    if (name == "Ore_Iron")    return VoxelType::Ore_Iron;
    if (name == "Ore_Gold")    return VoxelType::Ore_Gold;
    if (name == "Ore_Crystal") return VoxelType::Ore_Crystal;
    return VoxelType::Air;
}

constexpr int CHUNK_SIZE = 16;

struct Chunk {
    VoxelType voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {};
    int cx = 0, cy = 0, cz = 0;
    bool meshDirty = true;
    bool collisionDirty = true;
    bool meshed = false;

    VoxelType get(int x, int y, int z) const {
        if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE)
            return VoxelType::Air;
        return voxels[x][y][z];
    }

    void set(int x, int y, int z, VoxelType type) {
        if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE)
            return;
        voxels[x][y][z] = type;
        meshDirty = true;
        collisionDirty = true;
        meshed = false;
    }

    void markMeshClean() { meshDirty = false; meshed = true; }
    void markCollisionClean() { collisionDirty = false; }
    void markAllClean() { meshDirty = false; collisionDirty = false; meshed = true; }

    [[nodiscard]] bool isFullyAir() const {
        for (int x = 0; x < CHUNK_SIZE; ++x)
            for (int y = 0; y < CHUNK_SIZE; ++y)
                for (int z = 0; z < CHUNK_SIZE; ++z)
                    if (voxels[x][y][z] != VoxelType::Air) return false;
        return true;
    }

    [[nodiscard]] int solidCount() const {
        int count = 0;
        for (int x = 0; x < CHUNK_SIZE; ++x)
            for (int y = 0; y < CHUNK_SIZE; ++y)
                for (int z = 0; z < CHUNK_SIZE; ++z)
                    if (voxels[x][y][z] != VoxelType::Air) ++count;
        return count;
    }
};

// ── Chunk Serializer ─────────────────────────────────────────────

class ChunkSerializer {
public:
    static JsonValue toJson(const Chunk& chunk) {
        auto obj = JsonValue::object();
        obj.set("cx", JsonValue(chunk.cx));
        obj.set("cy", JsonValue(chunk.cy));
        obj.set("cz", JsonValue(chunk.cz));
        obj.set("size", JsonValue(CHUNK_SIZE));

        // RLE-encode the voxel data for compact storage
        auto voxelArray = JsonValue::array();
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            for (int y = 0; y < CHUNK_SIZE; ++y) {
                for (int z = 0; z < CHUNK_SIZE; ++z) {
                    voxelArray.push(JsonValue(static_cast<int32_t>(chunk.voxels[x][y][z])));
                }
            }
        }
        obj.set("voxels", std::move(voxelArray));
        return obj;
    }

    static Chunk fromJson(const JsonValue& json) {
        Chunk chunk;
        chunk.cx = json["cx"].asInt();
        chunk.cy = json["cy"].asInt();
        chunk.cz = json["cz"].asInt();

        const auto& voxelArray = json["voxels"];
        if (voxelArray.isArray()) {
            size_t idx = 0;
            for (int x = 0; x < CHUNK_SIZE; ++x) {
                for (int y = 0; y < CHUNK_SIZE; ++y) {
                    for (int z = 0; z < CHUNK_SIZE; ++z) {
                        if (idx < voxelArray.size()) {
                            chunk.voxels[x][y][z] = static_cast<VoxelType>(voxelArray[idx].asInt());
                            ++idx;
                        }
                    }
                }
            }
        }

        chunk.meshDirty = true;
        chunk.collisionDirty = true;
        chunk.meshed = false;
        return chunk;
    }

    // Full round-trip: serialize to JSON string and back
    static std::string serialize(const Chunk& chunk) {
        return toJson(chunk).toJson(2);
    }

    static Chunk deserialize(const std::string& jsonStr) {
        auto parsed = JsonParser::parse(jsonStr);
        return fromJson(parsed);
    }
};

// ── Voxel Edit Command (undo-safe) ──────────────────────────────

class VoxelEditCommand : public ICommand {
public:
    VoxelEditCommand(Chunk* chunk, int x, int y, int z, VoxelType newType)
        : m_chunk(chunk), m_x(x), m_y(y), m_z(z),
          m_newType(newType), m_oldType(chunk->get(x, y, z)) {}

    void execute() override { m_chunk->set(m_x, m_y, m_z, m_newType); }
    void undo() override { m_chunk->set(m_x, m_y, m_z, m_oldType); }

    [[nodiscard]] std::string description() const override {
        return std::string("Set voxel (") + std::to_string(m_x) + "," +
               std::to_string(m_y) + "," + std::to_string(m_z) + ") to " +
               voxelTypeName(m_newType);
    }

private:
    Chunk* m_chunk;
    int m_x, m_y, m_z;
    VoxelType m_newType;
    VoxelType m_oldType;
};

// ── R.I.G. (Rig Interface Gear) ─────────────────────────────────

struct RigState {
    float health = 100.f;
    float energy = 100.f;
    int activeTool = 0;  // 0=mine, 1=place, 2=repair
};

// ── Inventory ────────────────────────────────────────────────────

struct Inventory {
    int counts[static_cast<size_t>(VoxelType::Count)] = {};

    void add(VoxelType type, int amount = 1) {
        counts[static_cast<size_t>(type)] += amount;
    }

    bool remove(VoxelType type, int amount = 1) {
        auto& c = counts[static_cast<size_t>(type)];
        if (c < amount) return false;
        c -= amount;
        return true;
    }

    [[nodiscard]] int count(VoxelType type) const {
        return counts[static_cast<size_t>(type)];
    }
};

} // namespace NF
