#include <catch2/catch_test_macros.hpp>

#include "NF/UI/AtlasUI/DrawPrimitives.h"
#include "NF/UI/AtlasUI/Interfaces.h"
#include "NF/UI/AtlasUI/PanelHost.h"
#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/PanelBase.h"

#include "NF/UI/AtlasUI/Theme/Theme.h"
#include "NF/UI/AtlasUI/Theme/ThemeDefaults.h"
#include "NF/UI/AtlasUI/Theme/ThemeManager.h"
#include "NF/UI/AtlasUI/Theme/ThemeTokens.h"

#include "NF/UI/AtlasUI/Widgets/Button.h"
#include "NF/UI/AtlasUI/Widgets/Panel.h"
#include "NF/UI/AtlasUI/Widgets/TabBar.h"
#include "NF/UI/AtlasUI/Widgets/Splitter.h"
#include "NF/UI/AtlasUI/Widgets/Toolbar.h"
#include "NF/UI/AtlasUI/Widgets/Tooltip.h"
#include "NF/UI/AtlasUI/Widgets/TextInput.h"
#include "NF/UI/AtlasUI/Widgets/Dropdown.h"
#include "NF/UI/AtlasUI/Widgets/PropertyRow.h"
#include "NF/UI/AtlasUI/Widgets/NotificationCard.h"

#include "NF/UI/AtlasUI/Services/FocusService.h"
#include "NF/UI/AtlasUI/Services/PopupHost.h"
#include "NF/UI/AtlasUI/Services/TooltipService.h"
#include "NF/UI/AtlasUI/Services/NotificationHost.h"

#include "NF/UI/AtlasUI/Commands/CommandTypes.h"
#include "NF/UI/AtlasUI/Commands/CommandRegistry.h"
#include "NF/UI/AtlasUI/Commands/CommandManager.h"
#include "NF/UI/AtlasUI/Commands/ShortcutRouter.h"

using namespace NF::UI::AtlasUI;

// ── Concrete test panel (resolves diamond inheritance) ───────────

class TestPanel final : public IPanel {
public:
    TestPanel(std::string id, std::string title)
        : m_id(std::move(id)), m_title(std::move(title)) {}

    const char* panelId() const override { return m_id.c_str(); }
    const char* title() const override { return m_title.c_str(); }
    void initialize() override {}
    void activate() override {}
    void deactivate() override {}
    PanelState saveState() const override { return {m_id, m_title, m_visible, false, m_bounds}; }
    void loadState(const PanelState&) override {}

    void measure(ILayoutContext&) override {}
    void arrange(const NF::Rect& b) override { m_bounds = b; }
    void paint(IPaintContext&) override {}
    bool handleInput(IInputContext&) override { return false; }
    void setVisible(bool v) override { m_visible = v; }
    bool isVisible() const override { return m_visible; }
    NF::Rect bounds() const override { return m_bounds; }

private:
    std::string m_id, m_title;
    NF::Rect m_bounds{};
    bool m_visible = true;
};

// ── Test mock contexts ───────────────────────────────────────────

struct TestLayoutContext : ILayoutContext {
    float dpiScale() const override { return 1.0f; }
    NF::Vec2 availableSize() const override { return {800.f, 600.f}; }
    NF::Vec2 measureText(std::string_view text, float) const override {
        return {static_cast<float>(text.size()) * 8.f, 16.f};
    }
    void invalidateLayout() override {}
};

struct TestPaintContext : IPaintContext {
    DrawList dl;
    void drawRect(const NF::Rect& r, Color c) override { dl.push(DrawRectCmd{r, c}); }
    void fillRect(const NF::Rect& r, Color c) override { dl.push(FillRectCmd{r, c}); }
    void drawText(const NF::Rect& r, std::string_view t, FontId f, Color c) override {
        dl.push(DrawTextCmd{r, std::string(t), f, c});
    }
    void pushClip(const NF::Rect&) override {}
    void popClip() override {}
    DrawList& drawList() override { return dl; }
};

