add_library(NF_UI STATIC src/UI.cpp)
target_include_directories(NF_UI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NF_UI PUBLIC NF::Core NF::Renderer PRIVATE NF_CompileOptions)
add_library(NF::UI ALIAS NF_UI)
