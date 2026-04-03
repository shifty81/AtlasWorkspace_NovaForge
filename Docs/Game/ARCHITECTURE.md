# NovaForge — Architecture

## Module Dependency Graph

```
NF::Core ──────────────────────────────────────────────────────────────────┐
NF::Engine      → NF::Core                                                 │
NF::Renderer    → NF::Core, NF::Engine                                     │
NF::Physics     → NF::Core, NF::Engine                                     │
NF::Audio       → NF::Core                                                 │
NF::Animation   → NF::Core, NF::Engine                                     │
NF::Input       → NF::Core                                                 │
NF::Networking  → NF::Core                                                 │
NF::GraphVM     → NF::Core                                                 │
NF::AI          → NF::Core, NF::Engine, NF::GraphVM                        │
NF::World       → NF::Core, NF::Engine, NF::Game                           │
NF::UI          → NF::Core, NF::Renderer                                   │
NF::Game        → NF::Core, NF::Engine, NF::UI, NF::Renderer               │
NF::Editor      → NF::Core, NF::Engine, NF::Renderer, NF::Game, NF::UI    │
                                                                           │
NovaForgeEditor → NF::Editor + all modules ────────────────────────────────┘
NovaForgeGame   → NF::Game + NF::Renderer + NF::UI + supporting modules
NovaForgeServer → NF::Game + NF::Networking + NF::AI + NF::World
```

## Source Layout

```
Source/
├── Core/          # Math, memory, logging, events, reflection, serialization
├── Engine/        # ECS, world/level, behavior trees, asset system
├── Renderer/      # RHI (OpenGL via WGL/GLAD), forward pipeline, mesh, materials
├── Physics/       # Rigid bodies, collision detection, character controller
├── Audio/         # Device, spatial audio, mixer, cues
├── Animation/     # Skeleton, blend tree, state machine, IK
├── Input/         # Keyboard, mouse, gamepad, action mappings
├── Networking/    # Sockets, replication, sessions, lockstep/rollback
├── GraphVM/       # Deterministic bytecode VM, compiler, serialization
├── AI/            # Behavior graphs, memory, NPC logic, faction relationships
├── World/         # World gen: cube-sphere, voxel, terrain, galaxy, streaming
├── UI/            # Custom 2-D renderer (no ImGui) — quad batching + stb_easy_font
├── Game/          # Game layer: world, voxels, interaction loop, gameplay systems
├── Editor/        # Editor application, docking panels, viewport, toolbar
└── Programs/
    ├── NovaForgeEditor/   # Editor executable (development tool, does not ship)
    ├── NovaForgeGame/     # Standalone game client (shippable artifact)
    └── NovaForgeServer/   # Dedicated headless server
```

## Layer Responsibilities

| Layer | Responsibility |
|-------|---------------|
| **Core** | Foundation: math, memory, logging, events, reflection, serialization |
| **Engine** | ECS framework, world/level management, behavior trees, asset system |
| **Renderer** | OpenGL RHI, forward rendering pipeline, mesh, materials, shaders |
| **Physics** | Rigid body simulation, collision detection, character controller |
| **Audio** | Audio device, spatial audio, mixer, sound cues |
| **Animation** | Skeleton, blend tree, state machine, IK |
| **Input** | Keyboard, mouse, gamepad, action mapping |
| **Networking** | Client-server, P2P, lockstep/rollback, replication, sessions |
| **GraphVM** | Deterministic bytecode VM for visual scripting (14 graph types) |
| **AI** | Behavior graphs, NPC memory, faction relationships, fleet AI |
| **World** | Procedural world gen, chunk streaming, terrain, galaxies |
| **UI** | Custom 2D renderer, quad batching, text, panels (no ImGui) |
| **Game** | Voxel runtime, R.I.G. system, inventory, interaction loop |
| **Editor** | Docking layout, 14+ panels, project path service, command registry |

## Executables

| Binary | Purpose |
|--------|---------|
| `NovaForgeEditor` | Full editor + runtime (superset) — development tool |
| `NovaForgeGame` | Standalone game client — shippable artifact |
| `NovaForgeServer` | Dedicated headless server for multiplayer |
