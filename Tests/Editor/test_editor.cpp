#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include <filesystem>

// ── SelectionService ─────────────────────────────────────────────

TEST_CASE("SelectionService single select", "[Editor][Selection]") {
    NF::SelectionService sel;

    REQUIRE_FALSE(sel.hasSelection());
    REQUIRE(sel.selectionCount() == 0);

    sel.select(1);
    REQUIRE(sel.hasSelection());
    REQUIRE(sel.isSelected(1));
    REQUIRE(sel.primarySelection() == 1);
    REQUIRE(sel.selectionCount() == 1);
}

TEST_CASE("SelectionService multi-select", "[Editor][Selection]") {
    NF::SelectionService sel;

    sel.select(1);
    sel.select(2);
    sel.select(3);

    REQUIRE(sel.selectionCount() == 3);
    REQUIRE(sel.isSelected(1));
    REQUIRE(sel.isSelected(2));
    REQUIRE(sel.isSelected(3));
    REQUIRE(sel.primarySelection() == 3);  // last selected is primary
}

TEST_CASE("SelectionService deselect", "[Editor][Selection]") {
    NF::SelectionService sel;

    sel.select(1);
    sel.select(2);
    sel.deselect(1);

    REQUIRE_FALSE(sel.isSelected(1));
    REQUIRE(sel.isSelected(2));
    REQUIRE(sel.selectionCount() == 1);
}

TEST_CASE("SelectionService clearSelection", "[Editor][Selection]") {
    NF::SelectionService sel;

    sel.select(1);
    sel.select(2);
    sel.clearSelection();

    REQUIRE_FALSE(sel.hasSelection());
    REQUIRE(sel.selectionCount() == 0);
    REQUIRE(sel.primarySelection() == NF::INVALID_ENTITY);
}

TEST_CASE("SelectionService selectExclusive", "[Editor][Selection]") {
    NF::SelectionService sel;

    sel.select(1);
    sel.select(2);
    sel.selectExclusive(5);

    REQUIRE(sel.selectionCount() == 1);
    REQUIRE(sel.isSelected(5));
    REQUIRE_FALSE(sel.isSelected(1));
    REQUIRE_FALSE(sel.isSelected(2));
    REQUIRE(sel.primarySelection() == 5);
}

TEST_CASE("SelectionService toggleSelect", "[Editor][Selection]") {
    NF::SelectionService sel;

    sel.toggleSelect(1);
    REQUIRE(sel.isSelected(1));

    sel.toggleSelect(1);
    REQUIRE_FALSE(sel.isSelected(1));
}

TEST_CASE("SelectionService version increments on changes", "[Editor][Selection]") {
    NF::SelectionService sel;
    uint32_t v0 = sel.version();

    sel.select(1);
    REQUIRE(sel.version() > v0);

    uint32_t v1 = sel.version();
    sel.deselect(1);
    REQUIRE(sel.version() > v1);
}

TEST_CASE("SelectionService deselect primary reassigns", "[Editor][Selection]") {
    NF::SelectionService sel;

    sel.select(1);
    sel.select(2);
    sel.deselect(2);  // primary was 2

    // Primary should fall back to remaining selection
    REQUIRE(sel.primarySelection() == 1);
}

// ── EditorCommandRegistry ────────────────────────────────────────

TEST_CASE("EditorCommandRegistry register and execute", "[Editor][Commands]") {
    NF::EditorCommandRegistry reg;
    int counter = 0;

    reg.registerCommand("test.inc", [&]() { counter++; }, "Increment", "Ctrl+I");
    REQUIRE(reg.executeCommand("test.inc"));
    REQUIRE(counter == 1);

    REQUIRE_FALSE(reg.executeCommand("nonexistent"));
}

TEST_CASE("EditorCommandRegistry enabled check", "[Editor][Commands]") {
    NF::EditorCommandRegistry reg;
    bool canRun = false;
    int counter = 0;

    reg.registerCommand("test.guarded", [&]() { counter++; }, "Guarded", "");
    reg.setEnabledCheck("test.guarded", [&]() { return canRun; });

    REQUIRE_FALSE(reg.isCommandEnabled("test.guarded"));
    REQUIRE_FALSE(reg.executeCommand("test.guarded"));
    REQUIRE(counter == 0);

    canRun = true;
    REQUIRE(reg.isCommandEnabled("test.guarded"));
    REQUIRE(reg.executeCommand("test.guarded"));
    REQUIRE(counter == 1);
}

TEST_CASE("EditorCommandRegistry allCommandNames", "[Editor][Commands]") {
    NF::EditorCommandRegistry reg;
    reg.registerCommand("b.cmd", []() {});
    reg.registerCommand("a.cmd", []() {});

    auto names = reg.allCommandNames();
    REQUIRE(names.size() == 2);
    REQUIRE(names[0] == "a.cmd");  // sorted
    REQUIRE(names[1] == "b.cmd");
}

