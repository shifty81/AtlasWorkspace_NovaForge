#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

TEST_CASE("DecalProjectionType names cover all 5 values", "[Editor][S46]") {
    REQUIRE(std::string(decalProjectionTypeName(DecalProjectionType::Box))      == "Box");
    REQUIRE(std::string(decalProjectionTypeName(DecalProjectionType::Sphere))   == "Sphere");
    REQUIRE(std::string(decalProjectionTypeName(DecalProjectionType::Cylinder)) == "Cylinder");
    REQUIRE(std::string(decalProjectionTypeName(DecalProjectionType::Planar))   == "Planar");
    REQUIRE(std::string(decalProjectionTypeName(DecalProjectionType::Custom))   == "Custom");
}

TEST_CASE("DecalBlendMode names cover all 5 values", "[Editor][S46]") {
    REQUIRE(std::string(decalBlendModeName(DecalBlendMode::Translucent)) == "Translucent");
    REQUIRE(std::string(decalBlendModeName(DecalBlendMode::Additive))    == "Additive");
    REQUIRE(std::string(decalBlendModeName(DecalBlendMode::Modulate))    == "Modulate");
    REQUIRE(std::string(decalBlendModeName(DecalBlendMode::Stain))       == "Stain");
    REQUIRE(std::string(decalBlendModeName(DecalBlendMode::Emissive))    == "Emissive");
}

TEST_CASE("DecalState names cover all 5 values", "[Editor][S46]") {
    REQUIRE(std::string(decalStateName(DecalState::Inactive)) == "Inactive");
    REQUIRE(std::string(decalStateName(DecalState::Active))   == "Active");
    REQUIRE(std::string(decalStateName(DecalState::Fading))   == "Fading");
    REQUIRE(std::string(decalStateName(DecalState::Baked))    == "Baked");
    REQUIRE(std::string(decalStateName(DecalState::Error))    == "Error");
}

TEST_CASE("DecalAsset default values", "[Editor][S46]") {
    DecalAsset decal("bullet_hole");
    REQUIRE(decal.name()           == "bullet_hole");
    REQUIRE(decal.layerCount()     == 0u);
    REQUIRE(decal.paramCount()     == 0u);
    REQUIRE(decal.opacity()        == Catch::Approx(1.0f));
    REQUIRE(decal.projectionType() == DecalProjectionType::Box);
    REQUIRE(decal.blendMode()      == DecalBlendMode::Translucent);
    REQUIRE(decal.state()          == DecalState::Inactive);
    REQUIRE_FALSE(decal.isReceivingLighting());
    REQUIRE_FALSE(decal.isRealtime());
    REQUIRE_FALSE(decal.isDirty());
    REQUIRE_FALSE(decal.isActive());
    REQUIRE_FALSE(decal.isBaked());
    REQUIRE_FALSE(decal.hasError());
    REQUIRE_FALSE(decal.isComplex());
}

TEST_CASE("DecalAsset setters round-trip", "[Editor][S46]") {
    DecalAsset decal("blood_splat", 5, 3);
    decal.setProjectionType(DecalProjectionType::Sphere);
    decal.setBlendMode(DecalBlendMode::Additive);
    decal.setState(DecalState::Active);
    decal.setOpacity(0.75f);
    decal.setReceiveLighting(true);
    decal.setRealtime(true);
    decal.setDirty(true);

    REQUIRE(decal.projectionType()     == DecalProjectionType::Sphere);
    REQUIRE(decal.blendMode()          == DecalBlendMode::Additive);
    REQUIRE(decal.state()              == DecalState::Active);
    REQUIRE(decal.layerCount()         == 5u);
    REQUIRE(decal.paramCount()         == 3u);
    REQUIRE(decal.opacity()            == Catch::Approx(0.75f));
    REQUIRE(decal.isReceivingLighting());
    REQUIRE(decal.isRealtime());
    REQUIRE(decal.isDirty());
    REQUIRE(decal.isActive());
    REQUIRE(decal.isComplex());
}

TEST_CASE("DecalAsset isComplex requires layerCount>=4", "[Editor][S46]") {
    DecalAsset decal("test");
    decal.setLayerCount(3);
    REQUIRE_FALSE(decal.isComplex());

    decal.setLayerCount(4);
    REQUIRE(decal.isComplex());
}

