# NovaForge — Editor-First Roadmap

> This document tracks the **next milestones** from the current fully-implemented baseline
> (614 tests, G1–G17 complete, all 9 engine phases done, S0 pipeline core done)
> toward a shippable, usable editor-driven development environment.
>
> **Architecture principle:** Every tool (editor, Blender bridge, SwissAgent, ArbiterAI,
> ContractScanner, ReplayMinimizer) communicates exclusively through the shared
> `.novaforge/pipeline/` directory tree.  The editor is the only thing the user launches.
> AtlasAI (S9) sits at the far end, watching the pipeline and eventually orchestrating all tools.

---

## Milestone Overview

| Milestone | Goal | Status |
|-----------|------|--------|
| **S0** — Pipeline Core | `NF::Pipeline`: PipelineWatcher, Manifest, WatchLog, ChangeEvent | ✅ Done |
| **M1** — Usable Editor | Real window, ImGui docking, live viewport, panel interaction | 🔜 Next |
| **M2 / S1** — Dev World Editing | Place/remove voxels, entity placement, PCG panel, undo/redo | 📋 Planned |
| **M3 / S2** — Play-in-Editor (PIE) | Start/stop game session inside editor, sync transforms | 📋 Planned |
| **M4 / S3** — Asset Pipeline | Import meshes/textures, GUID DB, dependency graph, hot-reload | 📋 Planned |
| **S4** — Blender Bridge | Two-way mesh/rig/animation sync via pipeline | 📋 Planned |
| **S5** — Character & Animation Suite | FPS hands, IK, blend graphs, character grounding | 📋 Planned |
| **S6** — PCG World Tuning | Biome painter, structure seed overrides, ore-seam editor | 📋 Planned |
| **S7** — Logic Wiring UI | Entity logic graphs (GraphVM), event/action pins, templates | 📋 Planned |
| **S8** — Tool Ecosystem | SwissAgent, ArbiterAI, ContractScanner, ReplayMinimizer as standalones | 📋 Planned |
| **M5** — Content Tools Polish | Live reload, hot-swap shaders, content browser D&D | 📋 Planned |
| **M6** — Ship to Steam / Itch | Packaging, installer, update mechanism | 📋 Planned |
| **S9** — AtlasAI Integration | Pipeline intelligence, proactive suggestions, tool orchestration | 📋 Far future |

---

## S0 — Pipeline Core ✅

**Delivered:**
- `Source/Pipeline/` module (`NF::Pipeline`) wired into root CMake and Editor
- `ChangeEventType` enum (7 values) with name/fromString helpers
- `ChangeEvent` struct — `toJson` / `fromJson` / `writeToFile` / `readFromFile`
- `Manifest` class — GUID registry, save/load to `manifest.json`, findByGuid/Path, remove
- `WatchLog` — thread-safe append-only log
- `PipelineWatcher` — polling-based watcher with subscribe/poll/start/stop
- `PipelineDirectories` — derives all pipeline paths from workspace root, `ensureCreated()`
- 17 Catch2 tests, 96 assertions — all passing

**Pipeline directory layout established:**
```
<workspace>/.novaforge/
  pipeline/
    changes/       ← .change.json events (all tools write here)
    assets/        ← imported/generated assets
    worlds/        ← world-state snapshots
    scripts/       ← GraphVM bytecode / logic graphs
    animations/    ← rigs, clips, IK configs
    sessions/      ← AtlasAI session logs (S9)
  manifest.json    ← GUID → asset registry
  watch.log        ← append-only event log
```

---

## M1 — Usable Editor

**Goal:** A real, interactive editor window you can open and use on Windows (VS 2022). Everything that exists today compiles and links; M1 wires it to an actual OS window with GPU rendering.

### Tasks

- [ ] Integrate GLFW or Win32 window creation into `NovaForgeEditor` main
- [ ] Bootstrap ImGui docking layout (or custom UI renderer already present) inside the editor window
- [ ] Render a clear-color viewport via the existing `Renderer` module (OpenGL 3.3)
- [ ] Display basic panels: **Hierarchy** (scene outliner stub), **Inspector** (property stub), **Viewport**
- [ ] Frame delta-time loop with vsync toggle
- [ ] Log all editor startup messages to the **Console** panel
- [ ] Keyboard shortcut: `Ctrl+Q` to quit cleanly
- [ ] CI: Windows build job passes (already wired in `.github/workflows/ci.yml`)

