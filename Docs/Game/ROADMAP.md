# NovaForge — Phase-by-Phase Roadmap

> MasterRepo (v001) is the structural canonical reference. All phases are locked deliverables.
> tempnovaforge is the consolidation target — it is ahead of MasterRepo on implementation.

---

## Engine & Editor Phases

| Phase | Goal | Status |
|-------|------|--------|
| **Phase 0** — Bootstrap | Project scaffold, build system, directory structure, CMake, presets, vcpkg | ✅ Done |
| **Phase 1** — Core Engine | Core math/memory/reflection/serialization, Engine ECS, Input module | ✅ Done |
| **Phase 2** — Rendering & Physics | OpenGL RHI, physics rigid bodies, audio device, animation skeleton | ✅ Done |
| **Phase 3** — Voxel Runtime | Chunk data, ChunkMesher, WorldState, VoxelPickService, game loop scaffold | ✅ Done |
| **Phase 4** — Editor | Docking layout, 7-panel UI, viewport, inspector, project path service | ✅ Done |
| **Phase 5** — Graph VM | Deterministic bytecode VM, 14 graph types, visual scripting compiler | ✅ Done |
| **Phase 6** — Multiplayer | Server authority, replication, sessions, lockstep/rollback networking | ✅ Done |
| **Phase 7** — AI & Tooling | SwissAgent, ArbiterAI, BlenderGenerator, ContractScanner, ReplayMinimizer | ✅ Done |
| **Phase 8** — Custom IDE | Project-aware IDE: ProjectIndexer, CodeNavigator, BreadcrumbTrail, IDEPanel | ✅ Done |
| **Phase 9** — Pipeline Core | NF::Pipeline: PipelineWatcher, Manifest, WatchLog, ChangeEvent (S0) | ✅ Done |
| **Phase 10** — Polish & CI | Documentation, GitHub Actions, Docker, modding guide, final audit | ✅ Done |
| **Phase 11** — Suite Integration | Full tool suite wired through pipeline; AtlasAI broker enabled | ✅ Done |

---

## Game Phases (Nova Forge)

> Game phases use the G-series prefix. Each phase has a locked deliverable.

| Phase | Goal | Status |
|-------|------|--------|
| **G1** — First Interaction Loop | R.I.G. state, mining tool, resources, inventory, HUD, GameSession | ✅ Done |
| **G2** — Voxel Mesh Rendering | ChunkRenderer, VoxelShader, Frustum, ChunkRenderCache, lit terrain | ✅ Done |
| **G3** — Movement & FPS Camera | FPSCamera, PlayerMovement, VoxelCollider, PlayerController | ✅ Done |
| **G4** — Ship Systems | ShipClass, ShipModule, Ship, FlightController, CombatSystem | ✅ Done |
| **G5** — Fleet AI | Formation, CaptainPersonality, AICaptain, Fleet | ✅ Done |
| **G6** — Economy | Mining, refining, manufacturing, market pricing | ✅ Done |
| **G7** — Exploration | ProbeScanner, WormholeNetwork, AncientTechRegistry | ✅ Done |
| **G8** — FPS Interiors | ShipRoom, ShipInterior, EVAState, SurvivalStatus | ✅ Done |
| **G9** — Legend System | PlayerReputation, WorldBiasMap, NPCMemory, LegendStatus | ✅ Done |
| **G10** — Quest & Missions | MissionObjective, ActiveMission, MissionLog, QuestChain | ✅ Done |
| **G11** — Dialogue System | DialogueCondition, DialogueNode, DialogueGraph, DialogueRunner | ✅ Done |
| **G12** — Save/Load System | SaveSlot, SaveData, GameSaveSerializer, SaveSystem with auto-save | ✅ Done |
| **G13** — World Events | WorldEventType×7, EventEffect, WorldEvent, WorldEventSystem | ✅ Done |
| **G14** — Tech Tree | TechCategory×7, TechNode, TechTree with prereqs, tier bonuses | ✅ Done |
| **G15** — Player Progression | PlayerLevel (cap 50), SkillNode, SkillTree, ProgressionSystem | ✅ Done |
| **G16** — Crafting System | CraftingRecipe, CraftingQueue FIFO, CraftingSystem level-gated | ✅ Done |
| **G17** — Inventory & Equipment | ItemRarity×5, PlayerInventory (stacking), EquipmentLoadout | ✅ Done |
| **G18** — Status Effects | StatusEffectType×8, AilmentStack, StatusEffectSystem | ✅ Done |
| **G19** — Contracts & Bounties | ContractType×6, Contract lifecycle, BountyTarget, ContractBoard | ✅ Done |
| **G20** — Companion System | CompanionRole×6, CompanionPersonality, CompanionManager (max 4) | ✅ Done |
| **G21** — *(next)* | TBD — next locked game phase | ⬜ Queued |

