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
