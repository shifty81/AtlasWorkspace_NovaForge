# NovaForge — Repo Consolidation Plan

## Overview

All 12 source repos are being merged into `tempnovaforge` (this repo).
MasterRepo (v001) provides the structural skeleton.
Each repo is audited, refactored, and merged in phase order.
When a repo is fully merged, its contents are archived under `Archive/_RepoName/` and the source repo is retired.

## Phase Order

| Phase | Repo | Priority | Status |
|-------|------|----------|--------|
| 0 | MasterRepo | Structural seed | ✅ Done |
| 1 | MasterRepoRefactor | Structure + Atlas dirs | ✅ Done |
| 2 | AtlasToolingSuite | Tool suite | ✅ Done |
| 3 | Nova-Forge-Expeditions | Richest game codebase | ⬜ Queued |
| 4 | Atlas-NovaForge | Engine+game merge attempt | ⬜ Queued |
| 5 | AtlasForge | Original engine | ⬜ Queued |
| 6 | NovaForge-Project | Game project structure + rules | ⬜ Queued |
| 7 | SwissAgent | AI → Atlas_SwissAgent | ⬜ Queued |
| 8 | ArbiterAI | AI → Atlas_Arbiter | ⬜ Queued |
| 9 | Arbiter | AI → Atlas_Arbiter | ⬜ Queued |
| 10 | AtlasForge-EveOffline | Networking prototype | ⬜ Queued |
| 11 | Blender-Generator-for-AtlasForge | Tools/Atlas_BlenderGen | ⬜ Queued |

## Archive Protocol

When a repo is fully merged:
1. Create `Archive/_RepoName/ARCHIVE_SUMMARY.md`
2. Create `Archive/_RepoName/usable_snippets/` with extracted code snippets
3. Create `Archive/_RepoName/docs_archive/` with all original docs
4. Push to tempnovaforge
5. User pulls locally then removes/archives the source repo on GitHub

## Naming Convention

All AI systems are renamed to `Atlas_` prefix and live under `AtlasAI/`:
- Arbiter → Atlas_Arbiter
- ArbiterAI → merged into Atlas_Arbiter  
- SwissAgent → Atlas_SwissAgent
- All AI functionality routes through the AtlasAI broker

## Spaghetti Audit Flags (to fix during merge)

- [ ] Duplicate test dirs: `tests/`, `testing/`, `atlas_tests/` → consolidate to `Tests/`
- [ ] Mixed case dirs: `tools/` vs `Tools/` → normalize to `Tools/`
- [ ] Intake/DropBox at root → move to `Tools/Atlas_DevTools/`
- [ ] Loose oversized .md files at root (SwissAgent) → move to `Docs/`
- [ ] Loose .zip at root (tempnovaforge) → move to `Archive/`
- [ ] `My Repos.rtf` at root → move to `Archive/`
- [ ] Inconsistent AI naming → all Atlas_ prefix
- [ ] `From old repo/` + `legacy/` + `archive/` in active dirs → move to `Archive/`
