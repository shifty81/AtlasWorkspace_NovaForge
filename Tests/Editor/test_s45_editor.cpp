#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

TEST_CASE("SeqTrackType names cover all 5 values", "[Editor][S45]") {
    REQUIRE(std::string(seqTrackTypeName(SeqTrackType::Actor))    == "Actor");
    REQUIRE(std::string(seqTrackTypeName(SeqTrackType::Camera))   == "Camera");
    REQUIRE(std::string(seqTrackTypeName(SeqTrackType::Audio))    == "Audio");
    REQUIRE(std::string(seqTrackTypeName(SeqTrackType::Event))    == "Event");
    REQUIRE(std::string(seqTrackTypeName(SeqTrackType::Property)) == "Property");
}

TEST_CASE("SeqPlaybackMode names cover all 5 values", "[Editor][S45]") {
    REQUIRE(std::string(seqPlaybackModeName(SeqPlaybackMode::Once))     == "Once");
    REQUIRE(std::string(seqPlaybackModeName(SeqPlaybackMode::Loop))     == "Loop");
    REQUIRE(std::string(seqPlaybackModeName(SeqPlaybackMode::PingPong)) == "PingPong");
    REQUIRE(std::string(seqPlaybackModeName(SeqPlaybackMode::Hold))     == "Hold");
    REQUIRE(std::string(seqPlaybackModeName(SeqPlaybackMode::Custom))   == "Custom");
}

TEST_CASE("SeqState names cover all 5 values", "[Editor][S45]") {
    REQUIRE(std::string(seqStateName(SeqState::Idle))      == "Idle");
    REQUIRE(std::string(seqStateName(SeqState::Playing))   == "Playing");
    REQUIRE(std::string(seqStateName(SeqState::Paused))    == "Paused");
    REQUIRE(std::string(seqStateName(SeqState::Recording)) == "Recording");
    REQUIRE(std::string(seqStateName(SeqState::Finished))  == "Finished");
}

TEST_CASE("LevelSequenceAsset default values", "[Editor][S45]") {
    LevelSequenceAsset seq("intro_cinematic");
    REQUIRE(seq.name()         == "intro_cinematic");
    REQUIRE(seq.trackCount()   == 0u);
    REQUIRE(seq.clipCount()    == 0u);
    REQUIRE(seq.duration()     == Catch::Approx(0.0f));
    REQUIRE(seq.trackType()    == SeqTrackType::Actor);
    REQUIRE(seq.playbackMode() == SeqPlaybackMode::Once);
    REQUIRE(seq.state()        == SeqState::Idle);
    REQUIRE_FALSE(seq.isLocked());
    REQUIRE_FALSE(seq.isRealtime());
    REQUIRE_FALSE(seq.isDirty());
    REQUIRE_FALSE(seq.isPlaying());
    REQUIRE_FALSE(seq.isPaused());
    REQUIRE_FALSE(seq.isRecording());
    REQUIRE_FALSE(seq.isComplex());
}

TEST_CASE("LevelSequenceAsset setters round-trip", "[Editor][S45]") {
    LevelSequenceAsset seq("combat_seq", 10, 5);
    seq.setTrackType(SeqTrackType::Camera);
    seq.setPlaybackMode(SeqPlaybackMode::Loop);
    seq.setState(SeqState::Playing);
    seq.setDuration(30.0f);
    seq.setLocked(true);
    seq.setRealtime(true);
    seq.setDirty(true);

    REQUIRE(seq.trackType()    == SeqTrackType::Camera);
    REQUIRE(seq.playbackMode() == SeqPlaybackMode::Loop);
    REQUIRE(seq.state()        == SeqState::Playing);
    REQUIRE(seq.trackCount()   == 10u);
    REQUIRE(seq.clipCount()    == 5u);
    REQUIRE(seq.duration()     == Catch::Approx(30.0f));
    REQUIRE(seq.isLocked());
    REQUIRE(seq.isRealtime());
    REQUIRE(seq.isDirty());
    REQUIRE(seq.isPlaying());
    REQUIRE(seq.isComplex());
}

TEST_CASE("LevelSequenceAsset isComplex requires trackCount>=8", "[Editor][S45]") {
    LevelSequenceAsset seq("test");
    seq.setTrackCount(7);
    REQUIRE_FALSE(seq.isComplex());

    seq.setTrackCount(8);
    REQUIRE(seq.isComplex());
}