struct TestInputContext : IInputContext {
    NF::Vec2 pos{};
    bool primary = false;
    NF::Vec2 mousePosition() const override { return pos; }
    bool primaryDown() const override { return primary; }
    bool secondaryDown() const override { return false; }
    bool keyDown(int) const override { return false; }
    void requestFocus(IWidget*) override {}
    void capturePointer(IWidget*) override {}
    void releasePointer(IWidget*) override {}
};

// ── Theme Tests ──────────────────────────────────────────────────

TEST_CASE("ThemeManager singleton returns same instance", "[AtlasUI][Theme]") {
    auto& a = ThemeManager::Get();
    auto& b = ThemeManager::Get();
    REQUIRE(&a == &b);
}

TEST_CASE("ThemeManager::Current() returns valid theme", "[AtlasUI][Theme]") {
    const auto& theme = ThemeManager::Get().Current();
    REQUIRE(theme.Colors.TextPrimary.r > 0.f);
}

TEST_CASE("ThemeManager::SetTheme() changes current theme", "[AtlasUI][Theme]") {
    auto& mgr = ThemeManager::Get();
    auto original = mgr.Current();

    AtlasTheme custom{};
    custom.Colors.Accent = ColorRGBA{1.0f, 0.0f, 0.0f, 1.0f};
    mgr.SetTheme(custom);
    REQUIRE(mgr.Current().Colors.Accent.r == 1.0f);
    REQUIRE(mgr.Current().Colors.Accent.g == 0.0f);

    mgr.SetTheme(original);
}

TEST_CASE("Atlas Dark theme has non-zero color values for key tokens", "[AtlasUI][Theme]") {
    const auto theme = MakeAtlasDarkTheme();
    REQUIRE(theme.Colors.WindowBg.r > 0.f);
    REQUIRE(theme.Colors.PanelBg.r > 0.f);
    REQUIRE(theme.Colors.TextPrimary.r > 0.f);
    REQUIRE(theme.Colors.Accent.r > 0.f);
    REQUIRE(theme.Colors.Accent.b > 0.f);
    REQUIRE(theme.Colors.Error.r > 0.f);
    REQUIRE(theme.Colors.Warning.r > 0.f);
    REQUIRE(theme.Colors.Success.g > 0.f);
}

TEST_CASE("ThemeSpacing defaults are positive", "[AtlasUI][Theme]") {
    ThemeSpacing sp{};
    REQUIRE(sp.XXS > 0.f);
    REQUIRE(sp.XS > 0.f);
    REQUIRE(sp.SM > 0.f);
    REQUIRE(sp.MD > 0.f);
    REQUIRE(sp.LG > 0.f);
    REQUIRE(sp.XL > 0.f);
    REQUIRE(sp.XXL > 0.f);
    REQUIRE(sp.PanelPadding > 0.f);
}

TEST_CASE("ThemeSizes defaults are positive", "[AtlasUI][Theme]") {
    ThemeSizes sz{};
    REQUIRE(sz.TitleBarHeight > 0.f);
    REQUIRE(sz.TabHeight > 0.f);
    REQUIRE(sz.ToolbarHeight > 0.f);
    REQUIRE(sz.ButtonHeight > 0.f);
    REQUIRE(sz.InputHeight > 0.f);
    REQUIRE(sz.RowHeight > 0.f);
    REQUIRE(sz.IconSM > 0.f);
    REQUIRE(sz.Scrollbar > 0.f);
    REQUIRE(sz.Splitter > 0.f);
}

TEST_CASE("ThemeRadii has expected range values", "[AtlasUI][Theme]") {
    ThemeRadii radii{};
    REQUIRE(radii.None == 0.f);
    REQUIRE(radii.SM > 0.f);
    REQUIRE(radii.SM < radii.MD);
    REQUIRE(radii.MD < radii.XL);
    REQUIRE(radii.Button > 0.f);
    REQUIRE(radii.Panel > 0.f);
}

// ── Widget Tests ─────────────────────────────────────────────────

