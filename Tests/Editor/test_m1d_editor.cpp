#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── ProjectPathService.savePath() ────────────────────────────────

TEST_CASE("ProjectPathService savePath is populated after init", "[Editor][M1D]") {
    ProjectPathService svc;
    svc.init(".");
    // savePath should be the Saved/ subdirectory under the project root
    REQUIRE_FALSE(svc.savePath().empty());
    // savePath should end with "Saved" (or "Saved/" on some platforms)
    std::string sp = svc.savePath();
    bool endsSaved = (sp.find("Saved") != std::string::npos);
    REQUIRE(endsSaved);
}

TEST_CASE("ProjectPathService all four path accessors are non-empty after init", "[Editor][M1D]") {
    ProjectPathService svc;
    svc.init(".");
    REQUIRE_FALSE(svc.contentPath().empty());
    REQUIRE_FALSE(svc.dataPath().empty());
    REQUIRE_FALSE(svc.configPath().empty());
    REQUIRE_FALSE(svc.savePath().empty());
}

// ── ProjectEntry ─────────────────────────────────────────────────

TEST_CASE("ProjectEntry default construction isValid false", "[Editor][M1D]") {
    ProjectEntry pe;
    REQUIRE_FALSE(pe.isValid());
    REQUIRE(pe.name.empty());
    REQUIRE(pe.path.empty());
    REQUIRE(pe.description.empty());
    REQUIRE(pe.version.empty());
}

TEST_CASE("ProjectEntry with path isValid true", "[Editor][M1D]") {
    ProjectEntry pe;
    pe.name = "NovaForge";
    pe.path = "/some/path/NovaForge.atlas.json";
    pe.version = "0.1.0";
    REQUIRE(pe.isValid());
    REQUIRE(pe.name == "NovaForge");
    REQUIRE(pe.version == "0.1.0");
}

// ── ProjectPickerPanel — default state ───────────────────────────

TEST_CASE("ProjectPickerPanel default state is empty and hidden", "[Editor][M1D]") {
    ProjectPickerPanel picker;
    REQUIRE(picker.projectCount() == 0);
    REQUIRE_FALSE(picker.isVisible());
    REQUIRE_FALSE(picker.isLoaded());
    REQUIRE_FALSE(picker.hasSelection());
    REQUIRE(picker.selectedIndex() == -1);
    REQUIRE(picker.selectedProject() == nullptr);
    REQUIRE(picker.loadedProjectPath().empty());
}

// ── ProjectPickerPanel — show/hide ───────────────────────────────

TEST_CASE("ProjectPickerPanel show and hide toggle visibility", "[Editor][M1D]") {
    ProjectPickerPanel picker;
    REQUIRE_FALSE(picker.isVisible());
    picker.show();
    REQUIRE(picker.isVisible());
    picker.hide();
    REQUIRE_FALSE(picker.isVisible());
}

// ── ProjectPickerPanel — addProject and selection ─────────────────

TEST_CASE("ProjectPickerPanel addProject populates list", "[Editor][M1D]") {
    ProjectPickerPanel picker;
    ProjectEntry pe;
    pe.name = "TestProject";
    pe.path = "/tmp/test.atlas.json";
    pe.version = "1.0.0";

    picker.addProject(pe);
    REQUIRE(picker.projectCount() == 1);
    REQUIRE(picker.projects()[0].name == "TestProject");
}

TEST_CASE("ProjectPickerPanel selectProject returns true for valid index", "[Editor][M1D]") {
    ProjectPickerPanel picker;
    ProjectEntry pe;
    pe.name = "Alpha";
    pe.path = "/tmp/alpha.atlas.json";
    picker.addProject(pe);

    REQUIRE(picker.selectProject(0));
    REQUIRE(picker.selectedIndex() == 0);
    REQUIRE(picker.hasSelection());
    REQUIRE(picker.selectedProject() != nullptr);
    REQUIRE(picker.selectedProject()->name == "Alpha");
}

