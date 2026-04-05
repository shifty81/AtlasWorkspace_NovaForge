#include <catch2/catch_test_macros.hpp>
#include "NF/UI/GLFWWindowProvider.h"
#include "NF/UI/ImGuiLayer.h"
#include "NF/UI/ImGuiBackend.h"
#include "NF/UI/GLFWInputAdapter.h"
#include "NF/UI/UI.h"
#include "NF/Input/Input.h"

// ── GLFWWindowProvider ───────────────────────────────────────────

TEST_CASE("GLFWWindowProvider init and shutdown", "[UI][M1][GLFW]") {
    NF::GLFWWindowProvider window;
    REQUIRE_FALSE(window.isInitialized());

    NF::GLFWWindowProvider::WindowConfig cfg;
    REQUIRE(window.init(cfg));
    REQUIRE(window.isInitialized());
    REQUIRE(window.width() == 1280);
    REQUIRE(window.height() == 800);

    window.shutdown();
    REQUIRE_FALSE(window.isInitialized());
}

TEST_CASE("GLFWWindowProvider config defaults", "[UI][M1][GLFW]") {
    NF::GLFWWindowProvider::WindowConfig cfg;
    REQUIRE(cfg.width == 1280);
    REQUIRE(cfg.height == 800);
    REQUIRE(cfg.title == "NovaForge Editor");
    REQUIRE(cfg.vsync == true);
    REQUIRE(cfg.maximized == false);
    REQUIRE(cfg.decorated == true);
}

TEST_CASE("GLFWWindowProvider custom config", "[UI][M1][GLFW]") {
    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig cfg;
    cfg.width  = 800;
    cfg.height = 600;
    cfg.title  = "Test Window";
    cfg.vsync  = false;

    REQUIRE(window.init(cfg));
    REQUIRE(window.width() == 800);
    REQUIRE(window.height() == 600);
    window.shutdown();
}

TEST_CASE("GLFWWindowProvider shouldClose after pollEvents", "[UI][M1][GLFW]") {
    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig cfg;
    window.init(cfg);

    REQUIRE_FALSE(window.shouldClose());
    window.pollEvents();
    REQUIRE(window.shouldClose());

    window.shutdown();
}

TEST_CASE("GLFWWindowProvider frame timing", "[UI][M1][GLFW]") {
    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig cfg;
    window.init(cfg);

    REQUIRE(window.frameTime() > 0.0);
    REQUIRE(window.fps() > 0.0);

    window.shutdown();
}

TEST_CASE("GLFWWindowProvider setTitle", "[UI][M1][GLFW]") {
    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig cfg;
    window.init(cfg);

    window.setTitle("New Title");
    // Stub: just verifies no crash
    REQUIRE(window.isInitialized());

    window.shutdown();
}

TEST_CASE("GLFWWindowProvider nativeHandle is null in stub", "[UI][M1][GLFW]") {
    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig cfg;
    window.init(cfg);

    REQUIRE(window.nativeHandle() == nullptr);

    window.shutdown();
}

// ── ImGuiLayer ───────────────────────────────────────────────────

TEST_CASE("ImGuiLayer init and shutdown", "[UI][M1][ImGui]") {
    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig wCfg;
    window.init(wCfg);

    NF::ImGuiLayer layer;
    REQUIRE_FALSE(layer.isInitialized());
    REQUIRE(layer.init(window));
    REQUIRE(layer.isInitialized());

    layer.shutdown();
    REQUIRE_FALSE(layer.isInitialized());

    window.shutdown();
}

TEST_CASE("ImGuiLayer fails with uninitialized window", "[UI][M1][ImGui]") {
    NF::GLFWWindowProvider window; // not initialized
    NF::ImGuiLayer layer;
    REQUIRE_FALSE(layer.init(window));
    REQUIRE_FALSE(layer.isInitialized());
}

TEST_CASE("ImGuiLayer frame lifecycle", "[UI][M1][ImGui]") {
    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig wCfg;
    window.init(wCfg);

    NF::ImGuiLayer layer;
    layer.init(window);

    REQUIRE(layer.frameCount() == 0);
    layer.beginFrame();
    layer.endFrame();
    REQUIRE(layer.frameCount() == 1);
    layer.beginFrame();
    layer.endFrame();
    REQUIRE(layer.frameCount() == 2);

    layer.shutdown();
    window.shutdown();
}

