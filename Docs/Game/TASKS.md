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

- [x] Packet protocol (17 packet types: Connect, Disconnect, Heartbeat, EntitySpawn/Destroy/Update, WorldChunk, VoxelEdit, ChatMessage, RPC, Ping/Pong, Auth, PlayerInput, StateSnapshot, AckReliable)
- [x] PacketSerializer (JSON round-trip for all packet types)
- [x] Connection management (ConnectionState lifecycle, send/receive queues, reliable buffer, latency tracking, timeout detection)
- [x] ConnectionManager (add/remove/find connections, broadcast, timeout sweep)
- [x] Replication system (ReplicatedProperty with dirty tracking, ReplicationRule: ServerAuthority/ClientAuthority/PredictedOnClient)
- [x] ReplicationManager (register/unregister entities, collectDirtySnapshots, applySnapshot)
- [x] Session management (SessionState: Lobby/Loading/InGame/Paused/Ending, PlayerInfo, maxPlayers, host detection)
- [x] Lockstep system (InputFrame, input delay, confirmed inputs per frame per connection, advanceFrame)
- [x] Rollback system (save/load state callbacks, rollbackToFrame, confirmFrame, maxRollbackFrames)
- [x] RPC system (RPCTarget: Server/Client/AllClients/AllClientsExcept, RPCRegistry with invoke/dispatch)
- [x] NetworkManager expansion (owns all subsystems, processPacket routing, tick, send/broadcast, statistics)
- [x] Networking test suite (50 tests covering all subsystems)
- [x] Expand test coverage (299 tests, up from 249)
- [x] Multi-config build verified (Debug + Release with Ninja Multi-Config)
- [ ] Validate: server starts, client connects

## Phase 7 — AI & Tooling

- [x] Faction system (FactionManager, FactionRelation, FactionReputation with standing)
- [x] NPC personality (PersonalityTrait enum, PersonalityProfile with morale/confidence)
- [x] AI Blackboard (variant-based shared knowledge store with set/get/has/remove)
- [x] Utility AI scoring (UtilitySelector, UtilityCurve: Linear/Quadratic/Logistic/Inverse, evaluateCurve)
- [x] AI Agent (entity-bound agent with behavior, personality, memory, blackboard)
- [x] AISystem expansion (register/unregister/find agents, update all on tick)
- [x] SwissAgentTool (query processing with response generation)
- [x] ArbiterAITool (rule-based evaluation with context → result)
- [x] BlenderGeneratorTool (mesh generation by type with params)
- [x] ContractScannerTool (code scanning for issues)
- [x] ReplayMinimizerTool (frame sequence minimization with compression ratio)
- [x] ITool interface + ToolRegistry (register/find/list tools)
- [x] EditorAIAssistant (tool registry integration, query dispatch)
- [x] AI test suite (40 tests covering all subsystems)
- [x] Expand test coverage (339 tests, up from 299)
- [x] Multi-config build verified (Debug + Release with Ninja Multi-Config)
- [ ] Validate: AI assistant functional in editor loop

## Phase 8 — Custom IDE

- [x] IDE architecture (IDEService owns ProjectIndexer, CodeNavigator, BreadcrumbTrail)
- [x] ProjectIndexer (file classification: Header/Source/Shader/Script/Data/Config, module grouping, symbol indexing)
- [x] CodeNavigator (go-to-definition, find references, search symbols, filter by SymbolKind)
- [x] BreadcrumbTrail (navigation history with max depth 50, push/pop/current)
- [x] IDEPanel (EditorPanel subclass for IDE viewport)
- [x] IDEService lifecycle (init/shutdown, navigateTo/goBack)
- [x] EditorApp integration (ide.go_to_definition, ide.find_references, ide.go_back, ide.index_project commands)
- [x] IDE test suite (12 tests covering indexer, navigator, breadcrumbs, service, integration)
- [x] Expand test coverage (351 tests, up from 339)
- [x] Multi-config build verified (Debug + Release with Ninja Multi-Config)
- [ ] Validate: IDE can open project

