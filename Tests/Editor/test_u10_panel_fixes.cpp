#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "NF/UI/AtlasUI/GDIRenderBackend.h"
#include "NF/UI/AtlasUI/DrawPrimitives.h"
#include "NF/UI/AtlasUI/Contexts.h"
#include "NF/UI/AtlasUI/Panels/ContentBrowserPanel.h"
#include "NF/UI/AtlasUI/Panels/HierarchyPanel.h"
#include "NF/UI/AtlasUI/Panels/InspectorPanel.h"
#include "NF/UI/AtlasUI/Panels/ConsolePanel.h"
#include "NF/UI/AtlasUI/Panels/ViewportPanel.h"
#include "NF/UI/UIBackend.h"

using namespace NF::UI::AtlasUI;

// ── Shared helpers ────────────────────────────────────────────────

/// Minimal IPaintContext that records commands to an inspectable DrawList.
struct TrackingCtx final : public IPaintContext {
    DrawList& drawList() override { return m_dl; }
    void fillRect(const NF::Rect& r, Color c) override { m_dl.push(FillRectCmd{r, c}); }
    void drawRect(const NF::Rect& r, Color c) override { m_dl.push(DrawRectCmd{r, c}); }
    void drawText(const NF::Rect& r, std::string_view t, FontId f, Color c) override {
        m_dl.push(DrawTextCmd{r, std::string(t), f, c});
    }
    void pushClip(const NF::Rect& r) override { m_dl.push(PushClipCmd{r}); }
    void popClip()                    override { m_dl.push(PopClipCmd{}); }

    // Count commands of a given type
    template <typename T>
    size_t count() const {
        size_t n = 0;
        for (const auto& cmd : m_dl.commands())
            if (std::holds_alternative<T>(cmd)) ++n;
        return n;
    }

    DrawList m_dl;
};

// ── GDIRenderBackend clip tests ───────────────────────────────────

TEST_CASE("GDIRenderBackend clip stack: text outside clip rect is skipped", "[Editor][U10]") {
    NF::UIRenderer renderer;
    NF::NullBackend nullBackend;
    nullBackend.init(400, 300);
    renderer.setBackend(&nullBackend);
    renderer.beginFrame(400.f, 300.f);

    GDIRenderBackend backend;
    backend.setUIRenderer(&renderer);

    // Clip to {0, 0, 100, 100}; text origin is at x=200 (outside right edge)
    DrawList dl;
    dl.push(PushClipCmd{{0.f, 0.f, 100.f, 100.f}});
    dl.push(DrawTextCmd{{200.f, 10.f, 80.f, 14.f}, "Outside", 0, 0xFFFFFFFF});
    dl.push(PopClipCmd{});

    backend.beginFrame();
    backend.drawCommandBuffer(dl);
    backend.endFrame();

    // Text whose x origin is beyond clip.x+clip.w must be silently dropped
    REQUIRE(renderer.textCmds().empty());
}

TEST_CASE("GDIRenderBackend clip stack: text inside clip rect is kept", "[Editor][U10]") {
    NF::UIRenderer renderer;
    NF::NullBackend nullBackend;
    nullBackend.init(400, 300);
    renderer.setBackend(&nullBackend);
    renderer.beginFrame(400.f, 300.f);

    GDIRenderBackend backend;
    backend.setUIRenderer(&renderer);

    // Clip to {0, 0, 400, 300}; text origin at (10, 10) is inside
    DrawList dl;
    dl.push(PushClipCmd{{0.f, 0.f, 400.f, 300.f}});
    dl.push(DrawTextCmd{{10.f, 10.f, 80.f, 14.f}, "Hello", 0, 0xFFFFFFFF});
    dl.push(PopClipCmd{});

    backend.beginFrame();
    backend.drawCommandBuffer(dl);
    backend.endFrame();

    REQUIRE(renderer.textCmds().size() == 1);
}

TEST_CASE("GDIRenderBackend clip stack: long text is truncated when it overflows clip", "[Editor][U10]") {
    NF::UIRenderer renderer;
    NF::NullBackend nullBackend;
    nullBackend.init(400, 300);
    renderer.setBackend(&nullBackend);
    renderer.beginFrame(400.f, 300.f);

    GDIRenderBackend backend;
    backend.setUIRenderer(&renderer);

    // Clip to {0, 0, 80, 100} — only ~10 chars fit at ~8px each
    // Text is 30 chars long, starting inside the clip at x=0
    const std::string longText = "C:\\very\\long\\path\\that\\overflows";
    DrawList dl;
    dl.push(PushClipCmd{{0.f, 0.f, 80.f, 100.f}});
    dl.push(DrawTextCmd{{0.f, 10.f, 200.f, 14.f}, longText, 0, 0xFFFFFFFF});
    dl.push(PopClipCmd{});

    backend.beginFrame();
    backend.drawCommandBuffer(dl);
    backend.endFrame();

    // Text should be present but truncated (shorter than the original)
    REQUIRE(renderer.textCmds().size() == 1);
    REQUIRE(renderer.textCmds()[0].text.size() < longText.size());
}