TEST_CASE("EditorCommandRegistry findCommand", "[Editor][Commands]") {
    NF::EditorCommandRegistry reg;
    reg.registerCommand("file.save", []() {}, "Save", "Ctrl+S");

    auto* info = reg.findCommand("file.save");
    REQUIRE(info != nullptr);
    REQUIRE(info->displayName == "Save");
    REQUIRE(info->hotkey == "Ctrl+S");

    REQUIRE(reg.findCommand("nonexistent") == nullptr);
}

// ── RecentFilesList ──────────────────────────────────────────────

TEST_CASE("RecentFilesList add and retrieve", "[Editor][RecentFiles]") {
    NF::RecentFilesList recent;

    recent.addFile("/path/to/world1.json");
    recent.addFile("/path/to/world2.json");

    REQUIRE(recent.count() == 2);
    REQUIRE(recent.files()[0] == "/path/to/world2.json");  // most recent first
    REQUIRE(recent.files()[1] == "/path/to/world1.json");
}

TEST_CASE("RecentFilesList deduplicates", "[Editor][RecentFiles]") {
    NF::RecentFilesList recent;

    recent.addFile("/path/to/world1.json");
    recent.addFile("/path/to/world2.json");
    recent.addFile("/path/to/world1.json");  // re-add

    REQUIRE(recent.count() == 2);
    REQUIRE(recent.files()[0] == "/path/to/world1.json");  // moved to front
}

TEST_CASE("RecentFilesList max entries", "[Editor][RecentFiles]") {
    NF::RecentFilesList recent;
    recent.setMaxEntries(3);

    recent.addFile("a");
    recent.addFile("b");
    recent.addFile("c");
    recent.addFile("d");  // should evict "a"

    REQUIRE(recent.count() == 3);
    REQUIRE(recent.files()[0] == "d");
    REQUIRE(recent.files()[2] == "b");
}

// ── ContentBrowser ───────────────────────────────────────────────

TEST_CASE("ContentBrowser classifyExtension", "[Editor][ContentBrowser]") {
    REQUIRE(NF::ContentBrowser::classifyExtension(".json") == NF::ContentEntryType::Data);
    REQUIRE(NF::ContentBrowser::classifyExtension(".scene") == NF::ContentEntryType::Scene);
    REQUIRE(NF::ContentBrowser::classifyExtension(".png") == NF::ContentEntryType::Texture);
    REQUIRE(NF::ContentBrowser::classifyExtension(".wav") == NF::ContentEntryType::Audio);
    REQUIRE(NF::ContentBrowser::classifyExtension(".lua") == NF::ContentEntryType::Script);
    REQUIRE(NF::ContentBrowser::classifyExtension(".obj") == NF::ContentEntryType::Mesh);
    REQUIRE(NF::ContentBrowser::classifyExtension(".xyz") == NF::ContentEntryType::Unknown);
}

TEST_CASE("ContentBrowser navigates real directory", "[Editor][ContentBrowser]") {
    // Use /tmp as a guaranteed-existing directory for testing
    NF::ContentBrowser browser;
    browser.setRootPath("/tmp");
    browser.refresh();

    REQUIRE(browser.isAtRoot());
    // /tmp should always exist and be navigable
    REQUIRE(browser.rootPath() == "/tmp");
}

TEST_CASE("ContentBrowser navigateUp at root returns false", "[Editor][ContentBrowser]") {
    NF::ContentBrowser browser;
    browser.setRootPath("/tmp");
    REQUIRE_FALSE(browser.navigateUp());
}

// ── LaunchService ────────────────────────────────────────────────

TEST_CASE("LaunchService validate nonexistent exe", "[Editor][Launch]") {
    NF::LaunchService launch;
    launch.setGameExecutableName("nonexistent_binary_xyz");

    auto result = launch.validateLaunch();
    REQUIRE_FALSE(result.success);
    REQUIRE_FALSE(result.errorMessage.empty());
}

TEST_CASE("LaunchService resolve with build directory", "[Editor][Launch]") {
    NF::LaunchService launch;
    launch.setGameExecutableName("NovaForgeGame");
    launch.setBuildDirectory("./Builds/debug");

    // May or may not find it depending on build state, but should not crash
    auto path = launch.resolveGamePath();
    // path might be empty if not built, that's OK
    (void)path;
}

// ── EditorTheme ──────────────────────────────────────────────────

TEST_CASE("EditorTheme dark defaults", "[Editor][Theme]") {
    NF::EditorTheme theme = NF::EditorTheme::dark();
    REQUIRE(theme.panelBackground == 0x2B2B2BFF);
    REQUIRE(theme.panelText == 0xDCDCDCFF);
    REQUIRE(theme.fontSize == 14.f);
    REQUIRE(theme.panelPadding == 8.f);
}

