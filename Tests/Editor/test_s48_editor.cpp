#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

TEST_CASE("LandscapeBrushType names cover all 5 values", "[Editor][S48]") {
    REQUIRE(std::string(landscapeBrushTypeName(LandscapeBrushType::Circle))   == "Circle");
    REQUIRE(std::string(landscapeBrushTypeName(LandscapeBrushType::Square))   == "Square");
    REQUIRE(std::string(landscapeBrushTypeName(LandscapeBrushType::Triangle)) == "Triangle");
    REQUIRE(std::string(landscapeBrushTypeName(LandscapeBrushType::Gradient)) == "Gradient");
    REQUIRE(std::string(landscapeBrushTypeName(LandscapeBrushType::Custom))   == "Custom");
}

TEST_CASE("LandscapeLayerBlend names cover all 5 values", "[Editor][S48]") {
    REQUIRE(std::string(landscapeLayerBlendName(LandscapeLayerBlend::Normal))   == "Normal");
    REQUIRE(std::string(landscapeLayerBlendName(LandscapeLayerBlend::Additive)) == "Additive");
    REQUIRE(std::string(landscapeLayerBlendName(LandscapeLayerBlend::Multiply)) == "Multiply");
    REQUIRE(std::string(landscapeLayerBlendName(LandscapeLayerBlend::Overlay))  == "Overlay");
    REQUIRE(std::string(landscapeLayerBlendName(LandscapeLayerBlend::Screen))   == "Screen");
}

TEST_CASE("LandscapeState names cover all 5 values", "[Editor][S48]") {
    REQUIRE(std::string(landscapeStateName(LandscapeState::Unloaded)) == "Unloaded");
    REQUIRE(std::string(landscapeStateName(LandscapeState::Loading))  == "Loading");
    REQUIRE(std::string(landscapeStateName(LandscapeState::Idle))     == "Idle");
    REQUIRE(std::string(landscapeStateName(LandscapeState::Editing))  == "Editing");
    REQUIRE(std::string(landscapeStateName(LandscapeState::Error))    == "Error");
}

TEST_CASE("LandscapeAsset default values", "[Editor][S48]") {
    LandscapeAsset asset("terrain_01");
    REQUIRE(asset.name()        == "terrain_01");
    REQUIRE(asset.resolutionX() == 512u);
    REQUIRE(asset.resolutionY() == 512u);
    REQUIRE(asset.layerCount()  == 1u);
    REQUIRE(asset.heightScale() == Catch::Approx(1.0f));
    REQUIRE(asset.brushType()   == LandscapeBrushType::Circle);
    REQUIRE(asset.layerBlend()  == LandscapeLayerBlend::Normal);
    REQUIRE(asset.state()       == LandscapeState::Unloaded);
    REQUIRE_FALSE(asset.isDirty());
    REQUIRE_FALSE(asset.isLocked());
    REQUIRE_FALSE(asset.isIdle());
    REQUIRE_FALSE(asset.isEditing());
    REQUIRE_FALSE(asset.hasError());
    REQUIRE_FALSE(asset.isHighRes());
    REQUIRE_FALSE(asset.isMultiLayer());
}

TEST_CASE("LandscapeAsset setters round-trip", "[Editor][S48]") {
    LandscapeAsset asset("world_map", 2048, 2048);
    asset.setBrushType(LandscapeBrushType::Gradient);
    asset.setLayerBlend(LandscapeLayerBlend::Overlay);
    asset.setState(LandscapeState::Editing);
    asset.setLayerCount(4);
    asset.setHeightScale(2.5f);
    asset.setDirty(true);
    asset.setLocked(false);

    REQUIRE(asset.brushType()   == LandscapeBrushType::Gradient);
    REQUIRE(asset.layerBlend()  == LandscapeLayerBlend::Overlay);
    REQUIRE(asset.state()       == LandscapeState::Editing);
    REQUIRE(asset.resolutionX() == 2048u);
    REQUIRE(asset.resolutionY() == 2048u);
    REQUIRE(asset.layerCount()  == 4u);
    REQUIRE(asset.heightScale() == Catch::Approx(2.5f));
    REQUIRE(asset.isEditing());
    REQUIRE(asset.isDirty());
    REQUIRE_FALSE(asset.isLocked());
    REQUIRE(asset.isHighRes());
    REQUIRE(asset.isMultiLayer());
}

TEST_CASE("LandscapeAsset isHighRes requires resolution >= 1024", "[Editor][S48]") {
    LandscapeAsset asset("test");
    asset.setResolutionX(512);
    asset.setResolutionY(512);
    REQUIRE_FALSE(asset.isHighRes());

    asset.setResolutionX(1024);
    REQUIRE(asset.isHighRes());
}