### Acceptance Criteria
- `NovaForgeEditor.exe` opens a 1280×720 window on Windows 10/11
- Viewport clears to a solid colour every frame at ≥ 30 FPS
- Hierarchy and Inspector panels are visible (empty/stub OK)
- No crash on startup or shutdown

---

## M2 / S1 — Dev World Editing

**Goal:** An empty scene/world that you can actually edit — place and remove voxel blocks, spawn entities, edit component properties, undo/redo, and tweak PCG parameters.

### Tasks

- [ ] Load `Content/Definitions/DevWorld.json` on editor startup
- [ ] Scene outliner: display entities from active `Level`
- [ ] Viewport: left-click to select entity, highlight selection with bounding box
- [ ] Place/remove voxels via `ChunkEditAPI` from the viewport
- [ ] **PCG Tuning Panel**: sliders for biome noise octaves, ore density, structure spawn weight; writes params to `pipeline/worlds/active.world.json` on change
- [ ] **Entity Placement Tool**: entity palette → click in viewport → spawned at surface with terrain-normal orientation
- [ ] **Prop Alignment**: snap entity to terrain surface normal
- [ ] Inspector: display and edit `Transform`, `Name`, and one custom component
- [ ] Undo/Redo: `Ctrl+Z` / `Ctrl+Y` using existing `CommandHistory`
- [ ] Save/load world state (JSON round-trip via `GameSaveSerializer`); autosave every 5 min

### Acceptance Criteria
- Place ≥ 10 voxel blocks in the viewport
- Select an entity, edit its position, and undo the change
- Save the scene and reload it with entity positions preserved

---

## M3 / S2 — Play-in-Editor (PIE)

**Goal:** Press **Play** in the editor, run the game loop inside the editor window, and press **Stop** to return to editing with the world state restored.

### Tasks

- [ ] `EditorWorldSession` API: `startPlayInEditor()` / `stopPlayInEditor()`
- [ ] Serialize world snapshot before PIE starts
- [ ] Restore snapshot when PIE stops
- [ ] Viewport: switch to game camera when PIE is active
- [ ] Toolbar: **▶ Play / ⏸ Pause / ⏹ Stop** buttons
- [ ] Input: route raw OS events to game `InputSystem` during PIE
- [ ] Physics: step `PhysicsWorld` in game loop; freeze in edit mode

### Acceptance Criteria
- Enter PIE: `PlayerController` accepts WASD input, physics runs
- Stop PIE: entities return to pre-play positions
- No leak / crash across multiple PIE cycles

---

## M4 / S3 — Asset Pipeline

**Goal:** Import external assets (glTF meshes, PNG textures) via the editor, assign stable GUIDs, and reference them from scene files.  `PipelineWatcher` drives hot-reload.

### Tasks

- [ ] `AssetDatabase`: GUID → metadata registry backed by `manifest.json` (uses `Manifest` from S0)
- [ ] Import mesh: glTF/OBJ → `MeshAsset` (CPU buffer + GPU upload)
- [ ] Import texture: PNG/TGA → `TextureAsset` via stb_image
- [ ] Content browser: shows imported assets with thumbnail icons
- [ ] Drag asset from content browser into viewport to spawn mesh entity
- [ ] Scene serialization: store asset GUIDs, resolve on load
- [ ] Dependency graph: re-import when source file changes (uses `PipelineWatcher`)
- [ ] Any asset dropped into `pipeline/assets/` from Blender is auto-imported

### Acceptance Criteria
- Import a glTF cube, see it in content browser, drag it into viewport
- Save scene; reload; cube appears at same position

---

## S4 — Blender Bridge

**Goal:** Two-way mesh, rig, and animation sync between Blender and the NovaForge editor via the pipeline.

### Sub-milestones

| ID   | Goal |
|------|------|
| BG-1 | ✅ Add-on scaffold (`novaforge_bridge.py`), N-panel, Export Mesh + Export Rig + Export Anim Clip |
| BG-2 | Rig/skin export + animation clip round-trip to `pipeline/animations/` |
| BG-3 | Import-back: engine-side watch of `pipeline/assets/` → `AssetDatabase` auto-import |
| BG-4 | Procedural generation scripts: auto-generate ship hulls, interiors, terrain props from editor params |
| BG-5 | Material baking: PBR bake → export normal/albedo/roughness textures to pipeline |

---

## S5 — Character & Animation Suite

### Sub-milestones

