#include <catch2/catch_test_macros.hpp>
#include "NF/Game/Game.h"

TEST_CASE("Chunk voxel get/set", "[Game][Voxel]") {
    NF::Chunk chunk;

    // Default is air
    REQUIRE(chunk.get(0, 0, 0) == NF::VoxelType::Air);

    chunk.set(5, 5, 5, NF::VoxelType::Stone);
    REQUIRE(chunk.get(5, 5, 5) == NF::VoxelType::Stone);
    REQUIRE(chunk.dirty);
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
    REQUIRE_FALSE(chunk.dirty);
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
