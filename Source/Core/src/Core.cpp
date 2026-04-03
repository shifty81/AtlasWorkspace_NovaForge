#include "NF/Core/Core.h"

namespace NF {

// Core module initialization
static bool s_coreInitialized = false;

void coreInit() {
    if (s_coreInitialized) return;
    NF_LOG_INFO("Core", "NovaForge Core initialized");
    NF_LOG_INFO("Core", std::string("Version: ") + NF_VERSION_STRING);
    s_coreInitialized = true;
}

void coreShutdown() {
    if (!s_coreInitialized) return;
    NF_LOG_INFO("Core", "NovaForge Core shutdown");
    s_coreInitialized = false;
}

} // namespace NF