TEST_CASE("ProjectPickerPanel selectProject returns false for out-of-range index", "[Editor][M1D]") {
    ProjectPickerPanel picker;
    REQUIRE_FALSE(picker.selectProject(-1));
    REQUIRE_FALSE(picker.selectProject(0));
    REQUIRE_FALSE(picker.hasSelection());
}

// ── ProjectPickerPanel — loadSelected and loadProject ────────────

TEST_CASE("ProjectPickerPanel loadSelected sets loaded and hides picker", "[Editor][M1D]") {
    ProjectPickerPanel picker;
    ProjectEntry pe;
    pe.name    = "NovaForge";
    pe.path    = "/workspace/Project/NovaForge.atlas.json";
    pe.version = "0.1.0";
    picker.addProject(pe);
    picker.selectProject(0);
    picker.show();

    REQUIRE(picker.isVisible());
    bool ok = picker.loadSelected();
    REQUIRE(ok);
    REQUIRE(picker.isLoaded());
    REQUIRE(picker.loadedProjectPath() == "/workspace/Project/NovaForge.atlas.json");
    REQUIRE_FALSE(picker.isVisible()); // picker hides itself on load
}

TEST_CASE("ProjectPickerPanel loadSelected fails when nothing selected", "[Editor][M1D]") {
    ProjectPickerPanel picker;
    ProjectEntry pe;
    pe.name = "X"; pe.path = "/tmp/x.atlas.json";
    picker.addProject(pe);
    // Do NOT call selectProject

    REQUIRE_FALSE(picker.loadSelected());
    REQUIRE_FALSE(picker.isLoaded());
}

TEST_CASE("ProjectPickerPanel loadProject directly by path sets loaded state", "[Editor][M1D]") {
    ProjectPickerPanel picker;
    bool ok = picker.loadProject("/workspace/Project/NovaForge.atlas.json");
    REQUIRE(ok);
    REQUIRE(picker.isLoaded());
    REQUIRE(picker.loadedProjectPath() == "/workspace/Project/NovaForge.atlas.json");
    REQUIRE(picker.projectCount() == 1);
    REQUIRE_FALSE(picker.isVisible());
}

TEST_CASE("ProjectPickerPanel clearProjects resets list and selection", "[Editor][M1D]") {
    ProjectPickerPanel picker;
    ProjectEntry pe;
    pe.name = "A"; pe.path = "/tmp/a.atlas.json";
    picker.addProject(pe);
    picker.selectProject(0);
    REQUIRE(picker.projectCount() == 1);
    REQUIRE(picker.hasSelection());

    picker.clearProjects();
    REQUIRE(picker.projectCount() == 0);
    REQUIRE_FALSE(picker.hasSelection());
    REQUIRE(picker.selectedIndex() == -1);
}

// ── ProjectPickerPanel — scan (non-existent dir is safe) ─────────

TEST_CASE("ProjectPickerPanel scanDirectory on non-existent dir returns 0", "[Editor][M1D]") {
    ProjectPickerPanel picker;
    size_t found = picker.scanDirectory("/tmp/does_not_exist_xyzzy_99999");
    REQUIRE(found == 0);
    REQUIRE(picker.projectCount() == 0);
}

// ── EditorApp — project picker integration ───────────────────────

TEST_CASE("EditorApp projectPicker accessor and show/hide round-trip", "[Editor][M1D]") {
    EditorApp editor;
    // Access project picker without initializing (safe accessor test)
    editor.projectPicker().show();
    REQUIRE(editor.isProjectPickerVisible());
    REQUIRE(editor.projectPicker().isVisible());

    editor.hideProjectPicker();
    REQUIRE_FALSE(editor.isProjectPickerVisible());

    editor.showProjectPicker();
    REQUIRE(editor.isProjectPickerVisible());
}
