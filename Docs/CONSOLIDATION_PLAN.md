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
| 3 | Nova-Forge-Expeditions | Richest game codebase | ✅ Audited |
| 4 | Atlas-NovaForge | Engine+game merge attempt | ⬜ Queued |
| 5 | AtlasForge | Original engine | ⬜ Queued |
| 6 | NovaForge-Project | Game project structure + rules | ⬜ Queued |
| 7 | SwissAgent | AI → Atlas_SwissAgent | ⬜ Queued |
| 8 | ArbiterAI | AI → Atlas_Arbiter | ⬜ Queued |
| 9 | Arbiter | AI → Atlas_Arbiter | ⬜ Queued |
| 10 | AtlasForge-EveOffline | Networking prototype | ⬜ Queued |
| 11 | Blender-Generator-for-AtlasForge | Tools/Atlas_BlenderGen | ⬜ Queued |

## Internal Consolidation (C3)

Phase C3 consolidates internal duplicate structures and integrates pending
content that was already present in the repo but not yet landed in the main tree.

| Task | Description | Status |
|------|-------------|--------|
| C3-1 | Archive C2 (AtlasToolingSuite) | ✅ Done — `Archive/_AtlasToolingSuite/` |
| C3-2 | Integrate Additions/ starter packs into Source/UI/ | ✅ Done — AtlasUI.h, SimplePanel, Label, Tabs, StackPanel, WidgetKit |
| C3-3 | Consolidate duplicate AtlasAI locations | ✅ Done — `Tools/AtlasAI/` → `AtlasAI/` (root) |
| C3-4 | Deprecate legacy tool paths | ✅ Done — `Tools/ArbiterAI/`, `Tools/SwissAgent/` marked deprecated |
| C3-5 | Scaffold archive stubs for phases 3–11 | ✅ Done — `Archive/_RepoName/ARCHIVE_SUMMARY.md` for each |
| C3-6 | Update CMakeLists.txt with new sources | ✅ Done — 3 new .cpp files |
| C3-7 | Update tracking docs | ✅ Done |

## Archive Protocol

When a repo is fully merged:
1. Create `Archive/_RepoName/ARCHIVE_SUMMARY.md`
2. Create `Archive/_RepoName/usable_snippets/` with extracted code snippets
3. Create `Archive/_RepoName/docs_archive/` with all original docs
4. Push to tempnovaforge
5. User pulls locally then removes/archives the source repo on GitHub

## UI Panel Migration (U1–U8)

All legacy `EditorPanel`-derived panels have been migrated to the AtlasUI framework.
New implementations live under `Source/UI/include/NF/UI/AtlasUI/Panels/` and use
`NF::UI::AtlasUI::PanelBase` as their base class.

| Phase | Panel | AtlasUI Class | Panel ID | Status |
|-------|-------|---------------|----------|--------|
| U1 | InspectorPanel | `InspectorPanel` | `atlas.inspector` | ✅ Done |
| U2 | HierarchyPanel | `HierarchyPanel` | `atlas.hierarchy` | ✅ Done |
| U3 | ContentBrowserPanel | `ContentBrowserPanel` | `atlas.content_browser` | ✅ Done |
| U4 | ConsolePanel | `ConsolePanel` | `atlas.console` | ✅ Done |
| U5 | IDEPanel | `IDEPanel` | `atlas.ide` | ✅ Done |
| U6 | GraphEditorPanel | `GraphEditorPanel` | `atlas.graph_editor` | ✅ Done (shell) |
| U7 | ViewportPanel | `ViewportPanel` | `atlas.viewport` | ✅ Done (shell) |
| U8 | Legacy deprecation | — | — | ✅ Done |

Legacy `EditorPanel` classes in `Source/Editor/include/NF/Editor/Editor.h` are marked
as DEPRECATED with comments pointing to the new AtlasUI implementations.

## Archive Inventory

| Archive Directory | Phase | Status |
|-------------------|-------|--------|
| `Archive/_MasterRepo/` | C0 | ✅ Archived |
| `Archive/_MasterRepoRefactor/` | C1 | ✅ Archived |
| `Archive/_AtlasToolingSuite/` | C2 | ✅ Archived |
| `Archive/_Nova-Forge-Expeditions/` | 3 | ✅ Audited |
| `Archive/_Atlas-NovaForge/` | 4 | ⬜ Scaffolded |
| `Archive/_AtlasForge/` | 5 | ⬜ Scaffolded |
| `Archive/_NovaForge-Project/` | 6 | ⬜ Scaffolded |
| `Archive/_SwissAgent/` | 7 | ⬜ Scaffolded |
| `Archive/_ArbiterAI/` | 8 | ⬜ Scaffolded |
| `Archive/_Arbiter/` | 9 | ⬜ Scaffolded |
| `Archive/_AtlasForge-EveOffline/` | 10 | ⬜ Scaffolded |
| `Archive/_Blender-Generator-for-AtlasForge/` | 11 | ⬜ Scaffolded |

## Naming Convention

All AI systems are renamed to `Atlas_` prefix and live under `AtlasAI/`:
- Arbiter → Atlas_Arbiter
- ArbiterAI → merged into Atlas_Arbiter  
- SwissAgent → Atlas_SwissAgent
- All AI functionality routes through the AtlasAI broker

## Canonical Directory Locations

| System | Canonical Path | Legacy Paths (deprecated) |
|--------|---------------|--------------------------|
| AtlasAI Broker | `AtlasAI/` | `Tools/AtlasAI/`, `Tools/ArbiterAI/`, `Tools/SwissAgent/` |
| Atlas_Arbiter | `AtlasAI/Atlas_Arbiter/` | `Tools/ArbiterAI/` |
| Atlas_SwissAgent | `AtlasAI/Atlas_SwissAgent/` | `Tools/SwissAgent/` |
| AtlasUI Framework | `Source/UI/include/NF/UI/AtlasUI/` | `Additions/atlasui_widget_kit_starter_patch/` |
| Spec Rollup | `Docs/SpecRollup/` | `Additions/atlas_workspace_novaforge_spec_rollup_pack/` |
| BlenderGenerator | `Tools/BlenderGenerator/` | (future: `Tools/Atlas_BlenderGen/`) |
| ContractScanner | `Tools/ContractScanner/` | — |
| ReplayMinimizer | `Tools/ReplayMinimizer/` | — |

## Spaghetti Audit Flags

- [x] Duplicate test dirs: `tests/`, `testing/`, `atlas_tests/` → consolidated to `Tests/`
- [x] Mixed case dirs: `tools/` vs `Tools/` → normalized to `Tools/`
- [x] `My Repos.rtf` at root → moved to `Archive/`
- [x] Inconsistent AI naming → all `Atlas_` prefix, `AtlasAI/` canonical location
- [x] Duplicate AtlasAI directories (`Tools/AtlasAI/` vs `AtlasAI/`) → consolidated to root `AtlasAI/`
- [x] Legacy tool READMEs → updated with deprecation markers
- [ ] Intake/DropBox at root → move to `Tools/Atlas_DevTools/` (deferred to workspace phase)
- [ ] Loose oversized .md files at root (SwissAgent) → move to `Docs/` (pending Phase 7 merge)
- [ ] Loose .zip at root (tempnovaforge) → move to `Archive/` (none found — resolved)
- [ ] `From old repo/` + `legacy/` + `archive/` in active dirs → move to `Archive/` (none found — resolved)
