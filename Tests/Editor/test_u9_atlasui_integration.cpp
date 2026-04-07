#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "NF/Editor/Editor.h"
#include "NF/UI/AtlasUI/GDIRenderBackend.h"
#include "NF/UI/AtlasUI/DrawPrimitives.h"
#include "NF/UI/AtlasUI/Panels/ProjectPickerPanel.h"
#include "NF/UI/AtlasUI/Contexts.h"
#include "NF/UI/UIBackend.h"

// ── Helpers ───────────────────────────────────────────────────────

/// Minimal IPaintContext that records every command pushed via the DrawList.
struct TrackingPaintContext final : public NF::UI::AtlasUI::IPaintContext {
    NF::UI::AtlasUI::DrawList& drawList() override { return m_dl; }
    void fillRect(const NF::Rect& r, NF::UI::AtlasUI::Color c) override {
        m_dl.push(NF::UI::AtlasUI::FillRectCmd{r, c});
    }
    void drawRect(const NF::Rect& r, NF::UI::AtlasUI::Color c) override {
        m_dl.push(NF::UI::AtlasUI::DrawRectCmd{r, c});
    }
    void drawText(const NF::Rect& r, std::string_view t,
                  NF::UI::AtlasUI::FontId f, NF::UI::AtlasUI::Color c) override {
        m_dl.push(NF::UI::AtlasUI::DrawTextCmd{r, std::string(t), f, c});
    }
    void pushClip(const NF::Rect& r) override { m_dl.push(NF::UI::AtlasUI::PushClipCmd{r}); }
    void popClip()                    override { m_dl.push(NF::UI::AtlasUI::PopClipCmd{}); }
    NF::UI::AtlasUI::DrawList m_dl;
};

// ── Step 1: GDIRenderBackend colour conversion ────────────────────

TEST_CASE("GDIRenderBackend::drawCommandBuffer handles all 5 command types", "[Editor][U9]") {
    // Verify the backend dispatches every variant without crashing.
    NF::UIRenderer renderer;
    NF::NullBackend nullBackend;
    nullBackend.init(640, 480);
    renderer.setBackend(&nullBackend);
    renderer.beginFrame(640.f, 480.f);

    NF::UI::AtlasUI::GDIRenderBackend backend;
    backend.setUIRenderer(&renderer);

    NF::UI::AtlasUI::DrawList dl;
    dl.push(NF::UI::AtlasUI::FillRectCmd  {{10.f, 10.f, 50.f, 20.f}, 0xFF202020u}); // FillRect
    dl.push(NF::UI::AtlasUI::DrawRectCmd  {{10.f, 10.f, 50.f, 20.f}, 0xFF4A4A4Au}); // DrawRect
    dl.push(NF::UI::AtlasUI::DrawTextCmd  {{10.f, 10.f, 80.f, 14.f}, "Hello", 0, 0xFFF2F2F2u}); // DrawText
    dl.push(NF::UI::AtlasUI::PushClipCmd  {{0.f,  0.f, 640.f, 480.f}}); // no-op
    dl.push(NF::UI::AtlasUI::PopClipCmd   {});                           // no-op

    REQUIRE_NOTHROW([&]{ backend.beginFrame(); backend.drawCommandBuffer(dl); backend.endFrame(); }());
    REQUIRE(dl.size() == 5); // all 5 command types present
}

TEST_CASE("GDIRenderBackend setUIRenderer stores the pointer", "[Editor][U9]") {
    NF::UI::AtlasUI::GDIRenderBackend backend;
    REQUIRE(backend.uiRenderer() == nullptr);

    NF::UIRenderer renderer;
    backend.setUIRenderer(&renderer);
    REQUIRE(backend.uiRenderer() == &renderer);
}

TEST_CASE("GDIRenderBackend with null UIRenderer is a safe no-op", "[Editor][U9]") {
    NF::UI::AtlasUI::GDIRenderBackend backend; // uiRenderer = nullptr
    NF::UI::AtlasUI::DrawList dl;
    dl.push(NF::UI::AtlasUI::FillRectCmd{{0.f, 0.f, 10.f, 10.f}, 0xFFAABBCCu});
    REQUIRE_NOTHROW([&]{ backend.beginFrame(); backend.drawCommandBuffer(dl); backend.endFrame(); }());
}

// ── Step 2: AtlasUI ProjectPickerPanel ───────────────────────────