| ID   | Goal |
|------|------|
| AN-1 | `FPSHandRig` struct + `HandRigEditor` panel (camera-relative bone poses, save as named clip) |
| AN-2 | `TwoJointIK` analytic solver wired into PIE player controller |
| AN-3 | `AnimationClipLibrary` backed by `manifest.json`; Blender export round-trip |
| AN-4 | `AnimationBlendGraph` blend tree (2D blend space via GraphVM) |
| AN-5 | `CharacterGroundingSystem`: ray-down → adjust root height + tilt to terrain normal |
| AN-6 | `IKEditor` viewport overlay: colored bone lines, drag IK target in real-time |

---

## S6 — PCG World Tuning

| Task | Goal |
|------|------|
| BiomePainter | Brush tool to paint biome zones; writes biome mask to `pipeline/worlds/` |
| Structure Seed Override | Right-click empty space → "Force Structure Here" → baked into world JSON |
| Ore Seam Editor | Paint ore density volumes in 3D viewport |
| PCG Preview Mode | Split-screen: current world vs. proposed PCG params |

---

## S7 — Logic Wiring UI

| Task | Goal |
|------|------|
| Asset Logic Binding | Right-click entity → "Open Logic Graph" → GraphVM graph with event/action pins |
| New Asset + Logic flow | Import mesh → viewport → collision → logic graph → PIE test, all in editor |
| Logic Graph Templates | Preset graphs for "door", "loot crate", "NPC merchant", "turret" |

---

## S8 — Tool Ecosystem Standalones

See `Tools/<ToolName>/README.md` for the per-tool sub-milestone roadmaps.

| Tool | Path | Primary use |
|------|------|-------------|
| **SwissAgent** | `Tools/SwissAgent/` | Natural-language code queries + generation → pipeline/changes/ |
| **ArbiterAI** | `Tools/ArbiterAI/` | Rule-based balance checker → pipeline/changes/ |
| **BlenderGenerator** | `Tools/BlenderGenerator/` | Blender add-on; mesh/rig/anim export → pipeline/assets/ |
| **ContractScanner** | `Tools/ContractScanner/` | C++ static analysis → pipeline/changes/ |
| **ReplayMinimizer** | `Tools/ReplayMinimizer/` | Debug trace reduction → pipeline/replays/ |

---

## S9 — AtlasAI Integration

AtlasAI is **not** a separate app.  It is the intelligence layer that wraps the entire pipeline
once S0–S8 are fully operational.  It has **read-only access** to the pipeline until AT-4.

| ID   | Goal |
|------|------|
| AT-1 | `AtlasSession`: reads `watch.log`, builds event timeline, exposes query API |
| AT-2 | Project-state model: current world, recent changes, test status, open issues |
| AT-3 | Passive suggestions: surface ContractScanner + ArbiterAI findings in one unified panel |
| AT-4 | Tool orchestration: given a goal, plan and invoke multiple tools in sequence via pipeline |
| AT-5 | Learning: weight suggestions by developer accept/reject history |
| AT-6 | Full autonomous loop: draft a complete feature (code + asset + logic) for human review |

---

## M5 — Content Tools Polish

### Tasks

- [ ] Shader hot-reload: `F5` recompiles all GLSL shaders without restart
- [ ] Asset hot-reload: detect file-system changes via `PipelineWatcher`
- [ ] Content browser: filter by type, search by name, context-menu for import/delete
- [ ] Live profiler panel: frame time graph, CPU/GPU breakdown (already scaffolded)
- [ ] Output log: categorised, colour-coded, filterable by level
- [ ] Drag-and-drop: drag scene asset from content browser onto hierarchy

---

## M6 — Ship to Steam / Itch

### Tasks

- [ ] CMake install target: copies binaries + `Content/` to `install/`
- [ ] Release CI job: builds Release, zips `install/`, uploads GitHub Release artifact
- [ ] Windows installer script (NSIS or WiX) or itch.io butler upload
- [ ] Version stamping: `NF_VERSION_STRING` embedded in executable

---

## Design Principles

| Principle | Rule |
|-----------|------|
| Pipeline-only IPC | Tools communicate **only** through `.novaforge/pipeline/` — no sockets, no shared memory |
| Event-first | Every tool write goes to `pipeline/changes/` first — AtlasAI can replay full history |
| Editor-first | All game features accessible from the editor before they are in-game |
| Blender for 3D | Don't build modeling inside NovaForge — own the pipeline, not the DCC |
| AtlasAI read-only | It observes first; only orchestrates tools once proven as a passive watcher (AT-4) |
| No hard Windows paths | Use `std::filesystem` and CMake paths; POSIX portability maintained |
| Test everything | New systems need Catch2 tests before merging |
| Phases over features | Complete one milestone before starting the next |
| Data-driven | Game content lives in JSON under `Content/` and `Data/` |