TEST_CASE("EditorTheme light variant", "[Editor][Theme]") {
    NF::EditorTheme theme = NF::EditorTheme::light();
    // Light theme should have bright background, dark text
    REQUIRE(theme.panelBackground == 0xF0F0F0FF);
    REQUIRE(theme.panelText == 0x1E1E1EFF);
}

// ── PropertyEditor ───────────────────────────────────────────────

struct TestComponent {
    bool active = true;
    int32_t health = 100;
    float speed = 5.5f;
    NF::Vec3 position{1.f, 2.f, 3.f};
    NF::Color tint = NF::Color::red();
    std::string name = "TestEntity";
};

TEST_CASE("PropertyEditor read bool property", "[Editor][PropertyEditor]") {
    TestComponent comp;
    NF::PropertyInfo prop{"active", NF::PropertyType::Bool, offsetof(TestComponent, active), sizeof(bool)};

    auto val = NF::PropertyEditor::readProperty(&comp, prop);
    REQUIRE(val.isBool());
    REQUIRE(val.asBool() == true);
}

TEST_CASE("PropertyEditor read/write int property", "[Editor][PropertyEditor]") {
    TestComponent comp;
    NF::PropertyInfo prop{"health", NF::PropertyType::Int32, offsetof(TestComponent, health), sizeof(int32_t)};

    auto val = NF::PropertyEditor::readProperty(&comp, prop);
    REQUIRE(val.isInt());
    REQUIRE(val.asInt() == 100);

    REQUIRE(NF::PropertyEditor::writeProperty(&comp, prop, NF::JsonValue(42)));
    REQUIRE(comp.health == 42);
}

TEST_CASE("PropertyEditor read/write float property", "[Editor][PropertyEditor]") {
    TestComponent comp;
    NF::PropertyInfo prop{"speed", NF::PropertyType::Float, offsetof(TestComponent, speed), sizeof(float)};

    auto val = NF::PropertyEditor::readProperty(&comp, prop);
    REQUIRE(val.isFloat());

    REQUIRE(NF::PropertyEditor::writeProperty(&comp, prop, NF::JsonValue(10.f)));
    REQUIRE(comp.speed == 10.f);
}

TEST_CASE("PropertyEditor read/write Vec3 property", "[Editor][PropertyEditor]") {
    TestComponent comp;
    NF::PropertyInfo prop{"position", NF::PropertyType::Vec3, offsetof(TestComponent, position), sizeof(NF::Vec3)};

    auto val = NF::PropertyEditor::readProperty(&comp, prop);
    REQUIRE(val.isArray());
    REQUIRE(val.size() == 3);
    REQUIRE(val[static_cast<size_t>(0)].asFloat() == 1.f);
    REQUIRE(val[static_cast<size_t>(1)].asFloat() == 2.f);
    REQUIRE(val[static_cast<size_t>(2)].asFloat() == 3.f);

    auto newVal = NF::JsonValue::array();
    newVal.push(NF::JsonValue(10.f));
    newVal.push(NF::JsonValue(20.f));
    newVal.push(NF::JsonValue(30.f));
    REQUIRE(NF::PropertyEditor::writeProperty(&comp, prop, newVal));
    REQUIRE(comp.position.x == 10.f);
    REQUIRE(comp.position.y == 20.f);
    REQUIRE(comp.position.z == 30.f);
}

TEST_CASE("PropertyEditor read/write Color property", "[Editor][PropertyEditor]") {
    TestComponent comp;
    NF::PropertyInfo prop{"tint", NF::PropertyType::Color, offsetof(TestComponent, tint), sizeof(NF::Color)};

    auto val = NF::PropertyEditor::readProperty(&comp, prop);
    REQUIRE(val.isArray());
    REQUIRE(val.size() == 4);

    auto newVal = NF::JsonValue::array();
    newVal.push(NF::JsonValue(0.f));
    newVal.push(NF::JsonValue(1.f));
    newVal.push(NF::JsonValue(0.f));
    newVal.push(NF::JsonValue(0.5f));
    REQUIRE(NF::PropertyEditor::writeProperty(&comp, prop, newVal));
    REQUIRE(comp.tint.g == 1.f);
    REQUIRE(comp.tint.a == 0.5f);
}

TEST_CASE("PropertyEditor read/write string property", "[Editor][PropertyEditor]") {
    TestComponent comp;
    NF::PropertyInfo prop{"name", NF::PropertyType::String, offsetof(TestComponent, name), sizeof(std::string)};

    auto val = NF::PropertyEditor::readProperty(&comp, prop);
    REQUIRE(val.isString());
    REQUIRE(val.asString() == "TestEntity");

    REQUIRE(NF::PropertyEditor::writeProperty(&comp, prop, NF::JsonValue("NewName")));
    REQUIRE(comp.name == "NewName");
}

