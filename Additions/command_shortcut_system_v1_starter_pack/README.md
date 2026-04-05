# Command + Shortcut System v1 Starter Pack

## Included
- Command ids and specs
- Key chord model
- Command registry
- Shortcut router
- Command manager singleton
- Core command bootstrap examples

## Intended follow-on
- Menu and context menu binding
- Toolbar command binding
- Tooltip shortcut text wiring
- Command palette integration
- Panel-scoped command contexts
- AtlasAI action dispatch bridge

## Suggested CMake additions
Add these files to `NF_UI`:
- `Source/UI/src/AtlasUI/Commands/CommandTypes.cpp`
- `Source/UI/src/AtlasUI/Commands/CommandRegistry.cpp`
- `Source/UI/src/AtlasUI/Commands/ShortcutRouter.cpp`
- `Source/UI/src/AtlasUI/Commands/CommandManager.cpp`
- `Source/UI/src/AtlasUI/Commands/CommandDefaults.cpp`
- `Source/UI/src/AtlasUI/Commands/CommandHelpers.cpp`

And export umbrella includes from your AtlasUI root header.
