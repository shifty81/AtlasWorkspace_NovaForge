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
- [x] Win32 input backend: Win32InputAdapter.h (WM_KEYDOWN/UP, WM_MOUSEMOVE, WM_*BUTTON*, WM_MOUSEWHEEL, WM_KILLFOCUS → InputSystem)
- [x] Editor main loop wired: Win32InputAdapter + InputSystem integrated into NovaForgeEditor main.cpp message pump

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
- [x] Wire into editor panels (graph node editor UI)
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
- [x] Final audit (spaghetti cleanup: My Repos.rtf → Archive/, loose .zip removed from git)

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

## Game Phase G4 — Ship Systems

- [x] ShipClass enum (Fighter, Corvette, Frigate, Cruiser, Freighter) with shipClassName()
- [x] ModuleSlotType enum (Weapon, Shield, Engine, Reactor, Cargo, Utility) with moduleSlotTypeName()
- [x] ShipModule struct (health, damage, takeDamage, repair, tier, slot-specific stats)
- [x] ShipStats struct (computed totals: thrust, power, shield, cargo, DPS, module counts)
- [x] Ship class (init by class, module management, hull/shield damage, shield recharge, computeStats)
- [x] Ship damage model (shield absorbs first, overflow to hull)
- [x] Ship class stats (Fighter/Corvette/Frigate/Cruiser/Freighter hull/shield/maxModules)
- [x] FlightInput struct (throttle, pitch, yaw, roll, boost)
- [x] FlightState struct (position, velocity, forward/up/right, speed, maxSpeed, turnRate, boostMultiplier)
- [x] FlightController class (init, update with rotation + thrust + boost, position/velocity/speed accessors)
- [x] WeaponState struct (cooldown, range, isReady, tick)
- [x] CombatTarget struct (position, distance, inRange, inFiringArc)
- [x] CombatSystem class (calculateDamage, tickWeapons, evaluateTarget with arc check, applyDamage)
- [x] Comprehensive test coverage (22 new tests, 439 total)

## Game Phase G5 — Fleet AI

- [x] FormationType enum (Line, Wedge, Column, Spread, Defensive) with formationTypeName()
- [x] FormationSlot struct (shipIndex, offset, occupied)
- [x] Formation class (init, slotCount, slot, setSpacing, getSlotWorldPosition, generateSlotOffsets)
- [x] CaptainPersonality struct (aggression, caution, loyalty, initiative, morale, confidence, adjustMorale, adjustConfidence, willFlee, willCharge)
- [x] CaptainOrder enum (HoldPosition, AttackTarget, DefendTarget, FollowLeader, Patrol, Retreat, FreeEngage) with captainOrderName()
- [x] AICaptain class (init, name, personality, currentOrder, setOrder, overrideOrder, clearOverride, evaluate)
- [x] FleetShip struct (ship, captain, flight, formationSlot, active)
- [x] Fleet class (init, addShip, removeShip, ship, shipCount, activeShipCount, setFormation, issueOrder, issueOrderTo, setLeader, tick, fleetMorale, fleetStrength)
- [x] Comprehensive test coverage (20 new tests)

## Game Phase G6 — Economy

- [x] MarketItem struct (resource, quantity, buyPrice, sellPrice)
- [x] Market class (init, listItem, buy, sell, findItem, itemCount)
- [x] RefiningRecipe struct (input, inputAmount, output, outputAmount, timeRequired)
- [x] Refinery class (addRecipe, findRecipe, startRefining, collectOutput, recipeCount)
- [x] ManufacturingRecipe struct (name, inputs, output, outputAmount, timeRequired)
- [x] Manufacturer class (addRecipe, findRecipe, canCraft, craft, recipeCount)
- [x] Comprehensive test coverage (14 new tests)

## Game Phase G7 — Exploration

- [x] SectorType enum (Normal, Nebula, AsteroidField, DeepSpace, AncientRuins) with sectorTypeName()
- [x] SectorInfo struct (name, type, position, scanProgress, fullyScanned, hasWormhole, hasAncientTech)
- [x] ProbeScanner class (init, startScan, stopScan, isActive, tick, scanRate, setScanRate)
- [x] WormholeLink struct (fromSector, toSector, stability, twoWay, isTraversable, degrade)
- [x] StarMap class (addSector, findSector, addWormhole, getReachableSectors, sectorCount, wormholeCount, getAncientTechSectors)
- [x] AncientTechFragment struct (name, sectorFound, tier, analyzed, damageBonus, shieldBonus, speedBonus)
- [x] AncientTechRegistry class (add, find, analyze, count, analyzedCount)
- [x] Comprehensive test coverage (13 new tests)

## Game Phase G8 — FPS Interiors

- [x] RoomType enum (Bridge, Engineering, MedBay, Cargo, Airlock, Corridor) with roomTypeName()
- [x] ShipRoom struct (name, type, oxygenLevel, temperature, pressurized, connectedRooms, connect, isConnectedTo, isHabitable)
- [x] ShipInterior class (addRoom, findRoom, roomCount, decompress, repressurize, habitableRoomCount)
- [x] EVAState struct (active, suitIntegrity, oxygenSupply, jetpackFuel, velocity, isAlive, tick, useThruster, takeSuitDamage)
- [x] SurvivalStatus struct (radiation, temperature, inVacuum, onFire, isRadiationDangerous, isHypothermic, isHyperthermic, isInDanger, tick)
- [x] Comprehensive test coverage (11 new tests)

## Game Phase G9 — Legend System