TEST_CASE("PropertyEditor write rejects wrong type", "[Editor][PropertyEditor]") {
    TestComponent comp;
    NF::PropertyInfo prop{"health", NF::PropertyType::Int32, offsetof(TestComponent, health), sizeof(int32_t)};

    // Trying to write a string to an int field should fail
    REQUIRE_FALSE(NF::PropertyEditor::writeProperty(&comp, prop, NF::JsonValue("not a number")));
    REQUIRE(comp.health == 100); // unchanged
}

TEST_CASE("PropertyEditor undo-safe float change", "[Editor][PropertyEditor]") {
    float value = 5.f;
    NF::CommandStack stack;

    auto cmd = NF::PropertyEditor::makeFloatChange(&value, 10.f, "speed");
    stack.execute(std::move(cmd));
    REQUIRE(value == 10.f);

    stack.undo();
    REQUIRE(value == 5.f);

    stack.redo();
    REQUIRE(value == 10.f);
}

TEST_CASE("PropertyEditor undo-safe Vec3 change", "[Editor][PropertyEditor]") {
    NF::Vec3 pos{1.f, 2.f, 3.f};
    NF::CommandStack stack;

    auto cmd = NF::PropertyEditor::makeVec3Change(&pos, {10.f, 20.f, 30.f}, "position");
    stack.execute(std::move(cmd));
    REQUIRE(pos.x == 10.f);
    REQUIRE(pos.y == 20.f);

    stack.undo();
    REQUIRE(pos.x == 1.f);
    REQUIRE(pos.y == 2.f);
}

TEST_CASE("PropertyEditor undo-safe Color change", "[Editor][PropertyEditor]") {
    NF::Color col = NF::Color::red();
    NF::CommandStack stack;

    auto cmd = NF::PropertyEditor::makeColorChange(&col, NF::Color::blue(), "tint");
    stack.execute(std::move(cmd));
    REQUIRE(col.b == 1.f);
    REQUIRE(col.r == 0.f);

    stack.undo();
    REQUIRE(col.r == 1.f);
    REQUIRE(col.b == 0.f);
}

TEST_CASE("PropertyEditor undo-safe string change", "[Editor][PropertyEditor]") {
    std::string name = "OldName";
    NF::CommandStack stack;

    auto cmd = NF::PropertyEditor::makeStringChange(&name, "NewName", "name");
    stack.execute(std::move(cmd));
    REQUIRE(name == "NewName");

    stack.undo();
    REQUIRE(name == "OldName");
}

// ── DockLayout ───────────────────────────────────────────────────

TEST_CASE("DockLayout add and find panel", "[Editor][DockLayout]") {
    NF::DockLayout layout;
    layout.addPanel("Viewport", NF::DockSlot::Center);
    layout.addPanel("Inspector", NF::DockSlot::Right);

    REQUIRE(layout.panelCount() == 2);
    REQUIRE(layout.findPanel("Viewport") != nullptr);
    REQUIRE(layout.findPanel("Viewport")->slot == NF::DockSlot::Center);
    REQUIRE(layout.findPanel("Inspector") != nullptr);
    REQUIRE(layout.findPanel("Missing") == nullptr);
}

TEST_CASE("DockLayout remove panel", "[Editor][DockLayout]") {
    NF::DockLayout layout;
    layout.addPanel("A", NF::DockSlot::Left);
    layout.addPanel("B", NF::DockSlot::Right);
    layout.removePanel("A");

    REQUIRE(layout.panelCount() == 1);
    REQUIRE(layout.findPanel("A") == nullptr);
    REQUIRE(layout.findPanel("B") != nullptr);
}

TEST_CASE("DockLayout visibility toggle", "[Editor][DockLayout]") {
    NF::DockLayout layout;
    layout.addPanel("Console", NF::DockSlot::Bottom);

    REQUIRE(layout.findPanel("Console")->visible == true);
    layout.setPanelVisible("Console", false);
    REQUIRE(layout.findPanel("Console")->visible == false);
    layout.setPanelVisible("Console", true);
    REQUIRE(layout.findPanel("Console")->visible == true);
}

TEST_CASE("DockLayout computeLayout positions", "[Editor][DockLayout]") {
    NF::DockLayout layout;
    layout.addPanel("Left", NF::DockSlot::Left);
    layout.addPanel("Right", NF::DockSlot::Right);
    layout.addPanel("Top", NF::DockSlot::Top);
    layout.addPanel("Bottom", NF::DockSlot::Bottom);
    layout.addPanel("Center", NF::DockSlot::Center);

    float w = 1920.f, h = 1080.f, tbH = 40.f, sbH = 30.f;
    layout.computeLayout(w, h, tbH, sbH);

    auto* left = layout.findPanel("Left");
    REQUIRE(left->bounds.x == 0.f);
    REQUIRE(left->bounds.y == tbH);
    REQUIRE(left->bounds.w == NF::DockLayout::kDefaultLeftWidth);

    auto* right = layout.findPanel("Right");
    REQUIRE(right->bounds.x == w - NF::DockLayout::kDefaultRightWidth);

    auto* center = layout.findPanel("Center");
    REQUIRE(center->bounds.x == NF::DockLayout::kDefaultLeftWidth);
    REQUIRE(center->bounds.w > 0.f);
}

