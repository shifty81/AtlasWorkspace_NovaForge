#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "NF/Core/Core.h"
#include <thread>

using Catch::Matchers::WithinAbs;

// ── Vec3 ─────────────────────────────────────────────────────────

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

TEST_CASE("Vec3 normalization and length", "[Core][Math]") {
    NF::Vec3 v{3.0f, 4.0f, 0.0f};
    REQUIRE_THAT(v.length(), WithinAbs(5.0f, 1e-5));

    auto n = v.normalized();
    REQUIRE_THAT(n.x, WithinAbs(0.6f, 1e-5));
    REQUIRE_THAT(n.y, WithinAbs(0.8f, 1e-5));
    REQUIRE_THAT(n.z, WithinAbs(0.0f, 1e-5));

    NF::Vec3 zero{0.f, 0.f, 0.f};
    auto nz = zero.normalized();
    REQUIRE(nz.x == 0.f);
    REQUIRE(nz.y == 0.f);
    REQUIRE(nz.z == 0.f);
}

TEST_CASE("Vec3 negation and lerp", "[Core][Math]") {
    NF::Vec3 a{1.f, 2.f, 3.f};
    auto neg = -a;
    REQUIRE(neg.x == -1.f);
    REQUIRE(neg.y == -2.f);
    REQUIRE(neg.z == -3.f);

    NF::Vec3 b{5.f, 6.f, 7.f};
    auto mid = NF::Vec3::lerp(a, b, 0.5f);
    REQUIRE_THAT(mid.x, WithinAbs(3.f, 1e-5));
    REQUIRE_THAT(mid.y, WithinAbs(4.f, 1e-5));
    REQUIRE_THAT(mid.z, WithinAbs(5.f, 1e-5));
}

// ── Vec2 ─────────────────────────────────────────────────────────

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

TEST_CASE("Vec2 length and normalize", "[Core][Math]") {
    NF::Vec2 v{3.f, 4.f};
    REQUIRE_THAT(v.length(), WithinAbs(5.f, 1e-5));

    auto n = v.normalized();
    REQUIRE_THAT(n.x, WithinAbs(0.6f, 1e-5));
    REQUIRE_THAT(n.y, WithinAbs(0.8f, 1e-5));
}

// ── Rect ─────────────────────────────────────────────────────────

TEST_CASE("Rect contains", "[Core][Math]") {
    NF::Rect r{10.0f, 20.0f, 100.0f, 50.0f};

    REQUIRE(r.contains(50.0f, 40.0f));
    REQUIRE(r.contains(10.0f, 20.0f));       // top-left
    REQUIRE(r.contains(110.0f, 70.0f));      // bottom-right
    REQUIRE_FALSE(r.contains(5.0f, 40.0f));  // outside left
    REQUIRE_FALSE(r.contains(50.0f, 80.0f)); // outside bottom
}

TEST_CASE("Rect overlaps", "[Core][Math]") {
    NF::Rect a{0.f, 0.f, 10.f, 10.f};
    NF::Rect b{5.f, 5.f, 10.f, 10.f};
    NF::Rect c{20.f, 20.f, 5.f, 5.f};

    REQUIRE(a.overlaps(b));
    REQUIRE(b.overlaps(a));
    REQUIRE_FALSE(a.overlaps(c));
}

// ── EventBus ─────────────────────────────────────────────────────

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

TEST_CASE("EventBus clear and subscriber count", "[Core][Events]") {
    NF::EventBus bus;
    int count = 0;

    bus.subscribe(42, [&](const void*) { count++; });
    bus.subscribe(42, [&](const void*) { count++; });
    REQUIRE(bus.subscriberCount(42) == 2);

    bus.publish(42);
    REQUIRE(count == 2);

    bus.clear(42);
    REQUIRE(bus.subscriberCount(42) == 0);

    bus.publish(42);
    REQUIRE(count == 2); // no change
}

// ── Version ──────────────────────────────────────────────────────

TEST_CASE("Version constants", "[Core]") {
    REQUIRE(NF::NF_VERSION_MAJOR == 0);
    REQUIRE(NF::NF_VERSION_MINOR == 1);
    REQUIRE(NF::NF_VERSION_PATCH == 0);
    REQUIRE(std::string(NF::NF_VERSION_STRING) == "0.1.0");
}

