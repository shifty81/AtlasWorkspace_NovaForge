add_library(NF_UI STATIC
    src/UI.cpp

    # AtlasUI — Theme
    src/AtlasUI/Theme/ThemeDefaults.cpp
    src/AtlasUI/Theme/ThemeManager.cpp

    # AtlasUI — Core
    src/AtlasUI/PanelHost.cpp

    # AtlasUI — Widgets
    src/AtlasUI/Widgets/Button.cpp
    src/AtlasUI/Widgets/Panel.cpp
    src/AtlasUI/Widgets/TabBar.cpp
    src/AtlasUI/Widgets/Splitter.cpp
    src/AtlasUI/Widgets/Toolbar.cpp
    src/AtlasUI/Widgets/Tooltip.cpp
    src/AtlasUI/Widgets/TextInput.cpp
    src/AtlasUI/Widgets/Dropdown.cpp
    src/AtlasUI/Widgets/PropertyRow.cpp
    src/AtlasUI/Widgets/NotificationCard.cpp

    # AtlasUI — Commands
    src/AtlasUI/Commands/CommandTypes.cpp
    src/AtlasUI/Commands/CommandRegistry.cpp
    src/AtlasUI/Commands/CommandManager.cpp
    src/AtlasUI/Commands/ShortcutRouter.cpp
    src/AtlasUI/Commands/CommandDefaults.cpp

    # AtlasUI — Services
    src/AtlasUI/Services/FocusService.cpp
    src/AtlasUI/Services/PopupHost.cpp
    src/AtlasUI/Services/TooltipService.cpp
    src/AtlasUI/Services/NotificationHost.cpp
)
target_include_directories(NF_UI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NF_UI PUBLIC NF::Core NF::Renderer PRIVATE NF_CompileOptions)
add_library(NF::UI ALIAS NF_UI)
