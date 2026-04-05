# NovaForge — Master Convergence Roadmap
**Last Updated:** 2026-04-05  
**Active Repo:** `shifty81/tempnovaforge`  
**Strategy:** All repos converge into `tempnovaforge`. Once fully merged, each repo is archived with a summary + code snippets in `Archive/`. Standalone repos may be re-extracted after full convergence is complete.

---

## Naming Canon

| Old Name         | New Name              | Location               |
|------------------|-----------------------|------------------------|
| ArbiterAI        | Atlas_Arbiter         | Tools/AtlasAI/         |
| Arbiter          | Atlas_Arbiter         | Tools/AtlasAI/         |
| SwissAgent       | Atlas_SwissAgent      | Tools/AtlasAI/         |
| BlenderGenerator | Atlas_BlenderGenerator| Tools/BlenderGenerator/|
| ContractScanner  | Atlas_ContractScanner | Tools/ContractScanner/ |
| ReplayMinimizer  | Atlas_ReplayMinimizer | Tools/ReplayMinimizer/ |

All AI features: prefix `Atlas_` or namespace `AtlasAI::`  
All engine C++ classes: prefix `NF` (NovaForge) or `Atlas`  

---

## Phase 0 — Bootstrap & Convergence Setup ✅ IN PROGRESS

- [x] `tempnovaforge` base structure established
- [x] CMakeLists.txt + CMakePresets.json scaffolded
- [x] vcpkg.json base created
- [x] Docs structure created (Architecture, AI, Engine, Editor, Game, Systems, Design)
- [x] Tools stubs created (ArbiterAI→AtlasAI, SwissAgent, BlenderGenerator, ContractScanner, ReplayMinimizer)
- [ ] vcpkg.json fully expanded (all engine deps)
- [ ] ImGui integration fixed in CMakeLists.txt
- [ ] MasterRepo source migrated into Source/
- [ ] AtlasForge engine code reviewed and merged
- [ ] MasterRepoRefactor reviewed and merged
- [ ] All AI repos consolidated into Tools/AtlasAI/

---

## Phase 1 — Source Consolidation (All Repos → tempnovaforge)

### Engine Core (from AtlasForge + MasterRepo + Atlas-NovaForge)
- [ ] Source/Core migrated and spaghetti-audited
- [ ] Source/Engine migrated and spaghetti-audited
- [ ] Source/Renderer migrated (Vulkan/DX12 paths unified)
- [ ] Source/Audio migrated (OpenAL)
- [ ] Source/Physics migrated (Bullet3)
- [ ] Source/Input migrated
- [ ] Source/Networking migrated
- [ ] Source/Animation migrated
- [ ] Source/UI migrated (ImGui integration)
- [ ] Source/AI stubs created → AtlasAI runtime

### Game Layer (from NovaForge-Project + Nova-Forge-Expeditions + MasterRepo)
- [ ] Source/Game migrated
- [ ] Content/Definitions migrated
- [ ] Config/novaforge.project.json reconciled
- [ ] Docs/Game docs migrated from MasterRepo

### Editor (from MasterRepo editor progress — MasterRepoV001)
- [ ] Source/Editor migrated
- [ ] Source/Programs migrated
- [ ] Editor build target verified in CMakeLists.txt

### Tools (from AtlasToolingSuite + SwissAgent + ArbiterAI + Arbiter + BlenderGenerator)
- [ ] Tools/AtlasAI/Atlas_Arbiter source migrated
- [ ] Tools/AtlasAI/Atlas_SwissAgent source migrated
- [ ] Tools/BlenderGenerator source migrated from shifty81/Blender-Generator-for-AtlasForge
- [ ] Tools/ContractScanner reviewed
- [ ] Tools/ReplayMinimizer reviewed

---

## Phase 2 — Refactor & Spaghetti Audit

- [ ] Full codebase audit: identify circular dependencies
- [ ] Enforce layering: Engine → Game → Editor (no reverse deps)
- [ ] Naming pass: all AI symbols renamed to AtlasAI:: / Atlas_ prefix
- [ ] Remove dead code identified in audit
- [ ] CMake module graph cleaned up
- [ ] All Python tooling moved to Tools/ (none in Source/)
- [ ] GLSL shaders moved to Content/Shaders/

---

## Phase 3 — PCG & Voxel Runtime

- [ ] Voxel chunk system implemented (Source/World/)
- [ ] Heightmap + biome pass
- [ ] Structure generator
- [ ] PCG loot tables
- [ ] Integration with Atlas_BlenderGenerator for spaceship spawning

---

## Phase 4 — AtlasAI Runtime Integration

- [ ] Atlas_NPCController wired into game loop
- [ ] Atlas_Pathfinding (navmesh or flow field)
- [ ] Atlas_BehaviourTree
- [ ] Atlas_Dialogue (LLM-assisted, optional)
- [ ] Tools/AtlasAI connected to engine via IPC or embedded Python

---

## Phase 5 — Archive & Cleanup

For each source repo, once fully merged:
1. All source code migrated to tempnovaforge
2. Summary doc created in `Archive/<RepoName>/ARCHIVE_SUMMARY.md`
3. Key code snippets saved in `Archive/<RepoName>/Snippets/`
4. Repo description updated to "ARCHIVED — merged into shifty81/tempnovaforge"
5. Local clone removed

### Archive Checklist

| Repo                              | Migrated | Archived |
|-----------------------------------|----------|----------|
| shifty81/Atlas-NovaForge          | [ ]      | [ ]      |
| shifty81/AtlasForge               | [ ]      | [ ]      |
| shifty81/MasterRepo               | [ ]      | [ ]      |
| shifty81/AtlasToolingSuite        | [ ]      | [ ]      |
| shifty81/MasterRepoRefactor       | [ ]      | [ ]      |
| shifty81/SwissAgent               | [ ]      | [ ]      |
| shifty81/Nova-Forge-Expeditions   | [ ]      | [ ]      |
| shifty81/NovaForge-Project        | [ ]      | [ ]      |
| shifty81/AtlasForge-EveOffline    | [ ]      | [ ]      |
| shifty81/Blender-Generator-for-AtlasForge | [ ] | [ ]  |
| shifty81/ArbiterAI                | [ ]      | [ ]      |
| shifty81/Arbiter                  | [ ]      | [ ]      |

---

## Phase 6 — Standalone Extraction (Post-Convergence)

Once tempnovaforge is complete, consider extracting standalone repos for:
- `AtlasEngine` — pure engine, no game
- `AtlasAI` — standalone AI tooling suite
- `NovaForge` — the game itself (uses AtlasEngine)

These are **future decisions only**. Do not create these repos until Phase 5 is complete.

---

## Gap Analysis — Missing Systems (Not Yet in Any Repo)

| Gap                              | Priority | Notes                                      |
|----------------------------------|----------|--------------------------------------------|
| Save/Load system                 | HIGH     | No serialization layer exists yet          |
| Asset pipeline (cook + package)  | HIGH     | No asset cooker, raw assets only           |
| Localization system              | MED      | No i18n layer                              |
| Achievement / stats system       | MED      | No player progression backend              |
| Crash reporter                   | MED      | No telemetry / crash capture               |
| Mod support                      | LOW      | Planned post-launch                        |
| Console platform layer           | LOW      | PC-first, console deferred                 |
| Automated test harness           | MED      | Tests/ dir exists but empty                |
| CI/CD pipeline                   | MED      | .github/ exists but needs workflow files   |
| Dedicated server build target    | MED      | Networking exists, no server binary target |