TEST_CASE("DockLayout hidden panels excluded from layout", "[Editor][DockLayout]") {
    NF::DockLayout layout;
    layout.addPanel("Left", NF::DockSlot::Left);
    layout.addPanel("Center", NF::DockSlot::Center);
    layout.setPanelVisible("Left", false);

    layout.computeLayout(1920.f, 1080.f, 40.f, 30.f);

    // Center should start at x=0 since left is hidden
    auto* center = layout.findPanel("Center");
    REQUIRE(center->bounds.x == 0.f);
}

TEST_CASE("DockLayout panels accessor", "[Editor][DockLayout]") {
    NF::DockLayout layout;
    layout.addPanel("A", NF::DockSlot::Left);
    layout.addPanel("B", NF::DockSlot::Right);

    auto& panels = layout.panels();
    REQUIRE(panels.size() == 2);
    REQUIRE(panels[0].name == "A");
    REQUIRE(panels[1].name == "B");
}

// ── EditorPanel / ViewportPanel ──────────────────────────────────

TEST_CASE("ViewportPanel defaults", "[Editor][Panel]") {
    NF::ViewportPanel vp;
    REQUIRE(vp.name() == "Viewport");
    REQUIRE(vp.slot() == NF::DockSlot::Center);
    REQUIRE(vp.isVisible());
    REQUIRE(vp.gridEnabled());
    REQUIRE(vp.renderMode() == NF::RenderMode::Shaded);
    REQUIRE(vp.toolMode() == NF::ToolMode::Select);
    REQUIRE(vp.cameraZoom() == 1.f);
}

TEST_CASE("ViewportPanel setters", "[Editor][Panel]") {
    NF::ViewportPanel vp;
    vp.setCameraPosition({10.f, 20.f, 30.f});
    vp.setCameraZoom(2.5f);
    vp.setGridEnabled(false);
    vp.setRenderMode(NF::RenderMode::Wireframe);
    vp.setToolMode(NF::ToolMode::Rotate);
    vp.setVisible(false);

    REQUIRE(vp.cameraPosition().x == 10.f);
    REQUIRE(vp.cameraZoom() == 2.5f);
    REQUIRE_FALSE(vp.gridEnabled());
    REQUIRE(vp.renderMode() == NF::RenderMode::Wireframe);
    REQUIRE(vp.toolMode() == NF::ToolMode::Rotate);
    REQUIRE_FALSE(vp.isVisible());
}

// ── InspectorPanel ───────────────────────────────────────────────

TEST_CASE("InspectorPanel stores references", "[Editor][Panel]") {
    NF::SelectionService sel;
    NF::TypeRegistry& reg = NF::TypeRegistry::instance();
    NF::InspectorPanel panel(&sel, &reg);

    REQUIRE(panel.name() == "Inspector");
    REQUIRE(panel.slot() == NF::DockSlot::Right);
    REQUIRE(panel.selectionService() == &sel);
    REQUIRE(panel.typeRegistry() == &reg);
}

// ── HierarchyPanel ──────────────────────────────────────────────

TEST_CASE("HierarchyPanel search filter and entity list", "[Editor][Panel]") {
    NF::SelectionService sel;
    NF::HierarchyPanel panel(&sel);

    REQUIRE(panel.name() == "Hierarchy");
    REQUIRE(panel.slot() == NF::DockSlot::Left);
    REQUIRE(panel.selectionService() == &sel);
    REQUIRE(panel.searchFilter().empty());

    panel.setSearchFilter("Player");
    REQUIRE(panel.searchFilter() == "Player");

    panel.setEntityList({1, 2, 3});
    REQUIRE(panel.entityList().size() == 3);
    REQUIRE(panel.entityList()[0] == 1);
}

// ── ConsolePanel ─────────────────────────────────────────────────

TEST_CASE("ConsolePanel add messages", "[Editor][Panel]") {
    NF::ConsolePanel console;
    REQUIRE(console.name() == "Console");
    REQUIRE(console.slot() == NF::DockSlot::Bottom);
    REQUIRE(console.messageCount() == 0);

    console.addMessage("Hello", NF::ConsoleMessageLevel::Info, 1.0f);
    console.addMessage("Warn!", NF::ConsoleMessageLevel::Warning, 2.0f);

    REQUIRE(console.messageCount() == 2);
    REQUIRE(console.messages()[0].text == "Hello");
    REQUIRE(console.messages()[0].level == NF::ConsoleMessageLevel::Info);
    REQUIRE(console.messages()[1].level == NF::ConsoleMessageLevel::Warning);
}

