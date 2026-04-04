#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Game/Game.h"

TEST_CASE("Chunk voxel get/set", "[Game][Voxel]") {
    NF::Chunk chunk;

    // Default is air
    REQUIRE(chunk.get(0, 0, 0) == NF::VoxelType::Air);

    chunk.set(5, 5, 5, NF::VoxelType::Stone);
    REQUIRE(chunk.get(5, 5, 5) == NF::VoxelType::Stone);
    REQUIRE(chunk.meshDirty);
    REQUIRE(chunk.collisionDirty);
    REQUIRE_FALSE(chunk.meshed);
}

TEST_CASE("Chunk out-of-bounds returns Air", "[Game][Voxel]") {
    NF::Chunk chunk;

    REQUIRE(chunk.get(-1, 0, 0) == NF::VoxelType::Air);
    REQUIRE(chunk.get(0, -1, 0) == NF::VoxelType::Air);
    REQUIRE(chunk.get(0, 0, 16) == NF::VoxelType::Air);
    REQUIRE(chunk.get(16, 0, 0) == NF::VoxelType::Air);
}

TEST_CASE("Chunk set out-of-bounds is safe", "[Game][Voxel]") {
    NF::Chunk chunk;

    // Should not crash
    chunk.set(-1, 0, 0, NF::VoxelType::Stone);
    chunk.set(16, 0, 0, NF::VoxelType::Stone);
}

TEST_CASE("Inventory add and remove", "[Game][Inventory]") {
    NF::Inventory inv;

    inv.add(NF::VoxelType::Stone, 5);
    REQUIRE(inv.count(NF::VoxelType::Stone) == 5);

    REQUIRE(inv.remove(NF::VoxelType::Stone, 3));
    REQUIRE(inv.count(NF::VoxelType::Stone) == 2);

    REQUIRE_FALSE(inv.remove(NF::VoxelType::Stone, 5)); // not enough
    REQUIRE(inv.count(NF::VoxelType::Stone) == 2);      // unchanged
}

TEST_CASE("RigState defaults", "[Game][Rig]") {
    NF::RigState rig;
    REQUIRE(rig.health == 100.f);
    REQUIRE(rig.energy == 100.f);
    REQUIRE(rig.activeTool == 0);
    REQUIRE(rig.maxHealth == 100.f);
    REQUIRE(rig.maxEnergy == 100.f);
    REQUIRE(rig.oxygen == 100.f);
    REQUIRE(rig.stamina == 100.f);
    REQUIRE(rig.isAlive());
}

// ── Chunk dirty flag separation ──────────────────────────────────

TEST_CASE("Chunk separate mesh and collision dirty flags", "[Game][Voxel]") {
    NF::Chunk chunk;

    // Initial state: all dirty
    REQUIRE(chunk.meshDirty);
    REQUIRE(chunk.collisionDirty);

    // Mark mesh clean, collision should still be dirty
    chunk.markMeshClean();
    REQUIRE_FALSE(chunk.meshDirty);
    REQUIRE(chunk.collisionDirty);
    REQUIRE(chunk.meshed);

    // Mark collision clean
    chunk.markCollisionClean();
    REQUIRE_FALSE(chunk.collisionDirty);

    // Edit should dirty both
    chunk.set(0, 0, 0, NF::VoxelType::Stone);
    REQUIRE(chunk.meshDirty);
    REQUIRE(chunk.collisionDirty);
    REQUIRE_FALSE(chunk.meshed);

    // markAllClean should clear everything
    chunk.markAllClean();
    REQUIRE_FALSE(chunk.meshDirty);
    REQUIRE_FALSE(chunk.collisionDirty);
    REQUIRE(chunk.meshed);
}

// ── Chunk utility methods ────────────────────────────────────────

TEST_CASE("Chunk isFullyAir and solidCount", "[Game][Voxel]") {
    NF::Chunk chunk;
    REQUIRE(chunk.isFullyAir());
    REQUIRE(chunk.solidCount() == 0);

    chunk.set(0, 0, 0, NF::VoxelType::Stone);
    REQUIRE_FALSE(chunk.isFullyAir());
    REQUIRE(chunk.solidCount() == 1);

    chunk.set(1, 1, 1, NF::VoxelType::Dirt);
    REQUIRE(chunk.solidCount() == 2);
}

// ── Voxel type name round-trip ───────────────────────────────────

TEST_CASE("VoxelType name round-trip", "[Game][Voxel]") {
    // All named types should round-trip through name/fromName
    REQUIRE(NF::voxelTypeFromName("Stone") == NF::VoxelType::Stone);
    REQUIRE(NF::voxelTypeFromName("Dirt") == NF::VoxelType::Dirt);
    REQUIRE(NF::voxelTypeFromName("Grass") == NF::VoxelType::Grass);
    REQUIRE(NF::voxelTypeFromName("Metal") == NF::VoxelType::Metal);
    REQUIRE(NF::voxelTypeFromName("Glass") == NF::VoxelType::Glass);
    REQUIRE(NF::voxelTypeFromName("Water") == NF::VoxelType::Water);
    REQUIRE(NF::voxelTypeFromName("Ore_Iron") == NF::VoxelType::Ore_Iron);
    REQUIRE(NF::voxelTypeFromName("Ore_Gold") == NF::VoxelType::Ore_Gold);
    REQUIRE(NF::voxelTypeFromName("Ore_Crystal") == NF::VoxelType::Ore_Crystal);
    REQUIRE(NF::voxelTypeFromName("Unknown") == NF::VoxelType::Air);

    REQUIRE(std::string(NF::voxelTypeName(NF::VoxelType::Stone)) == "Stone");
    REQUIRE(std::string(NF::voxelTypeName(NF::VoxelType::Ore_Crystal)) == "Ore_Crystal");
}

