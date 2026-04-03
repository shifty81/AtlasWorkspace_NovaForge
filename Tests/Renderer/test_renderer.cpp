#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "NF/Renderer/Renderer.h"

using Catch::Matchers::WithinAbs;

// ── Mesh ─────────────────────────────────────────────────────────

TEST_CASE("Mesh vertex and index storage", "[Renderer][Mesh]") {
    NF::Mesh mesh;
    REQUIRE(mesh.vertexCount() == 0);
    REQUIRE(mesh.indexCount() == 0);

    std::vector<NF::Vertex> verts = {
        {{0, 0, 0}, {0, 1, 0}, {0, 0}},
        {{1, 0, 0}, {0, 1, 0}, {1, 0}},
        {{0, 1, 0}, {0, 1, 0}, {0, 1}},
    };
    std::vector<uint32_t> indices = {0, 1, 2};

    mesh.setVertices(verts);
    mesh.setIndices(indices);

    REQUIRE(mesh.vertexCount() == 3);
    REQUIRE(mesh.indexCount() == 3);
    REQUIRE(mesh.triangleCount() == 1);
    REQUIRE(mesh.isDirty());

    mesh.clearDirty();
    REQUIRE_FALSE(mesh.isDirty());
}

TEST_CASE("Mesh bounds computation", "[Renderer][Mesh]") {
    NF::Mesh mesh({
        {{-1, -2, -3}, {}, {}},
        {{4, 5, 6}, {}, {}},
        {{0, 0, 0}, {}, {}},
    }, {0, 1, 2});

    auto [lo, hi] = mesh.bounds();
    REQUIRE_THAT(lo.x, WithinAbs(-1.f, 1e-5));
    REQUIRE_THAT(lo.y, WithinAbs(-2.f, 1e-5));
    REQUIRE_THAT(lo.z, WithinAbs(-3.f, 1e-5));
    REQUIRE_THAT(hi.x, WithinAbs(4.f, 1e-5));
    REQUIRE_THAT(hi.y, WithinAbs(5.f, 1e-5));
    REQUIRE_THAT(hi.z, WithinAbs(6.f, 1e-5));
}

TEST_CASE("Mesh constructor with data", "[Renderer][Mesh]") {
    std::vector<NF::Vertex> verts = {
        {{0, 0, 0}, {0, 0, 1}, {0, 0}},
        {{1, 0, 0}, {0, 0, 1}, {1, 0}},
        {{1, 1, 0}, {0, 0, 1}, {1, 1}},
        {{0, 1, 0}, {0, 0, 1}, {0, 1}},
    };
    NF::Mesh mesh(verts, {0, 1, 2, 2, 3, 0});

    REQUIRE(mesh.vertexCount() == 4);
    REQUIRE(mesh.indexCount() == 6);
    REQUIRE(mesh.triangleCount() == 2);
}

// ── Shader ───────────────────────────────────────────────────────

TEST_CASE("Shader compile", "[Renderer][Shader]") {
    NF::Shader shader(NF::StringID("test_shader"), "vertex_src", "fragment_src");
    REQUIRE_FALSE(shader.isCompiled());

    bool ok = shader.compile();
    REQUIRE(ok);
    REQUIRE(shader.isCompiled());
    REQUIRE(shader.name() == NF::StringID("test_shader"));
}

TEST_CASE("Shader empty source fails compile", "[Renderer][Shader]") {
    NF::Shader shader(NF::StringID("empty"), "", "");
    REQUIRE_FALSE(shader.compile());
}

TEST_CASE("Shader uniforms", "[Renderer][Shader]") {
    NF::Shader shader;
    shader.setUniformFloat("brightness", 0.8f);
    shader.setUniformVec3("lightDir", {1, 0, 0});

    REQUIRE(shader.floatUniforms().size() == 1);
    REQUIRE_THAT(shader.floatUniforms().at("brightness"), WithinAbs(0.8f, 1e-5));
    REQUIRE(shader.vec3Uniforms().size() == 1);
}

// ── Material ─────────────────────────────────────────────────────

TEST_CASE("Material properties", "[Renderer][Material]") {
    NF::Material mat(NF::StringID("metal"));
    mat.setColor({0.5f, 0.5f, 0.5f, 1.f});
    mat.setMetallic(0.9f);
    mat.setRoughness(0.2f);

    REQUIRE(mat.name() == NF::StringID("metal"));
    REQUIRE_THAT(mat.metallic(), WithinAbs(0.9f, 1e-5));
    REQUIRE_THAT(mat.roughness(), WithinAbs(0.2f, 1e-5));
    REQUIRE_THAT(mat.color().x, WithinAbs(0.5f, 1e-5));
}

// ── Render Queue ─────────────────────────────────────────────────

TEST_CASE("RenderQueue submit and sort", "[Renderer][Queue]") {
    NF::RenderQueue queue;
    REQUIRE(queue.empty());

    queue.submit({nullptr, nullptr, NF::Mat4::identity(), 3.f});
    queue.submit({nullptr, nullptr, NF::Mat4::identity(), 1.f});
    queue.submit({nullptr, nullptr, NF::Mat4::identity(), 2.f});

    REQUIRE(queue.size() == 3);

    queue.sort();
    auto& cmds = queue.commands();
    REQUIRE_THAT(cmds[0].sortKey, WithinAbs(1.f, 1e-5));
    REQUIRE_THAT(cmds[1].sortKey, WithinAbs(2.f, 1e-5));
    REQUIRE_THAT(cmds[2].sortKey, WithinAbs(3.f, 1e-5));

    queue.clear();
    REQUIRE(queue.empty());
}

// ── Camera ───────────────────────────────────────────────────────

TEST_CASE("Camera view and projection matrices", "[Renderer][Camera]") {
    NF::Camera cam;
    cam.position = {0, 0, 5};
    cam.target = {0, 0, 0};

    auto view = cam.viewMatrix();
    // View matrix should transform the target to near origin
    auto result = view.transformPoint({0, 0, 0});
    REQUIRE_THAT(result.z, WithinAbs(-5.f, 1e-3));

    auto proj = cam.projectionMatrix(16.f / 9.f);
    // Perspective matrix should have -1 in [3,2]
    REQUIRE_THAT(proj.at(3, 2), WithinAbs(-1.f, 1e-5));
}

TEST_CASE("Camera forward direction", "[Renderer][Camera]") {
    NF::Camera cam;
    cam.position = {0, 0, 0};
    cam.target = {0, 0, -1};

    auto fwd = cam.forward();
    REQUIRE_THAT(fwd.z, WithinAbs(-1.f, 1e-4));
}

// ── Renderer ─────────────────────────────────────────────────────

TEST_CASE("Renderer init and frame lifecycle", "[Renderer]") {
    NF::Renderer renderer;
    REQUIRE(renderer.init(1920, 1080));

    REQUIRE(renderer.width() == 1920);
    REQUIRE(renderer.height() == 1080);
    REQUIRE_THAT(renderer.aspect(), WithinAbs(1920.f / 1080.f, 1e-3));

    renderer.beginFrame();
    renderer.submit({nullptr, nullptr, NF::Mat4::identity(), 1.f});
    renderer.submit({nullptr, nullptr, NF::Mat4::identity(), 2.f});
    renderer.endFrame();

    REQUIRE(renderer.drawCallCount() == 2);

    renderer.shutdown();
}
