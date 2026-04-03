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