- [x] ReputationTier enum (Infamous, Outlaw, Neutral, Trusted, Honored, Legend) with reputationTierName()
- [x] reputationTierFromScore() free function
- [x] PlayerReputation class (adjustReputation, getReputation, getTier, factionCount, globalFame)
- [x] WorldBias struct (sectorName, economyModifier, dangerLevel, loyaltyToPlayer, isFriendly, isHostile)
- [x] WorldBiasMap class (setBias, getBias, updateFromReputation, biasCount)
- [x] NPCMemoryEntry struct (eventType, timestamp, weight, positive)
- [x] NPCMemory class (remember, decay, dispositionTowardPlayer, entryCount, remembers)
- [x] LegendStatus class (init, reputation, worldBias, overallTier, isLegend)
- [x] Comprehensive test coverage (12 new tests)

## Game Phase G10 — Quest & Mission System

- [x] MissionObjectiveType enum (Kill, Collect, Deliver, Explore, Survive, Escort) with missionObjectiveTypeName()
- [x] MissionObjective struct (type, targetId, description, required, current, isComplete, progress)
- [x] MissionReward struct (credits, resources map, reputationFactionId, reputationAmount)
- [x] MissionStatus enum (Active, Completed, Failed)
- [x] ActiveMission class (init, objectives, reward, allObjectivesComplete, complete, fail)
- [x] MissionLog class (acceptMission, completeMission, failMission, findActive, counts)
- [x] QuestChain class (init, addMission, currentMissionId, advance, isComplete)
- [x] Comprehensive test coverage (10 new tests)

## Game Phase G11 — Dialogue System

- [x] DialogueConditionType enum (Always, HasReputation, HasItem, MissionActive, MissionComplete)
- [x] DialogueCondition struct (type, factionId, minReputation, itemType, missionId, evaluate)
- [x] DialogueEffect struct (reputationFactionId, reputationDelta, startMissionId, giveItem)
- [x] DialogueOption struct (text, condition, effect, nextNodeId)
- [x] DialogueNode struct (nodeId, speakerName, text, options)
- [x] DialogueGraph class (setStartNodeId, addNode, getNode, nodeCount)
- [x] DialogueRunner class (init, currentNode, selectOption, isComplete)
- [x] Comprehensive test coverage (7 new tests)

## Game Phase G12 — Save/Load System

- [x] SaveSlot struct (slotIndex, name, timestamp, playtimeSeconds, isEmpty)
- [x] SaveData struct (playerPosition, health, energy, oxygen, playtime, inventory map, active/completed missions, reputation, currentSector)
- [x] GameSaveSerializer (toJson/fromJson full round-trip for SaveData)
- [x] SaveSystem class (init, saveGame, loadGame, deleteSlot, listSlots, usedSlotCount)
- [x] Auto-save support (enableAutoSave, setAutoSaveInterval, tickAutoSave with slot 0)
- [x] Comprehensive test coverage (7 new tests)

## Game Phase G13 — World Events System

- [x] WorldEventType enum (AsteroidStorm, PirateRaid, TechDiscovery, FactionWar, TradeOpportunity, Plague, CelestialAnomaly) with worldEventTypeName()
- [x] EventEffect struct (priceModifier, dangerModifier, reputationChange, resourceBonus)
- [x] WorldEvent struct (eventId, type, sectorId, description, duration, elapsed, severity, isActive, tick, isExpired, remainingTime)
- [x] WorldEventSystem class (init, spawnEvent, endEvent, tick, getActiveEvents, getEventsInSector, findEvent, activeEventCount)
- [x] Severity-scaled EventEffect building (buildEffect per event type)
- [x] Comprehensive test coverage (7 new tests)

## Game Phase G14 — Tech Tree

- [x] TechCategory enum (Weapons, Shields, Propulsion, Mining, Construction, Biology, Computing) with techCategoryName()
- [x] TechNode struct (id, displayName, category, tier, cost, prerequisites, researched, damage/shield/speed/miningBonus)
- [x] TechTree class (addNode, canResearch, unlock, isUnlocked, findNode, getAvailable, getResearched, getByTier)
- [x] TechTree computeBonuses (aggregate damage/shield/speed/mining from researched nodes)
- [x] Comprehensive test coverage (7 new tests)

## Game Phase G15 — Player Progression

- [x] XPSource enum (Combat, Mining, Exploration, Trade, Quest, Crafting) with xpSourceName()
- [x] PlayerLevel class (init, addXP, currentLevel, xpThisLevel, totalXP, xpToNextLevel, progressToNextLevel, isMaxLevel, kMaxLevel=50)
- [x] Quadratic XP curve: level*(level-1)*50 cumulative XP
- [x] SkillNode struct (id, requiredLevel, pointCost, unlocked, health/energy/damage/miningBonus)
- [x] SkillTree class (addSkill, unlockSkill, isUnlocked, findSkill, getAvailable, computeBonuses, unlockedCount)
- [x] ProgressionSystem class (init, awardXP → skill points on level-up, spendSkillPoint, bonuses())
- [x] Comprehensive test coverage (8 new tests)

## Game Phase G16 — Crafting System

- [x] CraftingCategory enum (Weapon, Armor, Tool, Component, Consumable, Fuel, Decoration) with craftingCategoryName()
- [x] CraftingIngredient struct (itemId, quantity)
- [x] CraftingRecipe struct (recipeId, outputItemId, outputQuantity, category, ingredients, craftTime, requiredLevel)
- [x] CraftingJob struct (tick, progress, complete flag)
- [x] CraftingQueue class (enqueue, tick head-only FIFO, collectCompleted, currentJob)
- [x] CraftingSystem class (registerRecipe, findRecipe, canCraft with inventory/level check, enqueue with ingredient deduction, recipesByCategory)
- [x] Comprehensive test coverage (8 new tests)