TEST_CASE("Button creates with text", "[AtlasUI][Widget]") {
    Button btn("Click me");
    REQUIRE(btn.isVisible());
}

TEST_CASE("Button measure produces non-zero size", "[AtlasUI][Widget]") {
    Button btn("Hello");
    TestLayoutContext ctx;
    btn.measure(ctx);
    REQUIRE(btn.desiredSize().x > 0.f);
    REQUIRE(btn.desiredSize().y > 0.f);
}

TEST_CASE("Button handleInput detects hover", "[AtlasUI][Widget]") {
    Button btn("Hover");
    btn.arrange({10.f, 10.f, 100.f, 30.f});
    TestInputContext input;
    input.pos = {50.f, 20.f};
    bool result = btn.handleInput(input);
    REQUIRE(result);
}

TEST_CASE("Panel creates with title", "[AtlasUI][Widget]") {
    TestPanel p("panel.test", "Test Panel");
    REQUIRE(std::string(p.panelId()) == "panel.test");
    REQUIRE(std::string(p.title()) == "Test Panel");
}

TEST_CASE("TabBar adds tabs", "[AtlasUI][Widget]") {
    TabBar tb;
    auto w1 = std::make_shared<WidgetBase>();
    auto w2 = std::make_shared<WidgetBase>();
    tb.addTab("Tab1", w1);
    tb.addTab("Tab2", w2);
    REQUIRE(tb.children().size() == 2);
    REQUIRE(tb.activeIndex() == 0);
}

TEST_CASE("Splitter default ratio is reasonable", "[AtlasUI][Widget]") {
    Splitter sp(SplitOrientation::Horizontal);
    REQUIRE(sp.ratio() >= 0.1f);
    REQUIRE(sp.ratio() <= 0.9f);
}

TEST_CASE("Toolbar adds items", "[AtlasUI][Widget]") {
    Toolbar tb;
    bool called = false;
    tb.addItem("Build", [&]() { called = true; });
    tb.addSeparator();
    tb.addItem("Run", nullptr);

    TestLayoutContext lctx;
    tb.measure(lctx);
    REQUIRE(tb.bounds().w > 0.f);
}

TEST_CASE("Tooltip creates with text", "[AtlasUI][Widget]") {
    Tooltip tt;
    tt.hide();
    REQUIRE_FALSE(tt.isShowing());
    tt.show("Help text", {100.f, 200.f});
    REQUIRE(tt.isShowing());
    tt.hide();
    REQUIRE_FALSE(tt.isShowing());
}

TEST_CASE("TextInput creates empty", "[AtlasUI][Widget]") {
    TextInput ti;
    REQUIRE(ti.text().empty());
}

TEST_CASE("TextInput accepts character input via setText", "[AtlasUI][Widget]") {
    TextInput ti;
    ti.setText("Hello");
    REQUIRE(ti.text() == "Hello");
}

TEST_CASE("Dropdown adds options", "[AtlasUI][Widget]") {
    Dropdown dd({"Alpha", "Beta", "Gamma"});
    REQUIRE(dd.selectedIndex() == 0);
    REQUIRE(dd.selectedText() == "Alpha");
    dd.setSelectedIndex(2);
    REQUIRE(dd.selectedText() == "Gamma");
}

TEST_CASE("PropertyRow creates with label", "[AtlasUI][Widget]") {
    auto input = std::make_shared<TextInput>();
    PropertyRow row("Color", input);
    REQUIRE(row.label() == "Color");
}

TEST_CASE("NotificationCard creates with severity", "[AtlasUI][Widget]") {
    NotificationCard card("Build succeeded", NotificationLevel::Info);
    REQUIRE_FALSE(card.isExpired());
    card.dismiss();
    REQUIRE(card.isExpired());
}

// ── Service Tests ────────────────────────────────────────────────

TEST_CASE("FocusService sets and clears focus", "[AtlasUI][Service]") {
    auto& fs = FocusService::Get();
    WidgetBase w;
    fs.setFocus(&w);
    REQUIRE(fs.focusedWidget() == &w);
    fs.clearFocus();
    REQUIRE(fs.focusedWidget() == nullptr);
}

