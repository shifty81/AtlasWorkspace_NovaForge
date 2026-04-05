add_library(NF_UI STATIC
    src/UI.cpp
    src/AtlasUI/AtlasUI.cpp
    src/AtlasUI/PanelHost.cpp
    src/AtlasUI/Widgets/StackPanel.cpp
    src/AtlasUI/Widgets/Splitter.cpp
    src/AtlasUI/Widgets/Tabs.cpp
)

target_include_directories(NF_UI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NF_UI PUBLIC NF::Core NF::Renderer PRIVATE NF_CompileOptions)
add_library(NF::UI ALIAS NF_UI)