TEST_CASE("AtlasUI ProjectPickerPanel default state", "[Editor][U9]") {
    NF::UI::AtlasUI::ProjectPickerPanel picker;
    REQUIRE(picker.projectCount() == 0);
    REQUIRE_FALSE(picker.isLoaded());
    REQUIRE_FALSE(picker.isVisible());
    REQUIRE(picker.selectedIndex() == -1);
    REQUIRE_FALSE(picker.hasSelection());
}

TEST_CASE("AtlasUI ProjectPickerPanel show/hide toggle visibility", "[Editor][U9]") {
    NF::UI::AtlasUI::ProjectPickerPanel picker;
    picker.show();
    REQUIRE(picker.isVisible());
    picker.hide();
    REQUIRE_FALSE(picker.isVisible());
}

TEST_CASE("AtlasUI ProjectPickerPanel addProject and selectProject", "[Editor][U9]") {
    NF::UI::AtlasUI::ProjectPickerPanel picker;
    NF::UI::AtlasUI::ProjectEntry pe;
    pe.name = "TestProject";
    pe.path = "/fake/path/test.atlas.json";
    picker.addProject(pe);

    REQUIRE(picker.projectCount() == 1);
    REQUIRE(picker.selectProject(0));
    REQUIRE(picker.selectedIndex() == 0);
    REQUIRE(picker.hasSelection());
    REQUIRE(picker.selectedProject() != nullptr);
    REQUIRE(picker.selectedProject()->name == "TestProject");
}

TEST_CASE("AtlasUI ProjectPickerPanel selectProject out-of-range returns false", "[Editor][U9]") {
    NF::UI::AtlasUI::ProjectPickerPanel picker;
    REQUIRE_FALSE(picker.selectProject(0));
    REQUIRE_FALSE(picker.selectProject(-1));
}

TEST_CASE("AtlasUI ProjectPickerPanel loadSelected sets isLoaded and hides picker", "[Editor][U9]") {
    NF::UI::AtlasUI::ProjectPickerPanel picker;
    NF::UI::AtlasUI::ProjectEntry pe;
    pe.name = "Alpha";
    pe.path = "/projects/alpha.atlas.json";
    picker.addProject(pe);
    picker.show();
    picker.selectProject(0);

    REQUIRE(picker.loadSelected());
    REQUIRE(picker.isLoaded());
    REQUIRE_FALSE(picker.isVisible()); // loadSelected() hides the picker
    REQUIRE(picker.loadedProjectPath() == "/projects/alpha.atlas.json");
}

TEST_CASE("AtlasUI ProjectPickerPanel loadSelected fails when nothing selected", "[Editor][U9]") {
    NF::UI::AtlasUI::ProjectPickerPanel picker;
    REQUIRE_FALSE(picker.loadSelected());
    REQUIRE_FALSE(picker.isLoaded());
}

TEST_CASE("AtlasUI ProjectPickerPanel clearProjects resets list and selection", "[Editor][U9]") {
    NF::UI::AtlasUI::ProjectPickerPanel picker;
    NF::UI::AtlasUI::ProjectEntry pe; pe.name = "X"; pe.path = "/x.atlas.json";
    picker.addProject(pe);
    picker.selectProject(0);
    picker.clearProjects();
    REQUIRE(picker.projectCount() == 0);
    REQUIRE(picker.selectedIndex() == -1);
}

TEST_CASE("AtlasUI ProjectPickerPanel scanDirectory on non-existent dir returns 0", "[Editor][U9]") {
    NF::UI::AtlasUI::ProjectPickerPanel picker;
    size_t n = picker.scanDirectory("/tmp/this_path_does_not_exist_u9test");
    REQUIRE(n == 0);
    REQUIRE(picker.projectCount() == 0);
}

TEST_CASE("AtlasUI ProjectPickerPanel handleInput Cancel button hides picker", "[Editor][U9]") {
    NF::UI::AtlasUI::ProjectPickerPanel picker;
    picker.show();

    // Paint into a tracking context so hit-test geometry is established
    TrackingPaintContext paintCtx;
    picker.arrange({0.f, 0.f, 1280.f, 800.f});
    picker.paint(paintCtx);
    REQUIRE_FALSE(paintCtx.m_dl.empty()); // something was painted

    // Simulate a click on the Cancel button.
    // Dialog is centred in 1280x800: dx=(1280-520)*0.5=380, dy=(800-360)*0.5=220.
    // cancelBtn: {dx+dw-104, bby, 88, 26} = {796, 540, 88, 26}
    // Click centre: (840, 553)
    NF::UI::AtlasUI::BasicInputContext inputCtx;
    inputCtx.setMousePosition({840.f, 553.f});
    inputCtx.setPrimaryDown(false); // ensure rising edge on next call
    picker.handleInput(inputCtx);   // leading-edge detection: prev=false, cur=false → no click

    inputCtx.setPrimaryDown(true);  // press
    picker.handleInput(inputCtx);   // leading edge: prev=false, cur=true → click

    REQUIRE_FALSE(picker.isVisible()); // Cancel was hit
}