TEST_CASE("FocusService reports HasFocus correctly", "[AtlasUI][Service]") {
    auto& fs = FocusService::Get();
    WidgetBase a, b;
    fs.setFocus(&a);
    REQUIRE(fs.hasFocus(&a));
    REQUIRE_FALSE(fs.hasFocus(&b));
    fs.clearFocus();
}

TEST_CASE("PopupHost open/close popup", "[AtlasUI][Service]") {
    auto& ph = PopupHost::Get();
    ph.closePopup();
    REQUIRE_FALSE(ph.isPopupOpen());
    auto widget = std::make_shared<WidgetBase>();
    ph.openPopup({10.f, 10.f, 200.f, 100.f}, widget);
    REQUIRE(ph.isPopupOpen());
    ph.closePopup();
    REQUIRE_FALSE(ph.isPopupOpen());
}

TEST_CASE("TooltipService show/hide tooltip", "[AtlasUI][Service]") {
    auto& ts = TooltipService::Get();
    ts.hide();
    REQUIRE_FALSE(ts.isShowing());
    ts.show("Helpful tip", {50.f, 50.f});
    REQUIRE(ts.isShowing());
    ts.hide();
    REQUIRE_FALSE(ts.isShowing());
}

TEST_CASE("NotificationHost push/dismiss notification", "[AtlasUI][Service]") {
    auto& nh = NotificationHost::Get();
    nh.dismissAll();
    REQUIRE(nh.count() == 0);

    nh.post("Build finished");
    REQUIRE(nh.count() == 1);
    nh.post("Warning: deprecated API", NotificationLevel::Warning);
    REQUIRE(nh.count() == 2);

    nh.dismissAll();
    REQUIRE(nh.count() == 0);
}

// ── Command Tests ────────────────────────────────────────────────

TEST_CASE("CommandRegistry register and find", "[AtlasUI][Command]") {
    CommandRegistry reg;
    bool executed = false;
    CommandSpec spec;
    spec.Id = "test.save";
    spec.Label = "Save";
    spec.Scope = CommandScope::Global;
    spec.Binding.Primary = {true, false, false, false, 'S'};
    spec.Execute = [&](const CommandContext&) { executed = true; };

    REQUIRE(reg.Register(std::move(spec)));
    const auto* found = reg.Find("test.save");
    REQUIRE(found != nullptr);
    REQUIRE(found->Label == "Save");

    found->Execute(CommandContext{});
    REQUIRE(executed);

    REQUIRE(reg.Find("nonexistent") == nullptr);
}

TEST_CASE("CommandRegistry unregister", "[AtlasUI][Command]") {
    CommandRegistry reg;
    CommandSpec spec;
    spec.Id = "test.remove";
    spec.Label = "Remove";
    spec.Execute = [](const CommandContext&) {};

    REQUIRE(reg.Register(std::move(spec)));
    REQUIRE(reg.Find("test.remove") != nullptr);
    REQUIRE(reg.Unregister("test.remove"));
    REQUIRE(reg.Find("test.remove") == nullptr);
    REQUIRE_FALSE(reg.Unregister("test.remove"));
}

TEST_CASE("CommandManager singleton", "[AtlasUI][Command]") {
    auto& a = CommandManager::Get();
    auto& b = CommandManager::Get();
    REQUIRE(&a == &b);
}