## Game Phase G17 — Inventory & Equipment

- [x] ItemRarity enum (Common, Uncommon, Rare, Epic, Legendary) with itemRarityName()
- [x] ItemSlot enum (None, Head, Chest, Legs, Boots, Weapon, Shield, Accessory) with itemSlotName()
- [x] Item struct (id, displayName, rarity, slot, stackMax, count, weight, stat bonuses)
- [x] PlayerInventory class (addItem with stacking, removeItem, countItem, findItem, capacity, toCountMap)
- [x] EquipmentLoadout class (equip/unequip by slot, slot replace returns previous, computeBonuses aggregate)
- [x] Comprehensive test coverage (8 new tests)

## Suite Phase S0 — Pipeline Core

- [x] NF::Pipeline module scaffold (CMakeLists.txt, include/NF/Pipeline/Pipeline.h)
- [x] ChangeEventType enum (7 types: FileAdded, FileModified, FileDeleted, FileRenamed, DirectoryAdded, DirectoryDeleted, ManifestUpdated)
- [x] ChangeEvent struct (toJson, fromJson, writeToFile, readFromFile)
- [x] Manifest class (GUID registry, save manifest.json, load manifest.json)
- [x] WatchLog class (thread-safe append, toJson, fromJson)
- [x] PipelineDirectories struct (fromRoot, ensureCreated)
- [x] PipelineWatcher class (poll, start, stop — monitors .novaforge/pipeline/)
- [x] 17 Catch2 tests covering all pipeline types
- [x] NF::Editor links NF::Pipeline

## Repo Consolidation — Phase C0 (MasterRepo)

- [x] Update CMakeLists.txt with MasterRepo version (CMP0091, MSVC runtime fix, v3.20)
- [x] Update vcpkg.json with MasterRepo baseline (pinned baseline, scoped platforms)
- [x] Add ci-debug and ci-release presets to CMakePresets.json
- [x] Replace Config/novaforge.project.json with MasterRepo schema
- [x] Replace Content/Definitions/DevWorld.json with MasterRepo definition
- [x] Update Docs/Game/ROADMAP.md to MasterRepo phase structure
- [x] Update Docs/Game/ARCHITECTURE.md with Source/Game domain layout + design locks
- [x] Update Docs/Game/PROJECT_RULES.md with Hard Boundary (AtlasAI clause)
- [x] Create Docs/Game/BUILD_RULES.md
- [x] Create Docs/GITHUB_COPILOT_IMPLEMENTATION_DIRECTIONS.md
- [x] Create Docs/CONSOLIDATION_PLAN.md
- [x] Create Archive/_MasterRepo/ARCHIVE_SUMMARY.md
- [x] Create Archive/_MasterRepo/usable_snippets/cmake_msvc_runtime_fix.snippet.md

## Game Phase G18 — Status Effects

- [x] StatusEffectType enum (Poison, Burn, Freeze, Radiation, Bleed, Stun, Blind, Overcharge) with statusEffectTypeName()
- [x] StatusEffect struct (damage, duration, elapsed, tickRate, tickTimer, intensity, active, tick, isExpired, remaining)
- [x] AilmentStack class (apply with refresh-on-duplicate, remove by type, has, tick with auto-removal, count, clear)
- [x] StatusEffectSystem class (applyEffect, removeEffect, hasEffect, tick→damage map, getStack, clearEntity, clearAll, entityCount)
- [x] Comprehensive test coverage (7 new tests)

## Game Phase G19 — Contracts & Bounties

- [x] ContractType enum (Delivery, Assassination, Escort, Salvage, Patrol, Mining) with contractTypeName()
- [x] ContractStatus enum (Available, Accepted, InProgress, Completed, Failed, Expired)
- [x] Contract struct (contractId, title, type, status, faction, rewards, timeLimit, tick, lifecycle methods, isActive, isExpired)
- [x] BountyTarget struct (targetId, name, faction, reward, reputationCost, deadOrAlive, claimed, claim, isClaimable)
- [x] ContractBoard class (addContract, addBounty, findContract, findBounty, availableContracts by level, activeBounties, tick, removeExpired)
- [x] Comprehensive test coverage (7 new tests)

## Game Phase G20 — Companion System

- [x] CompanionRole enum (Combat, Engineer, Medic, Scout, Pilot, Trader) with companionRoleName()
- [x] CompanionPersonality struct (loyalty, bravery, curiosity, morale, trust, gainTrust, loseTrust, isLoyal, adjustMorale)
- [x] CompanionAbility struct (name, cooldown, isReady, use, tick, passive flag)
- [x] Companion class (init, name, role, health, isAlive, isActive, takeDamage, heal, dismiss, recall, addAbility, findAbility, tick)
- [x] CompanionManager class (addCompanion, removeCompanion, findCompanion, companionCount, activeCount, tick, healAll, hasRole, averageMorale, kMaxCompanions=4)
- [x] Comprehensive test coverage (9 new tests)

## Repo Consolidation — Phase C1 (MasterRepoRefactor)

