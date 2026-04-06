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
| **G21** — Faction System | FactionType×8, Faction, FactionTerritory, FactionRelation, FactionManager | ✅ Done |
| **G22** — Weather System | WeatherType×8, WeatherCondition, WeatherForecast, WeatherSystem | ✅ Done |
| **G23** — Trading System | TradeGoodCategory×8, TradingPost, TradeRoute, TradingSystem | ✅ Done |
| **G24** — Base Building | BasePartCategory×8, BaseLayout, BaseDefense, BaseSystem | ✅ Done |
| **G25** — Habitat System | HabitatZoneType×8, HabitatZone, LifeSupportModule, HabitatSystem | ✅ Done |
| **G26** — Power Grid System | PowerSourceType×8, PowerNode, PowerConduit, PowerGrid, PowerGridSystem | ✅ Done |
| **G27** — Vehicle System | VehicleType×8, VehicleSeat, VehicleComponent, Vehicle, VehicleSystem | ✅ Done |
| **G28** — Research System | ResearchCategory×8, ResearchProject, ResearchLab, ResearchTree, ResearchSystem | ✅ Done |
| **G29** — Diplomacy System | DiplomacyAction×8, DiplomaticStance×5, DiplomaticRelation, Treaty, DiplomaticChannel, DiplomacySystem | ✅ Done |
| **G30** — Espionage System | EspionageMissionType×8, SpyAgent, EspionageMission, IntelligenceNetwork, EspionageSystem | ✅ Done |
| **G31** — Colony Management | ColonyRole×8, Colonist, ColonyBuilding, Colony, ColonySystem | ✅ Done |
| **G32** — Archaeology System | ArtifactRarity×8, Artifact, ExcavationSite, ArtifactCollection, ArchaeologySystem | ✅ Done |
| **G33** — Migration System | MigrationTrigger×8, Migrant, MigrationWave, MigrationRoute, MigrationSystem | ✅ Done |
| **G34** — *(next)* | TBD — next locked game phase | ⬜ Queued |

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
| **M1** — Usable Editor | Real GLFW/ImGui window, viewport clear, basic panels | ✅ Done |
| **SP** — Spec Pack | 6 system contracts, 6 schemas, 25 docs, audit + roadmap | ✅ Done |
| **M2 / S1** — Dev World Editing | PCG tuning panel, entity placement, voxel paint, undo/redo | ✅ Done |
| **M3 / S2** — Play-in-Editor | EditorWorldSession, Play/Pause/Stop toolbar, PIE snapshot | ✅ Done |
| **M4 / S3** — Asset Pipeline | AssetDatabase (GUID), mesh/texture importers, content browser hot-reload | ✅ Done |
| **S4** — Blender Bridge | novaforge_bridge.py add-on (BG-1→5), engine-side auto-import | ✅ Done |
| **S5** — Character & Animation Suite | FPSHandRig, TwoJointIK, AnimationBlendGraph, CharacterGroundingSystem | ✅ Done |
| **S6** — PCG World Tuning | BiomePainter, structure seed overrides, ore-seam editor, PCG preview | ✅ Done |
| **S7** — Logic Wiring UI | Entity logic graphs (GraphVM), event pins, graph templates | ✅ Done |
| **S8** — Tool Ecosystem | SwissAgent, ArbiterAI, ContractScanner, ReplayMinimizer as real standalones | ✅ Done |
| **S9** — AtlasAI Integration | AIInsightType×8, AIAnalysisEngine, AIProactiveSuggester, AIPipelineBridge, AtlasAIIntegration | ✅ Done |
| **S10** — Performance Profiler | ProfileMetricType×8, ProfileSample, ProfileSession, FrameProfiler, MemoryProfiler, ProfilerTimeline, PerformanceProfiler | ✅ Done |
| **S11** — Live Collaboration | CollabUserRole×8, CollabEditType×8, CollabUser, CollabEditAction, CollabSession, CollabConflictResolver, LiveCollaborationSystem | ✅ Done |
| **S12** — Version Control Integration | VCSProviderType×8, VCSFileStatus×8, VCSCommitInfo, VCSBranchInfo, VCSDiffEntry, VCSRepository, VersionControlSystem | ✅ Done |
| **S13** — Localization System | LocaleId×8, LocalizedString, TranslationEntry, TranslationTable, LocaleManager, LocalizationSystem | ✅ Done |
| **S14** — Plugin System | PluginState×8, PluginManifest, PluginInstance, PluginRegistry, PluginLoader, PluginSystem | ✅ Done |
| **S15** — *(next)* | TBD — next workspace phase | 📋 Far future |

## Status Key

| Icon | Meaning |
|------|---------|
| ✅ | Done — implemented and tested |
| 🔧 | Active — in progress |
| 🔜 | Next — queued for implementation |
| 📋 | Planned — designed, not yet started |