TEST_CASE("LevelSequenceAsset state predicates", "[Editor][S45]") {
    LevelSequenceAsset seq("fx_seq");
    seq.setState(SeqState::Paused);
    REQUIRE(seq.isPaused());
    REQUIRE_FALSE(seq.isPlaying());

    seq.setState(SeqState::Recording);
    REQUIRE(seq.isRecording());
    REQUIRE_FALSE(seq.isPaused());
}

TEST_CASE("LevelSequenceEditor addSequence and duplicate rejection", "[Editor][S45]") {
    LevelSequenceEditor editor;
    LevelSequenceAsset a("a"), b("b"), dup("a");
    REQUIRE(editor.addSequence(a));
    REQUIRE(editor.addSequence(b));
    REQUIRE_FALSE(editor.addSequence(dup));
    REQUIRE(editor.sequenceCount() == 2);
}

TEST_CASE("LevelSequenceEditor removeSequence clears activeSequence", "[Editor][S45]") {
    LevelSequenceEditor editor;
    LevelSequenceAsset seq("main_seq");
    editor.addSequence(seq);
    editor.setActiveSequence("main_seq");
    REQUIRE(editor.activeSequence() == "main_seq");

    editor.removeSequence("main_seq");
    REQUIRE(editor.sequenceCount()  == 0);
    REQUIRE(editor.activeSequence().empty());
}

TEST_CASE("LevelSequenceEditor findSequence returns pointer or nullptr", "[Editor][S45]") {
    LevelSequenceEditor editor;
    LevelSequenceAsset seq("cinematic");
    editor.addSequence(seq);

    REQUIRE(editor.findSequence("cinematic") != nullptr);
    REQUIRE(editor.findSequence("cinematic")->name() == "cinematic");
    REQUIRE(editor.findSequence("missing") == nullptr);
}

TEST_CASE("LevelSequenceEditor aggregate counts", "[Editor][S45]") {
    LevelSequenceEditor editor;

    LevelSequenceAsset a("a"); a.setDirty(true); a.setState(SeqState::Playing); a.setRealtime(true); a.setTrackCount(9);
    LevelSequenceAsset b("b"); b.setDirty(true); b.setTrackCount(3); b.setLocked(true);
    LevelSequenceAsset c("c"); c.setRealtime(true); c.setTrackCount(8); c.setLocked(true);

    editor.addSequence(a); editor.addSequence(b); editor.addSequence(c);

    REQUIRE(editor.dirtyCount()    == 2);
    REQUIRE(editor.playingCount()  == 1);
    REQUIRE(editor.realtimeCount() == 2);
    REQUIRE(editor.lockedCount()   == 2);
    REQUIRE(editor.complexCount()  == 2); // a and c
}

TEST_CASE("LevelSequenceEditor countByTrackType and countByState", "[Editor][S45]") {
    LevelSequenceEditor editor;

    LevelSequenceAsset a("a"); a.setTrackType(SeqTrackType::Camera); a.setState(SeqState::Playing);
    LevelSequenceAsset b("b"); a.setTrackType(SeqTrackType::Camera); b.setTrackType(SeqTrackType::Audio); b.setState(SeqState::Idle);
    LevelSequenceAsset c("c"); c.setTrackType(SeqTrackType::Audio);  c.setState(SeqState::Playing);

    editor.addSequence(a); editor.addSequence(b); editor.addSequence(c);

    REQUIRE(editor.countByTrackType(SeqTrackType::Audio)  == 2);
    REQUIRE(editor.countByState(SeqState::Playing)        == 2);
    REQUIRE(editor.countByState(SeqState::Idle)           == 1);
}

TEST_CASE("LevelSequenceEditor setActiveSequence returns false for missing", "[Editor][S45]") {
    LevelSequenceEditor editor;
    REQUIRE_FALSE(editor.setActiveSequence("ghost"));
    REQUIRE(editor.activeSequence().empty());
}

TEST_CASE("LevelSequenceEditor MAX_SEQUENCES limit enforced", "[Editor][S45]") {
    LevelSequenceEditor editor;
    for (size_t i = 0; i < LevelSequenceEditor::MAX_SEQUENCES; ++i) {
        LevelSequenceAsset seq("SEQ" + std::to_string(i));
        REQUIRE(editor.addSequence(seq));
    }
    LevelSequenceAsset overflow("Overflow");
    REQUIRE_FALSE(editor.addSequence(overflow));
    REQUIRE(editor.sequenceCount() == LevelSequenceEditor::MAX_SEQUENCES);
}
