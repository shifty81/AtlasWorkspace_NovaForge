cmake_minimum_required(VERSION 3.22)
project(NovaForge VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ── Build options ──────────────────────────────────────────────────
option(NF_BUILD_EDITOR  "Build NovaForge Editor"                 ON)
option(NF_BUILD_GAME    "Build NovaForge standalone game client"  ON)
option(NF_BUILD_SERVER  "Build NovaForge dedicated server"        ON)
option(NF_BUILD_TESTS   "Build test suite"                        OFF)

# ── Global compile settings ───────────────────────────────────────
# Applied per-target via a common interface library to avoid polluting
# FetchContent dependencies with our warning flags.
add_library(NF_CompileOptions INTERFACE)
target_compile_options(NF_CompileOptions INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# ── Modules (static libraries) ────────────────────────────────────
add_subdirectory(Source/Core)
add_subdirectory(Source/Engine)
add_subdirectory(Source/Renderer)
add_subdirectory(Source/Physics)
add_subdirectory(Source/Audio)
add_subdirectory(Source/Animation)
add_subdirectory(Source/Input)
add_subdirectory(Source/Networking)
add_subdirectory(Source/UI)
add_subdirectory(Source/Game)
add_subdirectory(Source/GraphVM)
add_subdirectory(Source/AI)
add_subdirectory(Source/World)

if(NF_BUILD_EDITOR)
    add_subdirectory(Source/Editor)
endif()

# ── Executables ────────────────────────────────────────────────────
add_subdirectory(Source/Programs)

# ── Tests ──────────────────────────────────────────────────────────
if(NF_BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif()

# ── Summary ────────────────────────────────────────────────────────
message(STATUS "")
message(STATUS "╔══════════════════════════════════════════╗")
message(STATUS "║       NovaForge ${PROJECT_VERSION}                    ║")
message(STATUS "╠══════════════════════════════════════════╣")
message(STATUS "║  Build Editor:  ${NF_BUILD_EDITOR}                       ║")
message(STATUS "║  Build Game:    ${NF_BUILD_GAME}                       ║")
message(STATUS "║  Build Server:  ${NF_BUILD_SERVER}                       ║")
message(STATUS "║  Build Tests:   ${NF_BUILD_TESTS}                      ║")
message(STATUS "╚══════════════════════════════════════════╝")
message(STATUS "")
