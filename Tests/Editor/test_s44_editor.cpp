#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

TEST_CASE("PostFXCategory names cover all 5 values", "[Editor][S44]") {
    REQUIRE(std::string(postFXCategoryName(PostFXCategory::Bloom))            == "Bloom");
    REQUIRE(std::string(postFXCategoryName(PostFXCategory::Vignette))         == "Vignette");
    REQUIRE(std::string(postFXCategoryName(PostFXCategory::ColorGrade))       == "ColorGrade");
    REQUIRE(std::string(postFXCategoryName(PostFXCategory::DepthOfField))     == "DepthOfField");
    REQUIRE(std::string(postFXCategoryName(PostFXCategory::AmbientOcclusion)) == "AmbientOcclusion");
}

TEST_CASE("PostFXBlendMode names cover all 5 values", "[Editor][S44]") {
    REQUIRE(std::string(postFXBlendModeName(PostFXBlendMode::Override)) == "Override");
    REQUIRE(std::string(postFXBlendModeName(PostFXBlendMode::Additive)) == "Additive");
    REQUIRE(std::string(postFXBlendModeName(PostFXBlendMode::Multiply)) == "Multiply");
    REQUIRE(std::string(postFXBlendModeName(PostFXBlendMode::Screen))   == "Screen");
    REQUIRE(std::string(postFXBlendModeName(PostFXBlendMode::Normal))   == "Normal");
}

TEST_CASE("PostFXState names cover all 5 values", "[Editor][S44]") {
    REQUIRE(std::string(postFXStateName(PostFXState::Inactive))   == "Inactive");
    REQUIRE(std::string(postFXStateName(PostFXState::Active))     == "Active");
    REQUIRE(std::string(postFXStateName(PostFXState::Previewing)) == "Previewing");
    REQUIRE(std::string(postFXStateName(PostFXState::Baking))     == "Baking");
    REQUIRE(std::string(postFXStateName(PostFXState::Error))      == "Error");
}

TEST_CASE("PostFXAsset default values", "[Editor][S44]") {
    PostFXAsset asset("bloom_pass");
    REQUIRE(asset.name()        == "bloom_pass");
    REQUIRE(asset.layerCount()  == 0u);
    REQUIRE(asset.paramCount()  == 0u);
    REQUIRE(asset.category()    == PostFXCategory::Bloom);
    REQUIRE(asset.blendMode()   == PostFXBlendMode::Override);
    REQUIRE(asset.state()       == PostFXState::Inactive);
    REQUIRE_FALSE(asset.isEnabled());
    REQUIRE_FALSE(asset.isRealtime());
    REQUIRE_FALSE(asset.isDirty());
    REQUIRE_FALSE(asset.isActive());
    REQUIRE_FALSE(asset.isBaking());
    REQUIRE_FALSE(asset.hasError());
    REQUIRE_FALSE(asset.isComplex());
}

TEST_CASE("PostFXAsset setters round-trip", "[Editor][S44]") {
    PostFXAsset asset("grade", 3, 8);
    asset.setCategory(PostFXCategory::ColorGrade);
    asset.setBlendMode(PostFXBlendMode::Additive);
    asset.setState(PostFXState::Active);
    asset.setLayerCount(6);
    asset.setParamCount(12);
    asset.setEnabled(true);
    asset.setRealtime(true);
    asset.setDirty(true);

    REQUIRE(asset.category()    == PostFXCategory::ColorGrade);
    REQUIRE(asset.blendMode()   == PostFXBlendMode::Additive);
    REQUIRE(asset.state()       == PostFXState::Active);
    REQUIRE(asset.layerCount()  == 6u);
    REQUIRE(asset.paramCount()  == 12u);
    REQUIRE(asset.isEnabled());
    REQUIRE(asset.isRealtime());
    REQUIRE(asset.isDirty());
    REQUIRE(asset.isActive());
    REQUIRE(asset.isComplex());
}

TEST_CASE("PostFXAsset isComplex requires layerCount>=5", "[Editor][S44]") {
    PostFXAsset asset("test");
    asset.setLayerCount(4);
    REQUIRE_FALSE(asset.isComplex());

    asset.setLayerCount(5);
    REQUIRE(asset.isComplex());
}