TEST_CASE("ImGuiLayer docking", "[UI][M1][ImGui]") {
    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig wCfg;
    window.init(wCfg);

    NF::ImGuiLayer layer;
    layer.init(window);

    layer.beginDockSpace("TestDock");
    REQUIRE(layer.activeDockSpace() == "TestDock");
    layer.endDockSpace();
    REQUIRE(layer.activeDockSpace().empty());

    layer.shutdown();
    window.shutdown();
}

TEST_CASE("ImGuiLayer panel open and close", "[UI][M1][ImGui]") {
    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig wCfg;
    window.init(wCfg);

    NF::ImGuiLayer layer;
    layer.init(window);

    REQUIRE(layer.beginPanel("Properties"));
    REQUIRE(layer.activePanel() == "Properties");
    layer.endPanel();
    REQUIRE(layer.activePanel().empty());

    // Panel with open=false
    bool open = false;
    REQUIRE_FALSE(layer.beginPanel("Closed", &open));

    layer.shutdown();
    window.shutdown();
}

TEST_CASE("ImGuiLayer viewport clear", "[UI][M1][ImGui]") {
    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig wCfg;
    window.init(wCfg);

    NF::ImGuiLayer layer;
    layer.init(window);

    // Should not crash — stub is a no-op
    layer.renderViewportClear(0.1f, 0.2f, 0.3f, 1.0f);
    REQUIRE(layer.isInitialized());

    layer.shutdown();
    window.shutdown();
}

TEST_CASE("ImGuiLayer config defaults", "[UI][M1][ImGui]") {
    NF::ImGuiLayer::ImGuiConfig cfg;
    REQUIRE(cfg.dockingEnabled == true);
    REQUIRE(cfg.viewportsEnabled == false);
    REQUIRE(cfg.fontSize == 14.f);
    REQUIRE(cfg.fontPath.empty());
    REQUIRE(cfg.darkTheme == true);
}

// ── ImGuiBackend ─────────────────────────────────────────────────

TEST_CASE("ImGuiBackend as UIBackend", "[UI][M1][Backend]") {
    NF::ImGuiBackend backend;
    NF::UIBackend* base = &backend;

    REQUIRE(base->init(1280, 800));
    REQUIRE(std::string(base->backendName()) == "ImGui");
    REQUIRE(base->isGPUAccelerated());

    base->beginFrame(1280, 800);
    base->endFrame();
    base->shutdown();
}

TEST_CASE("ImGuiBackend flush records stats", "[UI][M1][Backend]") {
    NF::ImGuiBackend backend;
    backend.init(800, 600);

    NF::UIVertex verts[4] = {};
    uint32_t indices[6] = {0, 1, 2, 2, 3, 0};

    backend.beginFrame(800, 600);
    backend.flush(verts, 4, indices, 6);
    REQUIRE(backend.lastVertexCount() == 4);
    REQUIRE(backend.lastIndexCount() == 6);
    backend.endFrame();

    backend.shutdown();
}

TEST_CASE("ImGuiBackend flush with zero vertices is no-op", "[UI][M1][Backend]") {
    NF::ImGuiBackend backend;
    backend.init(800, 600);

    backend.beginFrame(800, 600);
    backend.flush(nullptr, 0, nullptr, 0);
    REQUIRE(backend.lastVertexCount() == 0);
    REQUIRE(backend.lastIndexCount() == 0);
    backend.endFrame();

    backend.shutdown();
}

TEST_CASE("ImGuiBackend setImGuiLayer", "[UI][M1][Backend]") {
    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig wCfg;
    window.init(wCfg);

    NF::ImGuiLayer layer;
    layer.init(window);

    NF::ImGuiBackend backend;
    backend.init(1280, 800);
    backend.setImGuiLayer(&layer);
    // Verify no crash; layer pointer is stored
    REQUIRE(backend.isGPUAccelerated());

    backend.shutdown();
    layer.shutdown();
    window.shutdown();
}

// ── GLFWInputAdapter ─────────────────────────────────────────────

TEST_CASE("GLFWInputAdapter event injection", "[UI][M1][Input]") {
    NF::InputSystem input;
    input.init();

    NF::GLFWInputAdapter adapter(input);
    REQUIRE_FALSE(adapter.isAttached());
    REQUIRE(adapter.eventCount() == 0);

    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig wCfg;
    window.init(wCfg);

    adapter.attach(window);
    REQUIRE(adapter.isAttached());

    adapter.injectKeyEvent(NF::KeyCode::A, true);
    REQUIRE(adapter.eventCount() == 1);
    REQUIRE(input.isKeyDown(NF::KeyCode::A));

    adapter.injectKeyEvent(NF::KeyCode::A, false);
    REQUIRE(adapter.eventCount() == 2);
    REQUIRE_FALSE(input.isKeyDown(NF::KeyCode::A));

    adapter.detach();
    REQUIRE_FALSE(adapter.isAttached());

    input.shutdown();
    window.shutdown();
}