// ── Chunk serialization: THE persistence proof ───────────────────

TEST_CASE("ChunkSerializer round-trip preserves all voxel data", "[Game][Serialization]") {
    NF::Chunk original;
    original.cx = 3;
    original.cy = -1;
    original.cz = 7;

    // Place diverse voxels
    original.set(0, 0, 0, NF::VoxelType::Stone);
    original.set(5, 5, 5, NF::VoxelType::Dirt);
    original.set(15, 15, 15, NF::VoxelType::Ore_Crystal);
    original.set(8, 0, 8, NF::VoxelType::Water);
    original.set(0, 15, 0, NF::VoxelType::Metal);

    // Serialize to JSON string
    std::string json = NF::ChunkSerializer::serialize(original);
    REQUIRE_FALSE(json.empty());

    // Deserialize from JSON string
    NF::Chunk loaded = NF::ChunkSerializer::deserialize(json);

    // Verify chunk coordinates
    REQUIRE(loaded.cx == original.cx);
    REQUIRE(loaded.cy == original.cy);
    REQUIRE(loaded.cz == original.cz);

    // Verify ALL voxels match — full 16x16x16 comparison
    for (int x = 0; x < NF::CHUNK_SIZE; ++x) {
        for (int y = 0; y < NF::CHUNK_SIZE; ++y) {
            for (int z = 0; z < NF::CHUNK_SIZE; ++z) {
                REQUIRE(loaded.get(x, y, z) == original.get(x, y, z));
            }
        }
    }
}

TEST_CASE("ChunkSerializer round-trip via JsonValue preserves data", "[Game][Serialization]") {
    NF::Chunk original;
    original.cx = 0;
    original.cy = 0;
    original.cz = 0;

    // Fill a layer with stone
    for (int x = 0; x < NF::CHUNK_SIZE; ++x)
        for (int z = 0; z < NF::CHUNK_SIZE; ++z)
            original.set(x, 0, z, NF::VoxelType::Stone);

    auto json = NF::ChunkSerializer::toJson(original);
    NF::Chunk loaded = NF::ChunkSerializer::fromJson(json);

    // Verify the stone layer
    for (int x = 0; x < NF::CHUNK_SIZE; ++x) {
        for (int z = 0; z < NF::CHUNK_SIZE; ++z) {
            REQUIRE(loaded.get(x, 0, z) == NF::VoxelType::Stone);
            REQUIRE(loaded.get(x, 1, z) == NF::VoxelType::Air);
        }
    }
}

TEST_CASE("ChunkSerializer empty chunk round-trip", "[Game][Serialization]") {
    NF::Chunk original;
    // Default chunk is all air
    std::string json = NF::ChunkSerializer::serialize(original);
    NF::Chunk loaded = NF::ChunkSerializer::deserialize(json);

    REQUIRE(loaded.isFullyAir());
    REQUIRE(loaded.cx == 0);
    REQUIRE(loaded.cy == 0);
    REQUIRE(loaded.cz == 0);
}

TEST_CASE("Voxel edit then save/reload round-trip", "[Game][Serialization][Persistence]") {
    // This is the critical editor-trust test: edit a voxel, save, reload, verify
    NF::Chunk chunk;
    chunk.cx = 1;
    chunk.cy = 2;
    chunk.cz = 3;

    // Step 1: Edit voxels
    chunk.set(7, 7, 7, NF::VoxelType::Ore_Gold);
    chunk.set(3, 12, 5, NF::VoxelType::Glass);
    REQUIRE(chunk.get(7, 7, 7) == NF::VoxelType::Ore_Gold);

    // Step 2: Save (serialize)
    std::string saved = NF::ChunkSerializer::serialize(chunk);

    // Step 3: Simulate process restart by deserializing into new chunk
    NF::Chunk reloaded = NF::ChunkSerializer::deserialize(saved);

    // Step 4: Verify edit persisted
    REQUIRE(reloaded.get(7, 7, 7) == NF::VoxelType::Ore_Gold);
    REQUIRE(reloaded.get(3, 12, 5) == NF::VoxelType::Glass);
    REQUIRE(reloaded.cx == 1);
    REQUIRE(reloaded.cy == 2);
    REQUIRE(reloaded.cz == 3);

    // Step 5: Edit again, save again, reload again (second round-trip)
    reloaded.set(7, 7, 7, NF::VoxelType::Air);  // undo the edit
    std::string saved2 = NF::ChunkSerializer::serialize(reloaded);
    NF::Chunk reloaded2 = NF::ChunkSerializer::deserialize(saved2);

    REQUIRE(reloaded2.get(7, 7, 7) == NF::VoxelType::Air);
    REQUIRE(reloaded2.get(3, 12, 5) == NF::VoxelType::Glass);
}

// ── VoxelEditCommand with undo/redo ──────────────────────────────

TEST_CASE("VoxelEditCommand undo/redo", "[Game][Undo]") {
    NF::Chunk chunk;
    NF::CommandStack stack;

    // Edit via command
    auto cmd = std::make_unique<NF::VoxelEditCommand>(&chunk, 5, 5, 5, NF::VoxelType::Stone);
    stack.execute(std::move(cmd));

    REQUIRE(chunk.get(5, 5, 5) == NF::VoxelType::Stone);
    REQUIRE(stack.undoCount() == 1);

    // Undo
    stack.undo();
    REQUIRE(chunk.get(5, 5, 5) == NF::VoxelType::Air);
    REQUIRE(stack.undoCount() == 0);
    REQUIRE(stack.redoCount() == 1);

    // Redo
    stack.redo();
    REQUIRE(chunk.get(5, 5, 5) == NF::VoxelType::Stone);
    REQUIRE(stack.undoCount() == 1);
    REQUIRE(stack.redoCount() == 0);
}

