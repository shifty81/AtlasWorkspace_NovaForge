#pragma once
// NF::Game — Game layer: voxels, interaction loop, R.I.G. system, inventory
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/Physics/Physics.h"

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

} // namespace NF