TEST_CASE("GLFWInputAdapter mouse move", "[UI][M1][Input]") {
    NF::InputSystem input;
    input.init();

    NF::GLFWInputAdapter adapter(input);
    adapter.injectMouseMove(100.f, 200.f);
    REQUIRE(adapter.eventCount() == 1);
    REQUIRE(input.state().mouse.x == 100.f);
    REQUIRE(input.state().mouse.y == 200.f);

    input.shutdown();
}

TEST_CASE("GLFWInputAdapter mouse button", "[UI][M1][Input]") {
    NF::InputSystem input;
    input.init();

    NF::GLFWInputAdapter adapter(input);
    adapter.injectMouseButton(0, true); // left button
    REQUIRE(input.isKeyDown(NF::KeyCode::Mouse1));
    REQUIRE(adapter.eventCount() == 1);

    adapter.injectMouseButton(0, false);
    REQUIRE_FALSE(input.isKeyDown(NF::KeyCode::Mouse1));

    input.shutdown();
}

TEST_CASE("GLFWInputAdapter scroll", "[UI][M1][Input]") {
    NF::InputSystem input;
    input.init();

    NF::GLFWInputAdapter adapter(input);
    adapter.injectScroll(0.f, 3.f);
    REQUIRE(adapter.eventCount() == 1);
    REQUIRE(input.state().mouse.scrollDelta == 3.f);

    input.shutdown();
}

TEST_CASE("GLFWInputAdapter attach requires initialized window", "[UI][M1][Input]") {
    NF::InputSystem input;
    input.init();

    NF::GLFWWindowProvider window; // not initialized
    NF::GLFWInputAdapter adapter(input);
    adapter.attach(window);
    REQUIRE_FALSE(adapter.isAttached());

    input.shutdown();
}

// ── Full M1 Integration ──────────────────────────────────────────

TEST_CASE("M1 integration: window, layer, backend, adapter", "[UI][M1][Integration]") {
    // 1. Window
    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig wCfg;
    wCfg.width  = 1024;
    wCfg.height = 768;
    wCfg.title  = "Integration Test";
    REQUIRE(window.init(wCfg));

    // 2. ImGui layer
    NF::ImGuiLayer layer;
    NF::ImGuiLayer::ImGuiConfig iCfg;
    iCfg.dockingEnabled = true;
    REQUIRE(layer.init(window, iCfg));

    // 3. Backend
    NF::ImGuiBackend backend;
    REQUIRE(backend.init(wCfg.width, wCfg.height));
    backend.setImGuiLayer(&layer);

    // 4. UIRenderer with backend
    NF::UIRenderer renderer;
    renderer.init();
    renderer.setBackend(&backend);

    // 5. Input
    NF::InputSystem input;
    input.init();
    NF::GLFWInputAdapter adapter(input);
    adapter.attach(window);

    // Simulate one frame
    REQUIRE_FALSE(window.shouldClose());
    window.pollEvents();

    layer.beginFrame();
    layer.beginDockSpace();

    layer.beginPanel("Scene");
    layer.endPanel();

    layer.endDockSpace();

    backend.beginFrame(window.width(), window.height());
    renderer.beginFrame(static_cast<float>(window.width()),
                        static_cast<float>(window.height()));
    renderer.drawRect({10.f, 10.f, 100.f, 50.f}, 0xFF0000FF);
    renderer.endFrame();
    backend.endFrame();

    layer.endFrame();

    REQUIRE(layer.frameCount() == 1);
    REQUIRE(backend.lastVertexCount() == 4);
    REQUIRE(backend.lastIndexCount() == 6);

    // Inject input
    adapter.injectKeyEvent(NF::KeyCode::Escape, true);
    REQUIRE(input.isKeyDown(NF::KeyCode::Escape));

    // Window should close after pollEvents
    REQUIRE(window.shouldClose());

    // Cleanup
    adapter.detach();
    renderer.shutdown();
    backend.shutdown();
    layer.shutdown();
    window.shutdown();
    input.shutdown();
}
