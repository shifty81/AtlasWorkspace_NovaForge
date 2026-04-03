#pragma once
// NF::Editor — Editor app, docking panels, viewport, toolbar. Does not ship.
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/Game/Game.h"

namespace NF {

// ── Dock layout ──────────────────────────────────────────────────

enum class DockSlot : uint8_t {
    Left,
    Right,
    Top,
    Bottom,
    Center
};

struct DockPanel {
    std::string name;
    DockSlot slot = DockSlot::Center;
    Rect bounds;
    bool visible = true;
};

// ── Editor services (from COPILOT_IMPLEMENTATION_DIRECTIONS) ─────

class ProjectPathService {
public:
    void init(const std::string& executablePath) {
        m_executablePath = executablePath;
        NF_LOG_INFO("Editor", "Executable path: " + m_executablePath);
    }

    [[nodiscard]] const std::string& executablePath() const { return m_executablePath; }

private:
    std::string m_executablePath;
};

class EditorCommandRegistry {
public:
    using CommandHandler = std::function<void()>;

    void registerCommand(const std::string& name, CommandHandler handler) {
        m_commands[name] = std::move(handler);
    }

    bool executeCommand(const std::string& name) {
        auto it = m_commands.find(name);
        if (it == m_commands.end()) {
            NF_LOG_WARN("Editor", "Unknown command: " + name);
            return false;
        }
        it->second();
        return true;
    }

private:
    std::unordered_map<std::string, CommandHandler> m_commands;
};

// ── Editor application ───────────────────────────────────────────

class EditorApp {
public:
    bool init(int width, int height) {
        m_renderer.init(width, height);
        m_ui.init();
        m_commands.registerCommand("file.new", []() {
            NF_LOG_INFO("Editor", "New project");
        });
        m_commands.registerCommand("file.open", []() {
            NF_LOG_INFO("Editor", "Open project");
        });
        m_commands.registerCommand("file.save", []() {
            NF_LOG_INFO("Editor", "Save project");
        });
        NF_LOG_INFO("Editor", "NovaForge Editor initialized");
        return true;
    }

    void shutdown() {
        m_ui.shutdown();
        m_renderer.shutdown();
        NF_LOG_INFO("Editor", "NovaForge Editor shutdown");
    }

    void update() {}
    void render() {}

private:
    Renderer m_renderer;
    UIRenderer m_ui;
    EditorCommandRegistry m_commands;
    std::vector<DockPanel> m_panels;
};

} // namespace NF