TEST_CASE("ShortcutRouter routes to correct command", "[AtlasUI][Command]") {
    CommandRegistry reg;
    bool saveCalled = false;
    bool openCalled = false;

    CommandSpec saveSpec;
    saveSpec.Id = "router.save";
    saveSpec.Label = "Save";
    saveSpec.Scope = CommandScope::Global;
    saveSpec.Binding.Primary = {true, false, false, false, 'S'};
    saveSpec.Execute = [&](const CommandContext&) { saveCalled = true; };
    reg.Register(std::move(saveSpec));

    CommandSpec openSpec;
    openSpec.Id = "router.open";
    openSpec.Label = "Open";
    openSpec.Scope = CommandScope::Global;
    openSpec.Binding.Primary = {true, false, false, false, 'O'};
    openSpec.Execute = [&](const CommandContext&) { openCalled = true; };
    reg.Register(std::move(openSpec));

    ShortcutRouter router(reg);
    CommandContext ctx;
    ctx.Scope = CommandScope::Global;

    REQUIRE(router.TryExecute({true, false, false, false, 'S'}, ctx));
    REQUIRE(saveCalled);
    REQUIRE_FALSE(openCalled);

    REQUIRE(router.TryExecute({true, false, false, false, 'O'}, ctx));
    REQUIRE(openCalled);

    REQUIRE_FALSE(router.TryExecute({true, false, false, false, 'X'}, ctx));
}

TEST_CASE("CommandTypes KeyChord creation and comparison", "[AtlasUI][Command]") {
    KeyChord a{true, false, true, false, 'Z'};
    KeyChord b{true, false, true, false, 'Z'};
    KeyChord c{false, false, true, false, 'Z'};

    REQUIRE(a == b);
    REQUIRE_FALSE(a == c);

    REQUIRE(a.IsValid());
    KeyChord empty{};
    REQUIRE_FALSE(empty.IsValid());

    REQUIRE(a.ToString() == "Ctrl+Shift+Z");
}

// ── PanelHost Tests ──────────────────────────────────────────────

TEST_CASE("PanelHost attach/detach", "[AtlasUI][PanelHost]") {
    PanelHost host;
    auto p1 = std::make_shared<TestPanel>("p1", "First");
    auto p2 = std::make_shared<TestPanel>("p2", "Second");

    host.attachPanel(p1);
    host.attachPanel(p2);
    REQUIRE(host.panels().size() == 2);

    host.detachPanel("p1");
    REQUIRE(host.panels().size() == 1);
    REQUIRE(host.findPanel("p1") == nullptr);
}

TEST_CASE("PanelHost find panel", "[AtlasUI][PanelHost]") {
    PanelHost host;
    auto p = std::make_shared<TestPanel>("viewport", "Viewport");
    host.attachPanel(p);

    auto found = host.findPanel("viewport");
    REQUIRE(found != nullptr);
    REQUIRE(std::string(found->title()) == "Viewport");

    REQUIRE(host.findPanel("missing") == nullptr);
}

TEST_CASE("PanelHost no duplicate attachment", "[AtlasUI][PanelHost]") {
    PanelHost host;
    auto p = std::make_shared<TestPanel>("dup", "Duplicate");
    host.attachPanel(p);
    host.attachPanel(p);
    REQUIRE(host.panels().size() == 1);
}

// ── DrawList Tests ───────────────────────────────────────────────

TEST_CASE("DrawList starts empty", "[AtlasUI][DrawList]") {
    DrawList dl;
    REQUIRE(dl.empty());
    REQUIRE(dl.size() == 0);
}

TEST_CASE("DrawList push commands", "[AtlasUI][DrawList]") {
    DrawList dl;
    dl.push(FillRectCmd{{0.f, 0.f, 100.f, 50.f}, 0xFF0000FF});
    dl.push(DrawRectCmd{{10.f, 10.f, 80.f, 30.f}, 0xFFFFFFFF});
    dl.push(DrawTextCmd{{20.f, 20.f, 60.f, 16.f}, "Hello", 0, 0xFFFFFFFF});
    REQUIRE(dl.size() == 3);
    REQUIRE_FALSE(dl.empty());
}

TEST_CASE("DrawList clear", "[AtlasUI][DrawList]") {
    DrawList dl;
    dl.push(FillRectCmd{{0.f, 0.f, 100.f, 50.f}, 0xFF0000FF});
    dl.push(DrawRectCmd{{0.f, 0.f, 50.f, 50.f}, 0xFFFFFFFF});
    REQUIRE(dl.size() == 2);
    dl.clear();
    REQUIRE(dl.empty());
    REQUIRE(dl.size() == 0);
}