// ── Quaternion ───────────────────────────────────────────────────

TEST_CASE("Quat identity", "[Core][Math]") {
    auto q = NF::Quat::identity();
    REQUIRE(q.x == 0.f);
    REQUIRE(q.y == 0.f);
    REQUIRE(q.z == 0.f);
    REQUIRE(q.w == 1.f);

    NF::Vec3 v{1.f, 2.f, 3.f};
    auto rotated = q.rotate(v);
    REQUIRE_THAT(rotated.x, WithinAbs(1.f, 1e-5));
    REQUIRE_THAT(rotated.y, WithinAbs(2.f, 1e-5));
    REQUIRE_THAT(rotated.z, WithinAbs(3.f, 1e-5));
}

TEST_CASE("Quat axis-angle rotation", "[Core][Math]") {
    constexpr float PI = 3.14159265358979f;
    auto q = NF::Quat::fromAxisAngle({0.f, 1.f, 0.f}, PI / 2.f);

    NF::Vec3 forward{0.f, 0.f, -1.f};
    auto rotated = q.rotate(forward);
    REQUIRE_THAT(rotated.x, WithinAbs(-1.f, 1e-4));
    REQUIRE_THAT(rotated.y, WithinAbs(0.f, 1e-4));
    REQUIRE_THAT(rotated.z, WithinAbs(0.f, 1e-4));
}

TEST_CASE("Quat multiply is composition", "[Core][Math]") {
    constexpr float PI = 3.14159265358979f;
    auto q1 = NF::Quat::fromAxisAngle({0.f, 1.f, 0.f}, PI / 2.f);
    auto q2 = NF::Quat::fromAxisAngle({0.f, 1.f, 0.f}, PI / 2.f);
    auto combined = q1 * q2; // 180 degrees around Y

    NF::Vec3 forward{0.f, 0.f, -1.f};
    auto rotated = combined.rotate(forward);
    REQUIRE_THAT(rotated.x, WithinAbs(0.f, 1e-4));
    REQUIRE_THAT(rotated.y, WithinAbs(0.f, 1e-4));
    REQUIRE_THAT(rotated.z, WithinAbs(1.f, 1e-4));
}

TEST_CASE("Quat slerp interpolation", "[Core][Math]") {
    auto a = NF::Quat::identity();
    constexpr float PI = 3.14159265358979f;
    // Slerp from 0 to 90 degrees around Y (well-defined interpolation)
    auto b = NF::Quat::fromAxisAngle({0.f, 1.f, 0.f}, PI / 2.f);

    auto mid = NF::Quat::slerp(a, b, 0.5f);
    // At t=0.5, should be 45 degrees around Y
    NF::Vec3 forward{0.f, 0.f, -1.f};
    auto rotated = mid.rotate(forward);
    // 45-degree rotation of (0,0,-1) around Y gives approximately (-0.707, 0, -0.707)
    float expected = -std::sin(PI / 4.f);
    REQUIRE_THAT(rotated.x, WithinAbs(expected, 1e-3));
    REQUIRE_THAT(rotated.z, WithinAbs(expected, 1e-3));
}

// ── Mat4 ─────────────────────────────────────────────────────────

TEST_CASE("Mat4 identity", "[Core][Math]") {
    auto m = NF::Mat4::identity();
    REQUIRE(m.at(0, 0) == 1.f);
    REQUIRE(m.at(1, 1) == 1.f);
    REQUIRE(m.at(2, 2) == 1.f);
    REQUIRE(m.at(3, 3) == 1.f);
    REQUIRE(m.at(0, 1) == 0.f);
}

TEST_CASE("Mat4 translation transforms point", "[Core][Math]") {
    auto m = NF::Mat4::translation(10.f, 20.f, 30.f);
    NF::Vec3 p{1.f, 2.f, 3.f};
    auto result = m.transformPoint(p);
    REQUIRE_THAT(result.x, WithinAbs(11.f, 1e-5));
    REQUIRE_THAT(result.y, WithinAbs(22.f, 1e-5));
    REQUIRE_THAT(result.z, WithinAbs(33.f, 1e-5));
}