- [x] Create AtlasAI/ directory structure (README.md, Atlas_Arbiter/, Atlas_SwissAgent/)
- [x] Create AtlasAI/Atlas_Arbiter/README.md (rule engine, AB-1→5 roadmap, pipeline contract)
- [x] Create AtlasAI/Atlas_SwissAgent/README.md (query agent, SA-1→5 roadmap, pipeline contract)
- [x] Create Archive/_MasterRepoRefactor/ARCHIVE_SUMMARY.md
- [x] Create Archive/_MasterRepoRefactor/usable_snippets/atlas_dir_layout.snippet.md
- [x] Create Archive/_MasterRepoRefactor/docs_archive/.gitkeep
- [x] Create Scripts/validate_project.sh (56-check project structure validator)
- [x] Create Tools/BlenderGenerator/README.md (BG-1→5 roadmap, pipeline contract)
- [x] Update Docs/Architecture/TEMPNOVAFORGE_GAP_AUDIT.md — all 12 GAPs marked ✅
- [x] Update Docs/Game/ROADMAP.md — G18-G20 done, Phase 10 done, C0/C1 done
- [x] Update Docs/CONSOLIDATION_PLAN.md — C0/C1 marked done
- [x] Validate: cmake build succeeds, 641 tests pass
- [x] Validate: Scripts/validate_project.sh reports 56/56 pass

## Repo Consolidation — Phase C2 (AtlasToolingSuite)

- [x] Land AtlasUI source into active tree (Source/UI/include/NF/UI/AtlasUI/, Source/UI/src/AtlasUI/)
- [x] AtlasUI Theme system: ThemeTypes, ThemeTokens, Theme, ThemeDefaults (Atlas Dark), ThemeManager singleton
- [x] AtlasUI Foundation: Interfaces (IWidget, IPanel, ILayoutContext, IPaintContext, IInputContext, IPopupHost), WidgetBase, PanelBase, PanelHost, DrawPrimitives, Contexts, WidgetHelpers, Types
- [x] AtlasUI Widgets: Button, Panel, TabBar, Splitter, Toolbar, Tooltip, TextInput, Dropdown, PropertyRow, NotificationCard, Container
- [x] AtlasUI Commands: CommandTypes, CommandRegistry, CommandManager, ShortcutRouter, CommandHelpers, CommandDefaults
- [x] AtlasUI Services: FocusService, PopupHost, TooltipService, NotificationHost
- [x] Move spec rollup docs to Docs/SpecRollup/ (12 architecture specs + README + manifest)
- [x] Update Source/UI/CMakeLists.txt (20 new .cpp source files compiled)
- [x] AtlasUI Catch2 test suite (36 tests: theme, widgets, commands, services, PanelHost, DrawList)
- [x] AtlasUI validation scripts: path, symbol, theme, migration-gate validators
- [x] AtlasUI CI workflow (.github/workflows/atlasui_validation.yml)
- [x] Update Scripts/validate_project.sh (69 checks, up from 56)
- [x] Update Docs/CONSOLIDATION_PLAN.md — C2 marked done
- [x] Validate: cmake build succeeds, 708 tests pass
- [x] Validate: all AtlasUI validators pass
- [x] Validate: Scripts/validate_project.sh reports 69/69 pass

## Repo Consolidation — Phase C3 (Internal Consolidation)

- [x] Create Archive/_AtlasToolingSuite/ with ARCHIVE_SUMMARY.md (C2 archive was missing)
- [x] Integrate remaining Additions/ starter pack content into Source/UI/:
  - AtlasUI.h umbrella header
  - SimplePanel.h (concrete panel implementation)
  - Widgets/Label.h (text label widget)
  - Widgets/Tabs.h + Tabs.cpp (tab container widget)
  - Widgets/StackPanel.h + StackPanel.cpp (stack layout widget)
  - Widgets/WidgetKit.h (widget umbrella header)
- [x] Update Source/UI/CMakeLists.txt (3 new .cpp files: AtlasUI, Tabs, StackPanel)
- [x] Consolidate duplicate AtlasAI directories: Tools/AtlasAI/ → root AtlasAI/ canonical
- [x] Merge ATLAS_AI_OVERVIEW.md into root AtlasAI/
- [x] Deprecate legacy tool directories with clear forwarding READMEs:
  - Tools/ArbiterAI/ → AtlasAI/Atlas_Arbiter/
  - Tools/SwissAgent/ → AtlasAI/Atlas_SwissAgent/
  - Tools/AtlasAI/ → AtlasAI/ (repo root)
- [x] Scaffold archive stubs for remaining merge phases 3–11 (9 repos)
- [x] Update Docs/CONSOLIDATION_PLAN.md with C3 tasks, archive inventory, canonical paths
- [x] Update spaghetti audit: 6 items resolved, 2 deferred to workspace/future phases
- [x] Update Docs/Game/TASKS.md with C3 phase tracking

## UI Migration — Phase U0 (AtlasUI Shell Standard)

- [x] AtlasUI Shell Standard v1 spec (Docs/SpecRollup/02_ATLASUI_STANDARD.md)
- [x] AtlasUI Theme + Token Mapping v2 spec (Docs/SpecRollup/04_THEME_AND_TOKEN_SYSTEM_V1.md)
- [x] AtlasUI Component Pack v1 spec (implemented as active source)
- [x] AtlasUI Integration Map v1 spec (execution bridge for migration)
- [x] AtlasUI Repo Enforcement Pack v1 (validation scripts + CI)
- [x] AtlasUI Validation Rule Set v1 (regex patterns, CI jobs, phase gates)
- [x] Migrate InspectorPanel to AtlasUI (Phase U1)
- [x] Migrate HierarchyPanel to AtlasUI (Phase U2)
- [x] Migrate ContentBrowserPanel to AtlasUI (Phase U3)
- [x] Migrate ConsolePanel to AtlasUI (Phase U4)
- [x] Migrate IDEPanel to AtlasUI (Phase U5)
- [x] Migrate GraphEditorPanel shell to AtlasUI (Phase U6)
- [x] Migrate ViewportPanel shell to AtlasUI (Phase U7)
- [x] Deprecate legacy UI path (Phase U8)