TEST_CASE("VoxelEditCommand multiple edits undo chain", "[Game][Undo]") {
    NF::Chunk chunk;
    NF::CommandStack stack;

    // Three sequential edits
    stack.execute(std::make_unique<NF::VoxelEditCommand>(&chunk, 0, 0, 0, NF::VoxelType::Stone));
    stack.execute(std::make_unique<NF::VoxelEditCommand>(&chunk, 1, 1, 1, NF::VoxelType::Dirt));
    stack.execute(std::make_unique<NF::VoxelEditCommand>(&chunk, 2, 2, 2, NF::VoxelType::Grass));

    REQUIRE(stack.undoCount() == 3);

    // Undo all three
    stack.undo();
    REQUIRE(chunk.get(2, 2, 2) == NF::VoxelType::Air);
    stack.undo();
    REQUIRE(chunk.get(1, 1, 1) == NF::VoxelType::Air);
    stack.undo();
    REQUIRE(chunk.get(0, 0, 0) == NF::VoxelType::Air);

    REQUIRE(stack.undoCount() == 0);
    REQUIRE(stack.redoCount() == 3);

    // Redo first, then new edit should clear redo stack
    stack.redo();
    REQUIRE(chunk.get(0, 0, 0) == NF::VoxelType::Stone);
    stack.execute(std::make_unique<NF::VoxelEditCommand>(&chunk, 5, 5, 5, NF::VoxelType::Metal));
    REQUIRE(stack.redoCount() == 0);  // redo stack cleared by new edit
    REQUIRE(stack.undoCount() == 2);
}

// ── ChunkMesher tests ───────────────────────────────────────────

TEST_CASE("ChunkMesher empty chunk produces no geometry", "[Game][Mesher]") {
    NF::Chunk chunk;
    // All air - should produce nothing
    auto result = NF::ChunkMesher::buildMesh(chunk);
    REQUIRE(result.vertices.empty());
    REQUIRE(result.indices.empty());
    REQUIRE(result.faceCount == 0);
}

TEST_CASE("ChunkMesher single voxel produces 6 faces", "[Game][Mesher]") {
    NF::Chunk chunk;
    chunk.set(8, 8, 8, NF::VoxelType::Stone);

    auto result = NF::ChunkMesher::buildMesh(chunk);
    // A lone voxel surrounded by air should have 6 exposed faces
    REQUIRE(result.faceCount == 6);
    REQUIRE(result.vertices.size() == 24);   // 4 verts per face * 6 faces
    REQUIRE(result.indices.size() == 36);    // 6 indices per face * 6 faces
}

TEST_CASE("ChunkMesher adjacent voxels cull shared face", "[Game][Mesher]") {
    NF::Chunk chunk;
    chunk.set(8, 8, 8, NF::VoxelType::Stone);
    chunk.set(9, 8, 8, NF::VoxelType::Stone);

    auto result = NF::ChunkMesher::buildMesh(chunk);
    // Two adjacent voxels: 6+6 faces minus 2 shared faces = 10 exposed
    REQUIRE(result.faceCount == 10);
}

TEST_CASE("ChunkMesher voxel colors match type", "[Game][Mesher]") {
    NF::Chunk chunk;
    chunk.set(0, 0, 0, NF::VoxelType::Grass);

    auto result = NF::ChunkMesher::buildMesh(chunk);
    REQUIRE_FALSE(result.vertices.empty());

    NF::Vec4 grassColor = NF::voxelColor(NF::VoxelType::Grass);
    // All vertices should have the grass color
    for (const auto& v : result.vertices) {
        REQUIRE(v.color.x == grassColor.x);
        REQUIRE(v.color.y == grassColor.y);
        REQUIRE(v.color.z == grassColor.z);
    }
}

TEST_CASE("ChunkMesher buildRenderMesh returns valid Mesh", "[Game][Mesher]") {
    NF::Chunk chunk;
    chunk.set(4, 4, 4, NF::VoxelType::Metal);

    NF::Mesh mesh = NF::ChunkMesher::buildRenderMesh(chunk);
    REQUIRE(mesh.vertexCount() == 24);
    REQUIRE(mesh.indexCount() == 36);
    REQUIRE(mesh.triangleCount() == 12);
}

TEST_CASE("ChunkMesher world offset is applied", "[Game][Mesher]") {
    NF::Chunk chunk;
    chunk.cx = 2; chunk.cy = 0; chunk.cz = 1;
    chunk.set(0, 0, 0, NF::VoxelType::Stone);

    auto result = NF::ChunkMesher::buildMesh(chunk);
    REQUIRE_FALSE(result.vertices.empty());

    // Vertices should be offset by chunk position * CHUNK_SIZE
    float expectedMinX = 2.f * NF::CHUNK_SIZE;
    float expectedMinZ = 1.f * NF::CHUNK_SIZE;
    bool foundOffsetVert = false;
    for (const auto& v : result.vertices) {
        if (v.position.x >= expectedMinX && v.position.z >= expectedMinZ) {
            foundOffsetVert = true;
            break;
        }
    }
    REQUIRE(foundOffsetVert);
}

// ── WorldState tests ─────────────────────────────────────────────