TEST_CASE("Mat4 scaling transforms point", "[Core][Math]") {
    auto m = NF::Mat4::scaling(2.f, 3.f, 4.f);
    NF::Vec3 p{1.f, 1.f, 1.f};
    auto result = m.transformPoint(p);
    REQUIRE_THAT(result.x, WithinAbs(2.f, 1e-5));
    REQUIRE_THAT(result.y, WithinAbs(3.f, 1e-5));
    REQUIRE_THAT(result.z, WithinAbs(4.f, 1e-5));
}

TEST_CASE("Mat4 multiply identity", "[Core][Math]") {
    auto a = NF::Mat4::translation(5.f, 0.f, 0.f);
    auto id = NF::Mat4::identity();
    auto result = a * id;
    NF::Vec3 p{0.f, 0.f, 0.f};
    auto rp = result.transformPoint(p);
    REQUIRE_THAT(rp.x, WithinAbs(5.f, 1e-5));
}

TEST_CASE("Mat4 perspective produces valid projection", "[Core][Math]") {
    constexpr float PI = 3.14159265358979f;
    auto m = NF::Mat4::perspective(PI / 4.f, 16.f / 9.f, 0.1f, 100.f);
    REQUIRE(m.at(3, 2) == -1.f); // perspective divide indicator
    REQUIRE(m.at(3, 3) == 0.f);
}

// ── Transform ────────────────────────────────────────────────────

TEST_CASE("Transform default is identity", "[Core][Math]") {
    NF::Transform t;
    REQUIRE(t.position.x == 0.f);
    REQUIRE(t.scale.x == 1.f);

    auto fwd = t.forward();
    REQUIRE_THAT(fwd.z, WithinAbs(-1.f, 1e-5));
}

TEST_CASE("Transform toMatrix with translation", "[Core][Math]") {
    NF::Transform t;
    t.position = {10.f, 20.f, 30.f};
    auto m = t.toMatrix();
    NF::Vec3 origin{0.f, 0.f, 0.f};
    auto result = m.transformPoint(origin);
    REQUIRE_THAT(result.x, WithinAbs(10.f, 1e-4));
    REQUIRE_THAT(result.y, WithinAbs(20.f, 1e-4));
    REQUIRE_THAT(result.z, WithinAbs(30.f, 1e-4));
}

// ── StringID ─────────────────────────────────────────────────────

TEST_CASE("StringID hashing", "[Core]") {
    NF::StringID a("test");
    NF::StringID b("test");
    NF::StringID c("other");

    REQUIRE(a == b);
    REQUIRE(a != c);
    REQUIRE(a.isValid());

    NF::StringID empty;
    REQUIRE_FALSE(empty.isValid());
}

TEST_CASE("StringID constexpr and runtime agree", "[Core]") {
    constexpr NF::StringID compiletime("hello");
    NF::StringID runtime(std::string_view("hello"));
    REQUIRE(compiletime == runtime);
}

// ── Timer ────────────────────────────────────────────────────────

TEST_CASE("Timer measures elapsed time", "[Core]") {
    NF::Timer timer;
    timer.reset();

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    float elapsed = timer.elapsed();
    REQUIRE(elapsed > 0.01f);
    REQUIRE(elapsed < 1.0f);
}

TEST_CASE("Timer delta returns frame time", "[Core]") {
    NF::Timer timer;
    timer.reset();

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    float dt1 = timer.delta();
    REQUIRE(dt1 > 0.01f);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    float dt2 = timer.delta();
    REQUIRE(dt2 > 0.01f);
}

// ── TypeID ───────────────────────────────────────────────────────

TEST_CASE("TypeID returns unique IDs per type", "[Core]") {
    auto intID = NF::TypeID::get<int>();
    auto floatID = NF::TypeID::get<float>();
    auto intID2 = NF::TypeID::get<int>();

    REQUIRE(intID != floatID);
    REQUIRE(intID == intID2);
}

// ── Core lifecycle ───────────────────────────────────────────────

TEST_CASE("Core init and shutdown", "[Core]") {
    NF::coreInit();
    REQUIRE(NF::isCoreInitialized());

    NF::coreShutdown();
    REQUIRE_FALSE(NF::isCoreInitialized());
}