## UI Migration — Phases U1-U8 (Panel Migration)

- [x] U1: Create AtlasUI InspectorPanel (Panels/InspectorPanel.h + .cpp)
  - Entity selection, transform display, custom properties
  - Theme-token-driven paint through IPaintContext
  - State save/load via PanelBase
- [x] U2: Create AtlasUI HierarchyPanel (Panels/HierarchyPanel.h + .cpp)
  - Entity tree with search filter, selection highlight, depth-based indent
- [x] U3: Create AtlasUI ContentBrowserPanel (Panels/ContentBrowserPanel.h + .cpp)
  - File/directory listing, path navigation, entry type icons
- [x] U4: Create AtlasUI ConsolePanel (Panels/ConsolePanel.h + .cpp)
  - Message levels (Info/Warning/Error), color-coded, 1000-msg max
- [x] U5: Create AtlasUI IDEPanel (Panels/IDEPanel.h + .cpp)
  - Symbol search, code navigation results, file path + line display
- [x] U6: Create AtlasUI GraphEditorPanel shell (Panels/GraphEditorPanel.h + .cpp)
  - Node/link management, selection, graph name, visual node rendering
- [x] U7: Create AtlasUI ViewportPanel shell (Panels/ViewportPanel.h + .cpp)
  - Camera position overlay, grid, tool mode, render mode indicators
- [x] U8: Add DEPRECATED markers to all legacy EditorPanel classes in Editor.h
- [x] Create PipelineMonitorPanel (Panels/PipelineMonitorPanel.h + .cpp)
  - Pipeline event logging, 500-event max, type/source/details display
- [x] Update Source/UI/CMakeLists.txt (8 new .cpp files)
- [x] Write comprehensive tests for all migrated panels (40 new tests)
- [x] All 8 AtlasUI panels register with PanelHost successfully

## Repo Consolidation — Phase 3 (Nova-Forge-Expeditions Extraction)

- [x] Audit source repo contents via GitHub API (~5,900 files)
- [x] Read and analyze AUDIT.md (three-way merge decisions)
- [x] Read and analyze ROADMAP.md (7-phase development plan)
- [x] Identify 449 unique game systems, PCG framework, rendering pipeline
- [x] Map content to canonical tempnovaforge locations
- [x] Create Archive/_Nova-Forge-Expeditions/ARCHIVE_SUMMARY.md with full audit
- [x] Create Archive/_Nova-Forge-Expeditions/usable_snippets/ with code patterns
- [x] Create Archive/_Nova-Forge-Expeditions/docs_archive/ with doc inventory
- [x] Update Docs/CONSOLIDATION_PLAN.md — Phase 3 marked as audited
- [x] Full content extraction (1,708 files extracted from source repo):
  - [x] 3A: 449 game systems → Source/Game/src/Systems/
  - [x] 3B: 541 server headers → Source/Game/include/NF/Game/Server/
  - [x] 3C: 548 game tests → Tests/Game/Server/
  - [x] 3D: 36 PCG framework files → Source/Engine/src/PCG/
  - [x] 3E: 28 rendering pipeline files → Source/Renderer/src/Pipeline/
  - [x] 3F: 40 editor tool files → Source/Editor/src/Tools/
  - [x] 3G: 8 gameplay module files → Source/Game/src/Modules/
  - [x] 3H: 8 blender addon files → Tools/BlenderGenerator/blender-addon/
  - [x] 3I: 40 unique docs → Docs/Nova-Forge-Expeditions/
  - [x] 3J: 7 superseded AtlasUI files → Archive (reference only)
  - [x] AUDIT.md + ROADMAP.md → Archive/_Nova-Forge-Expeditions/docs_archive/
- [x] Build verification: 745 tests passing, no breakage
- [x] Update Docs/CONSOLIDATION_PLAN.md — Phase 3 marked as extracted

## Repo Consolidation — Phase 4 (Atlas-NovaForge Extraction)

- [x] Audit source repo contents via GitHub API (~2,000 files, 22 top-level dirs)
- [x] Identify usable code, docs, and assets (engine modules, editor, client, game data, tools)
- [x] Map content to canonical tempnovaforge locations (overlap analysis with Phase 3)
- [x] Create Archive/_Atlas-NovaForge/ARCHIVE_SUMMARY.md with full audit
- [x] Full content extraction (1,264 files extracted from source repo):
  - [x] 4B: 241 engine modules (40 dirs) → Source/Engine/src/
  - [x] 4C: 89 editor files → Source/Editor/src/
  - [x] 4D: 244 client files → Source/Game/src/Client/ + include/NF/Game/Client/
  - [x] 4E: 95 game data files → Data/
  - [x] 4F: 5 JSON schemas → Schemas/
  - [x] 4G: 3 TLA+ specs → Docs/Atlas-NovaForge/specs/
  - [x] 4H: 52 tools → Tools/Atlas-NovaForge/ + Tools/BlenderGenerator/
  - [x] 4I: 130 sample project files → Project/samples/
  - [x] 4J: 104 docs → Docs/Atlas-NovaForge/
  - [x] 4K: 306 tests → Tests/Atlas-NovaForge/
  - [x] 4L: 18 archive files → Archive/_Atlas-NovaForge/
