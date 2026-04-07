#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

TEST_CASE("EnvProbeShape names cover all 5 values", "[Editor][S47]") {
    REQUIRE(std::string(envProbeShapeName(EnvProbeShape::Sphere))   == "Sphere");
    REQUIRE(std::string(envProbeShapeName(EnvProbeShape::Box))      == "Box");
    REQUIRE(std::string(envProbeShapeName(EnvProbeShape::Capsule))  == "Capsule");
    REQUIRE(std::string(envProbeShapeName(EnvProbeShape::Cylinder)) == "Cylinder");
    REQUIRE(std::string(envProbeShapeName(EnvProbeShape::Custom))   == "Custom");
}

TEST_CASE("EnvProbeCaptureMode names cover all 5 values", "[Editor][S47]") {
    REQUIRE(std::string(envProbeCaptureModeN(EnvProbeCaptureMode::Realtime))  == "Realtime");
    REQUIRE(std::string(envProbeCaptureModeN(EnvProbeCaptureMode::Baked))     == "Baked");
    REQUIRE(std::string(envProbeCaptureModeN(EnvProbeCaptureMode::Mixed))     == "Mixed");
    REQUIRE(std::string(envProbeCaptureModeN(EnvProbeCaptureMode::Once))      == "Once");
    REQUIRE(std::string(envProbeCaptureModeN(EnvProbeCaptureMode::OnDemand))  == "OnDemand");
}

TEST_CASE("EnvProbeState names cover all 5 values", "[Editor][S47]") {
    REQUIRE(std::string(envProbeStateName(EnvProbeState::Inactive))    == "Inactive");
    REQUIRE(std::string(envProbeStateName(EnvProbeState::Active))      == "Active");
    REQUIRE(std::string(envProbeStateName(EnvProbeState::Capturing))   == "Capturing");
    REQUIRE(std::string(envProbeStateName(EnvProbeState::Invalidated)) == "Invalidated");
    REQUIRE(std::string(envProbeStateName(EnvProbeState::Error))       == "Error");
}

TEST_CASE("EnvProbeAsset default values", "[Editor][S47]") {
    EnvProbeAsset probe("main_probe");
    REQUIRE(probe.name()        == "main_probe");
    REQUIRE(probe.resolution()  == 128u);
    REQUIRE(probe.layerCount()  == 0u);
    REQUIRE(probe.influence()   == Catch::Approx(1.0f));
    REQUIRE(probe.shape()       == EnvProbeShape::Sphere);
    REQUIRE(probe.captureMode() == EnvProbeCaptureMode::Baked);
    REQUIRE(probe.state()       == EnvProbeState::Inactive);
    REQUIRE_FALSE(probe.isRealtime());
    REQUIRE(probe.isBaked());
    REQUIRE_FALSE(probe.isDirty());
    REQUIRE_FALSE(probe.isLocked());
    REQUIRE_FALSE(probe.isActive());
    REQUIRE_FALSE(probe.isCapturing());
    REQUIRE_FALSE(probe.hasError());
    REQUIRE_FALSE(probe.isHighRes());
}

TEST_CASE("EnvProbeAsset setters round-trip", "[Editor][S47]") {
    EnvProbeAsset probe("sky_probe", 1024, 3);
    probe.setShape(EnvProbeShape::Box);
    probe.setCaptureMode(EnvProbeCaptureMode::Realtime);
    probe.setState(EnvProbeState::Active);
    probe.setInfluence(0.8f);
    probe.setDirty(true);
    probe.setLocked(true);

    REQUIRE(probe.shape()       == EnvProbeShape::Box);
    REQUIRE(probe.captureMode() == EnvProbeCaptureMode::Realtime);
    REQUIRE(probe.state()       == EnvProbeState::Active);
    REQUIRE(probe.resolution()  == 1024u);
    REQUIRE(probe.layerCount()  == 3u);
    REQUIRE(probe.influence()   == Catch::Approx(0.8f));
    REQUIRE(probe.isRealtime());
    REQUIRE_FALSE(probe.isBaked());
    REQUIRE(probe.isDirty());
    REQUIRE(probe.isLocked());
    REQUIRE(probe.isActive());
    REQUIRE(probe.isHighRes());
}

TEST_CASE("EnvProbeAsset isHighRes requires resolution>=512", "[Editor][S47]") {
    EnvProbeAsset probe("test");
    probe.setResolution(256);
    REQUIRE_FALSE(probe.isHighRes());

    probe.setResolution(512);
    REQUIRE(probe.isHighRes());
}

