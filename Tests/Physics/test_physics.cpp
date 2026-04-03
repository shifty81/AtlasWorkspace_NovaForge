#include <catch2/catch_test_macros.hpp>
#include "NF/Physics/Physics.h"

TEST_CASE("AABB intersection", "[Physics]") {
    NF::AABB a{{0, 0, 0}, {2, 2, 2}};
    NF::AABB b{{1, 1, 1}, {3, 3, 3}};
    NF::AABB c{{5, 5, 5}, {6, 6, 6}};

    REQUIRE(a.intersects(b));
    REQUIRE(b.intersects(a));
    REQUIRE_FALSE(a.intersects(c));
    REQUIRE_FALSE(c.intersects(a));
}

TEST_CASE("AABB touching edges intersect", "[Physics]") {
    NF::AABB a{{0, 0, 0}, {1, 1, 1}};
    NF::AABB b{{1, 0, 0}, {2, 1, 1}};

    REQUIRE(a.intersects(b)); // touching at x=1
}
