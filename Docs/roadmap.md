# NovaForge — Editor-First Roadmap

> This document tracks the **next milestones** from the current fully-implemented baseline (599 tests, G1–G17 complete, all 9 engine phases done) toward a shippable, usable editor-driven development environment.

---

## Milestone Overview

| Milestone | Goal | Status |
|-----------|------|--------|
| **M1** — Usable Editor | Real window, ImGui docking, live viewport, panel interaction | 🔜 Next |
| **M2** — Dev World Editing | Place/remove voxels, entity placement, property edits, undo/redo | 📋 Planned |
| **M3** — Play-in-Editor (PIE) | Start/stop game session inside editor, sync transforms | 📋 Planned |
| **M4** — Asset Pipeline | Import meshes/textures, asset DB with GUIDs, dependency graph | 📋 Planned |
| **M5** — Content Tools Polish | Live reload, hot-swap shaders, content browser drag-and-drop | 📋 Planned |
| **M6** — Ship to Steam / Itch | Packaging, installer, update mechanism | 📋 Planned |

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

## M2 — Dev World Editing

**Goal:** An empty scene/world that you can actually edit — place and remove voxel blocks, spawn entities, edit component properties, and undo/redo your changes.

### Tasks

- [ ] Load `Content/Definitions/DevWorld.json` on editor startup
- [ ] Scene outliner: display entities from active `Level`
- [ ] Viewport: left-click to select entity, highlight selection with bounding box
- [ ] Place/remove voxels via `ChunkEditAPI` from the viewport
- [ ] Inspector: display and edit `Transform`, `Name`, and one custom component
- [ ] Undo/Redo: `Ctrl+Z` / `Ctrl+Y` using existing `CommandHistory`
- [ ] Save/load world state (JSON round-trip via `GameSaveSerializer`)
- [ ] Camera: WASD fly-cam with right-click activation (already in `ViewportCameraController`)

### Acceptance Criteria
- Place ≥ 10 voxel blocks in the viewport
- Select an entity, edit its position, and undo the change
- Save the scene and reload it with entity positions preserved

---

## M3 — Play-in-Editor (PIE)

**Goal:** Press **Play** in the editor, run the game loop inside the editor window, and press **Stop** to return to editing with the world state restored.

### Tasks

- [ ] `EditorWorldSession` API: `startPlayInEditor()` / `stopPlayInEditor()`
- [ ] Serialize world snapshot before PIE starts
- [ ] Restore snapshot when PIE stops
- [ ] Viewport: switch to game camera when PIE is active
- [ ] Toolbar: **Play / Pause / Stop** buttons
- [ ] Input: route raw OS events to game `InputSystem` during PIE
- [ ] Physics: step `PhysicsWorld` in game loop; freeze in edit mode

### Acceptance Criteria
- Enter PIE: `PlayerController` accepts WASD input, physics runs
- Stop PIE: entities return to pre-play positions
- No leak / crash across multiple PIE cycles

---

## M4 — Asset Pipeline

**Goal:** Import external assets (glTF meshes, PNG textures) via the editor, assign stable GUIDs, and reference them from scene files.

### Tasks

- [ ] `AssetDatabase`: GUID → metadata registry backed by JSON manifest
- [ ] Import mesh: glTF/OBJ → `MeshAsset` (CPU buffer + GPU upload)
- [ ] Import texture: PNG/TGA → `TextureAsset` via stb_image
- [ ] Content browser: shows imported assets with thumbnail icons
- [ ] Drag asset from content browser into viewport to spawn mesh entity
- [ ] Scene serialization: store asset GUIDs, resolve on load
- [ ] Dependency graph: re-import when source file changes (file watcher)

### Acceptance Criteria
- Import a glTF cube, see it in content browser, drag it into viewport
- Save scene; reload; cube appears at same position

---

## M5 — Content Tools Polish

**Goal:** A smooth content iteration loop — hot-reload shaders, live reload assets, content browser drag-and-drop, and basic profiling overlay.

### Tasks

- [ ] Shader hot-reload: `F5` recompiles all GLSL shaders without restart
- [ ] Asset hot-reload: detect file-system changes via platform watcher
- [ ] Content browser: filter by type, search by name, context-menu for import/delete
- [ ] Live profiler panel: frame time graph, CPU/GPU breakdown (already scaffolded)
- [ ] Output log: categorised, colour-coded, filterable by level
- [ ] Drag-and-drop: drag scene asset from content browser onto hierarchy

---

## M6 — Ship to Steam / Itch

**Goal:** Package `NovaForgeGame.exe` and its data into a distributable archive.

### Tasks

- [ ] CMake install target: copies binaries + `Content/` to `install/`
- [ ] Release CI job: builds Release, zips `install/`, uploads GitHub Release artifact
- [ ] Windows installer script (NSIS or WiX) or itch.io butler upload
- [ ] Version stamping: `NF_VERSION_STRING` embedded in executable

---

## Design Principles (Reminder)

| Principle | Rule |
|-----------|------|
| Editor-first | All game features are accessible from the editor before they are accessible in-game |
| No hard Windows paths | Use `std::filesystem` and CMake paths; POSIX portability maintained |
| Test everything | New gameplay systems need Catch2 tests before merging |
| Phases over features | Complete one milestone before starting the next |
| Data-driven | Game content lives in JSON under `Content/` and `Data/` |
