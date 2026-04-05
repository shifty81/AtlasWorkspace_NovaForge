#pragma once
// NF::ImGuiBackend — UIBackend subclass that routes rendering through ImGui.
// Bridges the UIRenderer vertex pipeline to ImGui's draw list system.
// Compiles as a stub without ImGui headers; real implementation would
// append UIVertex data to ImGui draw lists for unified rendering.
#include "NF/UI/UIBackend.h"
#include "NF/UI/ImGuiLayer.h"

namespace NF {

class ImGuiBackend final : public UIBackend {
public:
    bool init(int width, int height) override {
        m_width  = width;
        m_height = height;
        m_initialized = true;
        NF_LOG_INFO("UI", "ImGuiBackend initialized (stub) " +
                    std::to_string(width) + "x" + std::to_string(height));
        // Real implementation would:
        //   1. Store viewport dimensions
        //   2. Set up orthographic projection for UI vertex mapping
        //   3. Prepare ImGui draw list channel for custom geometry
        return true;
    }

    void shutdown() override {
        m_initialized = false;
        m_imguiLayer = nullptr;
        // Real implementation: release any allocated ImGui resources
        NF_LOG_INFO("UI", "ImGuiBackend shutdown");
    }

    void beginFrame(int width, int height) override {
        m_width  = width;
        m_height = height;
        m_lastVertexCount = 0;
        m_lastIndexCount  = 0;
        // Real implementation:
        //   Update viewport dimensions for projection matrix
        //   Begin a new ImGui frame render pass
    }

    void endFrame() override {
        // Real implementation:
        //   Finalize ImGui draw commands
        //   Submit to GPU via ImGui_ImplOpenGL3_RenderDrawData
    }

    void flush(const UIVertex* vertices, size_t vertexCount,
               const uint32_t* indices, size_t indexCount) override {
        if (!m_initialized || vertexCount == 0) return;
        m_lastVertexCount = vertexCount;
        m_lastIndexCount  = indexCount;
        // Real implementation:
        //   ImDrawList* dl = ImGui::GetBackgroundDrawList();
        //   Reserve vertex/index space: dl->PrimReserve(indexCount, vertexCount)
        //   Convert UIVertex → ImDrawVert and append to draw list
        //   Map indices accordingly
        (void)vertices; (void)indices;
    }

    [[nodiscard]] const char* backendName() const override { return "ImGui"; }
    [[nodiscard]] bool isGPUAccelerated() const override { return true; }

    // ImGui-specific
    void setImGuiLayer(ImGuiLayer* layer) { m_imguiLayer = layer; }

    [[nodiscard]] size_t lastVertexCount() const { return m_lastVertexCount; }
    [[nodiscard]] size_t lastIndexCount()  const { return m_lastIndexCount; }

private:
    int m_width  = 0;
    int m_height = 0;
    bool m_initialized = false;
    ImGuiLayer* m_imguiLayer = nullptr;
    size_t m_lastVertexCount = 0;
    size_t m_lastIndexCount  = 0;
};

} // namespace NF