TEST_CASE("ConsolePanel clear messages", "[Editor][Panel]") {
    NF::ConsolePanel console;
    console.addMessage("test", NF::ConsoleMessageLevel::Error, 0.f);
    console.clearMessages();
    REQUIRE(console.messageCount() == 0);
}

TEST_CASE("ConsolePanel max messages cap", "[Editor][Panel]") {
    NF::ConsolePanel console;
    for (size_t i = 0; i < NF::ConsolePanel::kMaxMessages + 50; ++i) {
        console.addMessage("msg" + std::to_string(i), NF::ConsoleMessageLevel::Info,
                           static_cast<float>(i));
    }
    REQUIRE(console.messageCount() == NF::ConsolePanel::kMaxMessages);
    // Oldest messages should have been evicted
    REQUIRE(console.messages()[0].text == "msg50");
}

// ── ContentBrowserPanel ──────────────────────────────────────────

TEST_CASE("ContentBrowserPanel wraps ContentBrowser", "[Editor][Panel]") {
    NF::ContentBrowser browser;
    NF::ContentBrowserPanel panel(&browser);

    REQUIRE(panel.name() == "ContentBrowser");
    REQUIRE(panel.slot() == NF::DockSlot::Left);
    REQUIRE(panel.contentBrowser() == &browser);
    REQUIRE(panel.viewMode() == NF::ContentViewMode::Grid);

    panel.setViewMode(NF::ContentViewMode::List);
    REQUIRE(panel.viewMode() == NF::ContentViewMode::List);
}

// ── EditorToolbar ────────────────────────────────────────────────

TEST_CASE("EditorToolbar add items and separators", "[Editor][Toolbar]") {
    NF::EditorToolbar toolbar;
    REQUIRE(toolbar.itemCount() == 0);

    toolbar.addItem("Select", "select_icon", "Select tool", []() {});
    toolbar.addItem("Move", "move_icon", "Move tool", []() {});
    toolbar.addSeparator();
    toolbar.addItem("Play", "play_icon", "Play", []() {});

    REQUIRE(toolbar.itemCount() == 4);
    REQUIRE(toolbar.items()[0].name == "Select");
    REQUIRE(toolbar.items()[0].tooltip == "Select tool");
    REQUIRE(toolbar.items()[0].enabled == true);
    REQUIRE_FALSE(toolbar.items()[0].isSeparator);
    REQUIRE(toolbar.items()[2].isSeparator);
    REQUIRE(toolbar.items()[3].name == "Play");
}

// ── EditorApp expanded ───────────────────────────────────────────

TEST_CASE("EditorApp creates default panels on init", "[Editor][EditorApp]") {
    NF::EditorApp app;
    app.init(1280, 720);

    REQUIRE(app.dockLayout().panelCount() == 5);
    REQUIRE(app.dockLayout().findPanel("Viewport") != nullptr);
    REQUIRE(app.dockLayout().findPanel("Inspector") != nullptr);
    REQUIRE(app.dockLayout().findPanel("Hierarchy") != nullptr);
    REQUIRE(app.dockLayout().findPanel("Console") != nullptr);
    REQUIRE(app.dockLayout().findPanel("ContentBrowser") != nullptr);

    REQUIRE(app.editorPanels().size() == 5);
    app.shutdown();
}

TEST_CASE("EditorApp creates default toolbar items", "[Editor][EditorApp]") {
    NF::EditorApp app;
    app.init(1280, 720);

    // 4 tools + 1 separator + 3 play controls = 8
    REQUIRE(app.toolbar().itemCount() == 8);
    REQUIRE(app.toolbar().items()[0].name == "Select");
    REQUIRE(app.toolbar().items()[4].isSeparator);
    REQUIRE(app.toolbar().items()[5].name == "Play");
    app.shutdown();
}

TEST_CASE("EditorApp view toggle commands", "[Editor][EditorApp]") {
    NF::EditorApp app;
    app.init(1280, 720);

    // Inspector should be visible by default
    REQUIRE(app.dockLayout().findPanel("Inspector")->visible == true);

    // Toggle it off
    app.commands().executeCommand("view.toggle_inspector");
    REQUIRE(app.dockLayout().findPanel("Inspector")->visible == false);

    // Toggle it back on
    app.commands().executeCommand("view.toggle_inspector");
    REQUIRE(app.dockLayout().findPanel("Inspector")->visible == true);

    // Also check other toggle commands exist
    REQUIRE(app.commands().findCommand("view.toggle_hierarchy") != nullptr);
    REQUIRE(app.commands().findCommand("view.toggle_console") != nullptr);
    REQUIRE(app.commands().findCommand("view.toggle_content_browser") != nullptr);

    app.shutdown();
}

