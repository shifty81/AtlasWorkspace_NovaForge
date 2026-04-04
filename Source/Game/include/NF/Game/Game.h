#pragma once
// NF::Game — Game layer: voxels, interaction loop, R.I.G. system, inventory
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/Physics/Physics.h"
#include <map>

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
    float maxHealth = 100.f;
    float energy = 100.f;
    float maxEnergy = 100.f;
    float oxygen = 100.f;
    float maxOxygen = 100.f;
    float stamina = 100.f;
    float maxStamina = 100.f;
    float energyRegenRate = 2.f;
    float staminaRegenRate = 5.f;
    float oxygenDrainRate = 1.f;
    int activeTool = 0;

    [[nodiscard]] bool isAlive() const { return health > 0.f; }

    void tick(float dt) {
        if (!isAlive()) return;
        energy = std::min(energy + energyRegenRate * dt, maxEnergy);
        stamina = std::min(stamina + staminaRegenRate * dt, maxStamina);
        oxygen = std::max(oxygen - oxygenDrainRate * dt, 0.f);
        if (oxygen <= 0.f) health = std::max(health - 10.f * dt, 0.f);
    }

    void takeDamage(float amount) {
        health = std::max(health - amount, 0.f);
    }

    void heal(float amount) {
        health = std::min(health + amount, maxHealth);
    }

    void consumeEnergy(float amount) {
        energy = std::max(energy - amount, 0.f);
    }

    void consumeStamina(float amount) {
        stamina = std::max(stamina - amount, 0.f);
    }
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

// ── Voxel color mapping ─────────────────────────────────────────

inline Vec4 voxelColor(VoxelType type) {
    switch (type) {
        case VoxelType::Stone:       return {0.6f, 0.6f, 0.6f, 1.f};
        case VoxelType::Dirt:        return {0.55f, 0.35f, 0.2f, 1.f};
        case VoxelType::Grass:       return {0.3f, 0.7f, 0.2f, 1.f};
        case VoxelType::Metal:       return {0.75f, 0.75f, 0.8f, 1.f};
        case VoxelType::Glass:       return {0.7f, 0.85f, 0.95f, 0.5f};
        case VoxelType::Water:       return {0.2f, 0.4f, 0.8f, 0.7f};
        case VoxelType::Ore_Iron:    return {0.65f, 0.5f, 0.4f, 1.f};
        case VoxelType::Ore_Gold:    return {0.9f, 0.8f, 0.2f, 1.f};
        case VoxelType::Ore_Crystal: return {0.6f, 0.2f, 0.9f, 1.f};
        default:                     return {1.f, 1.f, 1.f, 1.f};
    }
}

// ── Chunk Mesher ─────────────────────────────────────────────────
// Generates renderable geometry from chunk voxel data.
// Uses face-culling: only emits faces where a solid voxel is adjacent to air.

struct ChunkMeshResult {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    size_t faceCount = 0;
};

class ChunkMesher {
public:
    // Generate mesh for a chunk with face-culling.
    // worldOffsetX/Y/Z positions the chunk in world space.
    static ChunkMeshResult buildMesh(const Chunk& chunk) {
        ChunkMeshResult result;

        // 6 face directions: +X, -X, +Y, -Y, +Z, -Z
        struct FaceInfo {
            int dx, dy, dz;         // neighbor offset
            Vec3 normal;
            // 4 vertex offsets for a unit cube face (CCW winding)
            Vec3 v0, v1, v2, v3;
        };

        static const FaceInfo faces[6] = {
            // +X face (right)
            { 1, 0, 0, {1,0,0}, {1,0,0}, {1,1,0}, {1,1,1}, {1,0,1} },
            // -X face (left)
            {-1, 0, 0, {-1,0,0}, {0,0,1}, {0,1,1}, {0,1,0}, {0,0,0} },
            // +Y face (top)
            { 0, 1, 0, {0,1,0}, {0,1,0}, {0,1,1}, {1,1,1}, {1,1,0} },
            // -Y face (bottom)
            { 0,-1, 0, {0,-1,0}, {0,0,1}, {0,0,0}, {1,0,0}, {1,0,1} },
            // +Z face (front)
            { 0, 0, 1, {0,0,1}, {0,0,1}, {0,1,1}, {1,1,1}, {1,0,1} },
            // -Z face (back)
            { 0, 0,-1, {0,0,-1}, {1,0,0}, {1,1,0}, {0,1,0}, {0,0,0} },
        };

        float ox = static_cast<float>(chunk.cx * CHUNK_SIZE);
        float oy = static_cast<float>(chunk.cy * CHUNK_SIZE);
        float oz = static_cast<float>(chunk.cz * CHUNK_SIZE);

        for (int x = 0; x < CHUNK_SIZE; ++x) {
            for (int y = 0; y < CHUNK_SIZE; ++y) {
                for (int z = 0; z < CHUNK_SIZE; ++z) {
                    VoxelType vt = chunk.voxels[x][y][z];
                    if (vt == VoxelType::Air) continue;

                    Vec4 col = voxelColor(vt);

                    for (int f = 0; f < 6; ++f) {
                        const auto& face = faces[f];
                        int nx = x + face.dx;
                        int ny = y + face.dy;
                        int nz = z + face.dz;

                        // Emit face only if neighbor is air (exposed face)
                        if (chunk.get(nx, ny, nz) != VoxelType::Air) continue;

                        uint32_t baseIdx = static_cast<uint32_t>(result.vertices.size());
                        float fx = static_cast<float>(x) + ox;
                        float fy = static_cast<float>(y) + oy;
                        float fz = static_cast<float>(z) + oz;

                        auto makeVert = [&](const Vec3& off) -> Vertex {
                            return {{fx + off.x, fy + off.y, fz + off.z},
                                    face.normal, {0.f, 0.f}, col};
                        };

                        result.vertices.push_back(makeVert(face.v0));
                        result.vertices.push_back(makeVert(face.v1));
                        result.vertices.push_back(makeVert(face.v2));
                        result.vertices.push_back(makeVert(face.v3));

                        // Two triangles per face
                        result.indices.push_back(baseIdx + 0);
                        result.indices.push_back(baseIdx + 1);
                        result.indices.push_back(baseIdx + 2);
                        result.indices.push_back(baseIdx + 0);
                        result.indices.push_back(baseIdx + 2);
                        result.indices.push_back(baseIdx + 3);

                        ++result.faceCount;
                    }
                }
            }
        }
        return result;
    }

    // Build a Mesh object directly (convenience)
    static Mesh buildRenderMesh(const Chunk& chunk) {
        auto result = buildMesh(chunk);
        return Mesh(std::move(result.vertices), std::move(result.indices));
    }
};

// ── World State (multi-chunk container) ──────────────────────────

struct ChunkCoord {
    int x, y, z;

    bool operator==(const ChunkCoord& o) const { return x == o.x && y == o.y && z == o.z; }
    bool operator!=(const ChunkCoord& o) const { return !(*this == o); }
    bool operator<(const ChunkCoord& o) const {
        if (x != o.x) return x < o.x;
        if (y != o.y) return y < o.y;
        return z < o.z;
    }
};

} // namespace NF