TEST_CASE("GDIRenderBackend clip stack: no clip allows any text", "[Editor][U10]") {
    NF::UIRenderer renderer;
    NF::NullBackend nullBackend;
    nullBackend.init(400, 300);
    renderer.setBackend(&nullBackend);
    renderer.beginFrame(400.f, 300.f);

    GDIRenderBackend backend;
    backend.setUIRenderer(&renderer);

    // No PushClipCmd — text at any position must pass through unchanged
    DrawList dl;
    dl.push(DrawTextCmd{{500.f, 500.f, 80.f, 14.f}, "NoCLip", 0, 0xFFFFFFFF});

    backend.beginFrame();
    backend.drawCommandBuffer(dl);
    backend.endFrame();

    REQUIRE(renderer.textCmds().size() == 1);
    REQUIRE(renderer.textCmds()[0].text == "NoCLip");
}

TEST_CASE("GDIRenderBackend clip stack: text below clip rect is skipped", "[Editor][U10]") {
    NF::UIRenderer renderer;
    NF::NullBackend nullBackend;
    nullBackend.init(400, 300);
    renderer.setBackend(&nullBackend);
    renderer.beginFrame(400.f, 300.f);

    GDIRenderBackend backend;
    backend.setUIRenderer(&renderer);

    // Clip to {0, 0, 400, 50}; text at y=100 is entirely below
    DrawList dl;
    dl.push(PushClipCmd{{0.f, 0.f, 400.f, 50.f}});
    dl.push(DrawTextCmd{{10.f, 100.f, 80.f, 14.f}, "Below", 0, 0xFFFFFFFF});
    dl.push(PopClipCmd{});

    backend.beginFrame();
    backend.drawCommandBuffer(dl);
    backend.endFrame();

    REQUIRE(renderer.textCmds().empty());
}

// ── Panel clipping: paint() emits pushClip/popClip ────────────────

TEST_CASE("ContentBrowserPanel::paint emits PushClipCmd and PopClipCmd", "[Editor][U10]") {
    ContentBrowserPanel panel;
    panel.setVisible(true);
    panel.arrange({0.f, 0.f, 200.f, 300.f});
    panel.setCurrentPath("/projects/content");
    panel.addEntry("Assets", true);

    TrackingCtx ctx;
    panel.paint(ctx);

    REQUIRE(ctx.count<PushClipCmd>() >= 1);
    REQUIRE(ctx.count<PopClipCmd>()  >= 1);
}

TEST_CASE("HierarchyPanel::paint emits PushClipCmd and PopClipCmd", "[Editor][U10]") {
    HierarchyPanel panel;
    panel.setVisible(true);
    panel.arrange({0.f, 0.f, 200.f, 300.f});
    panel.addEntity(1, "Player", false, 0);

    TrackingCtx ctx;
    panel.paint(ctx);

    REQUIRE(ctx.count<PushClipCmd>() >= 1);
    REQUIRE(ctx.count<PopClipCmd>()  >= 1);
}

TEST_CASE("InspectorPanel::paint emits PushClipCmd and PopClipCmd", "[Editor][U10]") {
    InspectorPanel panel;
    panel.setVisible(true);
    panel.arrange({0.f, 0.f, 200.f, 300.f});

    TrackingCtx ctx;
    panel.paint(ctx);

    REQUIRE(ctx.count<PushClipCmd>() >= 1);
    REQUIRE(ctx.count<PopClipCmd>()  >= 1);
}

TEST_CASE("ConsolePanel::paint emits PushClipCmd and PopClipCmd", "[Editor][U10]") {
    ConsolePanel panel;
    panel.setVisible(true);
    panel.arrange({0.f, 0.f, 400.f, 150.f});
    panel.addMessage("Hello from console", MessageLevel::Info, 0.f);

    TrackingCtx ctx;
    panel.paint(ctx);

    REQUIRE(ctx.count<PushClipCmd>() >= 1);
    REQUIRE(ctx.count<PopClipCmd>()  >= 1);
}

TEST_CASE("ViewportPanel::paint emits PushClipCmd and PopClipCmd", "[Editor][U10]") {
    ViewportPanel panel;
    panel.setVisible(true);
    panel.arrange({0.f, 0.f, 800.f, 600.f});

    TrackingCtx ctx;
    panel.paint(ctx);

    REQUIRE(ctx.count<PushClipCmd>() >= 1);
    REQUIRE(ctx.count<PopClipCmd>()  >= 1);
}

// ── ContentBrowserPanel::handleInput ─────────────────────────────