TEST_CASE("EditorApp graph commands registered", "[Editor][EditorApp]") {
    NF::EditorApp app;
    app.init(1280, 720);

    REQUIRE(app.commands().findCommand("graph.new_graph") != nullptr);
    REQUIRE(app.commands().findCommand("graph.open_graph") != nullptr);
    REQUIRE(app.commands().executeCommand("graph.new_graph"));
    REQUIRE(app.commands().executeCommand("graph.open_graph"));
    app.shutdown();
}

TEST_CASE("EditorApp graphVM accessor", "[Editor][EditorApp]") {
    NF::EditorApp app;
    app.init(1280, 720);

    REQUIRE(app.graphVM() == nullptr);
    NF::GraphVM vm;
    app.setGraphVM(&vm);
    REQUIRE(app.graphVM() == &vm);
    app.shutdown();
}

TEST_CASE("EditorApp addPanel", "[Editor][EditorApp]") {
    NF::EditorApp app;
    app.init(1280, 720);

    size_t before = app.editorPanels().size();
    auto custom = std::make_unique<NF::ViewportPanel>();
    app.addPanel(std::move(custom));
    REQUIRE(app.editorPanels().size() == before + 1);
    app.shutdown();
}

// ── IDE Tests ────────────────────────────────────────────────────

TEST_CASE("ProjectIndexer index files", "[Editor][IDE]") {
    NF::ProjectIndexer indexer;
    indexer.indexFile("src/Core/Core.h", NF::SourceFileType::Header, "Core");
    indexer.indexFile("src/Core/Core.cpp", NF::SourceFileType::Source, "Core");
    indexer.indexFile("src/Render/Shader.glsl", NF::SourceFileType::Shader, "Render");
    indexer.indexFile("src/Scripts/main.lua", NF::SourceFileType::Script, "Scripts");
    indexer.indexFile("data/config.json", NF::SourceFileType::Data, "Data");

    REQUIRE(indexer.fileCount() == 5);

    auto headers = indexer.findFilesByType(NF::SourceFileType::Header);
    REQUIRE(headers.size() == 1);
    REQUIRE(headers[0]->path == "src/Core/Core.h");

    auto coreFiles = indexer.findFilesByModule("Core");
    REQUIRE(coreFiles.size() == 2);

    auto found = indexer.findFilesByName("Shader");
    REQUIRE(found.size() == 1);
    REQUIRE(found[0]->path == "src/Render/Shader.glsl");

    indexer.clear();
    REQUIRE(indexer.fileCount() == 0);
}

TEST_CASE("ProjectIndexer add and find symbols", "[Editor][IDE]") {
    NF::ProjectIndexer indexer;
    indexer.indexFile("src/Engine.h", NF::SourceFileType::Header, "Engine");
    indexer.indexFile("src/Render.h", NF::SourceFileType::Header, "Render");

    indexer.addSymbol("src/Engine.h", "EngineInit");
    indexer.addSymbol("src/Engine.h", "EngineShutdown");
    indexer.addSymbol("src/Render.h", "RenderFrame");

    auto results = indexer.findSymbol("EngineInit");
    REQUIRE(results.size() == 1);
    REQUIRE(results[0]->path == "src/Engine.h");

    auto none = indexer.findSymbol("NonExistent");
    REQUIRE(none.empty());
}

TEST_CASE("CodeNavigator go to definition", "[Editor][IDE]") {
    NF::CodeNavigator nav;
    nav.addEntry({"myFunc", NF::SymbolKind::Function, "src/main.cpp", 42});
    nav.addEntry({"MyClass", NF::SymbolKind::Class, "src/types.h", 10});

    auto target = nav.goToDefinition("myFunc");
    REQUIRE(target.has_value());
    REQUIRE(target->filePath == "src/main.cpp");
    REQUIRE(target->line == 42);
    REQUIRE(target->symbolName == "myFunc");
    REQUIRE(target->kind == NF::SymbolKind::Function);

    auto missing = nav.goToDefinition("doesNotExist");
    REQUIRE_FALSE(missing.has_value());
}

TEST_CASE("CodeNavigator find references", "[Editor][IDE]") {
    NF::CodeNavigator nav;
    nav.addEntry({"update", NF::SymbolKind::Function, "src/a.cpp", 10});
    nav.addEntry({"update", NF::SymbolKind::Function, "src/b.cpp", 20});
    nav.addEntry({"update", NF::SymbolKind::Function, "src/c.cpp", 30});
    nav.addEntry({"render", NF::SymbolKind::Function, "src/d.cpp", 40});

    auto refs = nav.findReferences("update");
    REQUIRE(refs.size() == 3);
}

