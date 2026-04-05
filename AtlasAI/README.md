# AtlasAI — Unified AI Broker

> **Canonical Name:** AtlasAI
> **Legacy Names:** ArbiterAI, SwissAgent, Arbiter — all consolidated here.

## Overview

AtlasAI is the **single AI integration point** for the NovaForge project. It unifies
all AI-driven functionality under one broker that communicates exclusively through the
`.novaforge/pipeline/` directory.

No game code, editor code, or engine code calls any AI system directly. All AI requests
and responses are mediated by AtlasAI through pipeline `ChangeEvent` files.

## Subsystems

| Subsystem | Directory | Description |
|-----------|-----------|-------------|
| **Atlas_Arbiter** | `AtlasAI/Atlas_Arbiter/` | Rule-based decision engine — evaluates declarative rules against JSON context |
| **Atlas_SwissAgent** | `AtlasAI/Atlas_SwissAgent/` | Conversational AI query tool — workspace-aware code generation and analysis |

## Architecture

```
┌──────────────────────────────────────────┐
│              AtlasAI Broker              │
│  ┌────────────────┐ ┌─────────────────┐  │
│  │ Atlas_Arbiter   │ │ Atlas_SwissAgent│  │
│  │ (rule engine)   │ │ (query agent)   │  │
│  └────────┬───────┘ └────────┬────────┘  │
│           └────────┬─────────┘           │
│                    │                     │
│          .novaforge/pipeline/            │
│           (ChangeEvent I/O)              │
└──────────────────────────────────────────┘
```

## Pipeline Contract

- **Input:** Reads `pipeline/changes/*.change.json` events
- **Output:** Writes `pipeline/changes/<timestamp>_atlasai_<type>.change.json`
- **Manifest:** Registered in `pipeline/manifest.json`
- **No direct file writes** — all output goes through pipeline events

## Integration Points

| Integration | Method |
|-------------|--------|
| Editor → AtlasAI | Editor writes pipeline events; AtlasAI responds with change events |
| Build → AtlasAI | Build logs routed to `Logs/build.logger`; AtlasAI watches for issues |
| CI → AtlasAI | ContractScanner findings written as change events for AtlasAI review |

## Current Status

- **Phase:** C1 MasterRepoRefactor — directory structure established
- **S1 Tool Wiring:** ⬜ Queued — full end-to-end pipeline event handling
- **S4 ArbiterAI Integration:** ⬜ Queued — full reasoning broker active

## Legacy Tool Migration

The following Tools/ directories contain legacy implementations that are being
consolidated into AtlasAI:

| Legacy Path | Target | Status |
|-------------|--------|--------|
| `Tools/ArbiterAI/` | `AtlasAI/Atlas_Arbiter/` | 📋 Reference preserved |
| `Tools/SwissAgent/` | `AtlasAI/Atlas_SwissAgent/` | 📋 Reference preserved |

Both legacy directories remain in `Tools/` as reference implementations until S1 Tool
Wiring is complete. At that point, active code migrates here and legacy paths are archived.
