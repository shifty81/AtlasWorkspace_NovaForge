# NovaForge

> A voxel-first open-world PvE space simulator built in C++20 — engine, editor, game, and tooling in one unified monorepo.

NovaForge is a first-person, voxel-authoritative survival and construction game set in a procedurally generated galaxy. You pilot a **R.I.G. (Rig Interface Gear)** exo-frame, mine asteroids and terrain, salvage wrecks, build structures, and eventually command fleets across faction-contested sectors.

This repository is the **complete unified monorepo**: the engine core, the game-specific editor, the standalone game client, the dedicated server, the Graph VM for visual scripting, AI systems, development tooling, and the Blender pipeline — all context-aware across the full ecosystem.

---

## Current Status — Phase 0 Complete

| Phase | Goal | Status |
|-------|------|--------|
| **Phase 0** — Bootstrap | Project scaffold, build system, directory structure | ✅ Done |
| **Phase 1** — Core Engine | Core, Engine, Input modules with tests | 🔧 In Progress |
| **Phase 2** — Rendering & Physics | OpenGL RHI, physics, audio, animation | 📋 Planned |
| **Phase 3** — Voxel Runtime | Chunk data, edit API, game loop | 📋 Planned |
| **Phase 4** — Editor | Docking layout, panels, viewport, editor services | 📋 Planned |
| **Phase 5** — Graph VM | Deterministic bytecode VM, 14 graph types | 📋 Planned |
| **Phase 6** — Multiplayer | Server authority, replication, lockstep/rollback | 📋 Planned |
| **Phase 7** — AI & Tooling | SwissAgent, ArbiterAI, Blender pipeline | 📋 Planned |
| **Phase 8** — Custom IDE | Project-aware IDE with cross-module awareness | 📋 Planned |
| **Phase 9** — Polish & CI | Documentation, GitHub Actions, Docker, modding | 📋 Planned |

> See [`Docs/Game/ROADMAP.md`](Docs/Game/ROADMAP.md) and [`Docs/Game/TASKS.md`](Docs/Game/TASKS.md) for the full backlog.

---

## Architecture

```
Source/
├── Core/          # Math, memory, logging, events, reflection, serialization
├── Engine/        # ECS, world/level, behavior trees, asset system
├── Renderer/      # RHI (OpenGL), forward pipeline, mesh, materials
├── Physics/       # Rigid bodies, collision, character controller
├── Audio/         # Device, spatial audio, mixer, cues
├── Animation/     # Skeleton, blend tree, state machine, IK
├── Input/         # Keyboard, mouse, gamepad, action mappings
├── Networking/    # Sockets, replication, sessions, lockstep/rollback
├── GraphVM/       # Deterministic bytecode VM, visual scripting (14 graph types)
├── AI/            # Behavior graphs, memory, NPC logic, faction relationships
├── World/         # World gen: cube-sphere, voxel, terrain, galaxy, streaming
├── UI/            # Custom 2-D renderer (no ImGui) — quad batching + stb_easy_font
├── Game/          # Game layer: voxels, R.I.G., interaction loop, inventory
├── Editor/        # Editor app, docking panels, viewport, toolbar
└── Programs/
    ├── NovaForgeEditor/   # Editor executable (dev tool, does not ship)
    ├── NovaForgeGame/     # Standalone game client (shippable artifact)
    └── NovaForgeServer/   # Dedicated headless server
```

### Supporting Directories

```
Config/            # Project configuration (novaforge.project.json)
Content/           # Game content (level definitions, DevWorld, assets)
Data/              # Game data: ships, modules, skills, missions (JSON, moddable)
Schemas/           # Versioned JSON schemas
Docs/              # All documentation
Tools/             # Development tools (SwissAgent, ArbiterAI, BlenderGenerator)
Tests/             # Catch2 unit tests
ThirdParty/        # Third-party dependencies
Scripts/           # Build & utility scripts
Archive/           # Historical reference from consolidated repos
```

### Module Dependency Graph

```
NF::Core ──────────────────────────────────────────────────────────────┐
NF::Engine      → NF::Core                                             │
NF::Renderer    → NF::Core, NF::Engine                                 │
NF::Physics     → NF::Core, NF::Engine                                 │
NF::Audio       → NF::Core                                             │
NF::Animation   → NF::Core, NF::Engine                                 │
NF::Input       → NF::Core                                             │
NF::Networking  → NF::Core                                             │
NF::GraphVM     → NF::Core                                             │
NF::AI          → NF::Core, NF::Engine, NF::GraphVM                    │
NF::World       → NF::Core, NF::Engine, NF::Game                       │
NF::UI          → NF::Core, NF::Renderer                               │
NF::Game        → NF::Core, NF::Engine, NF::UI, NF::Renderer           │
NF::Editor      → NF::Core, NF::Engine, NF::Renderer, NF::Game, NF::UI│
                                                                       │
NovaForgeEditor → NF::Editor + all modules ────────────────────────────┘
NovaForgeGame   → NF::Game + NF::Renderer + NF::UI + supporting modules
NovaForgeServer → NF::Game + NF::Networking + NF::AI + NF::World
```

---

## Build Requirements

| Tool | Minimum Version |
|------|----------------|
| CMake | 3.22 |
| C++ Compiler | GCC 13 · Clang 15 · MSVC 2022 (C++20) |
| OpenGL | 3.3 core (system driver) |

GLAD and GLFW are fetched automatically via CMake FetchContent if not found locally.

---

## Quick Start

