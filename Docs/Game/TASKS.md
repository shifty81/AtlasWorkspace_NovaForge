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
- [x] Voxel color mapping (voxelColor per VoxelType)
- [x] ChunkMesher (face-culled geometry generation, normals, per-type vertex coloring)
- [x] WorldState (multi-chunk container with world-coordinate get/set, forEach)
- [x] WorldSerializer (multi-chunk world save/load with round-trip proven)
- [x] **World save/reload round-trip PROVEN** (test: multi-chunk edit → serialize → deserialize → verify)
- [x] VoxelPickService (DDA ray-voxel traversal, face identification, adjacent voxel calculation)
- [x] Game data definitions (21 JSON files: 5 ships, 6 modules, 5 skills, 3 missions, 2 universe sectors)
- [x] JSON schemas (4 schema files: ship, module, skill, mission)
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
- [x] EditorTheme (dark/light themes with panel, button, input, toolbar, inspector colors + font/spacing)
- [x] PropertyEditor (read/write via reflection offsets: Bool, Int32, Float, String, Vec3, Color)
- [x] Undo-safe property transactions (makeFloatChange, makeIntChange, makeVec3Change, makeColorChange, makeStringChange)
- [x] UI renderer implementation (quad batching drawRect, character-based drawText, drawRectOutline)
- [x] Editor + PropertyEditor test suite (42 tests covering all property types, undo, theme, selection, commands)
- [x] Expand test coverage (212 tests, up from 183)
- [x] DockLayout manager (add/remove/find/computeLayout with slot-based bounds)
- [x] EditorPanel base class with 5 concrete panels (Viewport, Inspector, Hierarchy, Console, ContentBrowser)
- [x] EditorToolbar with default tool items (Select, Move, Rotate, Scale, Play, Pause, Stop)
- [x] EditorApp owns DockLayout, Toolbar, Panels with view toggle commands
- [x] GraphVM integration (graph.new_graph, graph.open_graph commands)
- [x] Expand test coverage (249 tests, up from 212)
- [ ] Validate: editor boots, edit loop works

## Phase 5 — Graph VM & Visual Scripting

- [x] GraphVM bytecode execution (16 registers, 256 memory slots, arithmetic, control flow, call/return)
- [x] GraphNode/GraphPort/GraphLink/Graph model for visual scripting
- [x] GraphCompiler (topological sort, register allocation, bytecode emission)
- [x] GraphSerializer (JSON round-trip for programs and graphs)
- [x] Graph types (14 types: World, Strategy, Conversation, Behavior, etc.)
- [x] Expand test coverage (249 tests, up from 212)
- [ ] Wire into editor panels (graph node editor UI)
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
