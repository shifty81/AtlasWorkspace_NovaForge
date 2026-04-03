#include <catch2/catch_test_macros.hpp>
#include "NF/Core/Core.h"

TEST_CASE("Vec3 basic operations", "[Core][Math]") {
    NF::Vec3 a{1.0f, 2.0f, 3.0f};
    NF::Vec3 b{4.0f, 5.0f, 6.0f};

    SECTION("Addition") {
        auto c = a + b;
        REQUIRE(c.x == 5.0f);
        REQUIRE(c.y == 7.0f);
        REQUIRE(c.z == 9.0f);
    }

    SECTION("Subtraction") {
        auto c = b - a;
        REQUIRE(c.x == 3.0f);
        REQUIRE(c.y == 3.0f);
        REQUIRE(c.z == 3.0f);
    }

    SECTION("Scalar multiplication") {
        auto c = a * 2.0f;
        REQUIRE(c.x == 2.0f);
        REQUIRE(c.y == 4.0f);
        REQUIRE(c.z == 6.0f);
    }

    SECTION("Dot product") {
        float d = a.dot(b);
        REQUIRE(d == 32.0f); // 4+10+18
    }

    SECTION("Cross product") {
        auto c = a.cross(b);
        REQUIRE(c.x == -3.0f);
        REQUIRE(c.y == 6.0f);
        REQUIRE(c.z == -3.0f);
    }

    SECTION("Length squared") {
        float lsq = a.lengthSq();
        REQUIRE(lsq == 14.0f); // 1+4+9
    }
}

TEST_CASE("Vec2 basic operations", "[Core][Math]") {
    NF::Vec2 a{3.0f, 4.0f};
    NF::Vec2 b{1.0f, 2.0f};

    auto c = a + b;
    REQUIRE(c.x == 4.0f);
    REQUIRE(c.y == 6.0f);

    auto d = a - b;
    REQUIRE(d.x == 2.0f);
    REQUIRE(d.y == 2.0f);

    auto e = a * 0.5f;
    REQUIRE(e.x == 1.5f);
    REQUIRE(e.y == 2.0f);
}

TEST_CASE("Rect contains", "[Core][Math]") {
    NF::Rect r{10.0f, 20.0f, 100.0f, 50.0f};

    REQUIRE(r.contains(50.0f, 40.0f));
    REQUIRE(r.contains(10.0f, 20.0f));       // top-left
    REQUIRE(r.contains(110.0f, 70.0f));      // bottom-right
    REQUIRE_FALSE(r.contains(5.0f, 40.0f));  // outside left
    REQUIRE_FALSE(r.contains(50.0f, 80.0f)); // outside bottom
}

TEST_CASE("EventBus publish and subscribe", "[Core][Events]") {
    NF::EventBus bus;
    int callCount = 0;

    bus.subscribe(1, [&](const void*) { callCount++; });
    bus.subscribe(1, [&](const void*) { callCount += 10; });

    bus.publish(1);
    REQUIRE(callCount == 11);

    bus.publish(2); // no subscribers
    REQUIRE(callCount == 11);
}

TEST_CASE("Version constants", "[Core]") {
    REQUIRE(NF::NF_VERSION_MAJOR == 0);
    REQUIRE(NF::NF_VERSION_MINOR == 1);
    REQUIRE(NF::NF_VERSION_PATCH == 0);
    REQUIRE(std::string(NF::NF_VERSION_STRING) == "0.1.0");
}