TEST_CASE("ContentBrowserPanel handleInput: click on entry selects it", "[Editor][U10]") {
    ContentBrowserPanel panel;
    panel.setVisible(true);
    panel.arrange({0.f, 0.f, 300.f, 400.f});
    panel.addEntry("Assets", true);
    panel.addEntry("Levels", true);
    panel.addEntry("README.md", false);

    // Paint to establish hit-test geometry
    BasicPaintContext paintCtx;
    panel.paint(paintCtx);

    // Entry rows start after: title bar (22px) + path (20px) + separator (1+4px) = y≈51
    // first entry centre y ≈ 56 + 9 = 65 (approx — exact depends on spacing constants)
    // We use a y well within the first row to be safe; x in the middle of panel width
    BasicInputContext inputCtx;
    inputCtx.setMousePosition({150.f, 60.f});  // somewhere in first-row area
    inputCtx.setPrimaryDown(false);
    panel.handleInput(inputCtx); // prime prev-state

    // Scan down to find the first row that registers as a hit
    bool hit = false;
    for (float ty = 50.f; ty < 120.f && !hit; ty += 2.f) {
        BasicInputContext ic2;
        ic2.setMousePosition({150.f, ty});
        ic2.setPrimaryDown(false);
        panel.handleInput(ic2);

        ic2.setPrimaryDown(true);
        hit = panel.handleInput(ic2);
    }

    REQUIRE(hit);
    REQUIRE(panel.selectedIndex() >= 0);
    REQUIRE(panel.hasSelection());
}

TEST_CASE("ContentBrowserPanel handleInput: click outside entries returns false", "[Editor][U10]") {
    ContentBrowserPanel panel;
    panel.setVisible(true);
    panel.arrange({0.f, 0.f, 300.f, 400.f});
    panel.addEntry("Assets", true);

    BasicPaintContext paintCtx;
    panel.paint(paintCtx);

    // Click far below any entry
    BasicInputContext inputCtx;
    inputCtx.setMousePosition({150.f, 390.f});
    inputCtx.setPrimaryDown(false);
    panel.handleInput(inputCtx);

    inputCtx.setPrimaryDown(true);
    bool result = panel.handleInput(inputCtx);
    REQUIRE_FALSE(result);
    REQUIRE_FALSE(panel.hasSelection());
}

TEST_CASE("ContentBrowserPanel handleInput: not visible returns false", "[Editor][U10]") {
    ContentBrowserPanel panel;
    panel.setVisible(false);
    panel.arrange({0.f, 0.f, 300.f, 400.f});

    BasicInputContext inputCtx;
    inputCtx.setMousePosition({150.f, 100.f});
    inputCtx.setPrimaryDown(true);
    REQUIRE_FALSE(panel.handleInput(inputCtx));
}

// ── HierarchyPanel::handleInput ───────────────────────────────────

TEST_CASE("HierarchyPanel handleInput: click on entity row records clickedEntityId", "[Editor][U10]") {
    HierarchyPanel panel;
    panel.setVisible(true);
    panel.arrange({0.f, 0.f, 300.f, 400.f});
    panel.addEntity(42, "Player", false, 0);
    panel.addEntity(7,  "Camera", false, 0);

    BasicPaintContext paintCtx;
    panel.paint(paintCtx);

    // Scan for entity rows
    bool hit = false;
    for (float ty = 26.f; ty < 120.f && !hit; ty += 2.f) {
        BasicInputContext ic;
        ic.setMousePosition({150.f, ty});
        ic.setPrimaryDown(false);
        panel.handleInput(ic);

        ic.setPrimaryDown(true);
        hit = panel.handleInput(ic);
    }

    REQUIRE(hit);
    REQUIRE(panel.clickedEntityId() >= 0);
}

TEST_CASE("HierarchyPanel handleInput: clicking sets entity selected flag", "[Editor][U10]") {
    HierarchyPanel panel;
    panel.setVisible(true);
    panel.arrange({0.f, 0.f, 300.f, 400.f});
    panel.addEntity(1, "Alpha", false, 0);
    panel.addEntity(2, "Beta",  false, 0);

    BasicPaintContext paintCtx;
    panel.paint(paintCtx);

    bool hit = false;
    for (float ty = 26.f; ty < 120.f && !hit; ty += 2.f) {
        BasicInputContext ic;
        ic.setMousePosition({150.f, ty});
        ic.setPrimaryDown(false);
        panel.handleInput(ic);

        ic.setPrimaryDown(true);
        hit = panel.handleInput(ic);
    }

    REQUIRE(hit);
    // Exactly one entity should be selected
    size_t selectedCount = 0;
    for (const auto& e : panel.entities())
        if (e.selected) ++selectedCount;
    REQUIRE(selectedCount == 1);
}

TEST_CASE("HierarchyPanel handleInput: not visible returns false", "[Editor][U10]") {
    HierarchyPanel panel;
    panel.setVisible(false);
    panel.arrange({0.f, 0.f, 300.f, 400.f});

    BasicInputContext inputCtx;
    inputCtx.setMousePosition({150.f, 50.f});
    inputCtx.setPrimaryDown(true);
    REQUIRE_FALSE(panel.handleInput(inputCtx));
    REQUIRE(panel.clickedEntityId() == -1);
}
