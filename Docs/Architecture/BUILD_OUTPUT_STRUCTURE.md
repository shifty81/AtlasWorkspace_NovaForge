# NovaForge — Build Output Directory Structure

## Overview

NovaForge uses CMake with presets to manage builds across platforms. All build
artifacts land under `Builds/<preset>/` with a consistent layout.

## Canonical Directory Layout

```
Builds/<preset>/
├── bin/
│   ├── <Config>/                    # Debug, Release, RelWithDebInfo
│   │   ├── NovaForgeEditor(.exe)    # Development editor
│   │   ├── NovaForgeGame(.exe)      # Standalone game client
│   │   ├── NovaForgeServer(.exe)    # Dedicated headless server
│   │   └── Tests/                   # All test executables
│   │       ├── NF_CoreTests(.exe)
│   │       ├── NF_EngineTests(.exe)
│   │       ├── NF_PhysicsTests(.exe)
│   │       ├── NF_GameTests(.exe)
│   │       ├── NF_GraphVMTests(.exe)
│   │       ├── NF_InputTests(.exe)
│   │       ├── NF_RendererTests(.exe)
│   │       ├── NF_AudioTests(.exe)
│   │       ├── NF_AnimationTests(.exe)
│   │       ├── NF_NetworkingTests(.exe)
│   │       ├── NF_AITests(.exe)
│   │       ├── NF_EditorTests(.exe)
│   │       ├── NF_PipelineTests(.exe)
│   │       └── NF_UITests(.exe)
│   └── (single-config generators put executables here directly)
├── lib/
│   └── <Config>/                    # Static libraries (.a / .lib)
└── compile_commands.json            # For IDE integration
```

## Build Presets

| Preset | Generator | Platform | Build Dir |
|--------|-----------|----------|-----------|
| `debug` | Ninja | Linux/macOS | `Builds/debug` |
| `release` | Ninja | Linux/macOS | `Builds/release` |
| `vs2022` | VS 2022 | Windows | `Builds/vs2022` |
| `windows-x64-debug` | VS 2022 | Windows | `Builds/windows-x64-debug` |
| `windows-x64-debug-tests` | VS 2022 | Windows | `Builds/windows-x64-debug-tests` |
| `ci-debug` | Ninja | CI (Linux) | `Builds/ci-debug` |
| `ci-release` | Ninja | CI (Linux) | `Builds/ci-release` |

## Build Scripts

### Linux / macOS
```bash
# Full build with progress bars and log mirroring
./Scripts/build.sh Debug --test

# Build specific target
./Scripts/build.sh Debug --editor
./Scripts/build.sh Debug --game
./Scripts/build.sh Debug --server

# Quick build via Make
make build           # Debug, all targets
make editor          # Debug, editor only
make test            # Debug, build + run tests
```

### Windows
```batch
:: Full build with progress and logging
Scripts\build_win.cmd Debug --test

:: Build specific target
Scripts\build_win.cmd Debug --editor

:: Legacy MSBuild wrapper
build.cmd Debug
```

## Log Mirroring

All build output is simultaneously:
1. Displayed on screen with progress bars and colors
2. Written to `Logs/build.log` (plain text, ANSI stripped)
3. Test results written to `Logs/test.log`

The editor also writes session logs to `Logs/editor.log`.

## Runtime Directories

| Directory | Purpose | Gitignored |
|-----------|---------|------------|
| `Saved/` | Editor session state (layout, settings) | Yes |
| `Logs/` | Build and runtime log files | Yes (*.log) |
| `Content/` | Engine-level content (fonts, definitions) | No |
| `Data/` | Game configuration data (ships, modules, etc.) | No |
| `Config/` | Project configuration | No |
