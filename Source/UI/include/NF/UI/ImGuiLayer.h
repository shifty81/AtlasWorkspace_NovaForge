#pragma once
// NF::ImGuiLayer — ImGui integration layer for the editor.
// Manages ImGui context lifecycle, docking, panels, and theming.
// Compiles as a stub without ImGui headers; real implementation requires
// linking against Dear ImGui with GLFW+OpenGL backends.
#include "NF/Core/Core.h"
#include "NF/UI/GLFWWindowProvider.h"
#include <string>

namespace NF {

class ImGuiLayer {
public:
    struct ImGuiConfig {
        bool dockingEnabled = true;
        bool viewportsEnabled = false;
        float fontSize = 14.f;
        std::string fontPath;
        bool darkTheme = true;
    };

    bool init(GLFWWindowProvider& window) {
        return init(window, ImGuiConfig{});
    }

    bool init(GLFWWindowProvider& window, const ImGuiConfig& config) {
        if (!window.isInitialized()) return false;
        m_config = config;
        m_initialized = true;
        m_frameCount = 0;
        NF_LOG_INFO("UI", "ImGuiLayer initialized (stub) docking=" +
                    std::string(config.dockingEnabled ? "on" : "off") +
                    " fontSize=" + std::to_string(static_cast<int>(config.fontSize)));
        // Real implementation would:
        //   1. IMGUI_CHECKVERSION()
        //   2. ImGui::CreateContext()
        //   3. ImGuiIO& io = ImGui::GetIO()
        //   4. io.ConfigFlags |= ImGuiConfigFlags_DockingEnable (if dockingEnabled)
        //   5. io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable (if viewportsEnabled)
        //   6. ImGui_ImplGlfw_InitForOpenGL(window.nativeHandle(), true)
        //   7. ImGui_ImplOpenGL3_Init("#version 330")
        //   8. Apply theme (dark/light)
        //   9. Load custom font from fontPath if set
        return true;
    }

    void shutdown() {
        m_initialized = false;
        // Real implementation:
        //   ImGui_ImplOpenGL3_Shutdown()
        //   ImGui_ImplGlfw_Shutdown()
        //   ImGui::DestroyContext()
        NF_LOG_INFO("UI", "ImGuiLayer shutdown");
    }

    void beginFrame() {
        // Real implementation:
        //   ImGui_ImplOpenGL3_NewFrame()
        //   ImGui_ImplGlfw_NewFrame()
        //   ImGui::NewFrame()
    }

    void endFrame() {
        ++m_frameCount;
        // Real implementation:
        //   ImGui::Render()
        //   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData())
        //   If viewports enabled:
        //     ImGui::UpdatePlatformWindows()
        //     ImGui::RenderPlatformWindowsDefault()
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    // Docking
    void beginDockSpace(const std::string& name = "MainDockSpace") {
        m_activeDockSpace = name;
        // Real implementation:
        //   ImGuiViewport* vp = ImGui::GetMainViewport();
        //   ImGui::SetNextWindowPos(vp->WorkPos);
        //   ImGui::SetNextWindowSize(vp->WorkSize);
        //   ImGui::Begin(name.c_str(), nullptr, dockspace_flags);
        //   ImGui::DockSpace(ImGui::GetID(name.c_str()));
    }

    void endDockSpace() {
        m_activeDockSpace.clear();
        // Real implementation: ImGui::End();
    }

    // Panel helpers
    bool beginPanel(const std::string& name, bool* open = nullptr) {
        m_activePanel = name;
        if (open && !*open) return false;
        // Real implementation: return ImGui::Begin(name.c_str(), open);
        return true;
    }

    void endPanel() {
        m_activePanel.clear();
        // Real implementation: ImGui::End();
    }

    // Viewport
    void renderViewportClear(float r, float g, float b, float a = 1.0f) {
        (void)r; (void)g; (void)b; (void)a;
        // Real implementation:
        //   glClearColor(r, g, b, a);
        //   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // Stats
    [[nodiscard]] size_t frameCount() const { return m_frameCount; }

    // Accessors for testing
    [[nodiscard]] const std::string& activeDockSpace() const { return m_activeDockSpace; }
    [[nodiscard]] const std::string& activePanel() const { return m_activePanel; }

private:
    ImGuiConfig m_config;
    bool m_initialized = false;
    size_t m_frameCount = 0;
    std::string m_activeDockSpace;
    std::string m_activePanel;
};

} // namespace NF