TEST_CASE("PostFXAsset isBaking and hasError states", "[Editor][S44]") {
    PostFXAsset asset("bake");
    asset.setState(PostFXState::Baking);
    REQUIRE(asset.isBaking());
    REQUIRE_FALSE(asset.hasError());

    asset.setState(PostFXState::Error);
    REQUIRE(asset.hasError());
    REQUIRE_FALSE(asset.isBaking());
}

TEST_CASE("PostProcessEditor addAsset and duplicate rejection", "[Editor][S44]") {
    PostProcessEditor editor;
    PostFXAsset a("a"), b("b"), dup("a");
    REQUIRE(editor.addAsset(a));
    REQUIRE(editor.addAsset(b));
    REQUIRE_FALSE(editor.addAsset(dup));
    REQUIRE(editor.assetCount() == 2);
}

TEST_CASE("PostProcessEditor removeAsset clears activeAsset", "[Editor][S44]") {
    PostProcessEditor editor;
    PostFXAsset asset("vignette");
    editor.addAsset(asset);
    editor.setActiveAsset("vignette");
    REQUIRE(editor.activeAsset() == "vignette");

    editor.removeAsset("vignette");
    REQUIRE(editor.assetCount() == 0);
    REQUIRE(editor.activeAsset().empty());
}

TEST_CASE("PostProcessEditor findAsset returns pointer or nullptr", "[Editor][S44]") {
    PostProcessEditor editor;
    PostFXAsset asset("dof");
    editor.addAsset(asset);

    REQUIRE(editor.findAsset("dof") != nullptr);
    REQUIRE(editor.findAsset("dof")->name() == "dof");
    REQUIRE(editor.findAsset("missing") == nullptr);
}

TEST_CASE("PostProcessEditor aggregate counts", "[Editor][S44]") {
    PostProcessEditor editor;

    PostFXAsset a("a"); a.setDirty(true); a.setState(PostFXState::Active); a.setRealtime(true); a.setLayerCount(6);
    PostFXAsset b("b"); b.setDirty(true); b.setLayerCount(2);
    PostFXAsset c("c"); c.setRealtime(true); c.setLayerCount(5);

    editor.addAsset(a); editor.addAsset(b); editor.addAsset(c);

    REQUIRE(editor.dirtyCount()    == 2);
    REQUIRE(editor.activeCount()   == 1);
    REQUIRE(editor.realtimeCount() == 2);
    REQUIRE(editor.complexCount()  == 2); // a and c
}

TEST_CASE("PostProcessEditor countByCategory and countByState", "[Editor][S44]") {
    PostProcessEditor editor;

    PostFXAsset a("a"); a.setCategory(PostFXCategory::Bloom);      a.setState(PostFXState::Active);
    PostFXAsset b("b"); b.setCategory(PostFXCategory::Bloom);      b.setState(PostFXState::Inactive);
    PostFXAsset c("c"); c.setCategory(PostFXCategory::ColorGrade); c.setState(PostFXState::Active);

    editor.addAsset(a); editor.addAsset(b); editor.addAsset(c);

    REQUIRE(editor.countByCategory(PostFXCategory::Bloom)      == 2);
    REQUIRE(editor.countByCategory(PostFXCategory::ColorGrade) == 1);
    REQUIRE(editor.countByState(PostFXState::Active)           == 2);
    REQUIRE(editor.countByState(PostFXState::Inactive)         == 1);
}

TEST_CASE("PostProcessEditor setActiveAsset returns false for missing", "[Editor][S44]") {
    PostProcessEditor editor;
    REQUIRE_FALSE(editor.setActiveAsset("ghost"));
    REQUIRE(editor.activeAsset().empty());
}

TEST_CASE("PostProcessEditor MAX_ASSETS limit enforced", "[Editor][S44]") {
    PostProcessEditor editor;
    for (size_t i = 0; i < PostProcessEditor::MAX_ASSETS; ++i) {
        PostFXAsset asset("FX" + std::to_string(i));
        REQUIRE(editor.addAsset(asset));
    }
    PostFXAsset overflow("Overflow");
    REQUIRE_FALSE(editor.addAsset(overflow));
    REQUIRE(editor.assetCount() == PostProcessEditor::MAX_ASSETS);
}