TEST_CASE("EnvProbeAsset isCapturing and hasError states", "[Editor][S47]") {
    EnvProbeAsset probe("interior");
    probe.setState(EnvProbeState::Capturing);
    REQUIRE(probe.isCapturing());
    REQUIRE_FALSE(probe.hasError());

    probe.setState(EnvProbeState::Error);
    REQUIRE(probe.hasError());
    REQUIRE_FALSE(probe.isCapturing());
}

TEST_CASE("EnvironmentProbeEditor addProbe and duplicate rejection", "[Editor][S47]") {
    EnvironmentProbeEditor editor;
    EnvProbeAsset a("a"), b("b"), dup("a");
    REQUIRE(editor.addProbe(a));
    REQUIRE(editor.addProbe(b));
    REQUIRE_FALSE(editor.addProbe(dup));
    REQUIRE(editor.probeCount() == 2);
}

TEST_CASE("EnvironmentProbeEditor removeProbe clears activeProbe", "[Editor][S47]") {
    EnvironmentProbeEditor editor;
    EnvProbeAsset probe("lobby_probe");
    editor.addProbe(probe);
    editor.setActiveProbe("lobby_probe");
    REQUIRE(editor.activeProbe() == "lobby_probe");

    editor.removeProbe("lobby_probe");
    REQUIRE(editor.probeCount() == 0);
    REQUIRE(editor.activeProbe().empty());
}

TEST_CASE("EnvironmentProbeEditor findProbe returns pointer or nullptr", "[Editor][S47]") {
    EnvironmentProbeEditor editor;
    EnvProbeAsset probe("corridor");
    editor.addProbe(probe);

    REQUIRE(editor.findProbe("corridor") != nullptr);
    REQUIRE(editor.findProbe("corridor")->name() == "corridor");
    REQUIRE(editor.findProbe("missing") == nullptr);
}

TEST_CASE("EnvironmentProbeEditor aggregate counts", "[Editor][S47]") {
    EnvironmentProbeEditor editor;

    EnvProbeAsset a("a"); a.setDirty(true); a.setCaptureMode(EnvProbeCaptureMode::Realtime); a.setState(EnvProbeState::Active);   a.setResolution(1024);
    EnvProbeAsset b("b"); b.setDirty(true); b.setLocked(true); b.setResolution(256);
    EnvProbeAsset c("c"); c.setCaptureMode(EnvProbeCaptureMode::Baked);  c.setState(EnvProbeState::Inactive); c.setResolution(512);

    editor.addProbe(a); editor.addProbe(b); editor.addProbe(c);

    REQUIRE(editor.dirtyCount()    == 2);
    REQUIRE(editor.lockedCount()   == 1);
    REQUIRE(editor.realtimeCount() == 1);
    REQUIRE(editor.bakedCount()    == 2); // b default=Baked, c=Baked
    REQUIRE(editor.highResCount()  == 2); // a=1024, c=512
}

TEST_CASE("EnvironmentProbeEditor countByShape and countByState", "[Editor][S47]") {
    EnvironmentProbeEditor editor;

    EnvProbeAsset a("a"); a.setShape(EnvProbeShape::Box);    a.setState(EnvProbeState::Active);
    EnvProbeAsset b("b"); b.setShape(EnvProbeShape::Sphere); b.setState(EnvProbeState::Inactive);
    EnvProbeAsset c("c"); c.setShape(EnvProbeShape::Box);    c.setState(EnvProbeState::Active);

    editor.addProbe(a); editor.addProbe(b); editor.addProbe(c);

    REQUIRE(editor.countByShape(EnvProbeShape::Box)       == 2);
    REQUIRE(editor.countByState(EnvProbeState::Active)    == 2);
    REQUIRE(editor.countByState(EnvProbeState::Inactive)  == 1);
}

TEST_CASE("EnvironmentProbeEditor setActiveProbe returns false for missing", "[Editor][S47]") {
    EnvironmentProbeEditor editor;
    REQUIRE_FALSE(editor.setActiveProbe("ghost"));
    REQUIRE(editor.activeProbe().empty());
}

TEST_CASE("EnvironmentProbeEditor MAX_PROBES limit enforced", "[Editor][S47]") {
    EnvironmentProbeEditor editor;
    for (size_t i = 0; i < EnvironmentProbeEditor::MAX_PROBES; ++i) {
        EnvProbeAsset probe("P" + std::to_string(i));
        REQUIRE(editor.addProbe(probe));
    }
    EnvProbeAsset overflow("Overflow");
    REQUIRE_FALSE(editor.addProbe(overflow));
    REQUIRE(editor.probeCount() == EnvironmentProbeEditor::MAX_PROBES);
}
