# NovaForge — Task List

## Phase 0 — Bootstrap (Current)

- [x] Create unified directory tree
- [x] Create root CMakeLists.txt (C++20, modular, build options)
- [x] Create CMakePresets.json (Debug/Release presets)
- [x] Create vcpkg.json (dependency manifest)
- [x] Create .gitignore
- [x] Create comprehensive README.md
- [x] Create Config/novaforge.project.json
- [x] Create Content/Definitions/DevWorld.json
- [x] Create Makefile (dev task shortcuts)
- [x] Create Dockerfile (container build)
- [x] Create per-module CMakeLists.txt stubs for all Source/ modules
- [x] Create key docs (ROADMAP, ARCHITECTURE, PROJECT_RULES, TASKS)
- [x] Create placeholder main.cpp for each executable target
- [x] Create Catch2 test infrastructure in Tests/
- [x] Create Scripts/build_all.sh
- [x] Validate: CMake configure succeeds
- [x] Validate: Build succeeds
- [x] Validate: Tests pass

## Phase 1 — Core Engine (Next)

- [ ] Port full Core module from MasterRepoV001 (math, memory, logging, events, reflection, serialization)
- [ ] Port full Engine module (ECS, world/level, behavior trees, asset system)
- [ ] Port full Input module (keyboard, mouse, gamepad, action mappings)
- [ ] Expand Catch2 test coverage
- [ ] Validate: clean compile, core tests pass

## Phase 2 — Rendering & Physics

- [ ] Port Renderer module (OpenGL RHI, forward pipeline, mesh, materials)
- [ ] Port Physics module (rigid bodies, collision, character controller)
- [ ] Port Audio module (device, spatial audio, mixer)
- [ ] Port Animation module (skeleton, blend tree, state machine)
- [ ] Validate: rendering tests pass

## Phase 3 — Game & Voxel Runtime

- [ ] Port Game module (voxels, interaction loop, R.I.G., inventory)
- [ ] Port UI module (custom renderer)
- [ ] Port game data from Atlas-NovaForge into Data/
- [ ] Port schemas into Schemas/
- [ ] Validate: DevWorld loads, voxel edit loop works

## Phase 4 — Editor

- [ ] Port Editor module (docking, panels, toolbar)
- [ ] Implement ProjectPathService, EditorCommandRegistry, VoxelPickService, SelectionService
- [ ] Port 14+ panels from Atlas-NovaForge
- [ ] Validate: editor boots, edit loop works

## Phase 5 — Graph VM & Visual Scripting

- [ ] Port GraphVM module (bytecode VM, compiler, serialization)
- [ ] Port 14 graph system types
- [ ] Wire into editor
- [ ] Validate: graph round-trip

## Phase 6 — Server & Networking

- [ ] Port Networking module (client-server, P2P, lockstep/rollback)
- [ ] Port 164 game systems from Atlas-NovaForge server
- [ ] Port game data (102 ships, 159 modules, 137 skills)
- [ ] Validate: server starts, client connects

## Phase 7 — AI & Tooling

- [ ] Port SwissAgent into Tools/
- [ ] Port ArbiterAI into Tools/
- [ ] Port Blender generator into Tools/
- [ ] Wire AI assistant into editor
- [ ] Validate: tools functional

## Phase 8 — Custom IDE

- [ ] Define IDE architecture
- [ ] Implement project indexer
- [ ] Add code navigation
- [ ] Integrate with editor command system
- [ ] Validate: IDE can open project

## Phase 9 — Documentation & Polish

- [ ] Consolidate all docs from all repos
- [ ] Set up GitHub Actions CI
- [ ] Docker support for server
- [ ] Create modding guide
- [ ] Final audit