- [x] Build verification: 745 tests passing, no breakage
- [x] Update Docs/CONSOLIDATION_PLAN.md — Phase 4 marked as extracted
- [x] Update Archive/_Atlas-NovaForge/ARCHIVE_SUMMARY.md — all checklist items complete

## Repo Consolidation — Phase 5 (AtlasForge Extraction)

- [x] Audit source repo contents via GitHub API (~718 files, 14 directories)
- [x] Full audit report: Docs/ATLASFORGE_AUDIT_REPORT.md
- [x] Overlap analysis: AtlasForge is the origin repo — nearly all content already present from Phases 3–4
- [x] Identify unique content: ScriptSandbox, ScriptSystem, ABICapsule, ABIRegistry, TileEditor, font metadata
- [x] Extract 11 unique files to canonical locations:
  - [x] 5A: ScriptSandbox.h/.cpp → Source/Engine/src/Script/
  - [x] 5B: ScriptSystem.h/.cpp → Source/Engine/src/Script/
  - [x] 5C: ABICapsule + ABIRegistry (4 files) → Source/Engine/src/ABI/
  - [x] 5D: TileEditor main.cpp + CMakeLists.txt → Source/Programs/TileEditor/
  - [x] 5E: builtin_fallback.json → Content/Assets/Fonts/
- [x] Archive usable snippets (10 files) → Archive/_AtlasForge/usable_snippets/
- [x] Archive docs (2 files) → Archive/_AtlasForge/docs_archive/
- [x] Build verification: 745 tests passing, no breakage
- [x] Update Docs/CONSOLIDATION_PLAN.md — Phase 5 marked as extracted
- [x] Update Archive/_AtlasForge/ARCHIVE_SUMMARY.md — all checklist items complete

## Repo Consolidation — Phase 6 (NovaForge-Project Extraction)

- [x] Audit source repo via GitHub API
- [x] Extract 21 docs → Docs/NovaForge-Project/ (architecture, rules, specs, migration, runtime)
- [x] Archive 19 reference files → Archive/_NovaForge-Project/ (scripts, tests, configs, CI)
- [x] Update Archive/_NovaForge-Project/ARCHIVE_SUMMARY.md — all checklist items complete

## Repo Consolidation — Phases 7-9 (AI Repos Extraction)

- [x] Phase 7: Audit SwissAgent → 30 usable snippets + 6 docs archived
- [x] Phase 8: Audit ArbiterAI → 3 usable snippets + 1 doc archived
- [x] Phase 9: Audit Arbiter → 10 usable snippets + 4 docs archived
- [x] Update all three ARCHIVE_SUMMARY.md files — complete

## Repo Consolidation — Phase 10 (AtlasForge-EveOffline Extraction)

- [x] Audit source repo via GitHub API
- [x] Extract 56 docs → Docs/AtlasForge-EveOffline/
- [x] Archive 11 reference files → Archive/_AtlasForge-EveOffline/
- [x] Update Archive/_AtlasForge-EveOffline/ARCHIVE_SUMMARY.md — complete

## Repo Consolidation — Phase 11 (Blender-Generator-for-AtlasForge Extraction)

- [x] Audit source repo via GitHub API
- [x] Archive 31 files → Archive/_Blender-Generator-for-AtlasForge/ (Python addon + docs)
- [x] Update Archive/_Blender-Generator-for-AtlasForge/ARCHIVE_SUMMARY.md — complete

## All 12 Repos Consolidated ✅

- [x] Phases 0-11 complete — all source repos audited, extracted, and archived
- [x] Build verification: 745/745 tests pass, no regressions
- [x] Spaghetti audit: 7/8 items resolved (1 deferred to workspace phase)
- [ ] Post-consolidation: namespace migration (atlas:: → NF::), CMakeLists wiring, include path normalization

## Phase 11 — Suite Integration (S1: Tool Wiring) ✅

- [x] Add AIAnalysis ChangeEventType to Pipeline module
- [x] Implement ToolAdapter abstract base class with emitEvent helper
- [x] Implement BlenderGenAdapter — accepts AssetImported, emits AnimationExported
- [x] Implement ContractScannerAdapter — accepts ScriptUpdated, emits ContractIssue
- [x] Implement ReplayMinimizerAdapter — accepts ReplayExported, emits minimized replay
- [x] Implement SwissAgentAdapter — accepts all event types, emits AIAnalysis
- [x] Implement ArbiterAdapter — accepts ContractIssue + WorldChanged, emits AIAnalysis
- [x] Implement ToolRegistry — central dispatch connecting PipelineWatcher to adapters
- [x] Add ToolWiring.cpp source file, update Pipeline CMakeLists
- [x] Add test_tool_wiring.cpp with 17 tests covering all adapters + registry
- [x] Build verification: 762/762 tests pass (745 existing + 17 new)
- [x] Update Docs/Game/ROADMAP.md — S1 marked Done, Phase 11 marked Active
- [x] Update Docs/Game/TASKS.md — S1 checklist added

## Phase 11 — Suite Integration (S2: BlenderGen Bridge) ✅

