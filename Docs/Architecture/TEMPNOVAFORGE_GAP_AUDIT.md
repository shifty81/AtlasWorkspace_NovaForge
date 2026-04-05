# NovaForge — TempNovaForge Gap Audit

**Date:** 2026-04
**Auditor:** Architecture Review

---

## Purpose

This document catalogs the critical architecture and documentation gaps found in the
`tempnovaforge` repository after the MasterRepo consolidation phase (C0). It serves
as the authoritative action list for bringing the repo to a production-ready state.

---

## Current State Summary

- Engine phases 0–9: ✅ Complete
- Game phases G1–G17: ✅ Complete
- Game phases G18–G20: 🔄 In Progress
- Build system: ✅ CMake 3.20 with presets
- Test suite: ✅ Catch2 v3.5.2, 13 test executables
- Naming consistency: ❌ Legacy names (ArbiterAI, SwissAgent, Arbiter) still present
- Project manifest: ❌ Missing `Project/project.atlas.json`
- Hosted build mode: ❌ `NF_HOSTED` CMake option not implemented
- Documentation: ❌ Missing Architecture, Gameplay, Systems docs
- Directory stubs: ❌ Missing Logs/, Codex/, Content/Incoming/
- VS2022 test build: ❌ `Catch2::Catch2WithMain` causes MSVC LNK4098 on VS generator
- Program mains: ❌ No real Win32 window in Editor/Game programs
- Season config: ❌ Missing Config/season.config.json

---

## Critical Architecture Gaps

### GAP 1 — Naming Inconsistency
**Severity:** HIGH
**Problem:** The codebase references `ArbiterAI`, `Arbiter`, and `SwissAgent` as separate
tool identities. The canonical name is `AtlasAI` for all AI broker functionality.
**Action:** Create `Docs/Architecture/NAMING_CANON.md`. Update all READMEs in
`Tools/ArbiterAI/` and `Tools/SwissAgent/`. Update README.md.
**Status:** 🔄 Addressed in this pass

### GAP 2 — Missing Project Manifest
**Severity:** HIGH
**Problem:** `Project/project.atlas.json` does not exist. This is the authoritative
contract required for Atlas Workspace hosted loading.
**Action:** Create `Project/project.atlas.json` with full entrypoints, paths, build
config, pipeline config, and atlas_workspace contract block.
**Status:** 🔄 Addressed in this pass

### GAP 3 — No Hosted Build Mode
**Severity:** HIGH
**Problem:** CMakeLists.txt has no `NF_HOSTED` or `NF_STANDALONE` options. When Atlas
Workspace controls the build, there is no way to disable executables by default.
**Action:** Add `NF_STANDALONE` (ON) and `NF_HOSTED` (OFF) options. Gate
`NF_BUILD_EDITOR`, `NF_BUILD_GAME`, `NF_BUILD_SERVER` defaults on `_NF_DEFAULT_BUILD_TARGETS`.
**Status:** 🔄 Addressed in this pass

### GAP 4 — VS2022 Test Build Failure (LNK4098)
**Severity:** HIGH
**Problem:** `Catch2::Catch2WithMain` triggers MSVC static-lib extraction issues on the
Visual Studio generator, causing `LNK4098` linker warnings and potential test failures.
**Action:** Create `Tests/catch_main.cpp` with explicit `Catch::Session().run(argc, argv)`.
Update all 13 test targets in `Tests/CMakeLists.txt` to use `Catch2::Catch2` and include
`catch_main.cpp` directly.
**Status:** 🔄 Addressed in this pass

### GAP 5 — No Real Win32 Window in Editor/Game
**Severity:** MEDIUM
**Problem:** `Source/Programs/NovaForgeEditor/main.cpp` and `NovaForgeGame/main.cpp`
have no real Win32 window creation. The editor shows nothing on Windows — it is
effectively headless.
**Action:** Replace both mains with full Win32 window creation (double-buffered GDI),
proper WndProc, message loop, and frame timing. Editor renders the full panel layout.
**Status:** 🔄 Addressed in this pass

### GAP 6 — Game Phases G18–G20 Not Implemented
**Severity:** HIGH
**Problem:** G18 (Status Effects), G19 (Contracts & Bounties), and G20 (Companion System)
are tracked in the roadmap but have no code or tests.
**Action:** Implement G18, G19, G20 in `Source/Game/include/NF/Game/Game.h`.
Add comprehensive tests to `Tests/Game/test_game.cpp`.
**Status:** 🔄 Addressed in this pass