## Phase 9 — Documentation & Polish

- [x] GitHub Actions CI workflow (Linux Debug/Release, Multi-Config, Docker build)
- [x] Docker server image with OCI labels
- [x] Modding guide (ship, module, skill, mission JSON format + voxel + graph scripting)
- [x] Architecture documentation updated (Networking dependency correction)
- [ ] Final audit

## Game Phase G1 — First Interaction Loop

- [x] ResourceType enum (RawStone, RawDirt, RawIron, RawGold, RawCrystal, RefinedIron, RefinedGold, RefinedCrystal, SteelPlate, CircuitBoard, EnergyCell)
- [x] resourceTypeName / resourceTypeFromName round-trip functions
- [x] ResourceDrop table (getResourceDrops maps VoxelType → ResourceDrop vector)
- [x] ResourceInventory (add, remove, count, totalItems, isEmpty)
- [x] ToolType enum (MiningLaser, PlacementTool, RepairTool, Scanner)
- [x] ToolState (durability, energyCost, cooldown, cooldownRate, miningDamage, isReady, tick, use)
- [x] toolTypeName function
- [x] ToolBelt (4 slots, init, selectSlot, nextTool, prevTool, activeTool)
- [x] Expanded RigState (health, maxHealth, energy, maxEnergy, oxygen, stamina, regen rates, isAlive, tick, takeDamage, heal, consumeEnergy, consumeStamina)
- [x] HUDNotification and HUDState (crosshair, notifications, addNotification, tick, clearNotifications)
- [x] MineResult and PlaceResult structs
- [x] InteractionSystem (tryMine, tryPlace, tryScan, maxReach)
- [x] GameSession (init, shutdown, tick, all accessors, isActive)
- [x] Comprehensive test coverage (32 new tests, 383 total)

## Game Phase G2 — Voxel Mesh Rendering

- [x] LightType enum and LightSource struct (Directional, Point, Spot)
- [x] LightingState (add/remove lights, ambient, Phong computeLighting)
- [x] VoxelShader (pre-configured shader with VP, model, light uniforms)
- [x] FrustumPlane and Frustum (extract from VP matrix, AABB culling)
- [x] ChunkRenderData (coord, mesh, valid flag, version counter)
- [x] ChunkRenderCache (update, remove, get, clear, updateDirty)
- [x] ChunkRenderer (init, shutdown, render with frustum culling, countVisible)
- [x] Renderer tests (12 new: lighting, shader, frustum)
- [x] Game tests (8 new: render cache, chunk renderer, culling)
- [x] Comprehensive test coverage (19 new tests, 402 total)

## Game Phase G3 — Movement & FPS Camera

- [x] FPSCamera class (init, processMouseLook, yaw/pitch, forward/right/up vectors)
- [x] FPSCamera updateVectors (yaw/pitch to direction vectors via trig)
- [x] FPSCamera toCamera conversion (builds Renderer Camera struct)
- [x] FPSCamera pitch clamping (setPitchLimits, default ±89°)
- [x] FPSCamera sensitivity setting
- [x] MovementInput struct (forward, backward, left, right, jump, sprint, crouch)
- [x] PlayerMovement class (walk, sprint, crouch speeds, gravity, jump)
- [x] PlayerMovement update (camera-relative XZ movement, gravity, ground plane)
- [x] PlayerAABB struct (fromPosition factory, half-width AABB)
- [x] VoxelCollider class (axis-separated collision resolution)
- [x] VoxelCollider wouldCollide (AABB vs solid voxel overlap test)
- [x] VoxelCollider isOnGround (check voxel below feet)
- [x] PlayerController class (ties camera, movement, collider together)
- [x] PlayerController update (mouse look → movement → collision → eye height)
- [x] Comprehensive test coverage (15 new tests, 417 total)