```bash
# Clone
git clone https://github.com/shifty81/tempnovaforge.git NovaForge
cd NovaForge

# Configure (Debug, all targets + tests)
cmake --preset debug

# Build
cmake --build --preset debug --parallel

# Run tests
ctest --preset debug

# Run the Editor
./Builds/debug/bin/NovaForgeEditor

# Run the Game Client
./Builds/debug/bin/NovaForgeGame

# Run the Server
./Builds/debug/bin/NovaForgeServer
```

### Using the Makefile

```bash
make build           # Debug build (all targets)
make build-release   # Release build
make test            # Build and run tests
make editor          # Build editor only
make game            # Build game client only
make server          # Build server only
make clean           # Remove all build artifacts
make help            # Show all targets
```

### Using the build script

```bash
./Scripts/build_all.sh Debug         # Debug build
./Scripts/build_all.sh Release       # Release build
./Scripts/build_all.sh Debug --test  # Build + run tests
```

### Visual Studio Solution (Windows)

NovaForge supports generating Visual Studio `.sln` files via CMake presets:

```powershell
# Generate VS 2022 solution
cmake --preset vs2022

# Or use the convenience script
.\Scripts\generate_vs_solution.ps1

# Or batch file
.\Scripts\generate_vs_solution.bat
```

Open `Builds/vs2022/NovaForge.sln` in Visual Studio. Multi-configuration builds
(Debug/Release) are supported — select the configuration from the toolbar.

You can also open the project directly using **File → Open → CMake…** and
selecting the root `CMakeLists.txt`. Visual Studio 2019+ reads `CMakePresets.json`
natively.

### CMake Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `NF_BUILD_EDITOR` | `ON` | Build the NovaForge Editor |
| `NF_BUILD_GAME`   | `ON` | Build the standalone game client |
| `NF_BUILD_SERVER` | `ON` | Build the dedicated server |
| `NF_BUILD_TESTS`  | `OFF` | Build the Catch2 unit-test suite |

---

## Design Principles

- **Voxel layer is authoritative.** Structure, mining, repair, damage, and destruction are all voxel operations.
- **R.I.G. first.** The player suit platform is the primary gameplay object.
- **No ImGui.** The editor uses a fully custom UI renderer.
- **Determinism first.** All simulation must be bit-exact reproducible.
- **Phases over features.** Each phase has a tight, locked deliverable.
- **Editor does not ship.** The editor is a dev tool; `NovaForgeGame` is the artifact.
- **Everything is data.** Game content lives in JSON, fully moddable.
- **One repo, full context.** Engine, editor, game, server, tools, and data — all context-aware.

---

## Game Vision

Nova Forge is a **PvE space simulation** where you command ships, build fleets, explore procedurally generated star systems, and forge your legend across a living universe.

### Core Pillars

| Pillar | Description |
|--------|-------------|
| **Command** | Pilot ships from frigate to titan |
| **Build** | Modular ships, stations, habitats with snap-grid construction |
| **Lead** | AI fleet captains with personalities, morale, and grudges |
| **Discover** | Scan anomalies, salvage wrecks, decode ancient tech |
| **Forge** | Manufacture, refine, research — drive a living NPC economy |
| **Survive** | FPS interiors, EVA, oxygen management, planetary exploration |

### Four Factions

| Faction | Style | Specialty |
|---------|-------|-----------|
| **Solari** | Golden / elegant | Armor tanking, energy weapons |
| **Veyren** | Angular / utilitarian | Shield tanking, hybrid turrets |
| **Aurelian** | Sleek / organic | Speed, drones, electronic warfare |
| **Keldari** | Rugged / industrial | Missiles, shields, ECM |

---

## Documentation

| Document | Description |
|----------|-------------|
| [`Docs/Game/ROADMAP.md`](Docs/Game/ROADMAP.md) | Phase-by-phase delivery plan |
| [`Docs/Game/TASKS.md`](Docs/Game/TASKS.md) | Checked task list per phase |
| [`Docs/Game/ARCHITECTURE.md`](Docs/Game/ARCHITECTURE.md) | Module layout and design |
| [`Docs/Game/PROJECT_RULES.md`](Docs/Game/PROJECT_RULES.md) | Hard boundaries and coding rules |

---

## Consolidated From

This monorepo consolidates the following projects:
- [MasterRepoV001](https://github.com/shifty81/MasterRepoV001) — Canonical engine/editor structure
- [Atlas-NovaForge](https://github.com/shifty81/Atlas-NovaForge) — Full engine (418 files), server (537 files), client (156 files)
- [NovaForge](https://github.com/shifty81/NovaForge) — Game design, engine core, editor
- [SwissAgent](https://github.com/shifty81/SwissAgent) — AI development assistant
- [ArbiterAI](https://github.com/shifty81/ArbiterAI) — AI project memory/assistant
- [Blender-Generator-for-AtlasForge](https://github.com/shifty81/Blender-Generator-for-AtlasForge) — Procedural ship generation
- [Nova-Forge-Expeditions](https://github.com/shifty81/Nova-Forge-Expeditions) — Expedition content
- [Arbiter](https://github.com/shifty81/Arbiter) — AI arbiter system
- [MasterRepo](https://github.com/shifty81/MasterRepo) — Original master repo
- [MasterRepoRefactor](https://github.com/shifty81/MasterRepoRefactor) — Refactoring branch
- [NovaForge-Project](https://github.com/shifty81/NovaForge-Project) — Project planning

---

## License

GNU General Public License v3.0 — see [LICENSE](LICENSE).
