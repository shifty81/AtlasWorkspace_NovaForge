# SwissAgent — Multi-Purpose AI Query Tool

> **Naming Note**: SwissAgent capabilities are now consolidated into **AtlasAI**. This folder is preserved for reference. See `Docs/Architecture/NAMING_CANON.md`.

`Tools/SwissAgent/` — CLI + optional editor-panel integration.

SwissAgent is the conversational front-end to the NovaForge workspace suite.
It reads the project manifest, source index, and pipeline history to answer
natural-language questions about the codebase and the current game state.
All code-generation output is delivered as `.change.json` events dropped into
`pipeline/changes/`, never written directly to source files.

---

## Tool Roadmap

| ID   | Milestone | Goal |
|------|-----------|------|
| SA-1 | CLI scaffold | `swissagent query "<question>"` prints an answer |
| SA-2 | Context-aware | Reads `manifest.json` + source index; answers reference actual code |
| SA-3 | Code generation | `swissagent generate "<goal>"` writes a code-change event to `pipeline/changes/` |
| SA-4 | Editor panel | SwissAgent chat panel embedded in the NovaForge editor window |
| SA-5 | Full sessions | Multi-turn conversation; session history written to `pipeline/sessions/` |

---

## Pipeline integration

SwissAgent communicates **only** through the pipeline directory:

- Reads: `manifest.json`, `pipeline/sessions/`, source-index cache
- Writes: `pipeline/changes/<timestamp>_swissagent_<event>.change.json`
- Writes: `pipeline/sessions/<session_id>.session.json`

---

## Usage (SA-1 target)

```
swissagent query "what does FPSCamera.yaw do?"
swissagent generate "add a new ship class called Titan"
swissagent session start
```
