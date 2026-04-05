# AtlasAI — Rule-Based Decision Engine

> **Naming Note**: This tool is now canonically named **AtlasAI**. The folder name `ArbiterAI` is a legacy path and will be updated in a future migration pass. See `Docs/Architecture/NAMING_CANON.md`.

`Tools/ArbiterAI/` — C++ headless evaluation binary.

AtlasAI evaluates a set of declarative rules against a JSON context
(game stats, world state, economy snapshot) and reports which rules fired
and what actions they recommend.  Primary use cases are balance checking,
CI invariant enforcement, and real-time "bad state" detection during PIE.

---

## Tool Roadmap

| ID   | Milestone | Goal |
|------|-----------|------|
| AB-1 | Rule format | YAML/JSON rule file: `{when: <condition>, then: <action>}` |
| AB-2 | CLI eval | `atlasai eval rules.yaml context.json` → prints matched rules + actions |
| AB-3 | Balance rules | Read economy/combat stats from pipeline → flag balance issues |
| AB-4 | Editor panel | "Balance Checker" tab shows rule-firing counts for current world state |
| AB-5 | CI gate | Fails the build if any critical-severity rule fires |

---

## Pipeline integration

- Reads: `pipeline/worlds/active.world.json`, game stats exported by the engine
- Writes: `pipeline/changes/<timestamp>_atlasai_ContractIssue.change.json` for flagged issues

---

## Usage (AB-2 target)

```
atlasai eval rules/balance.yaml pipeline/worlds/active.world.json
```

### Example rule file

```yaml
rules:
  - name: "ore_price_floor"
    severity: warning
    when: "economy.orePrice < 10"
    then: "alert: ore price too low, check supply/demand balance"

  - name: "faction_relation_sanity"
    severity: critical
    when: "factions.pirates.standing > 90"
    then: "flag: pirates should never be friendly to player at game start"
```
