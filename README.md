# NovaForge

> An editor-first C++20 game engine and toolset — engine runtime, world editor, visual scripting, AI, networking, and a full game layer in one unified monorepo.

NovaForge is a first-person, voxel-authoritative survival and construction game set in a procedurally generated galaxy. You pilot a **R.I.G. (Rig Interface Gear)** exo-frame, mine asteroids and terrain, salvage wrecks, build structures, and eventually command fleets across faction-contested sectors.

This repository is the **complete unified monorepo**: the engine core, the game-specific editor, the standalone game client, the dedicated server, the Graph VM for visual scripting, AI systems, development tooling, and the Blender pipeline — all context-aware across the full ecosystem.

**Primary development platform: Windows 10/11 (Visual Studio 2022).** Cross-platform (Linux, macOS) is architecturally maintained but not continuously tested.

---

## Current Status — All Phases Complete

| Phase | Goal | Status |
|-------|------|--------|
| **Phase 0** — Bootstrap | Project scaffold, build system, directory structure | ✅ Done |
| **Phase 1** — Core Engine | Core, Engine, Input modules with tests | ✅ Done |
| **Phase 2** — Rendering & Physics | OpenGL RHI, physics, audio, animation | ✅ Done |
| **Phase 3** — Voxel Runtime | Chunk data, edit API, serialization, game loop | ✅ Done |
| **Phase 4** — Editor | Docking layout, panels, viewport, editor services | ✅ Done |
| **Phase 5** — Graph VM | Deterministic bytecode VM, 14 graph types | ✅ Done |
| **Phase 6** — Multiplayer | Server authority, replication, lockstep/rollback | ✅ Done |
| **Phase 7** — AI & Tooling | SwissAgent, ArbiterAI, Blender pipeline | ✅ Done |
| **Phase 8** — Custom IDE | Project-aware IDE with cross-module awareness | ✅ Done |
| **Phase 9** — Polish & CI | Documentation, GitHub Actions, Docker, modding | ✅ Done |
| **G1–G17** — Game Phases | Full game layer (ships, fleets, economy, quests, progression, crafting, inventory) | ✅ Done |

> **599 Catch2 tests** across 12 modules — Core(55), Engine(25), Physics(19), Renderer(22), Audio(12), Animation(12), Game(243), GraphVM(20), Input(10), Editor(95), Networking(50), AI(40).

> See [`Docs/roadmap.md`](Docs/roadmap.md) for next milestones and [`Docs/Game/ROADMAP.md`](Docs/Game/ROADMAP.md) for the full phase history.

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

| Tool | Minimum Version | Notes |
|------|----------------|-------|
| CMake | 3.22 | CMakePresets v6 |
| Visual Studio | 2022 (17.x) | Primary — C++ Desktop workload required |
| MSVC | 19.30+ (VS 2022) | C++20, `/std:c++20` |
| GCC | 13+ | Linux/CI only |
| Clang | 15+ | Optional |
| OpenGL | 3.3 core | System driver |

GLAD and GLFW are fetched automatically via CMake FetchContent if not found locally.

---

## Quick Start — Windows (Primary)

### Prerequisites
- **Visual Studio 2022** with the **"Desktop development with C++"** workload
- **CMake 3.22+** (bundled with VS 2022 or install separately)
- **Git**

### Option A — Visual Studio IDE (Recommended)

1. Clone the repo:
   ```
   git clone https://github.com/shifty81/tempnovaforge.git NovaForge
   cd NovaForge
   ```
2. Open Visual Studio → **File → Open → CMake…** → select `CMakeLists.txt`
3. VS reads `CMakePresets.json` automatically — pick **"Windows x64 Debug (VS 2022)"**
4. Select `NovaForgeEditor` as the startup project and press **F5**

### Option B — Command Line (Developer Command Prompt)

```bat
:: Open "Developer Command Prompt for VS 2022"
git clone https://github.com/shifty81/tempnovaforge.git NovaForge
cd NovaForge

:: Generate .sln and build Debug
cmake --preset windows-x64-debug
cmake --build --preset windows-x64-debug

:: Run the editor
Builds\windows-x64-debug\bin\Debug\NovaForgeEditor.exe
```

### Option C — Convenience Scripts

```bat
:: Generate VS 2022 solution then build via MSBuild
.\Scripts\generate_vs_solution.bat
build.cmd Debug

:: Or using PowerShell
.\Scripts\generate_vs_solution.ps1
```

### Build with Tests

```bat
cmake --preset windows-x64-debug-tests
cmake --build --preset windows-x64-debug-tests
ctest --preset windows-x64-debug-tests
```

---

## Quick Start — Linux / macOS

```bash
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

---

## Available CMake Presets

| Preset | Generator | Config | Tests | Purpose |
|--------|-----------|--------|-------|---------|
| `windows-x64-debug` | VS 2022 | Debug | Off | Local Windows dev (**primary**) |
| `windows-x64-debug-tests` | VS 2022 | Debug | On | Windows dev + test suite |
| `windows-x64` | VS 2022 | Release | Off | Windows release build |
| `vs2022` | VS 2022 | Multi | On | VS IDE / `.sln` generation |
| `vs2019` | VS 2019 | Multi | On | VS 2019 fallback |
| `debug` | Ninja | Debug | On | Linux/macOS CI debug |
| `release` | Ninja | Release | Off | Linux/macOS CI release |

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
| [`Docs/roadmap.md`](Docs/roadmap.md) | Editor-first next milestones |
| [`Docs/Game/ROADMAP.md`](Docs/Game/ROADMAP.md) | Phase-by-phase delivery plan (full history) |
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