### GAP 7 — Missing Architecture Documentation
**Severity:** MEDIUM
**Problem:** No canonical architecture documents exist in `Docs/Architecture/`.
The codebase has no single source of truth for naming, build modes, voxel pipeline,
hosted contract, or deferred workspace items.
**Action:** Create: NAMING_CANON.md, CURRENT_DIRECTION.md, HOSTED_PROJECT_CONTRACT.md,
BUILD_MODES.md, VOXEL_RENDER_PIPELINE.md, DEFERRED_TO_WORKSPACE.md.
**Status:** 🔄 Addressed in this pass

### GAP 8 — Missing Gameplay Documentation
**Severity:** MEDIUM
**Problem:** No `Docs/Gameplay/` directory or R.I.G. system specification exists.
**Action:** Create `Docs/Gameplay/RIG_SYSTEM.md`.
**Status:** 🔄 Addressed in this pass

### GAP 9 — Missing Systems Documentation
**Severity:** MEDIUM
**Problem:** No `Docs/Systems/` directory or GraphVM visual scripting documentation exists.
**Action:** Create `Docs/Systems/VISUAL_SCRIPTING.md`.
**Status:** 🔄 Addressed in this pass

### GAP 10 — Missing Directory Stubs
**Severity:** LOW
**Problem:** `Logs/`, `Codex/Snippets/`, `Codex/Fixes/`, `Codex/Graphs/`,
and `Content/Incoming/` are referenced in contracts and configs but do not exist
as tracked directories.
**Action:** Create `.gitkeep` files in each directory.
**Status:** 🔄 Addressed in this pass

### GAP 11 — Missing Logger Format and Season Config
**Severity:** LOW
**Problem:** `Logs/build.logger` format is undocumented. `Config/season.config.json`
does not exist despite season system being part of the game design.
**Action:** Create `Logs/logger.format.md` and `Config/season.config.json`.
**Status:** 🔄 Addressed in this pass

### GAP 12 — AtlasToolingSuite Reference in README
**Severity:** LOW
**Problem:** README.md references `AtlasToolingSuite` as a separate entity. The
canonical name is `Atlas Workspace`. The consolidation table still shows deprecated
repo names without proper canonical mapping.
**Action:** Surgical README.md updates — replace deprecated names, add fleet note,
remove ImGui references.
**Status:** 🔄 Addressed in this pass

---

## Documentation Reset Required

The following documents need to be created (none existed before this audit):

| File | Priority |
|---|---|
| `Docs/Architecture/NAMING_CANON.md` | P0 |
| `Docs/Architecture/CURRENT_DIRECTION.md` | P0 |
| `Docs/Architecture/HOSTED_PROJECT_CONTRACT.md` | P0 |
| `Docs/Architecture/BUILD_MODES.md` | P1 |
| `Docs/Architecture/VOXEL_RENDER_PIPELINE.md` | P1 |
| `Docs/Architecture/DEFERRED_TO_WORKSPACE.md` | P1 |
| `Docs/Gameplay/RIG_SYSTEM.md` | P1 |
| `Docs/Systems/VISUAL_SCRIPTING.md` | P1 |
| `Logs/logger.format.md` | P2 |

---

## Implementation Priority

| Priority | Items |
|---|---|
| P0 — Blockers | GAP 1 (naming), GAP 2 (manifest), GAP 3 (hosted build), GAP 4 (VS2022 test) |
| P1 — High Value | GAP 5 (Win32 window), GAP 6 (G18-G20 game phases), GAP 7-9 (docs) |
| P2 — Housekeeping | GAP 10 (stubs), GAP 11 (logger/season), GAP 12 (README) |

---

## Success Condition

The audit is resolved when:
- [ ] `cmake --preset debug -DNF_BUILD_TESTS=ON && cmake --build --preset debug` succeeds
- [ ] `ctest --preset debug` reports 0 failures
- [ ] No source file references ArbiterAI, SwissAgent, or ImGui outside Archive/
- [ ] `Project/project.atlas.json` exists and is valid JSON
- [ ] All 12 GAPs above are marked ✅

---

## Final Note

This audit was triggered by the discovery that the repo had grown to G17 of the game
phases without establishing the foundational architecture contracts that Atlas Workspace
requires for hosted deployment. The C0 consolidation phase created a strong code base.
This audit ensures the documentation and build infrastructure match the code quality.
