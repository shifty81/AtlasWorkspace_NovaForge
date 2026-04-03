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

- [x] Expand Core module (Mat4, Quat, Transform, Timer, StringID, TypeID)
- [x] Expand Engine module (ComponentStore, SystemBase, Level, BehaviorTree, AssetHandle)
- [x] Expand Input module (full KeyCode enum, ActionMapping, InputManager, mouse/gamepad)
- [x] Expand Catch2 test coverage (67 tests, up from 19)
- [x] Validate: clean compile, core tests pass
- [x] Core memory management (LinearAllocator, PoolAllocator)
- [x] Core reflection system (TypeDescriptor, PropertyInfo, TypeRegistry)
- [x] Core serialization (JsonValue, JsonParser with roundtrip support)
- [x] Engine scene graph (parent/child entity hierarchy, world transform)
- [x] Engine system registry (SystemRegistry with init/update/shutdown lifecycle)
- [x] Undo/redo command system (ICommand, CommandStack, PropertyChangeCommand)
- [x] Color type with RGBA8 pack/unpack
- [x] Extended PropertyType enum (Color, Enum support)
- [ ] Port full Input module (platform-specific backends)

## Phase 2 — Rendering & Physics

- [x] Renderer module (Vertex, Mesh, Shader, Material, RenderQueue, Camera)
- [x] Physics module (RigidBody, Ray, Sphere, RayHit, PhysicsWorld with gravity)
- [x] Audio module (AudioClip, AudioSource, AudioMixer, AudioListener, AudioDevice)
- [x] Animation module (Skeleton, AnimationClip, AnimationChannel, AnimationStateMachine)
- [x] Collision shape hierarchy (BoxShape, SphereShape, MeshCollisionShape)
- [x] Collision shape separate dirty flags from render mesh
- [x] Expand test coverage (183 tests, up from 143)
- [x] Validate: all tests pass

## Phase 3 — Game & Voxel Runtime

- [x] Voxel type names (voxelTypeName, voxelTypeFromName) for serialization
- [x] Chunk dirty flag separation (meshDirty, collisionDirty independent)
- [x] Chunk utility methods (isFullyAir, solidCount, markMeshClean, markCollisionClean)
- [x] ChunkSerializer (toJson, fromJson, serialize, deserialize)
- [x] VoxelEditCommand (undo-safe voxel edits via ICommand)
- [x] **Voxel edit save/reload round-trip PROVEN** (test: edit → serialize → deserialize → verify)
- [ ] Port UI module (custom renderer)
- [ ] Port game data from Atlas-NovaForge into Data/
- [ ] Port schemas into Schemas/
- [ ] Validate: DevWorld loads, voxel edit loop works in-editor

## Phase 4 — Editor

- [x] SelectionService (multi-select, toggle, exclusive, version tracking)
- [x] EditorCommandRegistry with enabled/disabled state and hotkey info
- [x] RecentFilesList (MRU with deduplication and max entries)
- [x] ContentBrowser (directory scanning, extension classification, navigation)
- [x] ProjectPathService (project root discovery, content/data/config path resolution)
- [x] LaunchService (game exe discovery, validation, deterministic path resolution)
- [x] Editor commands: file.new, file.open, file.save, file.save_as, edit.undo, edit.redo, edit.select_all, edit.deselect, view.reset_layout
- [x] EditorApp owns CommandStack, SelectionService, ContentBrowser, RecentFiles, LaunchService
- [x] Editor test suite (20 tests covering selection, commands, content browser, launch)
- [ ] Port Editor module (docking, panels, toolbar)
- [ ] Implement VoxelPickService
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
