// NovaForge Server — Dedicated headless server
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Networking/Networking.h"
#include "NF/Game/Game.h"
#include "NF/World/World.h"
#include "NF/AI/AI.h"

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    NF::coreInit();
    NF_LOG_INFO("Main", "=== NovaForge Dedicated Server ===");
    NF_LOG_INFO("Main", std::string("Version: ") + NF::NF_VERSION_STRING);

    NF::NetworkManager network;
    network.init(NF::NetRole::Server);

    NF::WorldGenerator worldGen;
    worldGen.init(42);

    NF::AISystem ai;
    ai.init();

    // Main loop placeholder
    NF_LOG_INFO("Main", "Server ready — listening for connections");

    ai.shutdown();
    worldGen.shutdown();
    network.shutdown();
    NF::coreShutdown();
    return 0;
}