// Hash specialization for ChunkCoord (must be before unordered_map usage)
template<> struct std::hash<NF::ChunkCoord> {
    size_t operator()(const NF::ChunkCoord& c) const noexcept {
        size_t h = std::hash<int>{}(c.x);
        h ^= std::hash<int>{}(c.y) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<int>{}(c.z) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

namespace NF {

class WorldState {
public:
    void clear() { m_chunks.clear(); }

    Chunk& getOrCreateChunk(int cx, int cy, int cz) {
        ChunkCoord coord{cx, cy, cz};
        auto it = m_chunks.find(coord);
        if (it == m_chunks.end()) {
            Chunk c;
            c.cx = cx; c.cy = cy; c.cz = cz;
            auto [newIt, _] = m_chunks.emplace(coord, std::move(c));
            return newIt->second;
        }
        return it->second;
    }

    Chunk* findChunk(int cx, int cy, int cz) {
        auto it = m_chunks.find({cx, cy, cz});
        return (it != m_chunks.end()) ? &it->second : nullptr;
    }

    const Chunk* findChunk(int cx, int cy, int cz) const {
        auto it = m_chunks.find({cx, cy, cz});
        return (it != m_chunks.end()) ? &it->second : nullptr;
    }

    void setChunk(const Chunk& chunk) {
        m_chunks[{chunk.cx, chunk.cy, chunk.cz}] = chunk;
    }

    void removeChunk(int cx, int cy, int cz) {
        m_chunks.erase({cx, cy, cz});
    }

    [[nodiscard]] size_t chunkCount() const { return m_chunks.size(); }

    // Iterate all chunks
    template<typename Fn>
    void forEach(Fn&& fn) {
        for (auto& [coord, chunk] : m_chunks) fn(chunk);
    }

    template<typename Fn>
    void forEach(Fn&& fn) const {
        for (const auto& [coord, chunk] : m_chunks) fn(chunk);
    }

    // Get voxel at world coordinates
    VoxelType getWorld(int wx, int wy, int wz) const {
        auto [cx, cy, cz, lx, ly, lz] = worldToLocal(wx, wy, wz);
        const Chunk* ch = findChunk(cx, cy, cz);
        return ch ? ch->get(lx, ly, lz) : VoxelType::Air;
    }

    // Set voxel at world coordinates (creates chunk if needed)
    void setWorld(int wx, int wy, int wz, VoxelType type) {
        auto [cx, cy, cz, lx, ly, lz] = worldToLocal(wx, wy, wz);
        getOrCreateChunk(cx, cy, cz).set(lx, ly, lz, type);
    }

private:
    // Convert world coordinates to chunk + local coordinates
    struct LocalCoord { int cx, cy, cz, lx, ly, lz; };
    static LocalCoord worldToLocal(int wx, int wy, int wz) {
        int cx = (wx >= 0) ? wx / CHUNK_SIZE : (wx - CHUNK_SIZE + 1) / CHUNK_SIZE;
        int cy = (wy >= 0) ? wy / CHUNK_SIZE : (wy - CHUNK_SIZE + 1) / CHUNK_SIZE;
        int cz = (wz >= 0) ? wz / CHUNK_SIZE : (wz - CHUNK_SIZE + 1) / CHUNK_SIZE;
        return {cx, cy, cz, wx - cx * CHUNK_SIZE, wy - cy * CHUNK_SIZE, wz - cz * CHUNK_SIZE};
    }
    std::unordered_map<ChunkCoord, Chunk> m_chunks;
};

// ── World Serializer ─────────────────────────────────────────────

class WorldSerializer {
public:
    static JsonValue toJson(const WorldState& world) {
        auto obj = JsonValue::object();
        obj.set("version", JsonValue(1));

        auto chunksArr = JsonValue::array();
        world.forEach([&](const Chunk& chunk) {
            chunksArr.push(ChunkSerializer::toJson(chunk));
        });
        obj.set("chunks", std::move(chunksArr));

        return obj;
    }

    static WorldState fromJson(const JsonValue& json) {
        WorldState world;
        const auto& chunksArr = json["chunks"];
        if (chunksArr.isArray()) {
            for (size_t i = 0; i < chunksArr.size(); ++i) {
                Chunk chunk = ChunkSerializer::fromJson(chunksArr[i]);
                world.setChunk(chunk);
            }
        }
        return world;
    }

    static std::string serialize(const WorldState& world) {
        return toJson(world).toJson(2);
    }

    static WorldState deserialize(const std::string& jsonStr) {
        auto parsed = JsonParser::parse(jsonStr);
        return fromJson(parsed);
    }
};

// ── Voxel Pick Service ──────────────────────────────────────────
// Raycast into a voxel world to find which voxel the ray hits.

struct VoxelHit {
    int wx, wy, wz;           // World-space voxel coordinates
    int face;                  // Which face was hit (0=+X, 1=-X, 2=+Y, 3=-Y, 4=+Z, 5=-Z)
    float distance;            // Distance along ray
    VoxelType type;            // Type of voxel hit
    Vec3 hitPoint;             // Exact hit point
    Vec3 normal;               // Hit face normal

    // Compute adjacent voxel position (for placing a new voxel)
    [[nodiscard]] ChunkCoord adjacentVoxel() const {
        static const int offsets[6][3] = {
            { 1, 0, 0}, {-1, 0, 0},
            { 0, 1, 0}, { 0,-1, 0},
            { 0, 0, 1}, { 0, 0,-1}
        };
        return {wx + offsets[face][0], wy + offsets[face][1], wz + offsets[face][2]};
    }
};

class VoxelPickService {
public:
    static constexpr float kEpsilon = 1e-8f;
    static constexpr float kMaxT    = 1e30f;

    // Cast a ray through the voxel world using DDA (Digital Differential Analyzer).
    // Returns the first solid voxel hit, or std::nullopt if nothing hit within maxDist.
    static std::optional<VoxelHit> raycast(const WorldState& world,
                                            const Vec3& origin, const Vec3& direction,
                                            float maxDist = 100.f) {
        // DDA voxel traversal
        Vec3 dir = direction.normalized();

        // Current voxel position
        int vx = static_cast<int>(std::floor(origin.x));
        int vy = static_cast<int>(std::floor(origin.y));
        int vz = static_cast<int>(std::floor(origin.z));

        // Step direction
        int stepX = (dir.x >= 0.f) ? 1 : -1;
        int stepY = (dir.y >= 0.f) ? 1 : -1;
        int stepZ = (dir.z >= 0.f) ? 1 : -1;

        // Distance to next voxel boundary
        float tMaxX = (std::abs(dir.x) > kEpsilon)
            ? ((stepX > 0 ? (vx + 1.f - origin.x) : (origin.x - vx)) / std::abs(dir.x))
            : kMaxT;
        float tMaxY = (std::abs(dir.y) > kEpsilon)
            ? ((stepY > 0 ? (vy + 1.f - origin.y) : (origin.y - vy)) / std::abs(dir.y))
            : kMaxT;
        float tMaxZ = (std::abs(dir.z) > kEpsilon)
            ? ((stepZ > 0 ? (vz + 1.f - origin.z) : (origin.z - vz)) / std::abs(dir.z))
            : kMaxT;

        // Distance between voxel boundaries
        float tDeltaX = (std::abs(dir.x) > kEpsilon) ? (1.f / std::abs(dir.x)) : kMaxT;
        float tDeltaY = (std::abs(dir.y) > kEpsilon) ? (1.f / std::abs(dir.y)) : kMaxT;
        float tDeltaZ = (std::abs(dir.z) > kEpsilon) ? (1.f / std::abs(dir.z)) : kMaxT;

        float t = 0.f;
        int lastFace = -1;

        // Check starting voxel
        VoxelType startType = world.getWorld(vx, vy, vz);
        if (startType != VoxelType::Air) {
            VoxelHit hit;
            hit.wx = vx; hit.wy = vy; hit.wz = vz;
            hit.face = 2; // default to +Y
            hit.distance = 0.f;
            hit.type = startType;
            hit.hitPoint = origin;
            hit.normal = {0, 1, 0};
            return hit;
        }

        while (t < maxDist) {
            // Step to next voxel
            if (tMaxX < tMaxY && tMaxX < tMaxZ) {
                t = tMaxX;
                vx += stepX;
                tMaxX += tDeltaX;
                lastFace = (stepX > 0) ? 1 : 0; // hit -X or +X face
            } else if (tMaxY < tMaxZ) {
                t = tMaxY;
                vy += stepY;
                tMaxY += tDeltaY;
                lastFace = (stepY > 0) ? 3 : 2; // hit -Y or +Y face
            } else {
                t = tMaxZ;
                vz += stepZ;
                tMaxZ += tDeltaZ;
                lastFace = (stepZ > 0) ? 5 : 4; // hit -Z or +Z face
            }

            if (t > maxDist) break;

            VoxelType vt = world.getWorld(vx, vy, vz);
            if (vt != VoxelType::Air) {
                static const Vec3 normals[6] = {
                    {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}
                };
                VoxelHit hit;
                hit.wx = vx; hit.wy = vy; hit.wz = vz;
                hit.face = lastFace;
                hit.distance = t;
                hit.type = vt;
                hit.hitPoint = origin + dir * t;
                hit.normal = normals[lastFace];
                return hit;
            }
        }

        return std::nullopt;
    }
};

// ── G1: Resource Types ──────────────────────────────────────────

enum class ResourceType : uint8_t {
    RawStone = 0,
    RawDirt,
    RawIron,
    RawGold,
    RawCrystal,
    RefinedIron,
    RefinedGold,
    RefinedCrystal,
    SteelPlate,
    CircuitBoard,
    EnergyCell,
    Count
};

inline const char* resourceTypeName(ResourceType type) {
    switch (type) {
        case ResourceType::RawStone:       return "RawStone";
        case ResourceType::RawDirt:        return "RawDirt";
        case ResourceType::RawIron:        return "RawIron";
        case ResourceType::RawGold:        return "RawGold";
        case ResourceType::RawCrystal:     return "RawCrystal";
        case ResourceType::RefinedIron:    return "RefinedIron";
        case ResourceType::RefinedGold:    return "RefinedGold";
        case ResourceType::RefinedCrystal: return "RefinedCrystal";
        case ResourceType::SteelPlate:     return "SteelPlate";
        case ResourceType::CircuitBoard:   return "CircuitBoard";
        case ResourceType::EnergyCell:     return "EnergyCell";
        default:                           return "Unknown";
    }
}

inline ResourceType resourceTypeFromName(std::string_view name) {
    if (name == "RawStone")       return ResourceType::RawStone;
    if (name == "RawDirt")        return ResourceType::RawDirt;
    if (name == "RawIron")        return ResourceType::RawIron;
    if (name == "RawGold")        return ResourceType::RawGold;
    if (name == "RawCrystal")     return ResourceType::RawCrystal;
    if (name == "RefinedIron")    return ResourceType::RefinedIron;
    if (name == "RefinedGold")    return ResourceType::RefinedGold;
    if (name == "RefinedCrystal") return ResourceType::RefinedCrystal;
    if (name == "SteelPlate")     return ResourceType::SteelPlate;
    if (name == "CircuitBoard")   return ResourceType::CircuitBoard;
    if (name == "EnergyCell")     return ResourceType::EnergyCell;
    return ResourceType::RawStone;
}

// ── G1: Resource Drop Table ─────────────────────────────────────

struct ResourceDrop {
    ResourceType resource;
    int minAmount;
    int maxAmount;
};

inline std::vector<ResourceDrop> getResourceDrops(VoxelType type) {
    switch (type) {
        case VoxelType::Stone:       return {{ResourceType::RawStone, 1, 1}};
        case VoxelType::Dirt:        return {{ResourceType::RawDirt, 1, 1}};
        case VoxelType::Ore_Iron:    return {{ResourceType::RawIron, 1, 2}};
        case VoxelType::Ore_Gold:    return {{ResourceType::RawGold, 1, 2}};
        case VoxelType::Ore_Crystal: return {{ResourceType::RawCrystal, 1, 3}};
        default:                     return {};
    }
}

// ── G1: Resource Inventory ──────────────────────────────────────

struct ResourceInventory {
    int counts[static_cast<size_t>(ResourceType::Count)] = {};

    void add(ResourceType type, int amount = 1) {
        counts[static_cast<size_t>(type)] += amount;
    }

    bool remove(ResourceType type, int amount = 1) {
        auto& c = counts[static_cast<size_t>(type)];
        if (c < amount) return false;
        c -= amount;
        return true;
    }

    [[nodiscard]] int count(ResourceType type) const {
        return counts[static_cast<size_t>(type)];
    }

    [[nodiscard]] int totalItems() const {
        int total = 0;
        for (int i = 0; i < static_cast<int>(ResourceType::Count); ++i)
            total += counts[i];
        return total;
    }

    [[nodiscard]] bool isEmpty() const {
        return totalItems() == 0;
    }
};

// ── G1: Tool System ─────────────────────────────────────────────

enum class ToolType : uint8_t { MiningLaser = 0, PlacementTool, RepairTool, Scanner, Count };

inline const char* toolTypeName(ToolType type) {
    switch (type) {
        case ToolType::MiningLaser:   return "MiningLaser";
        case ToolType::PlacementTool: return "PlacementTool";
        case ToolType::RepairTool:    return "RepairTool";
        case ToolType::Scanner:       return "Scanner";
        default:                      return "Unknown";
    }
}

struct ToolState {
    ToolType type = ToolType::MiningLaser;
    float durability = 100.f;
    float energyCost = 5.f;
    float cooldown = 0.f;
    float cooldownRate = 0.5f;
    float miningDamage = 25.f;

    [[nodiscard]] bool isReady() const { return cooldown <= 0.f && durability > 0.f; }

    void tick(float dt) {
        if (cooldown > 0.f) cooldown = std::max(cooldown - dt, 0.f);
    }

    void use() {
        cooldown = cooldownRate;
        durability = std::max(durability - 1.f, 0.f);
    }
};

// ── G1: Tool Belt ───────────────────────────────────────────────

class ToolBelt {
    static constexpr int MAX_SLOTS = 4;
    ToolState m_slots[MAX_SLOTS];
    int m_activeSlot = 0;
public:
    void init() {
        m_slots[0] = {ToolType::MiningLaser,   100.f, 5.f, 0.f, 0.5f, 25.f};
        m_slots[1] = {ToolType::PlacementTool,  100.f, 2.f, 0.f, 0.3f, 0.f};
        m_slots[2] = {ToolType::RepairTool,     100.f, 8.f, 0.f, 1.0f, 0.f};
        m_slots[3] = {ToolType::Scanner,        100.f, 3.f, 0.f, 0.2f, 0.f};
        m_activeSlot = 0;
    }

    ToolState& activeTool() { return m_slots[m_activeSlot]; }
    const ToolState& activeTool() const { return m_slots[m_activeSlot]; }

    void selectSlot(int slot) {
        m_activeSlot = std::max(0, std::min(slot, MAX_SLOTS - 1));
    }

    [[nodiscard]] int activeSlot() const { return m_activeSlot; }

    void nextTool() { m_activeSlot = (m_activeSlot + 1) % MAX_SLOTS; }
    void prevTool() { m_activeSlot = (m_activeSlot + MAX_SLOTS - 1) % MAX_SLOTS; }

    ToolState& slot(int i) { return m_slots[std::max(0, std::min(i, MAX_SLOTS - 1))]; }
    const ToolState& slot(int i) const { return m_slots[std::max(0, std::min(i, MAX_SLOTS - 1))]; }

    [[nodiscard]] int slotCount() const { return MAX_SLOTS; }
};

// ── G1: HUD State ───────────────────────────────────────────────

struct HUDNotification {
    std::string message;
    float timeRemaining = 3.f;
};

struct HUDState {
    bool showCrosshair = true;
    bool targetLocked = false;
    VoxelType targetVoxel = VoxelType::Air;

    std::vector<HUDNotification> notifications;

    void addNotification(const std::string& msg, float duration = 3.f) {
        notifications.push_back({msg, duration});
    }

    void tick(float dt) {
        for (auto& n : notifications) n.timeRemaining -= dt;
        notifications.erase(
            std::remove_if(notifications.begin(), notifications.end(),
                           [](const HUDNotification& n) { return n.timeRemaining <= 0.f; }),
            notifications.end());
    }

    void clearNotifications() { notifications.clear(); }
};

// ── G1: Interaction System ──────────────────────────────────────

struct MineResult {
    bool success = false;
    VoxelType minedType = VoxelType::Air;
    std::vector<ResourceDrop> drops;
    float energyUsed = 0.f;
};

struct PlaceResult {
    bool success = false;
    VoxelType placedType = VoxelType::Air;
};

class InteractionSystem {
public:
    MineResult tryMine(WorldState& world, RigState& rig, ToolBelt& belt,
                       Inventory& voxelInv, ResourceInventory& resInv,
                       const Vec3& origin, const Vec3& direction) {
        MineResult result;
        auto& tool = belt.activeTool();
        if (tool.type != ToolType::MiningLaser) return result;
        if (!tool.isReady()) return result;
        if (rig.energy < tool.energyCost) return result;

        auto hit = VoxelPickService::raycast(world, origin, direction, m_maxReach);
        if (!hit.has_value()) return result;

        result.minedType = hit->type;
        world.setWorld(hit->wx, hit->wy, hit->wz, VoxelType::Air);

        rig.consumeEnergy(tool.energyCost);
        tool.use();

        voxelInv.add(result.minedType);

        auto drops = getResourceDrops(result.minedType);
        for (auto& drop : drops) {
            resInv.add(drop.resource, drop.minAmount);
            result.drops.push_back(drop);
        }

        result.success = true;
        result.energyUsed = tool.energyCost;
        return result;
    }

    PlaceResult tryPlace(WorldState& world, RigState& rig, ToolBelt& belt,
                         Inventory& voxelInv,
                         const Vec3& origin, const Vec3& direction,
                         VoxelType typeToPlace) {
        PlaceResult result;
        auto& tool = belt.activeTool();
        if (tool.type != ToolType::PlacementTool) return result;
        if (!tool.isReady()) return result;
        if (rig.energy < tool.energyCost) return result;
        if (!voxelInv.remove(typeToPlace)) return result;

        auto hit = VoxelPickService::raycast(world, origin, direction, m_maxReach);
        if (!hit.has_value()) {
            voxelInv.add(typeToPlace);
            return result;
        }

        auto adj = hit->adjacentVoxel();
        if (world.getWorld(adj.x, adj.y, adj.z) != VoxelType::Air) {
            voxelInv.add(typeToPlace);
            return result;
        }

        world.setWorld(adj.x, adj.y, adj.z, typeToPlace);
        rig.consumeEnergy(tool.energyCost);
        tool.use();

        result.success = true;
        result.placedType = typeToPlace;
        return result;
    }

    std::optional<VoxelHit> tryScan(const WorldState& world, const ToolBelt& belt,
                                     const Vec3& origin, const Vec3& direction) {
        const auto& tool = belt.activeTool();
        if (tool.type != ToolType::Scanner) return std::nullopt;
        if (!tool.isReady()) return std::nullopt;
        return VoxelPickService::raycast(world, origin, direction, m_maxReach);
    }

    [[nodiscard]] float maxReach() const { return m_maxReach; }
    void setMaxReach(float r) { m_maxReach = r; }

private:
    float m_maxReach = 8.f;
};

// ── G1: Game Session ────────────────────────────────────────────

class GameSession {
public:
    void init(uint32_t /*seed*/ = 42) {
        m_rig = RigState{};
        m_voxelInv = Inventory{};
        m_resourceInv = ResourceInventory{};
        m_toolBelt.init();
        m_world.clear();
        m_hud = HUDState{};
        m_interaction = InteractionSystem{};
        m_active = true;
    }

    void shutdown() { m_active = false; }

    void tick(float dt) {
        if (!m_active) return;
        m_rig.tick(dt);
        for (int i = 0; i < m_toolBelt.slotCount(); ++i)
            m_toolBelt.slot(i).tick(dt);
        m_hud.tick(dt);
    }

    RigState& rig() { return m_rig; }
    const RigState& rig() const { return m_rig; }
    Inventory& voxelInventory() { return m_voxelInv; }
    const Inventory& voxelInventory() const { return m_voxelInv; }
    ResourceInventory& resourceInventory() { return m_resourceInv; }
    const ResourceInventory& resourceInventory() const { return m_resourceInv; }
    ToolBelt& toolBelt() { return m_toolBelt; }
    const ToolBelt& toolBelt() const { return m_toolBelt; }
    WorldState& world() { return m_world; }
    const WorldState& world() const { return m_world; }
    HUDState& hud() { return m_hud; }
    const HUDState& hud() const { return m_hud; }
    InteractionSystem& interaction() { return m_interaction; }
    const InteractionSystem& interaction() const { return m_interaction; }

    [[nodiscard]] bool isActive() const { return m_active; }

private:
    RigState m_rig;
    Inventory m_voxelInv;
    ResourceInventory m_resourceInv;
    ToolBelt m_toolBelt;
    WorldState m_world;
    HUDState m_hud;
    InteractionSystem m_interaction;
    bool m_active = false;
};

// ── G2: Chunk Render Data ───────────────────────────────────────

struct ChunkRenderData {
    ChunkCoord coord;
    Mesh mesh;
    bool valid = false;
    uint32_t version = 0;
};

// ── G2: Chunk Render Cache ─────────────────────────────────────

class ChunkRenderCache {
public:
    void update(const Chunk& chunk) {
        ChunkCoord coord{chunk.cx, chunk.cy, chunk.cz};
        auto& data = m_cache[coord];
        data.coord = coord;
        data.mesh = ChunkMesher::buildRenderMesh(chunk);
        data.valid = true;
        ++data.version;
    }

    void remove(const ChunkCoord& coord) {
        m_cache.erase(coord);
    }

    ChunkRenderData* get(const ChunkCoord& coord) {
        auto it = m_cache.find(coord);
        return (it != m_cache.end()) ? &it->second : nullptr;
    }

    const ChunkRenderData* get(const ChunkCoord& coord) const {
        auto it = m_cache.find(coord);
        return (it != m_cache.end()) ? &it->second : nullptr;
    }

    void clear() { m_cache.clear(); }

    size_t cacheSize() const { return m_cache.size(); }

    int updateDirty(WorldState& world) {
        int count = 0;
        world.forEach([&](Chunk& chunk) {
            if (chunk.meshDirty) {
                update(chunk);
                chunk.markMeshClean();
                ++count;
            }
        });
        return count;
    }

private:
    std::unordered_map<ChunkCoord, ChunkRenderData> m_cache;
};

// ── G2: Chunk Renderer ─────────────────────────────────────────

class ChunkRenderer {
public:
    void init() {
        m_voxelMaterial = Material(StringID("voxel_material"));
        m_initialized = true;
    }

    void shutdown() {
        m_initialized = false;
    }

    int render(const WorldState& world, ChunkRenderCache& cache,
               RenderQueue& queue, const Camera& camera, float aspect) {
        Mat4 vp = camera.projectionMatrix(aspect) * camera.viewMatrix();
        Frustum frustum;
        frustum.extractFromVP(vp);

        int visibleCount = 0;
        world.forEach([&](const Chunk& chunk) {
            float bx = static_cast<float>(chunk.cx * CHUNK_SIZE);
            float by = static_cast<float>(chunk.cy * CHUNK_SIZE);
            float bz = static_cast<float>(chunk.cz * CHUNK_SIZE);
            Vec3 aabbMin{bx, by, bz};
            Vec3 aabbMax{bx + CHUNK_SIZE, by + CHUNK_SIZE, bz + CHUNK_SIZE};

            if (!frustum.testAABB(aabbMin, aabbMax)) return;

            ChunkCoord coord{chunk.cx, chunk.cy, chunk.cz};
            auto* rd = cache.get(coord);
            if (!rd || !rd->valid || rd->mesh.vertexCount() == 0) return;

            RenderCommand cmd;
            cmd.mesh = &rd->mesh;
            cmd.material = &m_voxelMaterial;
            cmd.transform = Mat4::translation(0.f, 0.f, 0.f);
            cmd.sortKey = (camera.position - Vec3{bx + CHUNK_SIZE * 0.5f,
                                                   by + CHUNK_SIZE * 0.5f,
                                                   bz + CHUNK_SIZE * 0.5f}).lengthSq();
            queue.submit(std::move(cmd));
            ++visibleCount;
        });

        return visibleCount;
    }

    int countVisible(const WorldState& world, const Camera& camera, float aspect) const {
        Mat4 vp = camera.projectionMatrix(aspect) * camera.viewMatrix();
        Frustum frustum;
        frustum.extractFromVP(vp);

        int count = 0;
        world.forEach([&](const Chunk& chunk) {
            float bx = static_cast<float>(chunk.cx * CHUNK_SIZE);
            float by = static_cast<float>(chunk.cy * CHUNK_SIZE);
            float bz = static_cast<float>(chunk.cz * CHUNK_SIZE);
            Vec3 aabbMin{bx, by, bz};
            Vec3 aabbMax{bx + CHUNK_SIZE, by + CHUNK_SIZE, bz + CHUNK_SIZE};

            if (frustum.testAABB(aabbMin, aabbMax)) ++count;
        });

        return count;
    }

    Material& voxelMaterial() { return m_voxelMaterial; }

private:
    Material m_voxelMaterial;
    bool m_initialized = false;
};

// ── G3: Movement & FPS Camera ────────────────────────────────────

class FPSCamera {
public:
    void init(const Vec3& position, float yaw = -90.f, float pitch = 0.f) {
        m_position = position;
        m_yaw = yaw;
        m_pitch = pitch;
        updateVectors();
    }

    void processMouseLook(float deltaX, float deltaY) {
        m_yaw += deltaX * m_sensitivity;
        m_pitch += deltaY * m_sensitivity;
        if (m_pitch > m_maxPitch) m_pitch = m_maxPitch;
        if (m_pitch < m_minPitch) m_pitch = m_minPitch;
        updateVectors();
    }

    [[nodiscard]] Vec3 position() const { return m_position; }
    [[nodiscard]] Vec3 forward() const { return m_forward; }
    [[nodiscard]] Vec3 right() const { return m_right; }
    [[nodiscard]] Vec3 up() const { return m_up; }
    [[nodiscard]] float yaw() const { return m_yaw; }
    [[nodiscard]] float pitch() const { return m_pitch; }

    void setPosition(const Vec3& pos) { m_position = pos; }
    void setSensitivity(float s) { m_sensitivity = s; }
    [[nodiscard]] float sensitivity() const { return m_sensitivity; }

    void setPitchLimits(float minPitch, float maxPitch) {
        m_minPitch = minPitch;
        m_maxPitch = maxPitch;
    }

    [[nodiscard]] Camera toCamera(float fov = 60.f, float near = 0.1f, float far = 1000.f) const {
        Camera cam;
        cam.position = m_position;
        cam.target = m_position + m_forward;
        cam.up = m_up;
        cam.fov = fov;
        cam.nearPlane = near;
        cam.farPlane = far;
        return cam;
    }

private:
    Vec3 m_position{0, 0, 0};
    float m_yaw = -90.f;
    float m_pitch = 0.f;
    float m_sensitivity = 0.1f;
    float m_minPitch = -89.f;
    float m_maxPitch = 89.f;
    Vec3 m_forward{0, 0, -1};
    Vec3 m_right{1, 0, 0};
    Vec3 m_up{0, 1, 0};

    void updateVectors() {
        constexpr float DEG2RAD = 3.14159265358979f / 180.f;
        float yawRad = m_yaw * DEG2RAD;
        float pitchRad = m_pitch * DEG2RAD;
        Vec3 fwd;
        fwd.x = std::cos(yawRad) * std::cos(pitchRad);
        fwd.y = std::sin(pitchRad);
        fwd.z = std::sin(yawRad) * std::cos(pitchRad);
        m_forward = fwd.normalized();
        const Vec3 worldUp{0.f, 1.f, 0.f};
        m_right = m_forward.cross(worldUp).normalized();
        m_up = m_right.cross(m_forward).normalized();
    }
};

struct MovementInput {
    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
    bool jump = false;
    bool sprint = false;
    bool crouch = false;
};

class PlayerMovement {
public:
    void init(const Vec3& startPosition) {
        m_position = startPosition;
        m_velocity = {0, 0, 0};
        m_grounded = false;
        m_sprinting = false;
        m_crouching = false;
    }

    void update(float dt, const MovementInput& input, const FPSCamera& camera) {
        // Flatten camera vectors to XZ plane
        Vec3 camFwd = camera.forward();
        camFwd.y = 0.f;
        camFwd = camFwd.normalized();
        Vec3 camRight = camera.right();
        camRight.y = 0.f;
        camRight = camRight.normalized();

        Vec3 moveDir{0, 0, 0};
        if (input.forward)  moveDir = moveDir + camFwd;
        if (input.backward) moveDir = moveDir - camFwd;
        if (input.right)    moveDir = moveDir + camRight;
        if (input.left)     moveDir = moveDir - camRight;

        if (moveDir.lengthSq() > 1e-6f) moveDir = moveDir.normalized();

        m_sprinting = input.sprint && !input.crouch;
        m_crouching = input.crouch && !input.sprint;
        float speed = m_walkSpeed;
        if (m_sprinting) speed = m_sprintSpeed;
        else if (m_crouching) speed = m_crouchSpeed;

        m_velocity.x = moveDir.x * speed;
        m_velocity.z = moveDir.z * speed;

        // Gravity
        m_velocity.y += m_gravity * dt;

        // Jump
        if (m_grounded && input.jump) {
            m_velocity.y = m_jumpForce;
            m_grounded = false;
        }

        // Integrate
        m_position.x += m_velocity.x * dt;
        m_position.y += m_velocity.y * dt;
        m_position.z += m_velocity.z * dt;

        // Ground plane
        if (m_position.y < 0.f) {
            m_position.y = 0.f;
            m_velocity.y = 0.f;
            m_grounded = true;
        }
    }

    [[nodiscard]] Vec3 position() const { return m_position; }
    [[nodiscard]] Vec3 velocity() const { return m_velocity; }
    [[nodiscard]] bool isGrounded() const { return m_grounded; }
    [[nodiscard]] bool isSprinting() const { return m_sprinting; }
    [[nodiscard]] bool isCrouching() const { return m_crouching; }
    [[nodiscard]] float currentSpeed() const {
        return std::sqrt(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);
    }

    void setWalkSpeed(float s) { m_walkSpeed = s; }
    void setSprintSpeed(float s) { m_sprintSpeed = s; }
    void setCrouchSpeed(float s) { m_crouchSpeed = s; }
    void setJumpForce(float f) { m_jumpForce = f; }
    void setGravity(float g) { m_gravity = g; }
    [[nodiscard]] float walkSpeed() const { return m_walkSpeed; }
    [[nodiscard]] float sprintSpeed() const { return m_sprintSpeed; }
    [[nodiscard]] float crouchSpeed() const { return m_crouchSpeed; }
    [[nodiscard]] float jumpForce() const { return m_jumpForce; }
    [[nodiscard]] float gravity() const { return m_gravity; }

private:
    Vec3 m_position{0, 0, 0};
    Vec3 m_velocity{0, 0, 0};
    float m_walkSpeed = 5.f;
    float m_sprintSpeed = 8.f;
    float m_crouchSpeed = 2.5f;
    float m_jumpForce = 7.f;
    float m_gravity = -20.f;
    bool m_grounded = false;
    bool m_sprinting = false;
    bool m_crouching = false;
};

struct PlayerAABB {
    Vec3 min;
    Vec3 max;

    static PlayerAABB fromPosition(const Vec3& pos, float width = 0.6f, float height = 1.8f) {
        float hw = width * 0.5f;
        return {{pos.x - hw, pos.y, pos.z - hw},
                {pos.x + hw, pos.y + height, pos.z + hw}};
    }
};

class VoxelCollider {
public:
    Vec3 resolveCollision(const WorldState& world, const Vec3& position,
                          const Vec3& velocity, float dt,
                          float playerWidth = 0.6f, float playerHeight = 1.8f) {
        Vec3 pos = position;

        // X axis
        Vec3 tryX = pos;
        tryX.x += velocity.x * dt;
        if (aabbOverlapsSolid(world, PlayerAABB::fromPosition(tryX, playerWidth, playerHeight))) {
            tryX.x = pos.x;
        }
        pos = tryX;

        // Y axis
        Vec3 tryY = pos;
        tryY.y += velocity.y * dt;
        if (aabbOverlapsSolid(world, PlayerAABB::fromPosition(tryY, playerWidth, playerHeight))) {
            tryY.y = pos.y;
        }
        pos = tryY;

        // Z axis
        Vec3 tryZ = pos;
        tryZ.z += velocity.z * dt;
        if (aabbOverlapsSolid(world, PlayerAABB::fromPosition(tryZ, playerWidth, playerHeight))) {
            tryZ.z = pos.z;
        }
        pos = tryZ;

        return pos;
    }

    [[nodiscard]] bool wouldCollide(const WorldState& world, const Vec3& position,
                                    float playerWidth = 0.6f, float playerHeight = 1.8f) const {
        return aabbOverlapsSolid(world, PlayerAABB::fromPosition(position, playerWidth, playerHeight));
    }

    [[nodiscard]] bool isOnGround(const WorldState& world, const Vec3& position,
                                  float playerWidth = 0.6f) const {
        float hw = playerWidth * 0.5f;
        float belowY = position.y - 0.05f;
        int minVX = static_cast<int>(std::floor(position.x - hw));
        int maxVX = static_cast<int>(std::floor(position.x + hw));
        int minVZ = static_cast<int>(std::floor(position.z - hw));
        int maxVZ = static_cast<int>(std::floor(position.z + hw));
        int vy = static_cast<int>(std::floor(belowY));
        for (int vx = minVX; vx <= maxVX; ++vx)
            for (int vz = minVZ; vz <= maxVZ; ++vz)
                if (world.getWorld(vx, vy, vz) != VoxelType::Air) return true;
        return false;
    }

private:
    [[nodiscard]] bool aabbOverlapsSolid(const WorldState& world, const PlayerAABB& aabb) const {
        int minVX = static_cast<int>(std::floor(aabb.min.x));
        int maxVX = static_cast<int>(std::floor(aabb.max.x));
        int minVY = static_cast<int>(std::floor(aabb.min.y));
        int maxVY = static_cast<int>(std::floor(aabb.max.y));
        int minVZ = static_cast<int>(std::floor(aabb.min.z));
        int maxVZ = static_cast<int>(std::floor(aabb.max.z));
        for (int vx = minVX; vx <= maxVX; ++vx)
            for (int vy = minVY; vy <= maxVY; ++vy)
                for (int vz = minVZ; vz <= maxVZ; ++vz)
                    if (world.getWorld(vx, vy, vz) != VoxelType::Air) return true;
        return false;
    }
};

class PlayerController {
public:
    void init(const Vec3& startPosition) {
        m_movement.init(startPosition);
        m_camera.init(startPosition + Vec3{0, 1.6f, 0});
    }

    void update(float dt, const MovementInput& moveInput,
                float mouseDeltaX, float mouseDeltaY,
                WorldState& world) {
        m_camera.processMouseLook(mouseDeltaX, mouseDeltaY);
        m_movement.update(dt, moveInput, m_camera);

        Vec3 pos = m_movement.position();
        pos = m_collider.resolveCollision(world, pos, m_movement.velocity(), dt);

        m_camera.setPosition(pos + Vec3{0, 1.6f, 0});
    }

    FPSCamera& camera() { return m_camera; }
    const FPSCamera& camera() const { return m_camera; }
    PlayerMovement& movement() { return m_movement; }
    const PlayerMovement& movement() const { return m_movement; }
    VoxelCollider& collider() { return m_collider; }

    [[nodiscard]] Vec3 position() const { return m_movement.position(); }
    [[nodiscard]] Vec3 lookDirection() const { return m_camera.forward(); }

private:
    FPSCamera m_camera;
    PlayerMovement m_movement;
    VoxelCollider m_collider;
};

// ── G4: Ship Systems ─────────────────────────────────────────────

enum class ShipClass : uint8_t {
    Fighter,
    Corvette,
    Frigate,
    Cruiser,
    Freighter,
    Count
};

inline const char* shipClassName(ShipClass c) {
    switch (c) {
        case ShipClass::Fighter:   return "Fighter";
        case ShipClass::Corvette:  return "Corvette";
        case ShipClass::Frigate:   return "Frigate";
        case ShipClass::Cruiser:   return "Cruiser";
        case ShipClass::Freighter: return "Freighter";
        default:                   return "Unknown";
    }
}

enum class ModuleSlotType : uint8_t {
    Weapon,
    Shield,
    Engine,
    Reactor,
    Cargo,
    Utility,
    Count
};

inline const char* moduleSlotTypeName(ModuleSlotType t) {
    switch (t) {
        case ModuleSlotType::Weapon:  return "Weapon";
        case ModuleSlotType::Shield:  return "Shield";
        case ModuleSlotType::Engine:  return "Engine";
        case ModuleSlotType::Reactor: return "Reactor";
        case ModuleSlotType::Cargo:   return "Cargo";
        case ModuleSlotType::Utility: return "Utility";
        default:                      return "Unknown";
    }
}

struct ShipModule {
    StringID name;
    ModuleSlotType slotType = ModuleSlotType::Utility;
    int tier = 1;
    float health = 100.f;
    float maxHealth = 100.f;
    bool active = true;

    float damage = 0.f;
    float fireRate = 0.f;
    float shieldCapacity = 0.f;
    float shieldRegenRate = 0.f;
    float thrustPower = 0.f;
    float powerOutput = 0.f;
    float cargoCapacity = 0.f;

    bool isDestroyed() const { return health <= 0.f; }

    void takeDamage(float amount) {
        if (amount <= 0.f) return;
        health -= amount;
        if (health <= 0.f) {
            health = 0.f;
            active = false;
        }
    }

    void repair(float amount) {
        if (amount <= 0.f) return;
        health += amount;
        if (health > maxHealth) health = maxHealth;
        if (health > 0.f) active = true;
    }
};

struct ShipStats {
    float totalThrust = 0.f;
    float totalPowerOutput = 0.f;
    float totalPowerDraw = 0.f;
    float totalShieldCapacity = 0.f;
    float totalShieldRegen = 0.f;
    float totalCargoCapacity = 0.f;
    float maxWeaponDPS = 0.f;
    int activeModuleCount = 0;
    int destroyedModuleCount = 0;
};

class Ship {
public:
    void init(ShipClass shipClass, StringID shipName) {
        m_class = shipClass;
        m_name = shipName;
        m_modules.clear();
        switch (shipClass) {
            case ShipClass::Fighter:
                m_maxHull = 100.f; m_maxShield = 50.f; m_maxModules = 4; break;
            case ShipClass::Corvette:
                m_maxHull = 200.f; m_maxShield = 100.f; m_maxModules = 6; break;
            case ShipClass::Frigate:
                m_maxHull = 400.f; m_maxShield = 200.f; m_maxModules = 8; break;
            case ShipClass::Cruiser:
                m_maxHull = 800.f; m_maxShield = 400.f; m_maxModules = 12; break;
            case ShipClass::Freighter:
                m_maxHull = 300.f; m_maxShield = 75.f; m_maxModules = 10; break;
            default: break;
        }
        m_hull = m_maxHull;
        m_shield = m_maxShield;
        m_shieldRegenRate = 5.f;
    }

    bool addModule(const ShipModule& module) {
        if (static_cast<int>(m_modules.size()) >= m_maxModules) return false;
        m_modules.push_back(module);
        return true;
    }

    void removeModule(int index) {
        if (index >= 0 && index < static_cast<int>(m_modules.size()))
            m_modules.erase(m_modules.begin() + index);
    }

    ShipModule* module(int index) {
        if (index < 0 || index >= static_cast<int>(m_modules.size())) return nullptr;
        return &m_modules[static_cast<size_t>(index)];
    }

    const ShipModule* module(int index) const {
        if (index < 0 || index >= static_cast<int>(m_modules.size())) return nullptr;
        return &m_modules[static_cast<size_t>(index)];
    }

    int moduleCount() const { return static_cast<int>(m_modules.size()); }
    int maxModules() const { return m_maxModules; }

    float hull() const { return m_hull; }
    float maxHull() const { return m_maxHull; }

    void takeDamage(float amount) {
        if (amount <= 0.f) return;
        float remaining = amount;
        if (m_shield > 0.f) {
            if (m_shield >= remaining) {
                m_shield -= remaining;
                return;
            }
            remaining -= m_shield;
            m_shield = 0.f;
        }
        m_hull -= remaining;
        if (m_hull < 0.f) m_hull = 0.f;
    }

    void repair(float amount) {
        if (amount <= 0.f) return;
        m_hull += amount;
        if (m_hull > m_maxHull) m_hull = m_maxHull;
    }

    bool isDestroyed() const { return m_hull <= 0.f; }

    float shield() const { return m_shield; }
    float maxShield() const { return m_maxShield; }

    void rechargeShield(float dt) {
        float totalRegen = m_shieldRegenRate;
        for (const auto& mod : m_modules) {
            if (mod.active && mod.slotType == ModuleSlotType::Shield)
                totalRegen += mod.shieldRegenRate;
        }
        m_shield += totalRegen * dt;
        float maxS = m_maxShield;
        for (const auto& mod : m_modules) {
            if (mod.active && mod.slotType == ModuleSlotType::Shield)
                maxS += mod.shieldCapacity;
        }
        if (m_shield > maxS) m_shield = maxS;
    }

    ShipStats computeStats() const {
        ShipStats stats;
        for (const auto& mod : m_modules) {
            if (mod.isDestroyed()) {
                stats.destroyedModuleCount++;
                continue;
            }
            if (mod.active) {
                stats.activeModuleCount++;
                switch (mod.slotType) {
                    case ModuleSlotType::Engine:
                        stats.totalThrust += mod.thrustPower; break;
                    case ModuleSlotType::Reactor:
                        stats.totalPowerOutput += mod.powerOutput; break;
                    case ModuleSlotType::Shield:
                        stats.totalShieldCapacity += mod.shieldCapacity;
                        stats.totalShieldRegen += mod.shieldRegenRate; break;
                    case ModuleSlotType::Cargo:
                        stats.totalCargoCapacity += mod.cargoCapacity; break;
                    case ModuleSlotType::Weapon: {
                        float dps = (mod.fireRate > 0.f) ? mod.damage * mod.fireRate : 0.f;
                        if (dps > stats.maxWeaponDPS) stats.maxWeaponDPS = dps;
                        break;
                    }
                    default: break;
                }
            }
        }
        return stats;
    }

    ShipClass shipClass() const { return m_class; }
    StringID name() const { return m_name; }

private:
    ShipClass m_class = ShipClass::Fighter;
    StringID m_name;
    float m_hull = 100.f;
    float m_maxHull = 100.f;
    float m_shield = 50.f;
    float m_maxShield = 50.f;
    float m_shieldRegenRate = 5.f;
    std::vector<ShipModule> m_modules;
    int m_maxModules = 4;
};

// ── G4: Flight ───────────────────────────────────────────────────

struct FlightInput {
    float throttle = 0.f;
    float pitch = 0.f;
    float yaw = 0.f;
    float roll = 0.f;
    bool boost = false;
};

struct FlightState {
    Vec3 position{0, 0, 0};
    Vec3 velocity{0, 0, 0};
    Vec3 forward{0, 0, 1};
    Vec3 up{0, 1, 0};
    Vec3 right{1, 0, 0};
    float speed = 0.f;
    float maxSpeed = 100.f;
    float acceleration = 20.f;
    float turnRate = 90.f;
    float boostMultiplier = 2.f;
    bool boosting = false;
};

class FlightController {
public:
    void init(const Vec3& startPos) {
        m_state = FlightState{};
        m_state.position = startPos;
    }

    void update(float dt, const FlightInput& input, const Ship& ship) {
        constexpr float DEG2RAD = 3.14159265358979323846f / 180.f;
        float yawAngle   = -input.yaw   * m_state.turnRate * dt * DEG2RAD;
        float pitchAngle = -input.pitch  * m_state.turnRate * dt * DEG2RAD;

        // Yaw rotation around up axis
        if (std::fabs(yawAngle) > 1e-7f) {
            float cy = std::cos(yawAngle), sy = std::sin(yawAngle);
            Vec3 f = m_state.forward;
            m_state.forward = Vec3{
                f.x * cy + m_state.right.x * sy,
                f.y * cy + m_state.right.y * sy,
                f.z * cy + m_state.right.z * sy
            }.normalized();
            m_state.right = m_state.forward.cross(m_state.up).normalized();
        }

        // Pitch rotation around right axis
        if (std::fabs(pitchAngle) > 1e-7f) {
            float cp = std::cos(pitchAngle), sp = std::sin(pitchAngle);
            Vec3 f = m_state.forward;
            m_state.forward = Vec3{
                f.x * cp + m_state.up.x * sp,
                f.y * cp + m_state.up.y * sp,
                f.z * cp + m_state.up.z * sp
            }.normalized();
            m_state.up = m_state.right.cross(m_state.forward).normalized();
        }

        ShipStats stats = ship.computeStats();
        float thrust = stats.totalThrust > 0.f ? stats.totalThrust : m_state.acceleration;
        float accel = thrust * input.throttle;

        m_state.boosting = input.boost;
        if (m_state.boosting)
            accel *= m_state.boostMultiplier;

        float maxSpd = m_state.maxSpeed;
        if (m_state.boosting) maxSpd *= m_state.boostMultiplier;

        Vec3 targetVel = m_state.forward * (maxSpd * input.throttle);
        if (m_state.boosting)
            targetVel = m_state.forward * (maxSpd * std::fabs(input.throttle));

        Vec3 diff = targetVel - m_state.velocity;
        float diffLen = diff.length();
        float step = std::fabs(accel) * dt;
        if (diffLen > 0.f && step > 0.f) {
            if (step >= diffLen)
                m_state.velocity = targetVel;
            else
                m_state.velocity = m_state.velocity + diff.normalized() * step;
        }

        m_state.position = m_state.position + m_state.velocity * dt;
        m_state.speed = m_state.velocity.length();
    }

    const FlightState& state() const { return m_state; }
    FlightState& state() { return m_state; }
    Vec3 position() const { return m_state.position; }
    Vec3 velocity() const { return m_state.velocity; }
    float speed() const { return m_state.speed; }

private:
    FlightState m_state;
};

// ── G4: Combat ───────────────────────────────────────────────────

struct WeaponState {
    int moduleIndex = -1;
    float cooldown = 0.f;
    float range = 100.f;
    bool firing = false;

    bool isReady() const { return cooldown <= 0.f; }

    void tick(float dt) {
        if (cooldown > 0.f) {
            cooldown -= dt;
            if (cooldown < 0.f) cooldown = 0.f;
        }
    }
};

struct CombatTarget {
    Vec3 position;
    float distance = 0.f;
    bool inRange = false;
    bool inFiringArc = false;
};

class CombatSystem {
public:
    void init() { m_baseDamageVariance = 0.1f; }

    float calculateDamage(const Ship& ship, int weaponModuleIndex) const {
        const ShipModule* mod = ship.module(weaponModuleIndex);
        if (!mod || mod->slotType != ModuleSlotType::Weapon || !mod->active || mod->isDestroyed())
            return 0.f;
        return mod->damage;
    }

    void tickWeapons(float dt, std::vector<WeaponState>& weapons) {
        for (auto& w : weapons)
            w.tick(dt);
    }

    CombatTarget evaluateTarget(const Vec3& shipPos, const Vec3& shipForward,
                                const Vec3& targetPos, float weaponRange,
                                float firingArc = 30.f) const {
        CombatTarget ct;
        ct.position = targetPos;
        Vec3 toTarget = targetPos - shipPos;
        ct.distance = toTarget.length();
        ct.inRange = ct.distance <= weaponRange;

        if (ct.distance > 1e-7f) {
            Vec3 dir = toTarget.normalized();
            float dotVal = shipForward.dot(dir);
            if (dotVal > 1.f) dotVal = 1.f;
            if (dotVal < -1.f) dotVal = -1.f;
            float angle = std::acos(dotVal) * (180.f / 3.14159265358979323846f);
            ct.inFiringArc = angle <= firingArc;
        } else {
            ct.inFiringArc = true;
        }
        return ct;
    }

    float applyDamage(Ship& attacker, Ship& target, int weaponModuleIndex) {
        float dmg = calculateDamage(attacker, weaponModuleIndex);
        if (dmg <= 0.f) return 0.f;
        target.takeDamage(dmg);
        return dmg;
    }

private:
    float m_baseDamageVariance = 0.1f;
};

// ── G5: Fleet AI ─────────────────────────────────────────────────

enum class FormationType : uint8_t { Line, Wedge, Column, Spread, Defensive, Count };
inline const char* formationTypeName(FormationType t) {
    switch(t) {
        case FormationType::Line: return "Line";
        case FormationType::Wedge: return "Wedge";
        case FormationType::Column: return "Column";
        case FormationType::Spread: return "Spread";
        case FormationType::Defensive: return "Defensive";
        default: return "Unknown";
    }
}

struct FormationSlot {
    int shipIndex = -1;
    Vec3 offset{0,0,0};
    bool occupied = false;
};

class Formation {
public:
    void init(FormationType type, int shipCount, float spacing = 20.f) {
        m_type = type; m_spacing = spacing;
        m_slots.resize(shipCount);
        generateSlotOffsets();
    }
    FormationType type() const { return m_type; }
    int slotCount() const { return (int)m_slots.size(); }
    FormationSlot& slot(int i) { return m_slots[i]; }
    const FormationSlot& slot(int i) const { return m_slots[i]; }
    void setSpacing(float s) { m_spacing = s; generateSlotOffsets(); }
    float spacing() const { return m_spacing; }

    Vec3 getSlotWorldPosition(int idx, const Vec3& leaderPos, const Vec3& leaderFwd) const {
        if (idx < 0 || idx >= (int)m_slots.size()) return leaderPos;
        Vec3 up{0,1,0};
        Vec3 right = leaderFwd.cross(up).normalized();
        const Vec3& off = m_slots[idx].offset;
        return {leaderPos.x + right.x*off.x + leaderFwd.x*off.z,
                leaderPos.y + off.y,
                leaderPos.z + right.z*off.x + leaderFwd.z*off.z};
    }

private:
    FormationType m_type = FormationType::Line;
    std::vector<FormationSlot> m_slots;
    float m_spacing = 20.f;

    void generateSlotOffsets() {
        int n = (int)m_slots.size();
        for (int i = 0; i < n; ++i) {
            switch (m_type) {
                case FormationType::Line:
                    m_slots[i].offset = {(i - n/2) * m_spacing, 0, 0};
                    break;
                case FormationType::Wedge:
                    m_slots[i].offset = {(i - n/2) * m_spacing, 0, -(float)std::abs(i - n/2) * m_spacing};
                    break;
                case FormationType::Column:
                    m_slots[i].offset = {0, 0, -(float)i * m_spacing};
                    break;
                case FormationType::Spread:
                    m_slots[i].offset = {((i%3)-1) * m_spacing, 0, -(i/3) * m_spacing};
                    break;
                case FormationType::Defensive: {
                    float angle = (2.f * 3.14159265f * i) / (float)(n > 0 ? n : 1);
                    m_slots[i].offset = {std::cos(angle)*m_spacing, 0, std::sin(angle)*m_spacing};
                    break;
                }
                default: m_slots[i].offset = {0,0,0}; break;
            }
        }
    }
};

struct CaptainPersonality {
    float aggression = 0.5f;
    float caution = 0.5f;
    float loyalty = 0.5f;
    float initiative = 0.5f;
    float morale = 1.0f;
    float confidence = 1.0f;

    void adjustMorale(float delta) {
        morale += delta;
        if (morale < 0.f) morale = 0.f;
        if (morale > 1.f) morale = 1.f;
    }
    void adjustConfidence(float delta) {
        confidence += delta;
        if (confidence < 0.f) confidence = 0.f;
        if (confidence > 1.f) confidence = 1.f;
    }
    bool willFlee() const { return morale < 0.2f && caution > 0.6f; }
    bool willCharge() const { return aggression > 0.7f && confidence > 0.5f; }
};

enum class CaptainOrder : uint8_t {
    HoldPosition, AttackTarget, DefendTarget, FollowLeader, Patrol, Retreat, FreeEngage, Count
};
inline const char* captainOrderName(CaptainOrder o) {
    switch(o) {
        case CaptainOrder::HoldPosition: return "HoldPosition";
        case CaptainOrder::AttackTarget: return "AttackTarget";
        case CaptainOrder::DefendTarget: return "DefendTarget";
        case CaptainOrder::FollowLeader: return "FollowLeader";
        case CaptainOrder::Patrol: return "Patrol";
        case CaptainOrder::Retreat: return "Retreat";
        case CaptainOrder::FreeEngage: return "FreeEngage";
        default: return "Unknown";
    }
}

class AICaptain {
public:
    void init(StringID name, const CaptainPersonality& p) { m_name = name; m_personality = p; }
    StringID name() const { return m_name; }
    const CaptainPersonality& personality() const { return m_personality; }
    CaptainPersonality& personality() { return m_personality; }
    CaptainOrder currentOrder() const { return m_hasOverride ? m_overrideOrder : m_currentOrder; }
    void setOrder(CaptainOrder o) { m_currentOrder = o; }
    void overrideOrder(CaptainOrder o) { m_overrideOrder = o; m_hasOverride = true; }
    bool hasOverride() const { return m_hasOverride; }
    void clearOverride() { m_hasOverride = false; }

    CaptainOrder evaluate(float hullPct, float shieldPct, float enemyDist,
                          int alliesNearby, int enemiesNearby) const {
        if (m_personality.willFlee() && hullPct < 0.3f) return CaptainOrder::Retreat;
        if (m_personality.willCharge() && enemyDist < 200.f) return CaptainOrder::AttackTarget;
        if (enemiesNearby > alliesNearby * 2 && m_personality.caution > 0.5f)
            return CaptainOrder::DefendTarget;
        return m_hasOverride ? m_overrideOrder : m_currentOrder;
    }

private:
    StringID m_name;
    CaptainPersonality m_personality;
    CaptainOrder m_currentOrder = CaptainOrder::FollowLeader;
    CaptainOrder m_overrideOrder = CaptainOrder::FollowLeader;
    bool m_hasOverride = false;
};

struct FleetShip {
    Ship ship;
    AICaptain captain;
    FlightController flight;
    int formationSlot = -1;
    bool active = true;
};

class Fleet {
public:
    void init(StringID name) { m_name = name; }

    int addShip(FleetShip s) {
        int idx = (int)m_ships.size();
        m_ships.push_back(std::move(s));
        return idx;
    }
    void removeShip(int idx) {
        if (idx >= 0 && idx < (int)m_ships.size()) m_ships[idx].active = false;
    }
    FleetShip* ship(int i) { return (i>=0&&i<(int)m_ships.size()) ? &m_ships[i] : nullptr; }
    const FleetShip* ship(int i) const { return (i>=0&&i<(int)m_ships.size()) ? &m_ships[i] : nullptr; }
    int shipCount() const { return (int)m_ships.size(); }
    int activeShipCount() const {
        int c = 0; for (auto& s : m_ships) if (s.active) ++c; return c;
    }

    void setFormation(FormationType t, float spacing = 20.f) {
        m_formation.init(t, (int)m_ships.size(), spacing);
    }
    const Formation& formation() const { return m_formation; }
    Formation& formation() { return m_formation; }

    void issueOrder(CaptainOrder o) { for (auto& s : m_ships) s.captain.setOrder(o); }
    void issueOrderTo(int idx, CaptainOrder o) {
        if (auto* s = ship(idx)) s->captain.setOrder(o);
    }

    void setLeader(int idx) { m_leaderIndex = idx; }
    int leaderIndex() const { return m_leaderIndex; }
    FleetShip* leader() { return ship(m_leaderIndex); }

    void tick(float dt) {
        for (auto& s : m_ships) {
            if (!s.active) continue;
            float hullPct = s.ship.hull() / s.ship.maxHull();
            float shieldPct = s.ship.shield() / s.ship.maxShield();
            CaptainOrder ord = s.captain.evaluate(hullPct, shieldPct, 500.f, 1, 0);
            s.captain.setOrder(ord);
        }
    }

    float fleetMorale() const {
        if (m_ships.empty()) return 1.f;
        float sum = 0.f; int c = 0;
        for (auto& s : m_ships) if (s.active) { sum += s.captain.personality().morale; ++c; }
        return c > 0 ? sum / c : 1.f;
    }
    float fleetStrength() const {
        float sum = 0.f;
        for (auto& s : m_ships)
            if (s.active) sum += s.ship.hull() / s.ship.maxHull();
        return sum;
    }

    StringID name() const { return m_name; }

private:
    StringID m_name;
    std::vector<FleetShip> m_ships;
    Formation m_formation;
    int m_leaderIndex = 0;
};

// ── G6: Economy ───────────────────────────────────────────────────

struct MarketItem {
    ResourceType resource;
    int quantity = 0;
    float buyPrice = 10.f;
    float sellPrice = 8.f;
};

class Market {
public:
    void init() { m_items.clear(); }

    void listItem(ResourceType res, int qty, float buyPrice, float sellPrice) {
        for (auto& item : m_items) {
            if (item.resource == res) { item.quantity += qty; return; }
        }
        m_items.push_back({res, qty, buyPrice, sellPrice});
    }

    bool buy(ResourceInventory& inv, ResourceType res, int amount, float& credits) {
        for (auto& item : m_items) {
            if (item.resource != res) continue;
            if (item.quantity < amount) return false;
            float cost = item.buyPrice * amount;
            if (credits < cost) return false;
            credits -= cost;
            item.quantity -= amount;
            inv.add(res, amount);
            return true;
        }
        return false;
    }

    bool sell(ResourceInventory& inv, ResourceType res, int amount, float& credits) {
        if (inv.count(res) < amount) return false;
        float gain = 0.f;
        for (auto& item : m_items) {
            if (item.resource == res) { gain = item.sellPrice * amount; break; }
        }
        if (gain == 0.f) {
            gain = 5.f * amount;
        }
        inv.remove(res, amount);
        credits += gain;
        return true;
    }

    const MarketItem* findItem(ResourceType res) const {
        for (auto& item : m_items) if (item.resource == res) return &item;
        return nullptr;
    }

    int itemCount() const { return (int)m_items.size(); }

private:
    std::vector<MarketItem> m_items;
};

struct RefiningRecipe {
    ResourceType input;
    int inputAmount;
    ResourceType output;
    int outputAmount;
    float timeRequired;
};

class Refinery {
public:
    void addRecipe(const RefiningRecipe& r) { m_recipes.push_back(r); }

    const RefiningRecipe* findRecipe(ResourceType input) const {
        for (auto& r : m_recipes) if (r.input == input) return &r;
        return nullptr;
    }

    float startRefining(ResourceInventory& inv, ResourceType input, int amount) {
        const RefiningRecipe* r = findRecipe(input);
        if (!r) return 0.f;
        int batches = amount / r->inputAmount;
        if (batches <= 0) return 0.f;
        if (!inv.remove(input, batches * r->inputAmount)) return 0.f;
        m_pendingOutput.push_back({r->output, batches * r->outputAmount});
        return r->timeRequired * batches;
    }

    void collectOutput(ResourceInventory& inv) {
        for (auto& p : m_pendingOutput) inv.add(p.first, p.second);
        m_pendingOutput.clear();
    }

    int recipeCount() const { return (int)m_recipes.size(); }

private:
    std::vector<RefiningRecipe> m_recipes;
    std::vector<std::pair<ResourceType, int>> m_pendingOutput;
};

struct ManufacturingRecipe {
    StringID name;
    std::vector<std::pair<ResourceType, int>> inputs;
    ResourceType output;
    int outputAmount;
    float timeRequired;
};

class Manufacturer {
public:
    void addRecipe(const ManufacturingRecipe& r) { m_recipes.push_back(r); }

    const ManufacturingRecipe* findRecipe(StringID name) const {
        for (auto& r : m_recipes) if (r.name == name) return &r;
        return nullptr;
    }

    bool canCraft(const ResourceInventory& inv, StringID recipeName) const {
        const ManufacturingRecipe* r = findRecipe(recipeName);
        if (!r) return false;
        for (auto& [res, amt] : r->inputs)
            if (inv.count(res) < amt) return false;
        return true;
    }

    float craft(ResourceInventory& inv, StringID recipeName) {
        const ManufacturingRecipe* r = findRecipe(recipeName);
        if (!r || !canCraft(inv, recipeName)) return 0.f;
        for (auto& [res, amt] : r->inputs) inv.remove(res, amt);
        inv.add(r->output, r->outputAmount);
        return r->timeRequired;
    }

    int recipeCount() const { return (int)m_recipes.size(); }

private:
    std::vector<ManufacturingRecipe> m_recipes;
};

// ── G7: Exploration ───────────────────────────────────────────────

enum class SectorType : uint8_t { Normal, Nebula, AsteroidField, DeepSpace, AncientRuins, Count };
inline const char* sectorTypeName(SectorType t) {
    switch(t) {
        case SectorType::Normal: return "Normal";
        case SectorType::Nebula: return "Nebula";
        case SectorType::AsteroidField: return "AsteroidField";
        case SectorType::DeepSpace: return "DeepSpace";
        case SectorType::AncientRuins: return "AncientRuins";
        default: return "Unknown";
    }
}

struct SectorInfo {
    StringID name;
    SectorType type = SectorType::Normal;
    Vec3 position{0,0,0};
    float scanProgress = 0.f;
    bool fullyScanned = false;
    bool hasWormhole = false;
    bool hasAncientTech = false;
};

class ProbeScanner {
public:
    void init(float scanRate = 0.1f) { m_scanRate = scanRate; m_active = false; }

    void startScan(SectorInfo& sector) {
        m_targetSector = &sector;
        m_active = true;
    }
    void stopScan() { m_active = false; m_targetSector = nullptr; }
    bool isActive() const { return m_active; }

    bool tick(float dt) {
        if (!m_active || !m_targetSector) return false;
        m_targetSector->scanProgress += m_scanRate * dt;
        if (m_targetSector->scanProgress >= 1.f) {
            m_targetSector->scanProgress = 1.f;
            m_targetSector->fullyScanned = true;
            m_active = false;
            m_targetSector = nullptr;
            return true;
        }
        return false;
    }

    float scanRate() const { return m_scanRate; }
    void setScanRate(float r) { m_scanRate = r; }

private:
    float m_scanRate = 0.1f;
    bool m_active = false;
    SectorInfo* m_targetSector = nullptr;
};

struct WormholeLink {
    StringID fromSector;
    StringID toSector;
    float stability = 1.f;
    bool twoWay = true;

    bool isTraversable() const { return stability > 0.1f; }
    void degrade(float amount) {
        stability -= amount;
        if (stability < 0.f) stability = 0.f;
    }
};

class StarMap {
public:
    void addSector(const SectorInfo& sector) {
        m_sectors[sector.name] = sector;
    }
    SectorInfo* findSector(StringID name) {
        auto it = m_sectors.find(name);
        return it != m_sectors.end() ? &it->second : nullptr;
    }
    const SectorInfo* findSector(StringID name) const {
        auto it = m_sectors.find(name);
        return it != m_sectors.end() ? &it->second : nullptr;
    }

    void addWormhole(const WormholeLink& link) { m_wormholes.push_back(link); }

    std::vector<StringID> getReachableSectors(StringID from) const {
        std::vector<StringID> result;
        for (auto& wh : m_wormholes) {
            if (!wh.isTraversable()) continue;
            if (wh.fromSector == from) result.push_back(wh.toSector);
            else if (wh.twoWay && wh.toSector == from) result.push_back(wh.fromSector);
        }
        return result;
    }

    int sectorCount() const { return (int)m_sectors.size(); }
    int wormholeCount() const { return (int)m_wormholes.size(); }

    std::vector<StringID> getAncientTechSectors() const {
        std::vector<StringID> result;
        for (auto& [name, s] : m_sectors)
            if (s.hasAncientTech) result.push_back(name);
        return result;
    }

private:
    std::map<StringID, SectorInfo> m_sectors;
    std::vector<WormholeLink> m_wormholes;
};

struct AncientTechFragment {
    StringID name;
    StringID sectorFound;
    int tier = 1;
    bool analyzed = false;
    float damageBonus = 0.f;
    float shieldBonus = 0.f;
    float speedBonus = 0.f;
};

class AncientTechRegistry {
public:
    void add(const AncientTechFragment& f) { m_fragments.push_back(f); }
    AncientTechFragment* find(StringID name) {
        for (auto& f : m_fragments) if (f.name == name) return &f;
        return nullptr;
    }
    void analyze(StringID name) {
        if (auto* f = find(name)) { f->analyzed = true; f->damageBonus = f->tier * 0.05f; }
    }
    int count() const { return (int)m_fragments.size(); }
    int analyzedCount() const {
        int c = 0; for (auto& f : m_fragments) if (f.analyzed) ++c; return c;
    }

private:
    std::vector<AncientTechFragment> m_fragments;
};

// ── G8: FPS Interiors ─────────────────────────────────────────────

enum class RoomType : uint8_t { Bridge, Engineering, MedBay, Cargo, Airlock, Corridor, Count };
inline const char* roomTypeName(RoomType t) {
    switch(t) {
        case RoomType::Bridge: return "Bridge";
        case RoomType::Engineering: return "Engineering";
        case RoomType::MedBay: return "MedBay";
        case RoomType::Cargo: return "Cargo";
        case RoomType::Airlock: return "Airlock";
        case RoomType::Corridor: return "Corridor";
        default: return "Unknown";
    }
}

struct ShipRoom {
    StringID name;
    RoomType type = RoomType::Corridor;
    float oxygenLevel = 1.f;
    float temperature = 20.f;
    bool pressurized = true;
    std::vector<StringID> connectedRooms;

    void connect(StringID other) { connectedRooms.push_back(other); }
    bool isConnectedTo(StringID other) const {
        for (auto& r : connectedRooms) if (r == other) return true;
        return false;
    }
    bool isHabitable() const { return pressurized && oxygenLevel > 0.2f && temperature > -10.f; }
};

class ShipInterior {
public:
    void addRoom(const ShipRoom& room) { m_rooms[room.name] = room; }
    ShipRoom* findRoom(StringID name) {
        auto it = m_rooms.find(name);
        return it != m_rooms.end() ? &it->second : nullptr;
    }
    const ShipRoom* findRoom(StringID name) const {
        auto it = m_rooms.find(name);
        return it != m_rooms.end() ? &it->second : nullptr;
    }

    int roomCount() const { return (int)m_rooms.size(); }

    void decompress(StringID roomName) {
        if (auto* r = findRoom(roomName)) { r->oxygenLevel = 0.f; r->pressurized = false; }
    }

    void repressurize(StringID roomName) {
        if (auto* r = findRoom(roomName)) { r->oxygenLevel = 1.f; r->pressurized = true; }
    }

    int habitableRoomCount() const {
        int c = 0;
        for (auto& [n, r] : m_rooms) if (r.isHabitable()) ++c;
        return c;
    }

private:
    std::map<StringID, ShipRoom> m_rooms;
};

struct EVAState {
    bool active = false;
    float suitIntegrity = 100.f;
    float oxygenSupply = 300.f;
    float jetpackFuel = 100.f;
    Vec3 velocity{0,0,0};

    bool isAlive() const { return suitIntegrity > 0.f && oxygenSupply > 0.f; }

    void tick(float dt) {
        if (!active) return;
        oxygenSupply -= dt;
        if (oxygenSupply < 0.f) oxygenSupply = 0.f;
    }

    void useThruster(const Vec3& impulse, float fuelCost) {
        if (jetpackFuel <= 0.f) return;
        velocity = velocity + impulse;
        jetpackFuel -= fuelCost;
        if (jetpackFuel < 0.f) jetpackFuel = 0.f;
    }

    void takeSuitDamage(float amount) {
        suitIntegrity -= amount;
        if (suitIntegrity < 0.f) suitIntegrity = 0.f;
    }
};

struct SurvivalStatus {
    float radiation = 0.f;
    float temperature = 37.f;
    bool inVacuum = false;
    bool onFire = false;

    bool isRadiationDangerous() const { return radiation > 50.f; }
    bool isHypothermic() const { return temperature < 35.f; }
    bool isHyperthermic() const { return temperature > 40.f; }
    bool isInDanger() const { return isRadiationDangerous() || isHypothermic() || isHyperthermic() || inVacuum || onFire; }

    void tick(float dt, const ShipRoom* currentRoom) {
        if (!currentRoom || !currentRoom->isHabitable()) {
            inVacuum = true;
            radiation += dt * 2.f;
            temperature -= dt * 5.f;
        } else {
            inVacuum = false;
        }
    }
};

// ── G9: Legend System ─────────────────────────────────────────────

enum class ReputationTier : uint8_t {
    Infamous = 0, Outlaw, Neutral, Trusted, Honored, Legend, Count
};
inline const char* reputationTierName(ReputationTier t) {
    switch(t) {
        case ReputationTier::Infamous: return "Infamous";
        case ReputationTier::Outlaw: return "Outlaw";
        case ReputationTier::Neutral: return "Neutral";
        case ReputationTier::Trusted: return "Trusted";
        case ReputationTier::Honored: return "Honored";
        case ReputationTier::Legend: return "Legend";
        default: return "Unknown";
    }
}

inline ReputationTier reputationTierFromScore(float score) {
    if (score < -500.f) return ReputationTier::Infamous;
    if (score < -100.f) return ReputationTier::Outlaw;
    if (score < 100.f)  return ReputationTier::Neutral;
    if (score < 500.f)  return ReputationTier::Trusted;
    if (score < 1000.f) return ReputationTier::Honored;
    return ReputationTier::Legend;
}

class PlayerReputation {
public:
    void adjustReputation(StringID faction, float delta) {
        m_scores[faction] += delta;
    }
    float getReputation(StringID faction) const {
        auto it = m_scores.find(faction);
        return it != m_scores.end() ? it->second : 0.f;
    }
    ReputationTier getTier(StringID faction) const {
        return reputationTierFromScore(getReputation(faction));
    }
    int factionCount() const { return (int)m_scores.size(); }
    float globalFame() const {
        float sum = 0.f;
        for (auto& [k, v] : m_scores) sum += std::abs(v);
        return sum;
    }

private:
    std::map<StringID, float> m_scores;
};

struct WorldBias {
    StringID sectorName;
    float economyModifier = 1.f;
    float dangerLevel = 0.f;
    float loyaltyToPlayer = 0.f;

    bool isFriendly() const { return loyaltyToPlayer > 0.3f; }
    bool isHostile() const { return loyaltyToPlayer < -0.3f; }
};

class WorldBiasMap {
public:
    void setBias(const WorldBias& bias) { m_biases[bias.sectorName] = bias; }
    WorldBias* getBias(StringID sectorName) {
        auto it = m_biases.find(sectorName);
        return it != m_biases.end() ? &it->second : nullptr;
    }
    const WorldBias* getBias(StringID sectorName) const {
        auto it = m_biases.find(sectorName);
        return it != m_biases.end() ? &it->second : nullptr;
    }

    void updateFromReputation(StringID faction, float reputationDelta) {
        for (auto& [name, bias] : m_biases)
            bias.loyaltyToPlayer += reputationDelta * 0.01f;
    }

    int biasCount() const { return (int)m_biases.size(); }

private:
    std::map<StringID, WorldBias> m_biases;
};

struct NPCMemoryEntry {
    StringID eventType;
    float timestamp = 0.f;
    float weight = 1.f;
    bool positive = true;
};

class NPCMemory {
public:
    void remember(const NPCMemoryEntry& entry) { m_entries.push_back(entry); }

    void decay(float dt, float decayRate = 0.01f) {
        for (auto& e : m_entries) {
            e.weight -= decayRate * dt;
            if (e.weight < 0.f) e.weight = 0.f;
        }
        m_entries.erase(
            std::remove_if(m_entries.begin(), m_entries.end(),
                           [](const NPCMemoryEntry& e){ return e.weight <= 0.f; }),
            m_entries.end());
    }

    float dispositionTowardPlayer() const {
        float sum = 0.f;
        for (auto& e : m_entries)
            sum += e.positive ? e.weight : -e.weight;
        return sum;
    }

    int entryCount() const { return (int)m_entries.size(); }
    bool remembers(StringID eventType) const {
        for (auto& e : m_entries) if (e.eventType == eventType) return true;
        return false;
    }

private:
    std::vector<NPCMemoryEntry> m_entries;
};

class LegendStatus {
public:
    void init() {}

    PlayerReputation& reputation() { return m_reputation; }
    const PlayerReputation& reputation() const { return m_reputation; }
    WorldBiasMap& worldBias() { return m_worldBias; }
    const WorldBiasMap& worldBias() const { return m_worldBias; }

    ReputationTier overallTier() const {
        float fame = m_reputation.globalFame();
        return reputationTierFromScore(fame - 500.f);
    }

    bool isLegend() const {
        return m_reputation.globalFame() >= 2000.f;
    }

private:
    PlayerReputation m_reputation;
    WorldBiasMap m_worldBias;
};

// ── G10: Quest & Mission System ──────────────────────────────────

enum class MissionObjectiveType {
    Kill,
    Collect,
    Deliver,
    Explore,
    Survive,
    Escort
};

inline const char* missionObjectiveTypeName(MissionObjectiveType t) {
    switch (t) {
        case MissionObjectiveType::Kill:    return "Kill";
        case MissionObjectiveType::Collect: return "Collect";
        case MissionObjectiveType::Deliver: return "Deliver";
        case MissionObjectiveType::Explore: return "Explore";
        case MissionObjectiveType::Survive: return "Survive";
        case MissionObjectiveType::Escort:  return "Escort";
        default:                            return "Unknown";
    }
}

struct MissionObjective {
    MissionObjectiveType type = MissionObjectiveType::Kill;
    StringID targetId;
    std::string description;
    int required = 1;
    int current = 0;

    bool isComplete() const { return current >= required; }
    void progress(int amount = 1) { current = std::min(current + amount, required); }
};

struct MissionReward {
    int credits = 0;
    std::map<ResourceType, int> resources;
    StringID reputationFactionId;
    float reputationAmount = 0.f;
};

enum class MissionStatus { Active, Completed, Failed };

class ActiveMission {
public:
    void init(StringID missionId, const std::string& title) {
        m_missionId = missionId;
        m_title = title;
        m_status = MissionStatus::Active;
    }

    StringID missionId() const { return m_missionId; }
    const std::string& title() const { return m_title; }
    MissionStatus status() const { return m_status; }

    void addObjective(const MissionObjective& obj) { m_objectives.push_back(obj); }
    int objectiveCount() const { return (int)m_objectives.size(); }
    MissionObjective& objective(int i) { return m_objectives[i]; }
    const MissionObjective& objective(int i) const { return m_objectives[i]; }

    void setReward(const MissionReward& r) { m_reward = r; }
    const MissionReward& reward() const { return m_reward; }

    bool allObjectivesComplete() const {
        for (auto& o : m_objectives) if (!o.isComplete()) return false;
        return !m_objectives.empty();
    }

    void complete() { if (m_status == MissionStatus::Active) m_status = MissionStatus::Completed; }
    void fail()     { if (m_status == MissionStatus::Active) m_status = MissionStatus::Failed; }

private:
    StringID m_missionId;
    std::string m_title;
    MissionStatus m_status = MissionStatus::Active;
    std::vector<MissionObjective> m_objectives;
    MissionReward m_reward;
};

class MissionLog {
public:
    void acceptMission(const ActiveMission& mission) {
        m_active.push_back(mission);
    }

    bool completeMission(StringID missionId) {
        for (auto it = m_active.begin(); it != m_active.end(); ++it) {
            if (it->missionId() == missionId) {
                it->complete();
                m_completed.push_back(*it);
                m_active.erase(it);
                return true;
            }
        }
        return false;
    }

    bool failMission(StringID missionId) {
        for (auto it = m_active.begin(); it != m_active.end(); ++it) {
            if (it->missionId() == missionId) {
                it->fail();
                m_failed.push_back(*it);
                m_active.erase(it);
                return true;
            }
        }
        return false;
    }

    ActiveMission* findActive(StringID missionId) {
        for (auto& m : m_active) if (m.missionId() == missionId) return &m;
        return nullptr;
    }

    int activeMissionCount()    const { return (int)m_active.size(); }
    int completedMissionCount() const { return (int)m_completed.size(); }
    int failedMissionCount()    const { return (int)m_failed.size(); }

    const std::vector<ActiveMission>& activeMissions()    const { return m_active; }
    const std::vector<ActiveMission>& completedMissions() const { return m_completed; }

private:
    std::vector<ActiveMission> m_active;
    std::vector<ActiveMission> m_completed;
    std::vector<ActiveMission> m_failed;
};

class QuestChain {
public:
    void init(const std::string& name) { m_name = name; m_currentIndex = 0; }

    const std::string& name() const { return m_name; }
    void addMission(StringID missionId) { m_missionIds.push_back(missionId); }
    int missionCount() const { return (int)m_missionIds.size(); }

    StringID currentMissionId() const {
        if (m_currentIndex < (int)m_missionIds.size())
            return m_missionIds[m_currentIndex];
        return StringID{};
    }

    bool advance() {
        if (m_currentIndex < (int)m_missionIds.size()) {
            ++m_currentIndex;
            return true;
        }
        return false;
    }

    bool isComplete() const { return m_currentIndex >= (int)m_missionIds.size(); }
    int currentIndex() const { return m_currentIndex; }

private:
    std::string m_name;
    std::vector<StringID> m_missionIds;
    int m_currentIndex = 0;
};

// ── G11: Dialogue System ─────────────────────────────────────────

enum class DialogueConditionType {
    Always,
    HasReputation,
    HasItem,
    MissionActive,
    MissionComplete
};

struct DialogueCondition {
    DialogueConditionType type = DialogueConditionType::Always;
    StringID factionId;
    float minReputation = 0.f;
    ResourceType itemType = ResourceType::RawStone;
    int itemAmount = 1;
    StringID missionId;

    bool evaluate(float reputation, int itemCount, bool missionActive, bool missionComplete) const {
        switch (type) {
            case DialogueConditionType::Always:          return true;
            case DialogueConditionType::HasReputation:   return reputation >= minReputation;
            case DialogueConditionType::HasItem:         return itemCount >= itemAmount;
            case DialogueConditionType::MissionActive:   return missionActive;
            case DialogueConditionType::MissionComplete: return missionComplete;
            default:                                     return false;
        }
    }
};

struct DialogueEffect {
    StringID reputationFactionId;
    float reputationDelta = 0.f;
    StringID startMissionId;
    ResourceType giveItemType = ResourceType::RawStone;
    int giveItemAmount = 0;
};

struct DialogueOption {
    std::string text;
    DialogueCondition condition;
    DialogueEffect effect;
    int nextNodeId = -1;
};

struct DialogueNode {
    int nodeId = 0;
    std::string speakerName;
    std::string text;
    std::vector<DialogueOption> options;
};

class DialogueGraph {
public:
    void setStartNodeId(int id) { m_startNodeId = id; }
    int startNodeId() const { return m_startNodeId; }

    void addNode(const DialogueNode& node) { m_nodes[node.nodeId] = node; }
    const DialogueNode* getNode(int id) const {
        auto it = m_nodes.find(id);
        return it != m_nodes.end() ? &it->second : nullptr;
    }
    int nodeCount() const { return (int)m_nodes.size(); }

private:
    int m_startNodeId = 0;
    std::map<int, DialogueNode> m_nodes;
};

class DialogueRunner {
public:
    void init(const DialogueGraph* graph) {
        m_graph = graph;
        m_currentNodeId = graph ? graph->startNodeId() : -1;
        m_complete = false;
    }

    const DialogueNode* currentNode() const {
        return m_graph ? m_graph->getNode(m_currentNodeId) : nullptr;
    }

    bool isComplete() const { return m_complete; }

    const DialogueEffect* selectOption(int optionIndex) {
        const DialogueNode* node = currentNode();
        if (!node || optionIndex < 0 || optionIndex >= (int)node->options.size())
            return nullptr;
        const DialogueOption& opt = node->options[optionIndex];
        m_lastEffect = opt.effect;
        if (opt.nextNodeId < 0) {
            m_complete = true;
        } else {
            m_currentNodeId = opt.nextNodeId;
        }
        return &m_lastEffect;
    }

    int currentNodeId() const { return m_currentNodeId; }

private:
    const DialogueGraph* m_graph = nullptr;
    int m_currentNodeId = -1;
    bool m_complete = false;
    DialogueEffect m_lastEffect;
};

// ── Game Phase G12 — Save/Load System ────────────────────────────

struct SaveSlot {
    int         slotIndex   = 0;
    std::string name;
    std::string timestamp;
    float       playtimeSeconds = 0.f;
    bool        isEmpty = true;
};

struct SaveData {
    // Player state
    Vec3  playerPosition{0.f, 0.f, 0.f};
    float playerHealth    = 100.f;
    float playerEnergy    = 100.f;
    float playerOxygen    = 100.f;
    float playtimeSeconds = 0.f;

    // Inventory
    std::map<std::string, int> inventory;

    // Active missions (by id)
    std::vector<std::string> activeMissionIds;
    std::vector<std::string> completedMissionIds;

    // Reputation per faction
    std::map<std::string, float> reputation;

    // Current sector name
    std::string currentSector;
};

class GameSaveSerializer {
public:
    static JsonValue toJson(const SaveData& data) {
        JsonValue root;
        // Player state
        root["playerHealth"]    = JsonValue(data.playerHealth);
        root["playerEnergy"]    = JsonValue(data.playerEnergy);
        root["playerOxygen"]    = JsonValue(data.playerOxygen);
        root["playtimeSeconds"] = JsonValue(data.playtimeSeconds);
        root["currentSector"]   = JsonValue(data.currentSector);

        // Position
        JsonValue pos;
        pos["x"] = JsonValue(data.playerPosition.x);
        pos["y"] = JsonValue(data.playerPosition.y);
        pos["z"] = JsonValue(data.playerPosition.z);
        root["position"] = pos;

        // Inventory
        JsonValue inv;
        for (const auto& [key, qty] : data.inventory)
            inv[key] = JsonValue(qty);
        root["inventory"] = inv;

        // Missions
        auto activeMissions = JsonValue::array();
        for (const auto& id : data.activeMissionIds)
            activeMissions.push(JsonValue(id));
        root["activeMissions"] = activeMissions;

        auto completedMissions = JsonValue::array();
        for (const auto& id : data.completedMissionIds)
            completedMissions.push(JsonValue(id));
        root["completedMissions"] = completedMissions;

        // Reputation
        JsonValue rep;
        for (const auto& [faction, val] : data.reputation)
            rep[faction] = JsonValue(val);
        root["reputation"] = rep;

        return root;
    }

    static SaveData fromJson(const JsonValue& j) {
        SaveData data;
        if (j.hasKey("playerHealth"))    data.playerHealth    = j["playerHealth"].asFloat();
        if (j.hasKey("playerEnergy"))    data.playerEnergy    = j["playerEnergy"].asFloat();
        if (j.hasKey("playerOxygen"))    data.playerOxygen    = j["playerOxygen"].asFloat();
        if (j.hasKey("playtimeSeconds")) data.playtimeSeconds = j["playtimeSeconds"].asFloat();
        if (j.hasKey("currentSector"))   data.currentSector   = j["currentSector"].asString();

        if (j.hasKey("position")) {
            const auto& pos = j["position"];
            data.playerPosition.x = pos.hasKey("x") ? pos["x"].asFloat() : 0.f;
            data.playerPosition.y = pos.hasKey("y") ? pos["y"].asFloat() : 0.f;
            data.playerPosition.z = pos.hasKey("z") ? pos["z"].asFloat() : 0.f;
        }

        if (j.hasKey("inventory")) {
            const auto& inv = j["inventory"];
            for (const auto& [key, val] : inv.members())
                data.inventory[key] = val.asInt();
        }

        if (j.hasKey("activeMissions")) {
            const auto& arr = j["activeMissions"];
            for (size_t i = 0; i < arr.size(); ++i)
                data.activeMissionIds.push_back(arr[i].asString());
        }

        if (j.hasKey("completedMissions")) {
            const auto& arr = j["completedMissions"];
            for (size_t i = 0; i < arr.size(); ++i)
                data.completedMissionIds.push_back(arr[i].asString());
        }

        if (j.hasKey("reputation")) {
            const auto& rep = j["reputation"];
            for (const auto& [key, val] : rep.members())
                data.reputation[key] = val.asFloat();
        }

        return data;
    }
};

class SaveSystem {
public:
    static constexpr int kMaxSlots = 5;

    void init() {
        m_slots.resize(kMaxSlots);
        for (int i = 0; i < kMaxSlots; ++i) {
            m_slots[i].slotIndex = i;
            m_slots[i].isEmpty   = true;
        }
        NF_LOG_INFO("SaveSystem", "SaveSystem initialized with " +
                    std::to_string(kMaxSlots) + " slots");
    }

    // Persist a SaveData into a slot. Returns false if slotIndex is out of range.
    bool saveGame(int slotIndex, const SaveData& data, const std::string& saveName) {
        if (slotIndex < 0 || slotIndex >= kMaxSlots) return false;
        m_saveData[slotIndex]          = data;
        m_slots[slotIndex].slotIndex   = slotIndex;
        m_slots[slotIndex].name        = saveName;
        m_slots[slotIndex].playtimeSeconds = data.playtimeSeconds;
        m_slots[slotIndex].isEmpty     = false;
        // Serialise to JsonValue (represents the on-disk representation)
        m_serialized[slotIndex]        = GameSaveSerializer::toJson(data);
        NF_LOG_INFO("SaveSystem", "Saved '" + saveName + "' to slot " +
                    std::to_string(slotIndex));
        return true;
    }

    // Load game data from slot. Returns nullptr if slot is empty.
    const SaveData* loadGame(int slotIndex) {
        if (slotIndex < 0 || slotIndex >= kMaxSlots) return nullptr;
        if (m_slots[slotIndex].isEmpty) return nullptr;
        // Re-deserialize from the stored json (simulates disk round-trip)
        m_saveData[slotIndex] = GameSaveSerializer::fromJson(m_serialized[slotIndex]);
        NF_LOG_INFO("SaveSystem", "Loaded slot " + std::to_string(slotIndex));
        return &m_saveData[slotIndex];
    }

    bool deleteSlot(int slotIndex) {
        if (slotIndex < 0 || slotIndex >= kMaxSlots) return false;
        m_slots[slotIndex].isEmpty = true;
        m_slots[slotIndex].name.clear();
        m_saveData.erase(slotIndex);
        m_serialized.erase(slotIndex);
        return true;
    }

    void enableAutoSave(bool enable) { m_autoSaveEnabled = enable; }
    bool isAutoSaveEnabled() const   { return m_autoSaveEnabled; }

    void tickAutoSave(float dt, const SaveData& data) {
        if (!m_autoSaveEnabled) return;
        m_autoSaveTimer += dt;
        if (m_autoSaveTimer >= m_autoSaveIntervalSeconds) {
            m_autoSaveTimer = 0.f;
            saveGame(kAutoSaveSlot, data, "AutoSave");
        }
    }

    [[nodiscard]] const SaveSlot& slot(int index) const {
        static SaveSlot empty;
        if (index < 0 || index >= kMaxSlots) return empty;
        return m_slots[index];
    }

    [[nodiscard]] std::vector<SaveSlot> listSlots() const { return m_slots; }

    int usedSlotCount() const {
        int count = 0;
        for (const auto& s : m_slots) if (!s.isEmpty) ++count;
        return count;
    }

    static constexpr int kAutoSaveSlot = 0;
    float autoSaveIntervalSeconds() const { return m_autoSaveIntervalSeconds; }
    void setAutoSaveInterval(float seconds) { m_autoSaveIntervalSeconds = seconds; }

private:
    std::vector<SaveSlot>          m_slots;
    std::map<int, SaveData>        m_saveData;
    std::map<int, JsonValue>       m_serialized;
    bool  m_autoSaveEnabled        = false;
    float m_autoSaveTimer          = 0.f;
    float m_autoSaveIntervalSeconds = 300.f;  // 5 minutes default
};

// ── Game Phase G13 — World Events System ─────────────────────────

enum class WorldEventType : uint8_t {
    AsteroidStorm,
    PirateRaid,
    TechDiscovery,
    FactionWar,
    TradeOpportunity,
    Plague,
    CelestialAnomaly
};

inline std::string worldEventTypeName(WorldEventType t) {
    switch (t) {
        case WorldEventType::AsteroidStorm:    return "AsteroidStorm";
        case WorldEventType::PirateRaid:       return "PirateRaid";
        case WorldEventType::TechDiscovery:    return "TechDiscovery";
        case WorldEventType::FactionWar:       return "FactionWar";
        case WorldEventType::TradeOpportunity: return "TradeOpportunity";
        case WorldEventType::Plague:           return "Plague";
        case WorldEventType::CelestialAnomaly: return "CelestialAnomaly";
        default: return "Unknown";
    }
}

struct EventEffect {
    float priceModifier      = 1.f;   // multiplier on buy/sell prices
    float dangerModifier     = 1.f;   // multiplier on encounter danger
    float reputationChange   = 0.f;   // flat reputation change on resolution
    float resourceBonus      = 0.f;   // extra resource yield while active
};

struct WorldEvent {
    int            eventId        = 0;
    WorldEventType type           = WorldEventType::AsteroidStorm;
    std::string    sectorId;
    std::string    description;
    float          duration       = 60.f;  // seconds until expiry
    float          elapsed        = 0.f;
    float          severity       = 1.f;   // 0-1
    bool           isActive       = true;
    EventEffect    effect;

    bool isExpired() const { return elapsed >= duration; }
    float remainingTime() const { return std::max(0.f, duration - elapsed); }
    void tick(float dt) { if (isActive) elapsed += dt; }
};

class WorldEventSystem {
public:
    void init() {
        m_nextEventId = 1;
        NF_LOG_INFO("WorldEvents", "WorldEventSystem initialized");
    }

    // Spawn a new event in the specified sector. Returns the assigned event ID.
    int spawnEvent(WorldEventType type, const std::string& sectorId,
                   float duration, float severity, const std::string& description) {
        WorldEvent ev;
        ev.eventId     = m_nextEventId++;
        ev.type        = type;
        ev.sectorId    = sectorId;
        ev.duration    = duration;
        ev.severity    = std::max(0.f, std::min(1.f, severity));
        ev.description = description;
        ev.isActive    = true;
        ev.effect      = buildEffect(type, ev.severity);
        m_events.push_back(ev);
        NF_LOG_INFO("WorldEvents", "Spawned " + worldEventTypeName(type) +
                    " in '" + sectorId + "' id=" + std::to_string(ev.eventId));
        return ev.eventId;
    }

    // Manually end an event before it expires.
    bool endEvent(int eventId) {
        for (auto& ev : m_events) {
            if (ev.eventId == eventId && ev.isActive) {
                ev.isActive = false;
                return true;
            }
        }
        return false;
    }

    // Advance all active events; expired events are marked inactive.
    void tick(float dt) {
        for (auto& ev : m_events) {
            if (!ev.isActive) continue;
            ev.tick(dt);
            if (ev.isExpired()) ev.isActive = false;
        }
        // Prune fully-expired events beyond a history window
        while (m_events.size() > kMaxHistorySize) {
            m_events.erase(m_events.begin());
        }
    }

    [[nodiscard]] std::vector<WorldEvent> getActiveEvents() const {
        std::vector<WorldEvent> out;
        for (const auto& ev : m_events)
            if (ev.isActive) out.push_back(ev);
        return out;
    }

    [[nodiscard]] std::vector<WorldEvent> getEventsInSector(const std::string& sectorId) const {
        std::vector<WorldEvent> out;
        for (const auto& ev : m_events)
            if (ev.sectorId == sectorId) out.push_back(ev);
        return out;
    }

    [[nodiscard]] const WorldEvent* findEvent(int eventId) const {
        for (const auto& ev : m_events)
            if (ev.eventId == eventId) return &ev;
        return nullptr;
    }

    int activeEventCount() const {
        int n = 0;
        for (const auto& ev : m_events) if (ev.isActive) ++n;
        return n;
    }

    int totalEventCount() const { return (int)m_events.size(); }

    static constexpr size_t kMaxHistorySize = 100;

private:
    static EventEffect buildEffect(WorldEventType type, float severity) {
        EventEffect e;
        switch (type) {
            case WorldEventType::AsteroidStorm:
                e.dangerModifier = 1.f + severity;
                break;
            case WorldEventType::PirateRaid:
                e.dangerModifier   = 1.f + severity * 2.f;
                e.reputationChange = -5.f * severity;
                break;
            case WorldEventType::TechDiscovery:
                e.reputationChange = 10.f * severity;
                e.resourceBonus    = 0.5f * severity;
                break;
            case WorldEventType::FactionWar:
                e.dangerModifier   = 1.5f + severity;
                e.priceModifier    = 1.f + 0.3f * severity;
                break;
            case WorldEventType::TradeOpportunity:
                e.priceModifier  = 1.f - 0.3f * severity;  // discounted prices
                e.resourceBonus  = 1.f * severity;
                break;
            case WorldEventType::Plague:
                e.dangerModifier   = 1.f + 0.5f * severity;
                e.reputationChange = -2.f * severity;
                break;
            case WorldEventType::CelestialAnomaly:
                e.resourceBonus = 2.f * severity;
                break;
            default:
                break;
        }
        return e;
    }

    std::vector<WorldEvent> m_events;
    int m_nextEventId = 1;
};

} // namespace NF