TEST_CASE("LandscapeAsset hasError state", "[Editor][S48]") {
    LandscapeAsset asset("broken_terrain");
    REQUIRE_FALSE(asset.hasError());
    asset.setState(LandscapeState::Error);
    REQUIRE(asset.hasError());
    REQUIRE_FALSE(asset.isIdle());
}

TEST_CASE("LandscapeAsset isMultiLayer requires layerCount > 1", "[Editor][S48]") {
    LandscapeAsset asset("single");
    REQUIRE_FALSE(asset.isMultiLayer());
    asset.setLayerCount(2);
    REQUIRE(asset.isMultiLayer());
}

TEST_CASE("LandscapeEditor addLandscape and duplicate rejection", "[Editor][S48]") {
    LandscapeEditor editor;
    LandscapeAsset a("terrain_a"), b("terrain_b"), dup("terrain_a");
    REQUIRE(editor.addLandscape(a));
    REQUIRE(editor.addLandscape(b));
    REQUIRE_FALSE(editor.addLandscape(dup));
    REQUIRE(editor.landscapeCount() == 2);
}

TEST_CASE("LandscapeEditor removeLandscape clears activeLandscape", "[Editor][S48]") {
    LandscapeEditor editor;
    LandscapeAsset asset("highlands");
    editor.addLandscape(asset);
    editor.setActiveLandscape("highlands");
    REQUIRE(editor.activeLandscape() == "highlands");

    editor.removeLandscape("highlands");
    REQUIRE(editor.landscapeCount()  == 0);
    REQUIRE(editor.activeLandscape().empty());
}

TEST_CASE("LandscapeEditor findLandscape returns pointer or nullptr", "[Editor][S48]") {
    LandscapeEditor editor;
    LandscapeAsset asset("desert");
    editor.addLandscape(asset);

    REQUIRE(editor.findLandscape("desert") != nullptr);
    REQUIRE(editor.findLandscape("desert")->name() == "desert");
    REQUIRE(editor.findLandscape("missing") == nullptr);
}

TEST_CASE("LandscapeEditor aggregate counts", "[Editor][S48]") {
    LandscapeEditor editor;

    LandscapeAsset a("a"); a.setDirty(true);  a.setState(LandscapeState::Editing); a.setResolutionX(2048); a.setLayerCount(3);
    LandscapeAsset b("b"); b.setDirty(true);  b.setLocked(true); b.setResolutionX(256);
    LandscapeAsset c("c"); c.setState(LandscapeState::Idle); c.setResolutionX(1024); c.setLayerCount(2);

    editor.addLandscape(a); editor.addLandscape(b); editor.addLandscape(c);

    REQUIRE(editor.dirtyCount()      == 2);
    REQUIRE(editor.lockedCount()     == 1);
    REQUIRE(editor.editingCount()    == 1);
    REQUIRE(editor.highResCount()    == 2); // a=2048, c=1024
    REQUIRE(editor.multiLayerCount() == 2); // a=3, c=2
}

TEST_CASE("LandscapeEditor countByBrush and countByState", "[Editor][S48]") {
    LandscapeEditor editor;

    LandscapeAsset a("a"); a.setBrushType(LandscapeBrushType::Square);  a.setState(LandscapeState::Idle);
    LandscapeAsset b("b"); b.setBrushType(LandscapeBrushType::Circle);  b.setState(LandscapeState::Editing);
    LandscapeAsset c("c"); c.setBrushType(LandscapeBrushType::Square);  c.setState(LandscapeState::Idle);

    editor.addLandscape(a); editor.addLandscape(b); editor.addLandscape(c);

    REQUIRE(editor.countByBrush(LandscapeBrushType::Square)  == 2);
    REQUIRE(editor.countByBrush(LandscapeBrushType::Circle)  == 1);
    REQUIRE(editor.countByState(LandscapeState::Idle)        == 2);
    REQUIRE(editor.countByState(LandscapeState::Editing)     == 1);
}

TEST_CASE("LandscapeEditor setActiveLandscape returns false for missing", "[Editor][S48]") {
    LandscapeEditor editor;
    REQUIRE_FALSE(editor.setActiveLandscape("ghost_terrain"));
    REQUIRE(editor.activeLandscape().empty());
}

TEST_CASE("LandscapeEditor MAX_LANDSCAPES limit enforced", "[Editor][S48]") {
    LandscapeEditor editor;
    for (size_t i = 0; i < LandscapeEditor::MAX_LANDSCAPES; ++i) {
        LandscapeAsset asset("L" + std::to_string(i));
        REQUIRE(editor.addLandscape(asset));
    }
    LandscapeAsset overflow("Overflow");
    REQUIRE_FALSE(editor.addLandscape(overflow));
    REQUIRE(editor.landscapeCount() == LandscapeEditor::MAX_LANDSCAPES);
}