TEST_CASE("AtlasUI ProjectPickerPanel handleInput row click selects project", "[Editor][U9]") {
    NF::UI::AtlasUI::ProjectPickerPanel picker;
    NF::UI::AtlasUI::ProjectEntry pe; pe.name = "MyGame"; pe.path = "/games/mygame.atlas.json";
    picker.addProject(pe);
    picker.show();

    TrackingPaintContext paintCtx;
    picker.arrange({0.f, 0.f, 1280.f, 800.f});
    picker.paint(paintCtx);

    // First row is at firstRowY=dy+60: dy=(800-360)*0.5=220, firstRowY=280
    // Row centre y ~ 280 + 13 = 293; x centre ~ 380 + 8 + (520-16)/2 = ~642
    NF::UI::AtlasUI::BasicInputContext inputCtx;
    inputCtx.setMousePosition({642.f, 293.f});
    inputCtx.setPrimaryDown(false);
    picker.handleInput(inputCtx); // reset prev state

    inputCtx.setPrimaryDown(true);
    picker.handleInput(inputCtx); // click

    REQUIRE(picker.selectedIndex() == 0);
    REQUIRE(picker.isVisible()); // row click only selects, doesn't load
}

TEST_CASE("AtlasUI ProjectPickerPanel handleInput Open button loads selected", "[Editor][U9]") {
    NF::UI::AtlasUI::ProjectPickerPanel picker;
    NF::UI::AtlasUI::ProjectEntry pe; pe.name = "Beta"; pe.path = "/beta.atlas.json";
    picker.addProject(pe);
    picker.show();
    picker.selectProject(0);

    TrackingPaintContext paintCtx;
    picker.arrange({0.f, 0.f, 1280.f, 800.f});
    picker.paint(paintCtx);

    // Open button: {dx+dw-200, bby, 88, 26} = {700, 540, 88, 26}; centre ~ (744, 553)
    NF::UI::AtlasUI::BasicInputContext inputCtx;
    inputCtx.setMousePosition({744.f, 553.f});
    inputCtx.setPrimaryDown(false);
    picker.handleInput(inputCtx);

    inputCtx.setPrimaryDown(true);
    picker.handleInput(inputCtx);

    REQUIRE(picker.isLoaded());
    REQUIRE_FALSE(picker.isVisible()); // Open hides picker
}

// ── Step 3: EditorApp atlasHost has 8 panels ─────────────────────

TEST_CASE("EditorApp atlasHost has 8 AtlasUI panels after init", "[Editor][U9]") {
    NF::EditorApp editor;
    REQUIRE(editor.init(1280, 800));
    REQUIRE(editor.atlasHost().panels().size() == 8);
    editor.shutdown();
}

// ── Step 4: EditorApp::renderAll() uses AtlasUI path ─────────────

TEST_CASE("EditorApp renderAll does not crash with AtlasUI backend wired", "[Editor][U9]") {
    NF::EditorApp editor;
    REQUIRE(editor.init(1280, 800));

    NF::NullBackend nullBackend;
    nullBackend.init(1280, 800);
    editor.uiRenderer().setBackend(&nullBackend);
    editor.uiRenderer().beginFrame(1280.f, 800.f);

    REQUIRE_NOTHROW(editor.renderAll(1280.f, 800.f));

    editor.uiRenderer().endFrame();
    editor.shutdown();
}

// ── Step 5: EditorApp::update() smoke test ───────────────────────

TEST_CASE("EditorApp update() does not crash and processes input once", "[Editor][U9]") {
    NF::EditorApp editor;
    REQUIRE(editor.init(1280, 800));

    NF::InputSystem input;
    input.init();
    input.update();  // main loop calls this once before editor.update()

    REQUIRE_NOTHROW(editor.update(0.016f, input));

    input.shutdown();
    editor.shutdown();
}
