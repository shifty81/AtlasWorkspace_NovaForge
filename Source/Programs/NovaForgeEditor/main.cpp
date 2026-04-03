// NovaForge Editor — Development tool, does not ship with game
#include "NF/Core/Core.h"
#include "NF/Editor/Editor.h"

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    NF::coreInit();
    NF_LOG_INFO("Main", "=== NovaForge Editor ===");
    NF_LOG_INFO("Main", std::string("Version: ") + NF::NF_VERSION_STRING);

    NF::EditorApp editor;
    if (!editor.init(1920, 1080)) {
        NF_LOG_ERROR("Main", "Failed to initialize editor");
        return 1;
    }

    // Main loop placeholder
    NF_LOG_INFO("Main", "Editor ready — entering main loop");

    editor.shutdown();
    NF::coreShutdown();
    return 0;
}