- [x] Add AssetImportStatus enum with name helper to Pipeline.h
- [x] Add AssetImportResult struct (sourcePath, guid, assetType, status, error)
- [x] Implement BlenderBridge class — importAsset, isImported, guidForPath, history
- [x] BlenderBridge validates asset files exist and are non-empty
- [x] BlenderBridge detects asset type from metadata (type=mesh/rig/clip)
- [x] BlenderBridge falls back to extension / event type for type detection
- [x] BlenderBridge registers imported assets in Manifest with GUID
- [x] BlenderBridge handles re-imports by updating existing Manifest records
- [x] BlenderBridge tracks import/failure counts and full history
- [x] Add BlenderBridge.cpp source file, update Pipeline CMakeLists
- [x] Add test_blender_bridge.cpp with 15 tests covering all bridge functionality
- [x] End-to-end test: PipelineWatcher → ToolRegistry → BlenderBridge
- [x] Build verification: 777/777 tests pass (762 existing + 15 new)
- [x] Update Docs/Game/ROADMAP.md — S2 marked Done
- [x] Update Docs/Game/TASKS.md — S2 checklist added
- [x] Update Tools/BlenderGenerator/README.md — BG tasks marked Done

## Phase 11 — Suite Integration (S3: SwissAgent Integration) ✅

- [x] Add AnalysisRequest, AnalysisResult, BrokerSession structs to Pipeline.h
- [x] Add WorkspaceBroker class to Pipeline.h — session management, context indexing, analysis
- [x] SA-1: Session management — create, resume, close, list active sessions
- [x] SA-2: Context indexing — per-file event tracking, hot-path detection
- [x] SA-3: Analysis requests — event analysis with summary and recommendations
- [x] SA-4: Broker statistics — totalAnalyses, totalEventsIndexed counters
- [x] SA-5: Full pipeline integration — attachToWatcher with feedback loop prevention
- [x] Implement WorkspaceBroker.cpp source file, update Pipeline CMakeLists
- [x] Add test_workspace_broker.cpp with 18 tests covering all SA milestones
- [x] End-to-end test: PipelineWatcher → ToolRegistry + WorkspaceBroker
- [x] Feedback loop prevention: SwissAgent events skipped to avoid infinite loops
- [x] Build verification: 795/795 tests pass (777 existing + 18 new)
- [x] Update Docs/Game/ROADMAP.md — S3 marked Done
- [x] Update Docs/Game/TASKS.md — S3 checklist added

## Phase 11 — Suite Integration (S4: ArbiterAI Integration) ✅

- [x] Add RuleSeverity enum, ArbiterRule, RuleViolation structs to Pipeline.h
- [x] Add ArbiterReasoner class to Pipeline.h — rule engine, evaluation, CI gate
- [x] AB-1: Rule management — addRule, loadRulesFromJson, findRule
- [x] AB-2: Rule evaluation — glob-pattern path matching, event type filtering
- [x] AB-3: Default game balance rules — 8 built-in rules across all event types
- [x] AB-4: Violation tracking — processEvent, violationsForPath, event counting
- [x] AB-5: CI gate — passesGate (Error/Critical = fail), human-readable summary
- [x] Implement ArbiterReasoner.cpp source file, update Pipeline CMakeLists
- [x] Add test_arbiter_reasoner.cpp with 22 tests covering all AB milestones
- [x] End-to-end test: PipelineWatcher → ToolRegistry + ArbiterReasoner
- [x] Feedback loop prevention: ArbiterAI events skipped to avoid infinite loops
- [x] Build verification: 817/817 tests pass (795 existing + 22 new)
- [x] Update Docs/Game/ROADMAP.md — S4 marked Done
- [x] Update Docs/Game/TASKS.md — S4 checklist added

## Phase 11 — Suite Integration (S5: Full Suite Validation) ✅

- [x] All 5 tool adapters active simultaneously (BlenderGen, ContractScanner, ReplayMinimizer, SwissAgent, Arbiter)
- [x] BlenderBridge asset import concurrent with tool dispatch
- [x] WorkspaceBroker AI analysis concurrent with ToolRegistry dispatch
- [x] ArbiterReasoner rule evaluation concurrent with all other tools
- [x] Full event matrix test: all 6 event types routed to correct handlers
- [x] Feedback loop prevention verified: each tool skips its own events
- [x] Manifest persistence roundtrip after full suite run
- [x] CI gate summary after full suite run
- [x] Add test_full_suite.cpp with 9 comprehensive integration tests
- [x] Build verification: 826/826 tests pass (817 existing + 9 new)
- [x] Update Docs/Game/ROADMAP.md — S5 marked Done, Phase 11 marked Done

## Phase 11 Complete ✅

Phase 11 (Suite Integration) fully delivered:
- S0: Pipeline Core — PipelineWatcher, Manifest, WatchLog, ChangeEvent (17 tests)
- S1: Tool Wiring — 5 ToolAdapters + ToolRegistry (17 tests)
- S2: BlenderGen Bridge — BlenderBridge asset import + Manifest registration (15 tests)
- S3: SwissAgent Integration — WorkspaceBroker session/context/analysis (18 tests)
- S4: ArbiterAI Integration — ArbiterReasoner rules/violations/CI gate (22 tests)
- S5: Full Suite Validation — all tools active simultaneously (9 tests)
Total: 826 tests, 0 failures. Pipeline module: 5 source files, 6 test files.

## M1 — Usable Editor ✅

- [x] GLFWWindowProvider: GLFW window creation/lifecycle abstraction (stub-compilable without GLFW)
- [x] ImGuiLayer: ImGui initialization, frame lifecycle, docking space, panel helpers
- [x] ImGuiBackend: UIBackend subclass routing through ImGui draw lists
- [x] GLFWInputAdapter: GLFW callbacks → InputSystem bridge with event injection
- [x] Editor main.cpp: NF_USE_GLFW path alongside existing Win32/GDI backend
- [x] Add test_m1_editor.cpp with 23 comprehensive tests
- [x] Build verification: 850/850 tests pass (826 existing + 24 new)
- [x] Update Docs/Game/ROADMAP.md — M1 marked Done

