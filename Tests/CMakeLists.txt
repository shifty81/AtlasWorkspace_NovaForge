# ── Test suite ────────────────────────────────────────────────────

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

# Core tests
add_executable(NF_CoreTests
    Core/test_core.cpp
)
target_link_libraries(NF_CoreTests PRIVATE NF::Core Catch2::Catch2WithMain)
catch_discover_tests(NF_CoreTests)

# Engine tests
add_executable(NF_EngineTests
    Engine/test_engine.cpp
)
target_link_libraries(NF_EngineTests PRIVATE NF::Engine NF::Core Catch2::Catch2WithMain)
catch_discover_tests(NF_EngineTests)

# Physics tests
add_executable(NF_PhysicsTests
    Physics/test_physics.cpp
)
target_link_libraries(NF_PhysicsTests PRIVATE NF::Physics NF::Core Catch2::Catch2WithMain)
catch_discover_tests(NF_PhysicsTests)

# Game tests
add_executable(NF_GameTests
    Game/test_game.cpp
)
target_link_libraries(NF_GameTests PRIVATE NF::Game NF::Core Catch2::Catch2WithMain)
catch_discover_tests(NF_GameTests)

# GraphVM tests
add_executable(NF_GraphVMTests
    GraphVM/test_graphvm.cpp
)
target_link_libraries(NF_GraphVMTests PRIVATE NF::GraphVM NF::Core Catch2::Catch2WithMain)
catch_discover_tests(NF_GraphVMTests)

# Input tests
add_executable(NF_InputTests
    Input/test_input.cpp
)
target_link_libraries(NF_InputTests PRIVATE NF::Input NF::Core Catch2::Catch2WithMain)
catch_discover_tests(NF_InputTests)

# Renderer tests
add_executable(NF_RendererTests
    Renderer/test_renderer.cpp
)
target_link_libraries(NF_RendererTests PRIVATE NF::Renderer NF::Core Catch2::Catch2WithMain)
catch_discover_tests(NF_RendererTests)

# Audio tests
add_executable(NF_AudioTests
    Audio/test_audio.cpp
)
target_link_libraries(NF_AudioTests PRIVATE NF::Audio NF::Core Catch2::Catch2WithMain)
catch_discover_tests(NF_AudioTests)

# Animation tests
add_executable(NF_AnimationTests
    Animation/test_animation.cpp
)
target_link_libraries(NF_AnimationTests PRIVATE NF::Animation NF::Core Catch2::Catch2WithMain)
catch_discover_tests(NF_AnimationTests)

# Networking tests
add_executable(NF_NetworkingTests
    Networking/test_networking.cpp
)
target_link_libraries(NF_NetworkingTests PRIVATE NF::Networking NF::Engine NF::Core Catch2::Catch2WithMain)
catch_discover_tests(NF_NetworkingTests)

# AI tests
add_executable(NF_AITests
    AI/test_ai.cpp
)
target_link_libraries(NF_AITests PRIVATE NF::AI NF::Engine NF::Core NF::GraphVM Catch2::Catch2WithMain)
catch_discover_tests(NF_AITests)

# Editor tests
add_executable(NF_EditorTests
    Editor/test_editor.cpp
)
target_link_libraries(NF_EditorTests PRIVATE
    NF::Editor NF::Game NF::Renderer NF::UI NF::Engine NF::Core NF::GraphVM
    Catch2::Catch2WithMain
)
catch_discover_tests(NF_EditorTests)

# Pipeline tests
add_executable(NF_PipelineTests
    Pipeline/test_pipeline.cpp
)
target_link_libraries(NF_PipelineTests PRIVATE NF::Pipeline NF::Core Catch2::Catch2WithMain)
catch_discover_tests(NF_PipelineTests)