---

## Workspace Suite Milestones (S-series)

> All tools communicate only through the shared `.novaforge/pipeline/` directory.
> The editor is the sole user-facing entry point; all tools run headlessly.

| Milestone | Goal | Status |
|-----------|------|--------|
| **S0** — Pipeline Core | NF::Pipeline: PipelineWatcher, Manifest, WatchLog, ChangeEvent. 17 tests. | ✅ Done |
| **S1** — Tool Wiring | All 5 tools respond to pipeline ChangeEvents end-to-end. 17 tests. | ✅ Done |
| **S2** — BlenderGen Bridge | BG-1→5 fully wired through pipeline into editor asset pipeline | ✅ Done |
| **S3** — SwissAgent Integration | SA-1→5 workspace broker functional in editor | ✅ Done |
| **S4** — ArbiterAI Integration | AB-1→5 AI reasoning broker routes through AtlasAI/ | ✅ Done |
| **S5** — Full Suite Validation | All tools active simultaneously, CI passes with suite tests | ✅ Done |

---

## Repo Consolidation Milestones

| Milestone | Source Repo | Status |
|-----------|-------------|--------|
| **C0** — MasterRepo seed | MasterRepo (v001) | ✅ Done |
| **C1** — MasterRepoRefactor | Structure + Atlas dirs | ✅ Done |
| **C2** — AtlasToolingSuite | Full tool suite | ✅ Done |
| **C3** — Nova-Forge-Expeditions | Richest game codebase | ⬜ Queued |
| **C4–C11** — Remaining repos | See `Docs/CONSOLIDATION_PLAN.md` | ⬜ Queued |
| **M1** — Usable Editor | Real GLFW/ImGui window, viewport clear, basic panels | 🔜 Next |
| **M2 / S1** — Dev World Editing | PCG tuning panel, entity placement, voxel paint, undo/redo | 📋 Planned |
| **M3 / S2** — Play-in-Editor | EditorWorldSession, Play/Pause/Stop toolbar, PIE snapshot | 📋 Planned |
| **M4 / S3** — Asset Pipeline | AssetDatabase (GUID), mesh/texture importers, content browser hot-reload | 📋 Planned |
| **S4** — Blender Bridge | novaforge_bridge.py add-on (BG-1→5), engine-side auto-import | 📋 Planned |
| **S5** — Character & Animation Suite | FPSHandRig, TwoJointIK, AnimationBlendGraph, CharacterGroundingSystem | 📋 Planned |
| **S6** — PCG World Tuning | BiomePainter, structure seed overrides, ore-seam editor, PCG preview | 📋 Planned |
| **S7** — Logic Wiring UI | Entity logic graphs (GraphVM), event pins, graph templates | 📋 Planned |
| **S8** — Tool Ecosystem | SwissAgent, ArbiterAI, ContractScanner, ReplayMinimizer as real standalones | 📋 Planned |
| **S9** — AtlasAI Integration | Pipeline intelligence, proactive suggestions, tool orchestration | 📋 Far future |

## Status Key

| Icon | Meaning |
|------|---------|
| ✅ | Done — implemented and tested |
| 🔧 | Active — in progress |
| 🔜 | Next — queued for implementation |
| 📋 | Planned — designed, not yet started |