## G21 — Faction System ✅

- [x] FactionType enum (Military, Corporate, Scientific, Religious, Criminal, Pirate, Colonial, Independent) + name function
- [x] FactionStanding enum (Hostile, Unfriendly, Neutral, Friendly, Allied) + name function
- [x] FactionTerritory struct: sector control, resource output, erosion/reinforcement
- [x] Faction class: identity, influence, wealth, military power, territory management, tick
- [x] GameFactionRelation struct: reputation-based standing transitions, war/peace, treaties
- [x] GameFactionManager class: add/find/remove factions, relation management, allied/hostile queries
- [x] Add 10 test cases for G21 (FactionType names, standing names, init, territory, wealth, erosion, reputation, war/peace, manager CRUD, alliances)
- [x] Build verification: 860/860 tests pass (850 existing + 10 new)
- [x] Update Docs/Game/ROADMAP.md — G21 marked Done

## M1 + G21 Complete ✅

M1 (Usable Editor) fully delivered:
- GLFWWindowProvider, ImGuiLayer, ImGuiBackend, GLFWInputAdapter (4 new headers)
- Editor main.cpp updated with NF_USE_GLFW path
- 23 new tests, all passing

G21 (Faction System) fully delivered:
- FactionType×8, FactionStanding×5, FactionTerritory, Faction, GameFactionRelation, GameFactionManager
- 10 new tests, all passing
Total: 860 tests, 0 failures.

## SP — Spec Pack (System Contracts) ✅

- [x] SP1: VoxelMaterialDef + VoxelMaterialTable — material properties with mining yields
- [x] SP2: CentrifugeState + CentrifugeJob + CentrifugeSystem — resource processing with tiers and power
- [x] SP3: LinkState + InterfacePort — physical interaction state machine
- [x] SP4: CollapseEvent + SandPhysicsSystem — voxel collapse simulation
- [x] SP5: BreachState + BreachGrid + BreachMinigame — hacking minigame
- [x] SP6: RigAIEvent + RigAIFeatures + RigAICore — R.I.G. AI event bus
- [x] 6 new JSON schemas: rig, material, interaction, power, recipe, centrifuge
- [x] 25 documentation files across core, systems, rig, gameplay, workspace, roadmap, audit
- [x] 24 new Catch2 tests (SP1-SP6)
- [x] Build verification: 884/884 tests pass (860 existing + 24 new)

## SP Complete ✅

Spec Pack delivered:
- 6 C++ contract stubs (VoxelMaterialTable, CentrifugeSystem, InterfacePort, SandPhysicsSystem, BreachMinigame, RigAICore)
- 6 JSON schemas (rig, material, interaction, power, recipe, centrifuge)
- 25 documentation files (game design, system specs, audit reports, roadmap)
- 24 new tests, all passing
Total: 884 tests, 0 failures.

## M2/S1 — Dev World Editing ✅

- [x] M2-1: Vec3i integer vector type for voxel coordinates
- [x] M2-2: NoiseParams struct (frequency, amplitude, octaves, lacunarity, persistence, seed)
- [x] M2-3: PCGPreset struct and PCGTuningPanel (EditorPanel subclass, DockSlot::Right)
  - Noise parameter editing, preset management, seed control, dirty tracking
- [x] M2-4: PlacedEntity struct and EntityPlacementTool
  - Template-based entity placement, grid snap, auto-incrementing IDs
- [x] M2-5: VoxelBrushShape enum, VoxelBrushSettings, PaintStroke, VoxelPaintTool
  - Brush editing (sphere/cube/cylinder), stroke recording, material palette (8 slots)
- [x] M2-6: PlaceEntityCommand, RemoveEntityCommand, PaintStrokeCommand, PCGParamChangeCommand
  - All implement ICommand for full undo/redo through CommandStack
- [x] M2-7: EditorUndoSystem — composite undo manager wrapping CommandStack
  - executePlaceEntity, executeRemoveEntity, executePaintStroke, executePCGChange
- [x] M2-8: WorldPreviewService — live world preview state management
  - PreviewState enum (Idle/Loading/Ready/Error), view center/radius, dirty tracking
- [x] M2-9: EditorApp integration — member variables, accessors, menu items, commands
  - PCG Tuning panel added to Edit menu, Entity Placement and Voxel Paint to Tools menu
- [x] M2-10: test_m2_editor.cpp — 25 new Catch2 test cases
- [x] Build verification: 909/909 tests pass (884 existing + 25 new)
- [x] Update Docs/Game/ROADMAP.md — M2/S1 marked Done

## M2/S1 Complete ✅

M2/S1 (Dev World Editing) fully delivered:
- 5 new systems: PCGTuningPanel, EntityPlacementTool, VoxelPaintTool, EditorUndoSystem, WorldPreviewService
- 4 undo commands: PlaceEntityCommand, RemoveEntityCommand, PaintStrokeCommand, PCGParamChangeCommand
- Supporting types: Vec3i, NoiseParams, PCGPreset, PlacedEntity, VoxelBrushShape, VoxelBrushSettings, PaintStroke, PreviewState
- Fully wired into EditorApp with menu items and commands
- 25 new tests, all passing
Total: 909 tests, 0 failures.