TEST_CASE("WorldState get/set world coordinates", "[Game][World]") {
    NF::WorldState world;

    world.setWorld(5, 5, 5, NF::VoxelType::Stone);
    REQUIRE(world.getWorld(5, 5, 5) == NF::VoxelType::Stone);
    REQUIRE(world.getWorld(0, 0, 0) == NF::VoxelType::Air);
    REQUIRE(world.chunkCount() == 1);
}

TEST_CASE("WorldState negative coordinates", "[Game][World]") {
    NF::WorldState world;

    world.setWorld(-1, -1, -1, NF::VoxelType::Dirt);
    REQUIRE(world.getWorld(-1, -1, -1) == NF::VoxelType::Dirt);
    REQUIRE(world.chunkCount() == 1);
}

TEST_CASE("WorldState cross-chunk boundaries", "[Game][World]") {
    NF::WorldState world;

    // Set voxels in different chunks
    world.setWorld(0, 0, 0, NF::VoxelType::Stone);
    world.setWorld(16, 0, 0, NF::VoxelType::Dirt);   // next chunk
    world.setWorld(-1, 0, 0, NF::VoxelType::Grass);  // previous chunk

    REQUIRE(world.getWorld(0, 0, 0) == NF::VoxelType::Stone);
    REQUIRE(world.getWorld(16, 0, 0) == NF::VoxelType::Dirt);
    REQUIRE(world.getWorld(-1, 0, 0) == NF::VoxelType::Grass);
    REQUIRE(world.chunkCount() == 3);
}

TEST_CASE("WorldState forEach iterates all chunks", "[Game][World]") {
    NF::WorldState world;
    world.setWorld(0, 0, 0, NF::VoxelType::Stone);
    world.setWorld(16, 0, 0, NF::VoxelType::Dirt);

    int count = 0;
    world.forEach([&](const NF::Chunk&) { ++count; });
    REQUIRE(count == 2);
}

// ── WorldSerializer tests ────────────────────────────────────────

TEST_CASE("WorldSerializer round-trip preserves world", "[Game][Serialization]") {
    NF::WorldState original;
    original.setWorld(5, 5, 5, NF::VoxelType::Stone);
    original.setWorld(20, 0, 0, NF::VoxelType::Ore_Gold);
    original.setWorld(-5, 10, -5, NF::VoxelType::Glass);

    std::string json = NF::WorldSerializer::serialize(original);
    REQUIRE_FALSE(json.empty());

    NF::WorldState loaded = NF::WorldSerializer::deserialize(json);

    REQUIRE(loaded.chunkCount() == original.chunkCount());
    REQUIRE(loaded.getWorld(5, 5, 5) == NF::VoxelType::Stone);
    REQUIRE(loaded.getWorld(20, 0, 0) == NF::VoxelType::Ore_Gold);
    REQUIRE(loaded.getWorld(-5, 10, -5) == NF::VoxelType::Glass);
    REQUIRE(loaded.getWorld(0, 0, 0) == NF::VoxelType::Air);
}

TEST_CASE("WorldSerializer empty world round-trip", "[Game][Serialization]") {
    NF::WorldState original;
    std::string json = NF::WorldSerializer::serialize(original);
    NF::WorldState loaded = NF::WorldSerializer::deserialize(json);
    REQUIRE(loaded.chunkCount() == 0);
}

// ── VoxelPickService tests ───────────────────────────────────────

TEST_CASE("VoxelPickService hits solid voxel", "[Game][Pick]") {
    NF::WorldState world;
    // Place a stone block at (5, 5, 5)
    world.setWorld(5, 5, 5, NF::VoxelType::Stone);

    // Cast from above, looking down
    NF::Vec3 origin{5.5f, 20.f, 5.5f};
    NF::Vec3 dir{0.f, -1.f, 0.f};

    auto hit = NF::VoxelPickService::raycast(world, origin, dir);
    REQUIRE(hit.has_value());
    REQUIRE(hit->wx == 5);
    REQUIRE(hit->wy == 5);
    REQUIRE(hit->wz == 5);
    REQUIRE(hit->type == NF::VoxelType::Stone);
    REQUIRE(hit->face == 2); // hit the +Y face
}