TEST_CASE("CodeNavigator search symbols", "[Editor][IDE]") {
    NF::CodeNavigator nav;
    nav.addEntry({"initEngine", NF::SymbolKind::Function, "src/a.cpp", 1});
    nav.addEntry({"initRenderer", NF::SymbolKind::Function, "src/b.cpp", 2});
    nav.addEntry({"shutdown", NF::SymbolKind::Function, "src/c.cpp", 3});

    auto results = nav.searchSymbols("init");
    REQUIRE(results.size() == 2);

    auto all = nav.searchSymbols("i");
    REQUIRE(all.size() == 2);  // initEngine, initRenderer contain 'i'

    auto none = nav.searchSymbols("xyz");
    REQUIRE(none.empty());
}

TEST_CASE("CodeNavigator find by kind", "[Editor][IDE]") {
    NF::CodeNavigator nav;
    nav.addEntry({"foo", NF::SymbolKind::Function, "a.cpp", 1});
    nav.addEntry({"bar", NF::SymbolKind::Function, "b.cpp", 2});
    nav.addEntry({"MyClass", NF::SymbolKind::Class, "c.h", 3});
    nav.addEntry({"MyEnum", NF::SymbolKind::Enum, "d.h", 4});

    auto funcs = nav.findSymbolsByKind(NF::SymbolKind::Function);
    REQUIRE(funcs.size() == 2);

    auto classes = nav.findSymbolsByKind(NF::SymbolKind::Class);
    REQUIRE(classes.size() == 1);
    REQUIRE(classes[0].symbolName == "MyClass");

    auto enums = nav.findSymbolsByKind(NF::SymbolKind::Enum);
    REQUIRE(enums.size() == 1);
}

TEST_CASE("BreadcrumbTrail push and pop", "[Editor][IDE]") {
    NF::BreadcrumbTrail trail;
    REQUIRE(trail.depth() == 0);
    REQUIRE(trail.current() == nullptr);

    trail.push({"main", "src/main.cpp", 1});
    trail.push({"init", "src/init.cpp", 10});
    REQUIRE(trail.depth() == 2);
    REQUIRE(trail.current()->label == "init");

    auto popped = trail.pop();
    REQUIRE(popped.has_value());
    REQUIRE(popped->label == "init");
    REQUIRE(trail.depth() == 1);
    REQUIRE(trail.current()->label == "main");

    trail.clear();
    REQUIRE(trail.depth() == 0);
    REQUIRE(trail.pop() == std::nullopt);
}

TEST_CASE("BreadcrumbTrail max depth", "[Editor][IDE]") {
    NF::BreadcrumbTrail trail;
    for (int i = 0; i < 60; ++i) {
        trail.push({"item" + std::to_string(i), "file.cpp", static_cast<uint32_t>(i)});
    }
    REQUIRE(trail.depth() == 50);
    // Oldest items should have been evicted; newest should be item59
    REQUIRE(trail.current()->label == "item59");
    REQUIRE(trail.trail().front().label == "item10");
}

TEST_CASE("IDEService lifecycle", "[Editor][IDE]") {
    NF::IDEService ide;
    REQUIRE_FALSE(ide.isInitialized());

    ide.init();
    REQUIRE(ide.isInitialized());

    ide.shutdown();
    REQUIRE_FALSE(ide.isInitialized());
}

TEST_CASE("IDEService navigate and go back", "[Editor][IDE]") {
    NF::IDEService ide;
    ide.init();

    ide.navigateTo("src/main.cpp", 10, "main");
    ide.navigateTo("src/render.cpp", 42, "render");

    REQUIRE(ide.breadcrumbs().depth() == 2);
    REQUIRE(ide.breadcrumbs().current()->label == "render");

    REQUIRE(ide.goBack() == true);
    REQUIRE(ide.breadcrumbs().depth() == 1);
    REQUIRE(ide.breadcrumbs().current()->label == "main");

    REQUIRE(ide.goBack() == true);
    REQUIRE(ide.breadcrumbs().depth() == 0);

    REQUIRE(ide.goBack() == false);

    ide.shutdown();
}

TEST_CASE("EditorApp IDE commands registered", "[Editor][EditorApp]") {
    NF::EditorApp app;
    app.init(1280, 720);

    REQUIRE(app.commands().findCommand("ide.go_to_definition") != nullptr);
    REQUIRE(app.commands().findCommand("ide.find_references") != nullptr);
    REQUIRE(app.commands().findCommand("ide.go_back") != nullptr);
    REQUIRE(app.commands().findCommand("ide.index_project") != nullptr);

    app.shutdown();
}

TEST_CASE("EditorApp IDE service accessor", "[Editor][EditorApp]") {
    NF::EditorApp app;
    app.init(1280, 720);

    REQUIRE(app.ideService().isInitialized());
    app.ideService().navigateTo("test.cpp", 1, "test");
    REQUIRE(app.ideService().breadcrumbs().depth() == 1);

    app.shutdown();
    REQUIRE_FALSE(app.ideService().isInitialized());
}
