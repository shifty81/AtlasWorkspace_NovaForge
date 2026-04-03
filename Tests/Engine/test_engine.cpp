#include <catch2/catch_test_macros.hpp>
#include "NF/Engine/Engine.h"

TEST_CASE("EntityManager creates unique IDs", "[Engine][ECS]") {
    NF::EntityManager em;

    auto e1 = em.createEntity();
    auto e2 = em.createEntity();
    auto e3 = em.createEntity();

    REQUIRE(e1 != e2);
    REQUIRE(e2 != e3);
    REQUIRE(e1 != NF::INVALID_ENTITY);
    REQUIRE(em.count() == 3);
}

TEST_CASE("EntityManager destroys entities", "[Engine][ECS]") {
    NF::EntityManager em;

    auto e1 = em.createEntity();
    auto e2 = em.createEntity();
    REQUIRE(em.count() == 2);

    em.destroyEntity(e1);
    REQUIRE(em.count() == 1);

    const auto& alive = em.alive();
    REQUIRE(alive.size() == 1);
    REQUIRE(alive[0] == e2);
}

TEST_CASE("EntityManager handles empty state", "[Engine][ECS]") {
    NF::EntityManager em;
    REQUIRE(em.count() == 0);
    REQUIRE(em.alive().empty());

    // Destroying non-existent entity should not crash
    em.destroyEntity(999);
    REQUIRE(em.count() == 0);
}