TEST_CASE("VoxelPickService misses empty world", "[Game][Pick]") {
    NF::WorldState world;

    NF::Vec3 origin{0.f, 10.f, 0.f};
    NF::Vec3 dir{0.f, -1.f, 0.f};

    auto hit = NF::VoxelPickService::raycast(world, origin, dir, 50.f);
    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("VoxelPickService adjacentVoxel calculation", "[Game][Pick]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Stone);

    NF::Vec3 origin{5.5f, 20.f, 5.5f};
    NF::Vec3 dir{0.f, -1.f, 0.f};

    auto hit = NF::VoxelPickService::raycast(world, origin, dir);
    REQUIRE(hit.has_value());

    auto adj = hit->adjacentVoxel();
    // Hit +Y face of (5,5,5), adjacent should be (5,6,5)
    REQUIRE(adj.x == 5);
    REQUIRE(adj.y == 6);
    REQUIRE(adj.z == 5);
}

TEST_CASE("VoxelPickService respects maxDist", "[Game][Pick]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Stone);

    NF::Vec3 origin{5.5f, 100.f, 5.5f};
    NF::Vec3 dir{0.f, -1.f, 0.f};

    // maxDist too short to reach
    auto hit = NF::VoxelPickService::raycast(world, origin, dir, 10.f);
    REQUIRE_FALSE(hit.has_value());

    // maxDist long enough
    auto hit2 = NF::VoxelPickService::raycast(world, origin, dir, 200.f);
    REQUIRE(hit2.has_value());
}

// ── G1: RigState expanded tests ─────────────────────────────────

TEST_CASE("RigState tick regens energy and stamina", "[Game][Rig][G1]") {
    NF::RigState rig;
    rig.energy = 50.f;
    rig.stamina = 50.f;
    rig.tick(1.f);
    REQUIRE(rig.energy == Catch::Approx(52.f));
    REQUIRE(rig.stamina == Catch::Approx(55.f));
}

TEST_CASE("RigState tick drains oxygen", "[Game][Rig][G1]") {
    NF::RigState rig;
    rig.tick(10.f);
    REQUIRE(rig.oxygen == Catch::Approx(90.f));
}

TEST_CASE("RigState oxygen depletion damages health", "[Game][Rig][G1]") {
    NF::RigState rig;
    rig.oxygen = 0.f;
    rig.tick(1.f);
    REQUIRE(rig.health < 100.f);
    REQUIRE(rig.isAlive());
}

TEST_CASE("RigState takeDamage and heal", "[Game][Rig][G1]") {
    NF::RigState rig;
    rig.takeDamage(30.f);
    REQUIRE(rig.health == Catch::Approx(70.f));
    rig.heal(50.f);
    REQUIRE(rig.health == Catch::Approx(100.f)); // clamped to max
}

TEST_CASE("RigState death stops ticking", "[Game][Rig][G1]") {
    NF::RigState rig;
    rig.takeDamage(100.f);
    REQUIRE_FALSE(rig.isAlive());
    float prevEnergy = rig.energy;
    rig.tick(1.f);
    REQUIRE(rig.energy == prevEnergy); // no regen when dead
}

TEST_CASE("RigState consumeEnergy and consumeStamina", "[Game][Rig][G1]") {
    NF::RigState rig;
    rig.consumeEnergy(60.f);
    REQUIRE(rig.energy == Catch::Approx(40.f));
    rig.consumeStamina(80.f);
    REQUIRE(rig.stamina == Catch::Approx(20.f));
    rig.consumeEnergy(999.f);
    REQUIRE(rig.energy == Catch::Approx(0.f)); // clamped at 0
}

// ── G1: ResourceType tests ──────────────────────────────────────

TEST_CASE("ResourceType name round-trip", "[Game][Resource][G1]") {
    REQUIRE(NF::resourceTypeFromName("RawStone") == NF::ResourceType::RawStone);
    REQUIRE(NF::resourceTypeFromName("RawIron") == NF::ResourceType::RawIron);
    REQUIRE(NF::resourceTypeFromName("RefinedGold") == NF::ResourceType::RefinedGold);
    REQUIRE(NF::resourceTypeFromName("SteelPlate") == NF::ResourceType::SteelPlate);
    REQUIRE(NF::resourceTypeFromName("EnergyCell") == NF::ResourceType::EnergyCell);
    REQUIRE(std::string(NF::resourceTypeName(NF::ResourceType::CircuitBoard)) == "CircuitBoard");
    REQUIRE(std::string(NF::resourceTypeName(NF::ResourceType::RawCrystal)) == "RawCrystal");
}

// ── G1: Resource Drop Table tests ───────────────────────────────

TEST_CASE("ResourceDrop table returns correct drops", "[Game][Resource][G1]") {
    auto stoneDrops = NF::getResourceDrops(NF::VoxelType::Stone);
    REQUIRE(stoneDrops.size() == 1);
    REQUIRE(stoneDrops[0].resource == NF::ResourceType::RawStone);

    auto ironDrops = NF::getResourceDrops(NF::VoxelType::Ore_Iron);
    REQUIRE(ironDrops.size() == 1);
    REQUIRE(ironDrops[0].minAmount == 1);
    REQUIRE(ironDrops[0].maxAmount == 2);

    auto crystalDrops = NF::getResourceDrops(NF::VoxelType::Ore_Crystal);
    REQUIRE(crystalDrops[0].maxAmount == 3);

    auto airDrops = NF::getResourceDrops(NF::VoxelType::Air);
    REQUIRE(airDrops.empty());

    auto glassDrops = NF::getResourceDrops(NF::VoxelType::Glass);
    REQUIRE(glassDrops.empty());
}

// ── G1: ResourceInventory tests ─────────────────────────────────

TEST_CASE("ResourceInventory add and remove", "[Game][Resource][G1]") {
    NF::ResourceInventory inv;
    REQUIRE(inv.isEmpty());
    REQUIRE(inv.totalItems() == 0);

    inv.add(NF::ResourceType::RawIron, 5);
    REQUIRE(inv.count(NF::ResourceType::RawIron) == 5);
    REQUIRE(inv.totalItems() == 5);
    REQUIRE_FALSE(inv.isEmpty());

    REQUIRE(inv.remove(NF::ResourceType::RawIron, 3));
    REQUIRE(inv.count(NF::ResourceType::RawIron) == 2);

    REQUIRE_FALSE(inv.remove(NF::ResourceType::RawIron, 5));
    REQUIRE(inv.count(NF::ResourceType::RawIron) == 2);
}

TEST_CASE("ResourceInventory totalItems across types", "[Game][Resource][G1]") {
    NF::ResourceInventory inv;
    inv.add(NF::ResourceType::RawStone, 3);
    inv.add(NF::ResourceType::RefinedIron, 2);
    inv.add(NF::ResourceType::EnergyCell, 1);
    REQUIRE(inv.totalItems() == 6);
}

// ── G1: ToolState tests ─────────────────────────────────────────

TEST_CASE("ToolState isReady and use", "[Game][Tool][G1]") {
    NF::ToolState tool;
    tool.cooldownRate = 0.5f;
    REQUIRE(tool.isReady());

    tool.use();
    REQUIRE_FALSE(tool.isReady());
    REQUIRE(tool.cooldown == Catch::Approx(0.5f));
    REQUIRE(tool.durability == Catch::Approx(99.f));

    tool.tick(0.5f);
    REQUIRE(tool.isReady());
}

TEST_CASE("ToolState zero durability not ready", "[Game][Tool][G1]") {
    NF::ToolState tool;
    tool.durability = 0.f;
    REQUIRE_FALSE(tool.isReady());
}

TEST_CASE("ToolType name", "[Game][Tool][G1]") {
    REQUIRE(std::string(NF::toolTypeName(NF::ToolType::MiningLaser)) == "MiningLaser");
    REQUIRE(std::string(NF::toolTypeName(NF::ToolType::Scanner)) == "Scanner");
}

// ── G1: ToolBelt tests ──────────────────────────────────────────

TEST_CASE("ToolBelt init and slot access", "[Game][Tool][G1]") {
    NF::ToolBelt belt;
    belt.init();

    REQUIRE(belt.slotCount() == 4);
    REQUIRE(belt.activeSlot() == 0);
    REQUIRE(belt.activeTool().type == NF::ToolType::MiningLaser);
    REQUIRE(belt.slot(1).type == NF::ToolType::PlacementTool);
    REQUIRE(belt.slot(2).type == NF::ToolType::RepairTool);
    REQUIRE(belt.slot(3).type == NF::ToolType::Scanner);
}

TEST_CASE("ToolBelt selectSlot clamps", "[Game][Tool][G1]") {
    NF::ToolBelt belt;
    belt.init();
    belt.selectSlot(-1);
    REQUIRE(belt.activeSlot() == 0);
    belt.selectSlot(99);
    REQUIRE(belt.activeSlot() == 3);
    belt.selectSlot(2);
    REQUIRE(belt.activeSlot() == 2);
}

TEST_CASE("ToolBelt nextTool and prevTool cycle", "[Game][Tool][G1]") {
    NF::ToolBelt belt;
    belt.init();
    belt.nextTool();
    REQUIRE(belt.activeSlot() == 1);
    belt.nextTool();
    belt.nextTool();
    belt.nextTool();
    REQUIRE(belt.activeSlot() == 0); // wrapped around

    belt.prevTool();
    REQUIRE(belt.activeSlot() == 3); // wrapped backward
}

// ── G1: HUDState tests ──────────────────────────────────────────

TEST_CASE("HUDState defaults", "[Game][HUD][G1]") {
    NF::HUDState hud;
    REQUIRE(hud.showCrosshair);
    REQUIRE_FALSE(hud.targetLocked);
    REQUIRE(hud.targetVoxel == NF::VoxelType::Air);
    REQUIRE(hud.notifications.empty());
}

TEST_CASE("HUDState add and expire notifications", "[Game][HUD][G1]") {
    NF::HUDState hud;
    hud.addNotification("Mined stone!", 2.f);
    hud.addNotification("Low energy", 5.f);
    REQUIRE(hud.notifications.size() == 2);

    hud.tick(3.f);
    REQUIRE(hud.notifications.size() == 1);
    REQUIRE(hud.notifications[0].message == "Low energy");

    hud.tick(3.f);
    REQUIRE(hud.notifications.empty());
}

TEST_CASE("HUDState clearNotifications", "[Game][HUD][G1]") {
    NF::HUDState hud;
    hud.addNotification("test1");
    hud.addNotification("test2");
    hud.clearNotifications();
    REQUIRE(hud.notifications.empty());
}

// ── G1: InteractionSystem mining tests ──────────────────────────

TEST_CASE("InteractionSystem tryMine success", "[Game][Interaction][G1]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Stone);

    NF::RigState rig;
    NF::ToolBelt belt;
    belt.init();
    NF::Inventory voxelInv;
    NF::ResourceInventory resInv;

    NF::InteractionSystem interaction;
    NF::Vec3 origin{5.5f, 10.f, 5.5f};
    NF::Vec3 dir{0.f, -1.f, 0.f};

    auto result = interaction.tryMine(world, rig, belt, voxelInv, resInv, origin, dir);
    REQUIRE(result.success);
    REQUIRE(result.minedType == NF::VoxelType::Stone);
    REQUIRE(world.getWorld(5, 5, 5) == NF::VoxelType::Air);
    REQUIRE(voxelInv.count(NF::VoxelType::Stone) == 1);
    REQUIRE(resInv.count(NF::ResourceType::RawStone) == 1);
    REQUIRE(rig.energy < 100.f);
}

TEST_CASE("InteractionSystem tryMine fails wrong tool", "[Game][Interaction][G1]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Stone);

    NF::RigState rig;
    NF::ToolBelt belt;
    belt.init();
    belt.selectSlot(1); // placement tool
    NF::Inventory voxelInv;
    NF::ResourceInventory resInv;

    NF::InteractionSystem interaction;
    auto result = interaction.tryMine(world, rig, belt, voxelInv, resInv,
                                      {5.5f, 10.f, 5.5f}, {0.f, -1.f, 0.f});
    REQUIRE_FALSE(result.success);
}

TEST_CASE("InteractionSystem tryMine fails no energy", "[Game][Interaction][G1]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Stone);

    NF::RigState rig;
    rig.energy = 0.f;
    NF::ToolBelt belt;
    belt.init();
    NF::Inventory voxelInv;
    NF::ResourceInventory resInv;

    NF::InteractionSystem interaction;
    auto result = interaction.tryMine(world, rig, belt, voxelInv, resInv,
                                      {5.5f, 10.f, 5.5f}, {0.f, -1.f, 0.f});
    REQUIRE_FALSE(result.success);
}

TEST_CASE("InteractionSystem tryMine ore drops resources", "[Game][Interaction][G1]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Ore_Iron);

    NF::RigState rig;
    NF::ToolBelt belt;
    belt.init();
    NF::Inventory voxelInv;
    NF::ResourceInventory resInv;

    NF::InteractionSystem interaction;
    auto result = interaction.tryMine(world, rig, belt, voxelInv, resInv,
                                      {5.5f, 10.f, 5.5f}, {0.f, -1.f, 0.f});
    REQUIRE(result.success);
    REQUIRE_FALSE(result.drops.empty());
    REQUIRE(resInv.count(NF::ResourceType::RawIron) >= 1);
}

// ── G1: InteractionSystem placement tests ───────────────────────

TEST_CASE("InteractionSystem tryPlace success", "[Game][Interaction][G1]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Stone);

    NF::RigState rig;
    NF::ToolBelt belt;
    belt.init();
    belt.selectSlot(1); // placement tool
    NF::Inventory voxelInv;
    voxelInv.add(NF::VoxelType::Dirt, 5);

    NF::InteractionSystem interaction;
    NF::Vec3 origin{5.5f, 10.f, 5.5f};
    NF::Vec3 dir{0.f, -1.f, 0.f};

    auto result = interaction.tryPlace(world, rig, belt, voxelInv, origin, dir, NF::VoxelType::Dirt);
    REQUIRE(result.success);
    REQUIRE(result.placedType == NF::VoxelType::Dirt);
    REQUIRE(voxelInv.count(NF::VoxelType::Dirt) == 4);
    // Placed adjacent to stone (on top), at (5,6,5)
    REQUIRE(world.getWorld(5, 6, 5) == NF::VoxelType::Dirt);
}

TEST_CASE("InteractionSystem tryPlace fails no inventory", "[Game][Interaction][G1]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Stone);

    NF::RigState rig;
    NF::ToolBelt belt;
    belt.init();
    belt.selectSlot(1);
    NF::Inventory voxelInv; // empty

    NF::InteractionSystem interaction;
    auto result = interaction.tryPlace(world, rig, belt, voxelInv,
                                       {5.5f, 10.f, 5.5f}, {0.f, -1.f, 0.f},
                                       NF::VoxelType::Dirt);
    REQUIRE_FALSE(result.success);
}

// ── G1: InteractionSystem scan tests ────────────────────────────

TEST_CASE("InteractionSystem tryScan returns voxel info", "[Game][Interaction][G1]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Ore_Gold);

    NF::ToolBelt belt;
    belt.init();
    belt.selectSlot(3); // scanner

    NF::InteractionSystem interaction;
    auto hit = interaction.tryScan(world, belt, {5.5f, 10.f, 5.5f}, {0.f, -1.f, 0.f});
    REQUIRE(hit.has_value());
    REQUIRE(hit->type == NF::VoxelType::Ore_Gold);
}

TEST_CASE("InteractionSystem tryScan fails wrong tool", "[Game][Interaction][G1]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Stone);

    NF::ToolBelt belt;
    belt.init(); // active is mining laser

    NF::InteractionSystem interaction;
    auto hit = interaction.tryScan(world, belt, {5.5f, 10.f, 5.5f}, {0.f, -1.f, 0.f});
    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("InteractionSystem maxReach config", "[Game][Interaction][G1]") {
    NF::InteractionSystem interaction;
    REQUIRE(interaction.maxReach() == Catch::Approx(8.f));
    interaction.setMaxReach(12.f);
    REQUIRE(interaction.maxReach() == Catch::Approx(12.f));
}

// ── G1: GameSession tests ───────────────────────────────────────

TEST_CASE("GameSession init and shutdown", "[Game][Session][G1]") {
    NF::GameSession session;
    REQUIRE_FALSE(session.isActive());

    session.init();
    REQUIRE(session.isActive());
    REQUIRE(session.rig().health == 100.f);
    REQUIRE(session.toolBelt().slotCount() == 4);
    REQUIRE(session.voxelInventory().count(NF::VoxelType::Stone) == 0);
    REQUIRE(session.resourceInventory().isEmpty());

    session.shutdown();
    REQUIRE_FALSE(session.isActive());
}

TEST_CASE("GameSession tick updates rig and tools", "[Game][Session][G1]") {
    NF::GameSession session;
    session.init();

    session.rig().energy = 50.f;
    session.toolBelt().activeTool().use(); // put on cooldown
    float cd = session.toolBelt().activeTool().cooldown;
    REQUIRE(cd > 0.f);

    session.tick(1.f);
    REQUIRE(session.rig().energy > 50.f); // regen
    REQUIRE(session.toolBelt().activeTool().cooldown < cd); // reduced
}

TEST_CASE("GameSession tick expires HUD notifications", "[Game][Session][G1]") {
    NF::GameSession session;
    session.init();
    session.hud().addNotification("test", 1.f);
    REQUIRE(session.hud().notifications.size() == 1);

    session.tick(2.f);
    REQUIRE(session.hud().notifications.empty());
}

TEST_CASE("GameSession full mine interaction", "[Game][Session][G1]") {
    NF::GameSession session;
    session.init();

    session.world().setWorld(5, 5, 5, NF::VoxelType::Ore_Crystal);

    NF::Vec3 origin{5.5f, 10.f, 5.5f};
    NF::Vec3 dir{0.f, -1.f, 0.f};

    auto result = session.interaction().tryMine(
        session.world(), session.rig(), session.toolBelt(),
        session.voxelInventory(), session.resourceInventory(),
        origin, dir);

    REQUIRE(result.success);
    REQUIRE(session.voxelInventory().count(NF::VoxelType::Ore_Crystal) == 1);
    REQUIRE(session.resourceInventory().count(NF::ResourceType::RawCrystal) >= 1);
    REQUIRE(session.world().getWorld(5, 5, 5) == NF::VoxelType::Air);
}

// ── G2: ChunkRenderData tests ───────────────────────────────────

TEST_CASE("ChunkRenderData defaults", "[Game][G2]") {
    NF::ChunkRenderData data;
    REQUIRE_FALSE(data.valid);
    REQUIRE(data.version == 0);
    REQUIRE(data.mesh.vertexCount() == 0);
}

// ── G2: ChunkRenderCache tests ──────────────────────────────────

TEST_CASE("ChunkRenderCache update and get", "[Game][RenderCache][G2]") {
    NF::Chunk chunk;
    chunk.cx = 0; chunk.cy = 0; chunk.cz = 0;
    chunk.set(5, 5, 5, NF::VoxelType::Stone);

    NF::ChunkRenderCache cache;
    REQUIRE(cache.cacheSize() == 0);

    cache.update(chunk);
    REQUIRE(cache.cacheSize() == 1);

    auto* data = cache.get({0, 0, 0});
    REQUIRE(data != nullptr);
    REQUIRE(data->valid);
    REQUIRE(data->version == 1);
    REQUIRE(data->mesh.vertexCount() > 0);

    // Update again increments version
    cache.update(chunk);
    data = cache.get({0, 0, 0});
    REQUIRE(data->version == 2);
}

TEST_CASE("ChunkRenderCache remove and clear", "[Game][RenderCache][G2]") {
    NF::Chunk chunk;
    chunk.set(0, 0, 0, NF::VoxelType::Dirt);
    chunk.cx = 1; chunk.cy = 2; chunk.cz = 3;

    NF::ChunkRenderCache cache;
    cache.update(chunk);
    REQUIRE(cache.cacheSize() == 1);

    cache.remove({1, 2, 3});
    REQUIRE(cache.cacheSize() == 0);
    REQUIRE(cache.get({1, 2, 3}) == nullptr);

    cache.update(chunk);
    cache.clear();
    REQUIRE(cache.cacheSize() == 0);
}

TEST_CASE("ChunkRenderCache updateDirty", "[Game][RenderCache][G2]") {
    NF::WorldState world;
    world.setWorld(0, 0, 0, NF::VoxelType::Stone);
    world.setWorld(16, 0, 0, NF::VoxelType::Dirt);

    NF::ChunkRenderCache cache;
    int rebuilt = cache.updateDirty(world);
    REQUIRE(rebuilt == 2);
    REQUIRE(cache.cacheSize() == 2);

    // All chunks now clean, no more rebuilds
    int rebuilt2 = cache.updateDirty(world);
    REQUIRE(rebuilt2 == 0);

    // Dirty one chunk
    world.setWorld(0, 0, 0, NF::VoxelType::Grass);
    int rebuilt3 = cache.updateDirty(world);
    REQUIRE(rebuilt3 == 1);
}

// ── G2: ChunkRenderer tests ────────────────────────────────────

TEST_CASE("ChunkRenderer init/shutdown", "[Game][ChunkRenderer][G2]") {
    NF::ChunkRenderer renderer;
    renderer.init();
    REQUIRE(renderer.voxelMaterial().name() == NF::StringID("voxel_material"));
    renderer.shutdown();
}

TEST_CASE("ChunkRenderer render submits commands", "[Game][ChunkRenderer][G2]") {
    NF::WorldState world;
    world.setWorld(0, 0, 0, NF::VoxelType::Stone);

    NF::ChunkRenderCache cache;
    cache.updateDirty(world);

    NF::Camera cam;
    cam.position = {8, 8, 50};
    cam.target = {8, 8, 0};

    NF::RenderQueue queue;
    NF::ChunkRenderer renderer;
    renderer.init();

    int visible = renderer.render(world, cache, queue, cam, 1.f);
    REQUIRE(visible >= 1);
    REQUIRE(queue.size() >= 1);
}

TEST_CASE("ChunkRenderer countVisible", "[Game][ChunkRenderer][G2]") {
    NF::WorldState world;
    world.setWorld(0, 0, 0, NF::VoxelType::Stone);

    NF::Camera cam;
    cam.position = {8, 8, 50};
    cam.target = {8, 8, 0};

    NF::ChunkRenderer renderer;
    renderer.init();

    int visible = renderer.countVisible(world, cam, 1.f);
    REQUIRE(visible >= 1);
}

TEST_CASE("ChunkRenderer frustum culling", "[Game][ChunkRenderer][G2]") {
    NF::WorldState world;
    // Chunk at origin
    world.setWorld(0, 0, 0, NF::VoxelType::Stone);
    // Chunk far behind camera
    world.setWorld(5000, 5000, 5000, NF::VoxelType::Dirt);

    NF::Camera cam;
    cam.position = {8, 8, 50};
    cam.target = {8, 8, 0};
    cam.farPlane = 200.f;

    NF::ChunkRenderer renderer;
    renderer.init();

    // Should see chunk at origin but not the one at 5000,5000,5000
    int visible = renderer.countVisible(world, cam, 1.f);
    REQUIRE(visible < static_cast<int>(world.chunkCount()));
}