TEST_CASE("DecalAsset isBaked and hasError states", "[Editor][S46]") {
    DecalAsset decal("crack");
    decal.setState(DecalState::Baked);
    REQUIRE(decal.isBaked());
    REQUIRE_FALSE(decal.hasError());

    decal.setState(DecalState::Error);
    REQUIRE(decal.hasError());
    REQUIRE_FALSE(decal.isBaked());
}

TEST_CASE("DecalEditor addDecal and duplicate rejection", "[Editor][S46]") {
    DecalEditor editor;
    DecalAsset a("a"), b("b"), dup("a");
    REQUIRE(editor.addDecal(a));
    REQUIRE(editor.addDecal(b));
    REQUIRE_FALSE(editor.addDecal(dup));
    REQUIRE(editor.decalCount() == 2);
}

TEST_CASE("DecalEditor removeDecal clears activeDecal", "[Editor][S46]") {
    DecalEditor editor;
    DecalAsset decal("scorch_mark");
    editor.addDecal(decal);
    editor.setActiveDecal("scorch_mark");
    REQUIRE(editor.activeDecal() == "scorch_mark");

    editor.removeDecal("scorch_mark");
    REQUIRE(editor.decalCount() == 0);
    REQUIRE(editor.activeDecal().empty());
}

TEST_CASE("DecalEditor findDecal returns pointer or nullptr", "[Editor][S46]") {
    DecalEditor editor;
    DecalAsset decal("graffiti");
    editor.addDecal(decal);

    REQUIRE(editor.findDecal("graffiti") != nullptr);
    REQUIRE(editor.findDecal("graffiti")->name() == "graffiti");
    REQUIRE(editor.findDecal("missing") == nullptr);
}

TEST_CASE("DecalEditor aggregate counts", "[Editor][S46]") {
    DecalEditor editor;

    DecalAsset a("a"); a.setDirty(true); a.setState(DecalState::Active); a.setRealtime(true); a.setLayerCount(5);
    DecalAsset b("b"); b.setDirty(true); b.setLayerCount(2);
    DecalAsset c("c"); c.setRealtime(true); c.setLayerCount(4); c.setState(DecalState::Baked);

    editor.addDecal(a); editor.addDecal(b); editor.addDecal(c);

    REQUIRE(editor.dirtyCount()    == 2);
    REQUIRE(editor.activeCount()   == 1);
    REQUIRE(editor.bakedCount()    == 1);
    REQUIRE(editor.realtimeCount() == 2);
    REQUIRE(editor.complexCount()  == 2); // a and c
}

TEST_CASE("DecalEditor countByProjectionType and countByState", "[Editor][S46]") {
    DecalEditor editor;

    DecalAsset a("a"); a.setProjectionType(DecalProjectionType::Sphere); a.setState(DecalState::Active);
    DecalAsset b("b"); b.setProjectionType(DecalProjectionType::Planar);  b.setState(DecalState::Inactive);
    DecalAsset c("c"); c.setProjectionType(DecalProjectionType::Sphere);  c.setState(DecalState::Active);

    editor.addDecal(a); editor.addDecal(b); editor.addDecal(c);

    REQUIRE(editor.countByProjectionType(DecalProjectionType::Sphere)  == 2);
    REQUIRE(editor.countByProjectionType(DecalProjectionType::Planar)  == 1);
    REQUIRE(editor.countByState(DecalState::Active)                    == 2);
    REQUIRE(editor.countByState(DecalState::Inactive)                  == 1);
}

TEST_CASE("DecalEditor setActiveDecal returns false for missing", "[Editor][S46]") {
    DecalEditor editor;
    REQUIRE_FALSE(editor.setActiveDecal("ghost"));
    REQUIRE(editor.activeDecal().empty());
}

TEST_CASE("DecalEditor MAX_DECALS limit enforced", "[Editor][S46]") {
    DecalEditor editor;
    for (size_t i = 0; i < DecalEditor::MAX_DECALS; ++i) {
        DecalAsset decal("D" + std::to_string(i));
        REQUIRE(editor.addDecal(decal));
    }
    DecalAsset overflow("Overflow");
    REQUIRE_FALSE(editor.addDecal(overflow));
    REQUIRE(editor.decalCount() == DecalEditor::MAX_DECALS);
}
