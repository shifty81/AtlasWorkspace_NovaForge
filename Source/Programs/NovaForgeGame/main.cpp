// NovaForge Game — Standalone game client (shippable artifact)
#include "NF/Core/Core.h"
#include "NF/Game/Game.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    NF::coreInit();
    NF_LOG_INFO("Main", "=== NovaForge Game ===");
    NF_LOG_INFO("Main", std::string("Version: ") + NF::NF_VERSION_STRING);

    NF::Renderer renderer;
    if (!renderer.init(1920, 1080)) {
        NF_LOG_ERROR("Main", "Failed to initialize renderer");
        return 1;
    }

    // Main loop placeholder
    NF_LOG_INFO("Main", "Game ready — entering main loop");

    renderer.shutdown();
    NF::coreShutdown();
    return 0;
}
