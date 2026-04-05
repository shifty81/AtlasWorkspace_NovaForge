// NovaForge Editor — Development tool, does not ship with game
#include "NF/Core/Core.h"
#include "NF/Editor/Editor.h"
#include "NF/Input/Input.h"

#ifdef _WIN32
#include "NF/Input/Win32InputAdapter.h"
#endif

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

    NF::InputSystem input;
    input.init();

#ifdef _WIN32
    // Win32InputAdapter translates WM_KEYDOWN / WM_MOUSEMOVE / etc.
    // into NF::InputSystem calls.  The Win32 message loop below calls
    // adapter.processMessage() for each window message before dispatching.
    NF::Win32InputAdapter inputAdapter(input);
    NF_LOG_INFO("Main", "Win32 input adapter initialized");
#endif

    NF_LOG_INFO("Main", "Editor ready — entering main loop");

    // Main loop — runs until the window is closed.
    // On Windows this drives the Win32 message pump; on other platforms
    // it is a headless simulation loop used for CI/testing.
    bool running = true;
    while (running) {
#ifdef _WIN32
        MSG msg{};
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
            // Route to input adapter first, then dispatch normally
            inputAdapter.processMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (!running) break;
#else
        // Non-Windows CI build: no real window, exit immediately after init
        running = false;
#endif
        input.update();
        editor.update(0.016f, input);
    }

    input.shutdown();
    editor.shutdown();
    NF::coreShutdown();
    return 0;
}
