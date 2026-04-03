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
