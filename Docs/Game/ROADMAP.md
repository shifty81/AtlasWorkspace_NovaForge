# NovaForge — Phase-by-Phase Roadmap

> Aligned to MasterRepoV001 phase structure. Each phase has tight, locked deliverables.

## Engine & Editor Phases

| Phase | Goal | Status |
|-------|------|--------|
| **Phase 0** — Bootstrap | Project scaffold, build system, directory structure | ✅ Done |
| **Phase 1** — Core Engine | Core, Engine, Input modules compiling with tests | ✅ Done |
| **Phase 2** — Rendering & Physics | OpenGL RHI, physics, audio, animation | ✅ Done |
| **Phase 3** — Voxel Runtime | Chunk data, edit API, serialization, game loop | ✅ Done |
| **Phase 4** — Editor | Docking layout, panels, viewport, editor services | ✅ Done |
| **Phase 5** — Graph VM | Deterministic bytecode VM, 14 graph types, visual scripting | ✅ Done |
| **Phase 6** — Multiplayer | Server authority, replication, sessions, lockstep/rollback | ✅ Done |
| **Phase 7** — AI & Tooling | SwissAgent, ArbiterAI, Blender generator integration | ✅ Done |
| **Phase 8** — Custom IDE | Project-aware IDE with cross-module awareness | ✅ Done |
| **Phase 9** — Polish & CI | Documentation, GitHub Actions, Docker, modding guide | ✅ Done |

## Game Phases (Nova Forge)

| Phase | Goal | Status |
|-------|------|--------|
| **G1** — First Interaction Loop | R.I.G. state, mining tool, resources, inventory, HUD | ✅ Done |
| **G2** — Voxel Mesh Rendering | Mesher, GPU cache, Phong shader, lit terrain | ✅ Done |
| **G3** — Movement & FPS Camera | WASD + mouse look, jump, sprint, voxel collision | ✅ Done |
| **G4** — Ship Systems | Ship classes, modules, combat, flight controls | ✅ Done |
| **G5** — Fleet AI | AI captains, personality, morale, formation | ✅ Done |
| **G6** — Economy | Mining, refining, manufacturing, market | ✅ Done |
| **G7** — Exploration | Probe scanning, wormholes, ancient tech | ✅ Done |
| **G8** — FPS Interiors | Walkable ships, EVA, survival mechanics | ✅ Done |
| **G9** — Legend System | Player reputation, world bias, NPC memory | ✅ Done |
| **G10** — Quest & Mission System | Mission objectives, rewards, quest chains | ✅ Done |
| **G11** — Dialogue System | Branching dialogue, conditions, effects | ✅ Done |
| **G12** — Save/Load System | Full game state serialization, auto-save, 5 slots | ✅ Done |
| **G13** — World Events | Dynamic sector events, severity effects, expiry | ✅ Done |
| **G14** — Tech Tree | Category-tiered research tree with prereqs and bonuses | ✅ Done |
| **G15** — Player Progression | XP, levelling (cap 50), skill tree, stat bonuses | ✅ Done |
| **G16** — Crafting System | Recipes, ingredients, FIFO crafting queue, level-gated | ✅ Done |
| **G17** — Inventory & Equipment | Stackable items, rarity, slot-based equipment, stat bonuses | ✅ Done |

## Workspace Suite Milestones (S-series)

> All tools report back to the suite through the shared pipeline.
> The editor is the only thing the user launches; all other tools run
> headlessly or are embedded as panels.

| Milestone | Goal | Status |
|-----------|------|--------|
| **S0** — Pipeline Core | `NF::Pipeline` module: PipelineWatcher, Manifest, WatchLog, ChangeEvent. 17 tests. | ✅ Done |
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
