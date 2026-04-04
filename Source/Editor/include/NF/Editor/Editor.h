#pragma once
// NF::Editor — Editor app, docking panels, viewport, toolbar. Does not ship.
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/Game/Game.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include <filesystem>
#include <set>
#include <deque>

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

// ── Selection Service ────────────────────────────────────────────

class SelectionService {
public:
    void select(EntityID id) {
        m_selection.insert(id);
        m_primary = id;
        ++m_version;
    }

    void deselect(EntityID id) {
        m_selection.erase(id);
        if (m_primary == id) {
            m_primary = m_selection.empty() ? INVALID_ENTITY : *m_selection.begin();
        }
        ++m_version;
    }

    void toggleSelect(EntityID id) {
        if (isSelected(id)) deselect(id);
        else select(id);
    }

    void clearSelection() {
        m_selection.clear();
        m_primary = INVALID_ENTITY;
        ++m_version;
    }

    void selectExclusive(EntityID id) {
        m_selection.clear();
        m_selection.insert(id);
        m_primary = id;
        ++m_version;
    }

    [[nodiscard]] bool isSelected(EntityID id) const {
        return m_selection.count(id) > 0;
    }

    [[nodiscard]] EntityID primarySelection() const { return m_primary; }
    [[nodiscard]] const std::set<EntityID>& selection() const { return m_selection; }
    [[nodiscard]] size_t selectionCount() const { return m_selection.size(); }
    [[nodiscard]] bool hasSelection() const { return !m_selection.empty(); }
    [[nodiscard]] uint32_t version() const { return m_version; }

private:
    std::set<EntityID> m_selection;
    EntityID m_primary = INVALID_ENTITY;
    uint32_t m_version = 0;
};

// ── Content Browser ──────────────────────────────────────────────

enum class ContentEntryType : uint8_t {
    Directory,
    Scene,
    Mesh,
    Material,
    Texture,
    Audio,
    Script,
    Data,
    Unknown
};

struct ContentEntry {
    std::string name;
    std::string path;
    ContentEntryType type = ContentEntryType::Unknown;
    size_t sizeBytes = 0;
    bool isDirectory = false;
};

class ContentBrowser {
public:
    void setRootPath(const std::string& root) {
        m_rootPath = root;
        m_currentPath = root;
    }

    bool navigateTo(const std::string& path) {
        if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
            NF_LOG_WARN("ContentBrowser", "Invalid path: " + path);
            return false;
        }
        m_currentPath = path;
        refresh();
        return true;
    }

    bool navigateUp() {
        if (m_currentPath == m_rootPath) return false;
        auto parent = std::filesystem::path(m_currentPath).parent_path().string();
        if (parent.size() < m_rootPath.size()) return false;
        return navigateTo(parent);
    }

    void refresh() {
        m_entries.clear();
        if (!std::filesystem::exists(m_currentPath)) return;

        for (auto& entry : std::filesystem::directory_iterator(m_currentPath)) {
            ContentEntry ce;
            ce.name = entry.path().filename().string();
            ce.path = entry.path().string();
            ce.isDirectory = entry.is_directory();

            if (ce.isDirectory) {
                ce.type = ContentEntryType::Directory;
            } else {
                auto ext = entry.path().extension().string();
                ce.type = classifyExtension(ext);
                if (entry.is_regular_file()) {
                    ce.sizeBytes = static_cast<size_t>(entry.file_size());
                }
            }

            // Skip hidden files
            if (!ce.name.empty() && ce.name[0] != '.') {
                m_entries.push_back(std::move(ce));
            }
        }

        // Sort: directories first, then alphabetical
        std::sort(m_entries.begin(), m_entries.end(), [](const ContentEntry& a, const ContentEntry& b) {
            if (a.isDirectory != b.isDirectory) return a.isDirectory;
            return a.name < b.name;
        });
    }

    [[nodiscard]] const std::string& rootPath() const { return m_rootPath; }
    [[nodiscard]] const std::string& currentPath() const { return m_currentPath; }
    [[nodiscard]] const std::vector<ContentEntry>& entries() const { return m_entries; }
    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }

    [[nodiscard]] bool isAtRoot() const { return m_currentPath == m_rootPath; }

    static ContentEntryType classifyExtension(const std::string& ext) {
        if (ext == ".json") return ContentEntryType::Data;
        if (ext == ".scene" || ext == ".world") return ContentEntryType::Scene;
        if (ext == ".mesh" || ext == ".obj" || ext == ".fbx") return ContentEntryType::Mesh;
        if (ext == ".mat" || ext == ".material") return ContentEntryType::Material;
        if (ext == ".png" || ext == ".jpg" || ext == ".tga" || ext == ".bmp") return ContentEntryType::Texture;
        if (ext == ".wav" || ext == ".ogg" || ext == ".mp3") return ContentEntryType::Audio;
        if (ext == ".lua" || ext == ".graph") return ContentEntryType::Script;
        return ContentEntryType::Unknown;
    }

private:
    std::string m_rootPath;
    std::string m_currentPath;
    std::vector<ContentEntry> m_entries;
};

// ── Project Path Service ─────────────────────────────────────────

class ProjectPathService {
public:
    static constexpr const char* kProjectFileName = "novaforge.project.json";

    void init(const std::string& executablePath) {
        m_executablePath = executablePath;

        // Derive project root from executable path
        auto exePath = std::filesystem::path(executablePath);
        // Walk up from bin/ or Builds/ to find project root
        constexpr int kMaxProjectRootSearchDepth = 5;
        auto dir = exePath.parent_path();
        for (int i = 0; i < kMaxProjectRootSearchDepth; ++i) {
            if (std::filesystem::exists(dir / "Config" / kProjectFileName)) {
                m_projectRoot = dir.string();
                break;
            }
            dir = dir.parent_path();
        }

        if (m_projectRoot.empty()) {
            m_projectRoot = std::filesystem::current_path().string();
        }

        m_contentPath = (std::filesystem::path(m_projectRoot) / "Content").string();
        m_dataPath = (std::filesystem::path(m_projectRoot) / "Data").string();
        m_configPath = (std::filesystem::path(m_projectRoot) / "Config").string();

        NF_LOG_INFO("Editor", "Project root: " + m_projectRoot);
        NF_LOG_INFO("Editor", "Content path: " + m_contentPath);
    }

    [[nodiscard]] const std::string& executablePath() const { return m_executablePath; }
    [[nodiscard]] const std::string& projectRoot() const { return m_projectRoot; }
    [[nodiscard]] const std::string& contentPath() const { return m_contentPath; }
    [[nodiscard]] const std::string& dataPath() const { return m_dataPath; }
    [[nodiscard]] const std::string& configPath() const { return m_configPath; }

    [[nodiscard]] std::string resolvePath(const std::string& relativePath) const {
        return (std::filesystem::path(m_projectRoot) / relativePath).string();
    }

private:
    std::string m_executablePath;
    std::string m_projectRoot;
    std::string m_contentPath;
    std::string m_dataPath;
    std::string m_configPath;
};

// ── Launch Service ───────────────────────────────────────────────

struct LaunchResult {
    bool success = false;
    std::string executablePath;
    std::string errorMessage;
};

class LaunchService {
public:
    void setGameExecutableName(const std::string& name) { m_gameExeName = name; }
    void setBuildDirectory(const std::string& dir) { m_buildDir = dir; }

    [[nodiscard]] std::string resolveGamePath() const {
        // Search order: explicit build dir, then common locations
        std::vector<std::string> searchPaths;

        if (!m_buildDir.empty()) {
            searchPaths.push_back(m_buildDir + "/bin/" + m_gameExeName);
            searchPaths.push_back(m_buildDir + "/" + m_gameExeName);
        }

        searchPaths.push_back("./Builds/debug/bin/" + m_gameExeName);
        searchPaths.push_back("./Builds/release/bin/" + m_gameExeName);
        searchPaths.push_back("./bin/" + m_gameExeName);
        searchPaths.push_back("./" + m_gameExeName);

        for (auto& path : searchPaths) {
            if (std::filesystem::exists(path)) {
                return std::filesystem::canonical(path).string();
            }
        }

        return "";
    }

    [[nodiscard]] LaunchResult validateLaunch() const {
        LaunchResult result;
        result.executablePath = resolveGamePath();

        if (result.executablePath.empty()) {
            result.success = false;
            result.errorMessage = "Game executable '" + m_gameExeName + "' not found in search paths";
            return result;
        }

        if (!std::filesystem::exists(result.executablePath)) {
            result.success = false;
            result.errorMessage = "Game executable does not exist: " + result.executablePath;
            return result;
        }

        result.success = true;
        return result;
    }

private:
    std::string m_gameExeName = "NovaForgeGame";
    std::string m_buildDir;
};

// ── Editor Command State ─────────────────────────────────────────

struct CommandInfo {
    std::string name;
    std::string displayName;
    std::string hotkey;
    std::function<void()> handler;
    std::function<bool()> enabledCheck;  // returns true if command is available
    bool enabled = true;
};

class EditorCommandRegistry {
public:
    void registerCommand(const std::string& name, std::function<void()> handler,
                         const std::string& displayName = "",
                         const std::string& hotkey = "") {
        CommandInfo info;
        info.name = name;
        info.displayName = displayName.empty() ? name : displayName;
        info.hotkey = hotkey;
        info.handler = std::move(handler);
        info.enabled = true;
        m_commands[name] = std::move(info);
    }

    void setEnabledCheck(const std::string& name, std::function<bool()> check) {
        auto it = m_commands.find(name);
        if (it != m_commands.end()) {
            it->second.enabledCheck = std::move(check);
        }
    }

    bool executeCommand(const std::string& name) {
        auto it = m_commands.find(name);
        if (it == m_commands.end()) {
            NF_LOG_WARN("Editor", "Unknown command: " + name);
            return false;
        }
        if (it->second.enabledCheck && !it->second.enabledCheck()) {
            NF_LOG_WARN("Editor", "Command disabled: " + name);
            return false;
        }
        it->second.handler();
        return true;
    }

    [[nodiscard]] bool isCommandEnabled(const std::string& name) const {
        auto it = m_commands.find(name);
        if (it == m_commands.end()) return false;
        if (it->second.enabledCheck) return it->second.enabledCheck();
        return it->second.enabled;
    }

    [[nodiscard]] const CommandInfo* findCommand(const std::string& name) const {
        auto it = m_commands.find(name);
        return (it != m_commands.end()) ? &it->second : nullptr;
    }

    [[nodiscard]] std::vector<std::string> allCommandNames() const {
        std::vector<std::string> names;
        names.reserve(m_commands.size());
        for (auto& [k, _] : m_commands) names.push_back(k);
        std::sort(names.begin(), names.end());
        return names;
    }

    [[nodiscard]] size_t commandCount() const { return m_commands.size(); }

private:
    std::unordered_map<std::string, CommandInfo> m_commands;
};

// ── Recent Files ─────────────────────────────────────────────────

class RecentFilesList {
public:
    void addFile(const std::string& path) {
        // Remove if already present
        m_files.erase(std::remove(m_files.begin(), m_files.end(), path), m_files.end());
        m_files.insert(m_files.begin(), path);
        if (m_files.size() > m_maxEntries) {
            m_files.resize(m_maxEntries);
        }
    }

    void clear() { m_files.clear(); }

    [[nodiscard]] const std::vector<std::string>& files() const { return m_files; }
    [[nodiscard]] size_t count() const { return m_files.size(); }
    [[nodiscard]] bool empty() const { return m_files.empty(); }

    void setMaxEntries(size_t max) { m_maxEntries = max; }

private:
    std::vector<std::string> m_files;
    size_t m_maxEntries = 10;
};

// ── Editor Theme ─────────────────────────────────────────────────

struct EditorTheme {
    // Panel colors
    uint32_t panelBackground     = 0x2B2B2BFF;
    uint32_t panelHeader         = 0x3C3C3CFF;
    uint32_t panelBorder         = 0x555555FF;
    uint32_t panelText           = 0xDCDCDCFF;

    // Selection colors
    uint32_t selectionHighlight  = 0x264F78FF;
    uint32_t selectionBorder     = 0x007ACCFF;
    uint32_t hoverHighlight      = 0x383838FF;

    // Input field colors
    uint32_t inputBackground     = 0x1E1E1EFF;
    uint32_t inputBorder         = 0x474747FF;
    uint32_t inputText           = 0xD4D4D4FF;
    uint32_t inputFocusBorder    = 0x007ACCFF;

    // Button colors
    uint32_t buttonBackground    = 0x3C3C3CFF;
    uint32_t buttonHover         = 0x505050FF;
    uint32_t buttonPressed       = 0x007ACCFF;
    uint32_t buttonText          = 0xCCCCCCFF;
    uint32_t buttonDisabledText  = 0x666666FF;

    // Toolbar
    uint32_t toolbarBackground   = 0x333333FF;
    uint32_t toolbarSeparator    = 0x4A4A4AFF;

    // Status bar
    uint32_t statusBarBackground = 0x007ACCFF;
    uint32_t statusBarText       = 0xFFFFFFFF;

    // Viewport
    uint32_t viewportBackground  = 0x1A1A1AFF;
    uint32_t gridColor           = 0x333333FF;

    // Inspector
    uint32_t propertyLabel       = 0xBBBBBBFF;
    uint32_t propertyValue       = 0xE0E0E0FF;
    uint32_t propertySeparator   = 0x404040FF;
    uint32_t dirtyIndicator      = 0xE8A435FF;

    // Font sizing
    float fontSize               = 14.f;
    float headerFontSize         = 16.f;
    float smallFontSize          = 12.f;

    // Spacing
    float panelPadding           = 8.f;
    float itemSpacing            = 4.f;
    float sectionSpacing         = 12.f;

    static EditorTheme dark() { return {}; }

    static EditorTheme light() {
        EditorTheme t;
        t.panelBackground     = 0xF0F0F0FF;
        t.panelHeader         = 0xE0E0E0FF;
        t.panelBorder         = 0xCCCCCCFF;
        t.panelText           = 0x1E1E1EFF;
        t.selectionHighlight  = 0xCCE8FFFF;
        t.selectionBorder     = 0x0078D4FF;
        t.hoverHighlight      = 0xE5E5E5FF;
        t.inputBackground     = 0xFFFFFFFF;
        t.inputBorder         = 0xCCCCCCFF;
        t.inputText           = 0x1E1E1EFF;
        t.buttonBackground    = 0xE0E0E0FF;
        t.buttonHover         = 0xD0D0D0FF;
        t.buttonText          = 0x1E1E1EFF;
        t.toolbarBackground   = 0xE8E8E8FF;
        t.statusBarBackground = 0x0078D4FF;
        t.viewportBackground  = 0xD4D4D4FF;
        t.gridColor           = 0xBBBBBBFF;
        t.propertyLabel       = 0x444444FF;
        t.propertyValue       = 0x1E1E1EFF;
        return t;
    }
};

// ── DockLayout ───────────────────────────────────────────────────

class DockLayout {
public:
    void addPanel(const std::string& name, DockSlot slot) {
        DockPanel panel;
        panel.name = name;
        panel.slot = slot;
        panel.visible = true;
        m_panels.push_back(std::move(panel));
        NF_LOG_INFO("Editor", "DockLayout: added panel '" + name + "'");
    }

    void removePanel(const std::string& name) {
        auto it = std::find_if(m_panels.begin(), m_panels.end(),
            [&](const DockPanel& p) { return p.name == name; });
        if (it != m_panels.end()) {
            m_panels.erase(it);
            NF_LOG_INFO("Editor", "DockLayout: removed panel '" + name + "'");
        }
    }

    [[nodiscard]] DockPanel* findPanel(const std::string& name) {
        auto it = std::find_if(m_panels.begin(), m_panels.end(),
            [&](const DockPanel& p) { return p.name == name; });
        return (it != m_panels.end()) ? &(*it) : nullptr;
    }

    void setPanelVisible(const std::string& name, bool visible) {
        if (auto* p = findPanel(name)) {
            p->visible = visible;
        }
    }

    void computeLayout(float width, float height, float toolbarHeight, float statusBarHeight) {
        float usableHeight = height - toolbarHeight - statusBarHeight;

        float leftWidth = 0.f;
        float rightWidth = 0.f;
        float topHeight = 0.f;
        float bottomHeight = 0.f;

        for (auto& p : m_panels) {
            if (!p.visible) continue;
            switch (p.slot) {
                case DockSlot::Left:   leftWidth   = kDefaultLeftWidth;   break;
                case DockSlot::Right:  rightWidth  = kDefaultRightWidth;  break;
                case DockSlot::Top:    topHeight   = kDefaultTopHeight;   break;
                case DockSlot::Bottom: bottomHeight = kDefaultBottomHeight; break;
                default: break;
            }
        }

        float centerX = leftWidth;
        float centerW = width - leftWidth - rightWidth;

        for (auto& p : m_panels) {
            if (!p.visible) continue;
            switch (p.slot) {
                case DockSlot::Left:
                    p.bounds = { 0.f, toolbarHeight, leftWidth, usableHeight };
                    break;
                case DockSlot::Right:
                    p.bounds = { width - rightWidth, toolbarHeight, rightWidth, usableHeight };
                    break;
                case DockSlot::Bottom:
                    p.bounds = { centerX, height - statusBarHeight - bottomHeight, centerW, bottomHeight };
                    break;
                case DockSlot::Top:
                    p.bounds = { centerX, toolbarHeight, centerW, topHeight };
                    break;
                case DockSlot::Center:
                    p.bounds = { centerX, toolbarHeight + topHeight, centerW,
                                 usableHeight - topHeight - bottomHeight };
                    break;
            }
        }
    }

    [[nodiscard]] size_t panelCount() const { return m_panels.size(); }
    [[nodiscard]] const std::vector<DockPanel>& panels() const { return m_panels; }

    static constexpr float kDefaultLeftWidth   = 250.f;
    static constexpr float kDefaultRightWidth  = 300.f;
    static constexpr float kDefaultTopHeight   = 200.f;
    static constexpr float kDefaultBottomHeight = 200.f;

private:
    std::vector<DockPanel> m_panels;
};

// ── EditorPanel (abstract) ───────────────────────────────────────

class EditorPanel {
public:
    virtual ~EditorPanel() = default;

    [[nodiscard]] virtual const std::string& name() const = 0;
    [[nodiscard]] virtual DockSlot slot() const = 0;
    virtual void update(float dt) = 0;
    virtual void render(const UIRenderer& ui, const Rect& bounds, const EditorTheme& theme) = 0;

    [[nodiscard]] bool isVisible() const { return m_visible; }
    void setVisible(bool v) { m_visible = v; }

private:
    bool m_visible = true;
};

// ── ViewportPanel ────────────────────────────────────────────────

enum class RenderMode : uint8_t { Shaded, Wireframe, Unlit };
enum class ToolMode   : uint8_t { Select, Move, Rotate, Scale, Paint, Erase };

// Controls a fly-camera inside the viewport when the right mouse button is held.
// Uses the same yaw/pitch math as FPSCamera but is self-contained here so the
// Editor module does not depend on Game logic directly.
struct ViewportCameraController {
    float moveSpeed      = 10.f;   // world units per second
    float mouseSensitivity = 0.15f; // degrees per pixel
    float sprintMultiplier = 3.f;  // held Shift boost

    // Call each frame; mutates cameraPos, yaw, pitch.
    // Only moves the camera when Mouse2 (right button) is held.
    void update(float dt, const InputSystem& input,
                Vec3& cameraPos, float& yaw, float& pitch) {
        bool rmb = input.isKeyDown(KeyCode::Mouse2);
        if (!rmb) {
            m_active = false;
            return;
        }
        m_active = true;

        // -- Mouse look --
        float dx = input.state().mouse.deltaX * mouseSensitivity;
        float dy = input.state().mouse.deltaY * mouseSensitivity;
        yaw   += dx;
        pitch -= dy;  // invert Y so dragging up looks up
        if (pitch >  89.f) pitch =  89.f;
        if (pitch < -89.f) pitch = -89.f;

        // -- Rebuild forward/right vectors from yaw & pitch --
        float yawRad   = yaw   * (3.14159265f / 180.f);
        float pitchRad = pitch * (3.14159265f / 180.f);
        Vec3 fwd{
            std::cos(yawRad) * std::cos(pitchRad),
            std::sin(pitchRad),
            std::sin(yawRad) * std::cos(pitchRad)
        };
        fwd = fwd.normalized();
        Vec3 worldUp{0.f, 1.f, 0.f};
        Vec3 right = fwd.cross(worldUp).normalized();

        // -- WASD movement --
        float speed = moveSpeed;
        if (input.isKeyDown(KeyCode::LShift) || input.isKeyDown(KeyCode::RShift))
            speed *= sprintMultiplier;

        Vec3 move{0.f, 0.f, 0.f};
        if (input.isKeyDown(KeyCode::W)) move = move + fwd  *  speed * dt;
        if (input.isKeyDown(KeyCode::S)) move = move + fwd  * -speed * dt;
        if (input.isKeyDown(KeyCode::D)) move = move + right *  speed * dt;
        if (input.isKeyDown(KeyCode::A)) move = move + right * -speed * dt;
        // Q/E for up/down in world space
        if (input.isKeyDown(KeyCode::E)) move = move + worldUp *  speed * dt;
        if (input.isKeyDown(KeyCode::Q)) move = move + worldUp * -speed * dt;

        cameraPos = cameraPos + move;
    }

    [[nodiscard]] bool isActive() const { return m_active; }

private:
    bool m_active = false;
};

class ViewportPanel : public EditorPanel {
public:
    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Center; }
    void update(float /*dt*/) override {}
    void render(const UIRenderer& /*ui*/, const Rect& /*bounds*/, const EditorTheme& /*theme*/) override {}

    // Called by EditorApp::update(dt, input) each frame.
    // Activates fly-camera only while right mouse button is held.
    void updateCamera(float dt, const InputSystem& input) {
        m_camController.update(dt, input, m_cameraPos, m_cameraYaw, m_cameraPitch);
    }

    [[nodiscard]] Vec3 cameraPosition() const { return m_cameraPos; }
    void setCameraPosition(Vec3 pos) { m_cameraPos = pos; }

    [[nodiscard]] float cameraYaw()   const { return m_cameraYaw; }
    [[nodiscard]] float cameraPitch() const { return m_cameraPitch; }
    void setCameraYaw(float y)   { m_cameraYaw   = y; }
    void setCameraPitch(float p) { m_cameraPitch = p; }

    [[nodiscard]] bool isFlyCamActive() const { return m_camController.isActive(); }

    [[nodiscard]] ViewportCameraController& cameraController() { return m_camController; }
    [[nodiscard]] const ViewportCameraController& cameraController() const { return m_camController; }

    [[nodiscard]] float cameraZoom() const { return m_cameraZoom; }
    void setCameraZoom(float z) { m_cameraZoom = z; }

    [[nodiscard]] bool gridEnabled() const { return m_gridEnabled; }
    void setGridEnabled(bool e) { m_gridEnabled = e; }

    [[nodiscard]] RenderMode renderMode() const { return m_renderMode; }
    void setRenderMode(RenderMode m) { m_renderMode = m; }

    [[nodiscard]] ToolMode toolMode() const { return m_toolMode; }
    void setToolMode(ToolMode m) { m_toolMode = m; }

private:
    std::string m_name = "Viewport";
    Vec3  m_cameraPos{0.f, 0.f, 0.f};
    float m_cameraYaw   = -90.f;  // degrees; -90 = look down -Z
    float m_cameraPitch =   0.f;  // degrees
    float m_cameraZoom  =   1.f;
    bool  m_gridEnabled = true;
    RenderMode m_renderMode = RenderMode::Shaded;
    ToolMode   m_toolMode   = ToolMode::Select;
    ViewportCameraController m_camController;
};

// ── InspectorPanel ───────────────────────────────────────────────

class InspectorPanel : public EditorPanel {
public:
    InspectorPanel() = default;
    InspectorPanel(SelectionService* sel, TypeRegistry* reg)
        : m_selection(sel), m_typeRegistry(reg) {}

    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Right; }
    void update(float /*dt*/) override {}
    void render(const UIRenderer& /*ui*/, const Rect& /*bounds*/, const EditorTheme& /*theme*/) override {}

    [[nodiscard]] SelectionService* selectionService() const { return m_selection; }
    void setSelectionService(SelectionService* s) { m_selection = s; }

    [[nodiscard]] TypeRegistry* typeRegistry() const { return m_typeRegistry; }
    void setTypeRegistry(TypeRegistry* r) { m_typeRegistry = r; }

private:
    std::string m_name = "Inspector";
    SelectionService* m_selection = nullptr;
    TypeRegistry* m_typeRegistry = nullptr;
};

// ── HierarchyPanel ──────────────────────────────────────────────

class HierarchyPanel : public EditorPanel {
public:
    HierarchyPanel() = default;
    explicit HierarchyPanel(SelectionService* sel) : m_selection(sel) {}

    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Left; }
    void update(float /*dt*/) override {}
    void render(const UIRenderer& /*ui*/, const Rect& /*bounds*/, const EditorTheme& /*theme*/) override {}

    [[nodiscard]] SelectionService* selectionService() const { return m_selection; }
    void setSelectionService(SelectionService* s) { m_selection = s; }

    [[nodiscard]] const std::string& searchFilter() const { return m_searchFilter; }
    void setSearchFilter(const std::string& f) { m_searchFilter = f; }

    void setEntityList(const std::vector<EntityID>& ids) { m_entityList = ids; }
    [[nodiscard]] const std::vector<EntityID>& entityList() const { return m_entityList; }

private:
    std::string m_name = "Hierarchy";
    SelectionService* m_selection = nullptr;
    std::string m_searchFilter;
    std::vector<EntityID> m_entityList;
};

// ── ConsolePanel ─────────────────────────────────────────────────

enum class ConsoleMessageLevel : uint8_t { Info, Warning, Error };

struct ConsoleMessage {
    std::string text;
    ConsoleMessageLevel level = ConsoleMessageLevel::Info;
    float timestamp = 0.f;
};

class ConsolePanel : public EditorPanel {
public:
    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Bottom; }
    void update(float /*dt*/) override {}
    void render(const UIRenderer& /*ui*/, const Rect& /*bounds*/, const EditorTheme& /*theme*/) override {}

    void addMessage(const std::string& text, ConsoleMessageLevel level, float timestamp) {
        ConsoleMessage msg;
        msg.text = text;
        msg.level = level;
        msg.timestamp = timestamp;
        m_messages.push_back(std::move(msg));
        if (m_messages.size() > kMaxMessages) {
            m_messages.erase(m_messages.begin());
        }
    }

    void clearMessages() { m_messages.clear(); }
    [[nodiscard]] size_t messageCount() const { return m_messages.size(); }
    [[nodiscard]] const std::vector<ConsoleMessage>& messages() const { return m_messages; }

    static constexpr size_t kMaxMessages = 1000;

private:
    std::string m_name = "Console";
    std::vector<ConsoleMessage> m_messages;
};

// ── ContentBrowserPanel ──────────────────────────────────────────

enum class ContentViewMode : uint8_t { Grid, List };

class ContentBrowserPanel : public EditorPanel {
public:
    ContentBrowserPanel() = default;
    explicit ContentBrowserPanel(ContentBrowser* browser) : m_browser(browser) {}

    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Left; }
    void update(float /*dt*/) override {}
    void render(const UIRenderer& /*ui*/, const Rect& /*bounds*/, const EditorTheme& /*theme*/) override {}

    [[nodiscard]] ContentBrowser* contentBrowser() const { return m_browser; }
    void setContentBrowser(ContentBrowser* b) { m_browser = b; }

    [[nodiscard]] ContentViewMode viewMode() const { return m_viewMode; }
    void setViewMode(ContentViewMode m) { m_viewMode = m; }

private:
    std::string m_name = "ContentBrowser";
    ContentBrowser* m_browser = nullptr;
    ContentViewMode m_viewMode = ContentViewMode::Grid;
};

// ── EditorToolbar ────────────────────────────────────────────────

struct ToolbarItem {
    std::string name;
    std::string icon;
    std::string tooltip;
    std::function<void()> action;
    bool enabled = true;
    bool isSeparator = false;
};

class EditorToolbar {
public:
    void addItem(const std::string& name, const std::string& icon,
                 const std::string& tooltip, std::function<void()> action, bool enabled = true) {
        ToolbarItem item;
        item.name = name;
        item.icon = icon;
        item.tooltip = tooltip;
        item.action = std::move(action);
        item.enabled = enabled;
        item.isSeparator = false;
        m_items.push_back(std::move(item));
    }

    void addSeparator() {
        ToolbarItem sep;
        sep.isSeparator = true;
        m_items.push_back(std::move(sep));
    }

    [[nodiscard]] const std::vector<ToolbarItem>& items() const { return m_items; }
    [[nodiscard]] size_t itemCount() const { return m_items.size(); }

private:
    std::vector<ToolbarItem> m_items;
};

// ── Project Indexer ──────────────────────────────────────────────

enum class SourceFileType : uint8_t {
    Header, Source, Shader, Script, Data, Config, Unknown
};

struct IndexedFile {
    std::string path;
    SourceFileType fileType = SourceFileType::Unknown;
    std::string moduleName;
    uint32_t lineCount = 0;
    uint64_t lastModified = 0;
    std::vector<std::string> symbols;
};

class ProjectIndexer {
public:
    void indexDirectory(const std::string& rootPath) {
        namespace fs = std::filesystem;
        if (!fs::exists(rootPath) || !fs::is_directory(rootPath)) {
            NF_LOG_WARN("IDE", "Directory not found: " + rootPath);
            return;
        }
        for (auto& entry : fs::recursive_directory_iterator(rootPath)) {
            if (!entry.is_regular_file()) continue;
            auto ext = entry.path().extension().string();
            SourceFileType ft = classifyExtension(ext);
            auto rel = fs::relative(entry.path(), rootPath).string();
            std::string mod;
            auto sep = rel.find_first_of("/\\");
            if (sep != std::string::npos) mod = rel.substr(0, sep);
            IndexedFile f;
            f.path = entry.path().string();
            f.fileType = ft;
            f.moduleName = mod;
            f.lineCount = 0;
            f.lastModified = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::seconds>(
                    entry.last_write_time().time_since_epoch()).count());
            m_files.push_back(std::move(f));
        }
        NF_LOG_INFO("IDE", "Indexed " + std::to_string(m_files.size()) + " files from " + rootPath);
    }

    void indexFile(const std::string& path, SourceFileType type, const std::string& moduleName) {
        IndexedFile f;
        f.path = path;
        f.fileType = type;
        f.moduleName = moduleName;
        m_files.push_back(std::move(f));
    }

    [[nodiscard]] std::vector<const IndexedFile*> findFilesByType(SourceFileType type) const {
        std::vector<const IndexedFile*> result;
        for (auto& f : m_files) {
            if (f.fileType == type) result.push_back(&f);
        }
        return result;
    }

    [[nodiscard]] std::vector<const IndexedFile*> findFilesByModule(const std::string& moduleName) const {
        std::vector<const IndexedFile*> result;
        for (auto& f : m_files) {
            if (f.moduleName == moduleName) result.push_back(&f);
        }
        return result;
    }

    [[nodiscard]] std::vector<const IndexedFile*> findFilesByName(const std::string& nameSubstring) const {
        std::vector<const IndexedFile*> result;
        for (auto& f : m_files) {
            if (f.path.find(nameSubstring) != std::string::npos) result.push_back(&f);
        }
        return result;
    }

    [[nodiscard]] const std::vector<IndexedFile>& allFiles() const { return m_files; }
    [[nodiscard]] size_t fileCount() const { return m_files.size(); }

    void clear() { m_files.clear(); }

    void addSymbol(const std::string& filePath, const std::string& symbolName) {
        for (auto& f : m_files) {
            if (f.path == filePath) {
                f.symbols.push_back(symbolName);
                return;
            }
        }
    }

    [[nodiscard]] std::vector<const IndexedFile*> findSymbol(const std::string& symbolName) const {
        std::vector<const IndexedFile*> result;
        for (auto& f : m_files) {
            for (auto& s : f.symbols) {
                if (s == symbolName) {
                    result.push_back(&f);
                    break;
                }
            }
        }
        return result;
    }

private:
    static SourceFileType classifyExtension(const std::string& ext) {
        if (ext == ".h" || ext == ".hpp") return SourceFileType::Header;
        if (ext == ".cpp" || ext == ".cc" || ext == ".cxx") return SourceFileType::Source;
        if (ext == ".glsl" || ext == ".hlsl") return SourceFileType::Shader;
        if (ext == ".lua" || ext == ".py") return SourceFileType::Script;
        if (ext == ".json") return SourceFileType::Data;
        if (ext == ".cfg" || ext == ".ini") return SourceFileType::Config;
        return SourceFileType::Unknown;
    }

    std::vector<IndexedFile> m_files;
};

// ── Code Navigation ─────────────────────────────────────────────

enum class SymbolKind : uint8_t {
    Function, Class, Struct, Enum, Variable, Namespace, Macro, Type, Unknown
};

struct NavigationTarget {
    std::string filePath;
    uint32_t line = 0;
    uint32_t column = 0;
    std::string symbolName;
    SymbolKind kind = SymbolKind::Unknown;
};

struct NavigationEntry {
    std::string symbol;
    SymbolKind kind = SymbolKind::Unknown;
    std::string filePath;
    uint32_t line = 0;
};

class CodeNavigator {
public:
    void addEntry(NavigationEntry entry) {
        m_entries.push_back(std::move(entry));
    }

    [[nodiscard]] std::optional<NavigationTarget> goToDefinition(const std::string& symbolName) const {
        for (auto& e : m_entries) {
            if (e.symbol == symbolName) {
                NavigationTarget t;
                t.filePath = e.filePath;
                t.line = e.line;
                t.column = 0;
                t.symbolName = e.symbol;
                t.kind = e.kind;
                return t;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::vector<NavigationTarget> findReferences(const std::string& symbolName) const {
        std::vector<NavigationTarget> result;
        for (auto& e : m_entries) {
            if (e.symbol == symbolName) {
                NavigationTarget t;
                t.filePath = e.filePath;
                t.line = e.line;
                t.column = 0;
                t.symbolName = e.symbol;
                t.kind = e.kind;
                result.push_back(std::move(t));
            }
        }
        return result;
    }

    [[nodiscard]] std::vector<NavigationTarget> findSymbolsByKind(SymbolKind kind) const {
        std::vector<NavigationTarget> result;
        for (auto& e : m_entries) {
            if (e.kind == kind) {
                NavigationTarget t;
                t.filePath = e.filePath;
                t.line = e.line;
                t.column = 0;
                t.symbolName = e.symbol;
                t.kind = e.kind;
                result.push_back(std::move(t));
            }
        }
        return result;
    }

    [[nodiscard]] std::vector<NavigationTarget> searchSymbols(const std::string& query) const {
        std::vector<NavigationTarget> result;
        for (auto& e : m_entries) {
            if (e.symbol.find(query) != std::string::npos) {
                NavigationTarget t;
                t.filePath = e.filePath;
                t.line = e.line;
                t.column = 0;
                t.symbolName = e.symbol;
                t.kind = e.kind;
                result.push_back(std::move(t));
            }
        }
        return result;
    }

    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }

    void clear() { m_entries.clear(); }

private:
    std::vector<NavigationEntry> m_entries;
};

// ── Breadcrumb Trail ─────────────────────────────────────────────

struct BreadcrumbItem {
    std::string label;
    std::string filePath;
    uint32_t line = 0;
};

class BreadcrumbTrail {
public:
    void push(BreadcrumbItem item) {
        if (m_trail.size() >= maxDepth) {
            m_trail.erase(m_trail.begin());
        }
        m_trail.push_back(std::move(item));
    }

    std::optional<BreadcrumbItem> pop() {
        if (m_trail.empty()) return std::nullopt;
        auto item = std::move(m_trail.back());
        m_trail.pop_back();
        return item;
    }

    [[nodiscard]] const BreadcrumbItem* current() const {
        if (m_trail.empty()) return nullptr;
        return &m_trail.back();
    }

    [[nodiscard]] const std::vector<BreadcrumbItem>& trail() const { return m_trail; }
    [[nodiscard]] size_t depth() const { return m_trail.size(); }
    void clear() { m_trail.clear(); }

private:
    std::vector<BreadcrumbItem> m_trail;
    static constexpr size_t maxDepth = 50;
};

// ── IDE Panel ────────────────────────────────────────────────────

class IDEPanel : public EditorPanel {
public:
    IDEPanel(ProjectIndexer* indexer, CodeNavigator* navigator)
        : m_indexer(indexer), m_navigator(navigator) {}

    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Center; }
    void update(float /*dt*/) override {}
    void render(const UIRenderer& /*ui*/, const Rect& /*bounds*/, const EditorTheme& /*theme*/) override {}

    [[nodiscard]] ProjectIndexer* indexer() const { return m_indexer; }
    [[nodiscard]] CodeNavigator* navigator() const { return m_navigator; }

    std::string searchQuery;
    std::vector<NavigationTarget> searchResults;

private:
    std::string m_name = "IDE";
    ProjectIndexer* m_indexer = nullptr;
    CodeNavigator* m_navigator = nullptr;
};

// ── IDE Service ──────────────────────────────────────────────────

class IDEService {
public:
    void init() {
        m_indexer = ProjectIndexer{};
        m_navigator = CodeNavigator{};
        m_breadcrumbs = BreadcrumbTrail{};
        m_initialized = true;
        NF_LOG_INFO("IDE", "IDEService initialized");
    }

    void shutdown() {
        m_indexer.clear();
        m_navigator.clear();
        m_breadcrumbs.clear();
        m_initialized = false;
        NF_LOG_INFO("IDE", "IDEService shutdown");
    }

    [[nodiscard]] ProjectIndexer& indexer() { return m_indexer; }
    [[nodiscard]] CodeNavigator& navigator() { return m_navigator; }
    [[nodiscard]] BreadcrumbTrail& breadcrumbs() { return m_breadcrumbs; }

    void navigateTo(const std::string& filePath, uint32_t line, const std::string& symbolName) {
        BreadcrumbItem item;
        item.label = symbolName;
        item.filePath = filePath;
        item.line = line;
        m_breadcrumbs.push(std::move(item));
        NF_LOG_INFO("IDE", "Navigate to " + symbolName + " at " + filePath + ":" + std::to_string(line));
    }

    bool goBack() {
        auto item = m_breadcrumbs.pop();
        if (!item) return false;
        NF_LOG_INFO("IDE", "Go back to " + item->label);
        return true;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

private:
    ProjectIndexer m_indexer;
    CodeNavigator m_navigator;
    BreadcrumbTrail m_breadcrumbs;
    bool m_initialized = false;
};

// ── MenuBar ──────────────────────────────────────────────────────

struct MenuItem {
    std::string name;
    std::string command;    // command name to execute via EditorCommandRegistry
    std::string hotkey;
    bool enabled = true;
    bool isSeparator = false;
    std::vector<MenuItem> children;

    static MenuItem separator() {
        MenuItem m;
        m.isSeparator = true;
        return m;
    }
};

struct MenuCategory {
    std::string name;
    std::vector<MenuItem> items;

    void addItem(const std::string& itemName, const std::string& command,
                 const std::string& hotkey = "", bool enabled = true) {
        MenuItem m;
        m.name = itemName;
        m.command = command;
        m.hotkey = hotkey;
        m.enabled = enabled;
        items.push_back(std::move(m));
    }

    void addSeparator() { items.push_back(MenuItem::separator()); }
};

class MenuBar {
public:
    MenuCategory& addCategory(const std::string& name) {
        m_categories.push_back(MenuCategory{name, {}});
        return m_categories.back();
    }

    MenuCategory* findCategory(const std::string& name) {
        for (auto& c : m_categories) if (c.name == name) return &c;
        return nullptr;
    }

    const MenuCategory* findCategory(const std::string& name) const {
        for (auto& c : m_categories) if (c.name == name) return &c;
        return nullptr;
    }

    const std::vector<MenuCategory>& categories() const { return m_categories; }
    size_t categoryCount() const { return m_categories.size(); }

private:
    std::vector<MenuCategory> m_categories;
};

// ── EditorStatusBar ──────────────────────────────────────────────

struct StatusBarState {
    std::string modeName;
    std::string worldPath;
    bool isDirty = false;
    int selectionCount = 0;
    float fps = 0.f;
    std::string statusMessage;
};

class EditorStatusBar {
public:
    void update(const std::string& mode, const std::string& worldPath,
                bool dirty, int selection, float fps,
                const std::string& msg = "") {
        m_state.modeName = mode;
        m_state.worldPath = worldPath;
        m_state.isDirty = dirty;
        m_state.selectionCount = selection;
        m_state.fps = fps;
        m_state.statusMessage = msg;
    }

    const StatusBarState& state() const { return m_state; }

    std::string buildText() const {
        std::string s = m_state.modeName;
        if (!m_state.worldPath.empty()) {
            s += "  |  " + m_state.worldPath;
            if (m_state.isDirty) s += " *";
        }
        if (m_state.selectionCount > 0)
            s += "  |  " + std::to_string(m_state.selectionCount) + " selected";
        s += "  |  " + std::to_string(static_cast<int>(m_state.fps)) + " FPS";
        if (!m_state.statusMessage.empty())
            s += "  |  " + m_state.statusMessage;
        return s;
    }

private:
    StatusBarState m_state;
};

// ── Notification System ──────────────────────────────────────────

enum class NotificationType : uint8_t { Info, Success, Warning, Error };

struct EditorNotification {
    NotificationType type = NotificationType::Info;
    std::string message;
    float ttl = 3.f;      // seconds before it expires
    float elapsed = 0.f;

    bool isExpired() const { return elapsed >= ttl; }
    float progress() const { return ttl > 0.f ? std::min(elapsed / ttl, 1.f) : 1.f; }
};

class NotificationQueue {
public:
    void push(NotificationType type, const std::string& message, float ttl = 3.f) {
        EditorNotification n;
        n.type = type;
        n.message = message;
        n.ttl = ttl;
        n.elapsed = 0.f;
        m_queue.push_back(std::move(n));
    }

    void tick(float dt) {
        for (auto& n : m_queue) n.elapsed += dt;
        m_queue.erase(
            std::remove_if(m_queue.begin(), m_queue.end(),
                           [](const EditorNotification& n){ return n.isExpired(); }),
            m_queue.end());
    }

    const EditorNotification* current() const {
        return m_queue.empty() ? nullptr : &m_queue.front();
    }

    bool hasActive() const { return !m_queue.empty(); }
    int count() const { return (int)m_queue.size(); }
    void clear() { m_queue.clear(); }

private:
    std::vector<EditorNotification> m_queue;
};

// ── Orbital Editor Camera ─────────────────────────────────────────

struct EditorCameraOrbit {
    Vec3  target    = {0.f, 0.f, 0.f};
    float distance  = 10.f;
    float yaw       = -90.f;   // degrees
    float pitch     = 30.f;    // degrees
    float fovDeg    = 60.f;
    float nearPlane = 0.1f;
    float farPlane  = 1000.f;

    Vec3 computePosition() const {
        float y = yaw   * (3.14159265f / 180.f);
        float p = pitch * (3.14159265f / 180.f);
        return Vec3{
            target.x + distance * std::cos(y) * std::cos(p),
            target.y + distance * std::sin(p),
            target.z + distance * std::sin(y) * std::cos(p)
        };
    }

    void orbit(float dyaw, float dpitch) {
        yaw   += dyaw;
        pitch += dpitch;
        if (pitch >  89.f) pitch =  89.f;
        if (pitch < -89.f) pitch = -89.f;
    }

    void zoom(float delta) {
        distance -= delta;
        if (distance < 0.5f) distance = 0.5f;
    }

    void pan(float dx, float dy) {
        float y = yaw * (3.14159265f / 180.f);
        Vec3 right{std::cos(y), 0.f, std::sin(y)};
        Vec3 up{0.f, 1.f, 0.f};
        target = target + right * (-dx * distance * 0.001f);
        target = target + up    * ( dy * distance * 0.001f);
    }

    // aspectRatio is not stored on Camera; it is passed per-frame to projectionMatrix().
    Camera buildCamera(float aspectRatio) const {
        Camera cam;
        cam.position = computePosition();
        cam.target   = target;
        cam.up       = Vec3{0.f, 1.f, 0.f};
        cam.fov      = fovDeg;
        cam.nearPlane = nearPlane;
        cam.farPlane  = farPlane;
        (void)aspectRatio;  // stored externally by caller for use with projectionMatrix()
        return cam;
    }
};

// ── Gizmo ─────────────────────────────────────────────────────────

enum class GizmoMode : uint8_t { Translate, Rotate, Scale };
enum class GizmoAxis : uint8_t { None, X, Y, Z, XY, YZ, XZ, All };

struct GizmoState {
    GizmoMode mode       = GizmoMode::Translate;
    GizmoAxis activeAxis = GizmoAxis::None;
    bool isDragging      = false;
    bool snapEnabled     = false;
    float snapValue      = 0.25f;   // world units for translate, degrees for rotate

    void activate(GizmoAxis axis) { activeAxis = axis; isDragging = true; }
    void deactivate() { activeAxis = GizmoAxis::None; isDragging = false; }
    void setMode(GizmoMode m) { mode = m; deactivate(); }
};

// ── Editor Settings ───────────────────────────────────────────────

struct SnapSettings {
    float gridSize  = 0.25f;   // world units
    float angleStep = 15.f;    // degrees
    float scaleStep = 0.1f;
    bool  enabled   = false;
};

struct EditorSettings {
    bool        darkMode             = true;
    SnapSettings snap;
    bool        showGrid             = true;
    bool        showGizmos           = true;
    float       cameraSpeed          = 10.f;
    bool        autosave             = true;
    float       autosaveIntervalSecs = 300.f;
    int         undoHistorySize      = 100;
};

class EditorSettingsService {
public:
    void reset() { m_settings = EditorSettings{}; }

    EditorSettings& settings() { return m_settings; }
    const EditorSettings& settings() const { return m_settings; }

    void applyTheme(EditorTheme& theme) const {
        if (m_settings.darkMode) theme = EditorTheme::dark();
        else                     theme = EditorTheme::light();
    }

    void setDarkMode(bool dark) { m_settings.darkMode = dark; }
    void setShowGrid(bool show) { m_settings.showGrid = show; }
    void setSnapEnabled(bool on) { m_settings.snap.enabled = on; }
    void setCameraSpeed(float s) { m_settings.cameraSpeed = s; }

private:
    EditorSettings m_settings;
};

// ── Hotkey Dispatcher ────────────────────────────────────────────

struct HotkeyBinding {
    std::string hotkey;      // e.g. "Ctrl+Z", "F12", "Ctrl+Shift+S"
    std::string commandName;
};

class HotkeyDispatcher {
public:
    void bind(const std::string& hotkey, const std::string& commandName) {
        m_bindings.push_back({hotkey, commandName});
    }

    void unbind(const std::string& hotkey) {
        m_bindings.erase(
            std::remove_if(m_bindings.begin(), m_bindings.end(),
                           [&](const HotkeyBinding& b){ return b.hotkey == hotkey; }),
            m_bindings.end());
    }

    // Returns command name matched for a given hotkey string (empty if none)
    std::string findCommand(const std::string& hotkey) const {
        for (auto& b : m_bindings)
            if (b.hotkey == hotkey) return b.commandName;
        return {};
    }

    // Dispatch all matching bindings given a pressed hotkey string.
    // Returns number of commands dispatched.
    int dispatch(const std::string& hotkey, EditorCommandRegistry& commands) {
        int count = 0;
        for (auto& b : m_bindings) {
            if (b.hotkey == hotkey) {
                commands.executeCommand(b.commandName);
                ++count;
            }
        }
        return count;
    }

    void loadDefaults(EditorCommandRegistry& commands) {
        // Mirror the hotkeys already registered on commands
        auto names = commands.allCommandNames();
        for (auto& n : names) {
            if (auto* info = commands.findCommand(n)) {
                if (!info->hotkey.empty())
                    bind(info->hotkey, n);
            }
        }
    }

    const std::vector<HotkeyBinding>& bindings() const { return m_bindings; }
    int bindingCount() const { return (int)m_bindings.size(); }

private:
    std::vector<HotkeyBinding> m_bindings;
};

// ── Graph Editor Panel ───────────────────────────────────────────

class GraphEditorPanel : public EditorPanel {
public:
    explicit GraphEditorPanel(GraphVM* vm = nullptr) : m_graphVM(vm) {}

    const std::string& name() const override { return m_name; }
    DockSlot slot() const override { return DockSlot::Center; }
    void update(float /*dt*/) override {}

    void render(const UIRenderer& ui, const Rect& bounds,
                const EditorTheme& theme) override {
        // UIRenderer has mutable internal batching state; cast is safe here.
        UIRenderer& mutableUi = const_cast<UIRenderer&>(ui);

        // Background
        mutableUi.drawRect(bounds, theme.panelBackground);

        if (!m_currentGraph) {
            mutableUi.drawText(bounds.x + 8.f, bounds.y + 8.f, "No graph open", theme.panelText);
            return;
        }

        // Draw nodes as simple labelled rectangles
        for (const auto& node : m_currentGraph->nodes()) {
            Rect nr{node.position.x + bounds.x, node.position.y + bounds.y, 120.f, 60.f};
            uint32_t nodeColor = (node.id == m_selectedNodeId)
                                 ? theme.selectionHighlight : theme.toolbarBackground;
            mutableUi.drawRect(nr, nodeColor);
            mutableUi.drawRectOutline(nr, theme.panelText, 1.f);
            mutableUi.drawText(nr.x + 4.f, nr.y + 4.f, node.name, theme.panelText);
        }

        // Draw link count annotation
        float ly = bounds.y + bounds.h - 20.f;
        mutableUi.drawText(bounds.x + 4.f, ly,
                           std::to_string(m_currentGraph->links().size()) + " link(s)",
                           theme.propertyLabel);
    }

    void setGraphVM(GraphVM* vm) { m_graphVM = vm; }
    GraphVM* graphVM() const { return m_graphVM; }

    // Create a brand-new Graph and make it the current one.
    void newGraph(GraphType type, const std::string& graphName) {
        m_ownedGraph = std::make_unique<Graph>();
        m_currentGraph = m_ownedGraph.get();
        m_currentGraphName = graphName;
        m_currentGraphType = type;
        m_nextNodeId = 1;
        m_selectedNodeId = -1;
        NF_LOG_INFO("Editor", "GraphEditorPanel: new graph '" + graphName + "'");
    }

    // Open an externally-owned graph (e.g. loaded from disk).
    bool openGraph(const std::string& graphName) {
        if (!m_graphVM) return false;
        m_currentGraphName = graphName;
        NF_LOG_INFO("Editor", "GraphEditorPanel: opened graph '" + graphName + "'");
        return true;
    }

    // Add a node to the current graph; returns the assigned node ID or -1 on failure.
    int addNode(const std::string& nodeName, GraphType type = GraphType::World,
                float x = 0.f, float y = 0.f) {
        if (!m_currentGraph) return -1;
        GraphNode n;
        n.id       = m_nextNodeId++;
        n.name     = nodeName;
        n.type     = type;
        n.position = {x, y};
        m_currentGraph->addNode(n);
        NF_LOG_INFO("Editor", "GraphEditorPanel: added node '" + nodeName + "' id=" +
                    std::to_string(n.id));
        return n.id;
    }

    // Remove a node (and its connected links) from the current graph.
    bool removeNode(int nodeId) {
        if (!m_currentGraph) return false;
        if (m_currentGraph->findNode(nodeId) == nullptr) return false;
        m_currentGraph->removeNode(nodeId);
        if (m_selectedNodeId == nodeId) m_selectedNodeId = -1;
        return true;
    }

    // Add a link between two node ports.
    bool addLink(int srcNode, uint32_t srcPort, int dstNode, uint32_t dstPort) {
        if (!m_currentGraph) return false;
        GraphLink lk;
        lk.sourceNode = srcNode;
        lk.sourcePort = srcPort;
        lk.targetNode = dstNode;
        lk.targetPort = dstPort;
        m_currentGraph->addLink(lk);
        return true;
    }

    // Compile the current graph and load the resulting program into the VM.
    bool compileAndLoad() {
        if (!m_currentGraph || !m_graphVM) return false;
        auto prog = GraphCompiler::compile(*m_currentGraph);
        m_graphVM->loadProgram(prog);
        NF_LOG_INFO("Editor", "GraphEditorPanel: compiled graph '" + m_currentGraphName + "'");
        return true;
    }

    [[nodiscard]] Graph* currentGraph() { return m_currentGraph; }
    [[nodiscard]] const Graph* currentGraph() const { return m_currentGraph; }
    const std::string& currentGraphName() const { return m_currentGraphName; }
    bool hasOpenGraph() const { return m_currentGraph != nullptr; }

    int selectedNodeId() const { return m_selectedNodeId; }
    void selectNode(int id) { m_selectedNodeId = id; }
    void clearSelection() { m_selectedNodeId = -1; }

    int nodeCount() const { return m_currentGraph ? (int)m_currentGraph->nodes().size() : 0; }
    int linkCount() const { return m_currentGraph ? (int)m_currentGraph->links().size() : 0; }

private:
    std::string m_name = "GraphEditor";
    GraphVM* m_graphVM = nullptr;
    std::unique_ptr<Graph> m_ownedGraph;
    Graph* m_currentGraph = nullptr;
    std::string m_currentGraphName;
    GraphType m_currentGraphType = GraphType::World;
    int m_selectedNodeId = -1;
    int m_nextNodeId = 1;
};

// ── Frame Stats ──────────────────────────────────────────────────

struct FrameStats {
    float fps          = 0.f;
    float frameTimeMs  = 0.f;
    float updateTimeMs = 0.f;
    float renderTimeMs = 0.f;
    uint64_t frameCount = 0;
};

class FrameStatsTracker {
public:
    void beginFrame(float dtSeconds) {
        m_frameTimeMs = dtSeconds * 1000.f;
        // Exponential moving average for FPS
        float newFps = dtSeconds > 0.f ? 1.f / dtSeconds : 0.f;
        m_stats.fps = m_stats.fps * 0.9f + newFps * 0.1f;
        m_stats.frameTimeMs = m_frameTimeMs;
        ++m_stats.frameCount;
        m_updateStart = m_stats.frameCount;  // reuse as a simple "step" marker
    }

    void recordUpdateTime(float ms) { m_stats.updateTimeMs = ms; }
    void recordRenderTime(float ms) { m_stats.renderTimeMs = ms; }

    const FrameStats& stats() const { return m_stats; }

private:
    FrameStats m_stats;
    float m_frameTimeMs = 0.f;
    uint64_t m_updateStart = 0;
};

// ── Editor application ───────────────────────────────────────────

class EditorApp {
public:
    // Full init with an explicit executable path so the editor can locate the project root.
    bool init(int width, int height, const std::string& executablePath) {
        // Initialise project path service first so commands can reference paths.
        m_projectPaths.init(executablePath);

        // Wire ContentBrowser to the project's Content directory.
        if (!m_projectPaths.contentPath().empty()) {
            m_contentBrowser.setRootPath(m_projectPaths.contentPath());
            m_contentBrowser.refresh();
        }

        m_renderer.init(width, height);
        m_ui.init();

        // Register core editor commands
        m_commands.registerCommand("file.new", [this]() {
            NF_LOG_INFO("Editor", "New world");
            m_currentWorldPath.clear();
            m_commandStack.clear();
        }, "New World", "Ctrl+N");

        m_commands.registerCommand("file.open", [this]() {
            NF_LOG_INFO("Editor", "Open world");
        }, "Open World", "Ctrl+O");

        m_commands.registerCommand("file.save", [this]() {
            NF_LOG_INFO("Editor", "Save world");
            if (!m_currentWorldPath.empty()) {
                m_commandStack.markClean();
            }
        }, "Save", "Ctrl+S");

        m_commands.registerCommand("file.save_as", [this]() {
            NF_LOG_INFO("Editor", "Save world as...");
        }, "Save As", "Ctrl+Shift+S");

        m_commands.registerCommand("edit.undo", [this]() {
            if (m_commandStack.canUndo()) {
                std::string desc = m_commandStack.undoDescription();
                m_commandStack.undo();
                NF_LOG_INFO("Editor", "Undo: " + desc);
            }
        }, "Undo", "Ctrl+Z");
        m_commands.setEnabledCheck("edit.undo", [this]() { return m_commandStack.canUndo(); });

        m_commands.registerCommand("edit.redo", [this]() {
            if (m_commandStack.canRedo()) {
                std::string desc = m_commandStack.redoDescription();
                m_commandStack.redo();
                NF_LOG_INFO("Editor", "Redo: " + desc);
            }
        }, "Redo", "Ctrl+Y");
        m_commands.setEnabledCheck("edit.redo", [this]() { return m_commandStack.canRedo(); });

        m_commands.registerCommand("edit.select_all", [this]() {
            NF_LOG_INFO("Editor", "Select all");
        }, "Select All", "Ctrl+A");

        m_commands.registerCommand("edit.deselect", [this]() {
            m_selection.clearSelection();
        }, "Deselect All", "Ctrl+D");

        m_commands.registerCommand("view.reset_layout", [this]() {
            NF_LOG_INFO("Editor", "Reset panel layout");
        }, "Reset Layout", "");

        // View toggle commands
        m_commands.registerCommand("view.toggle_inspector", [this]() {
            togglePanelVisibility("Inspector");
        }, "Toggle Inspector", "");

        m_commands.registerCommand("view.toggle_hierarchy", [this]() {
            togglePanelVisibility("Hierarchy");
        }, "Toggle Hierarchy", "");

        m_commands.registerCommand("view.toggle_console", [this]() {
            togglePanelVisibility("Console");
        }, "Toggle Console", "");

        m_commands.registerCommand("view.toggle_content_browser", [this]() {
            togglePanelVisibility("ContentBrowser");
        }, "Toggle Content Browser", "");

        // Graph commands
        m_commands.registerCommand("graph.new_graph", [this]() {
            if (auto* gep = graphEditorPanel()) {
                gep->newGraph(GraphType::World, "Untitled");
                m_notifications.push(NotificationType::Success, "New graph created");
            }
            NF_LOG_INFO("Editor", "New graph");
        }, "New Graph", "");

        m_commands.registerCommand("graph.open_graph", [this]() {
            if (auto* gep = graphEditorPanel()) {
                gep->openGraph("Untitled");
            }
            NF_LOG_INFO("Editor", "Open graph");
        }, "Open Graph", "");

        m_commands.registerCommand("graph.add_node", [this]() {
            if (auto* gep = graphEditorPanel()) {
                int id = gep->addNode("NewNode");
                if (id >= 0)
                    m_notifications.push(NotificationType::Info, "Added node #" + std::to_string(id));
            }
        }, "Add Node", "");

        m_commands.registerCommand("graph.remove_node", [this]() {
            if (auto* gep = graphEditorPanel()) {
                if (gep->selectedNodeId() >= 0) {
                    gep->removeNode(gep->selectedNodeId());
                    m_notifications.push(NotificationType::Info, "Node removed");
                }
            }
        }, "Remove Node", "Delete");
        m_commands.setEnabledCheck("graph.remove_node",
            [this]() {
                auto* gep = graphEditorPanel();
                return gep && gep->selectedNodeId() >= 0;
            });

        m_commands.registerCommand("graph.compile", [this]() {
            if (auto* gep = graphEditorPanel()) {
                if (gep->compileAndLoad())
                    m_notifications.push(NotificationType::Success, "Graph compiled");
            }
        }, "Compile Graph", "F7");

        // IDE commands
        m_ideService.init();

        m_commands.registerCommand("ide.go_to_definition", [this]() {
            NF_LOG_INFO("IDE", "Go to definition");
        }, "Go To Definition", "F12");

        m_commands.registerCommand("ide.find_references", [this]() {
            NF_LOG_INFO("IDE", "Find references");
        }, "Find References", "Shift+F12");

        m_commands.registerCommand("ide.go_back", [this]() {
            m_ideService.goBack();
        }, "Go Back", "Alt+Left");

        m_commands.registerCommand("ide.index_project", [this]() {
            // Index Source/, Content/, and Config/ so the entire repo is searchable.
            auto& indexer = m_ideService.indexer();
            indexer.clear();
            for (const std::string rel : {"Source", "Content", "Config"}) {
                std::string dir = m_projectPaths.resolvePath(rel);
                indexer.indexDirectory(dir);
            }
            // Populate CodeNavigator from all indexed symbols.
            auto& nav = m_ideService.navigator();
            nav.clear();
            for (auto& f : indexer.allFiles()) {
                for (auto& sym : f.symbols) {
                    NavigationEntry entry;
                    entry.symbol = sym;
                    entry.kind = SymbolKind::Unknown;
                    entry.filePath = f.path;
                    entry.line = 0;
                    nav.addEntry(std::move(entry));
                }
            }
            NF_LOG_INFO("IDE", "Project indexed: " +
                std::to_string(indexer.fileCount()) + " files, " +
                std::to_string(nav.entryCount()) + " symbols");
            m_notifications.push(NotificationType::Success,
                "Project indexed: " + std::to_string(indexer.fileCount()) + " files");
        }, "Index Project", "");

        // Entity commands
        m_commands.registerCommand("entity.create", [this]() {
            NF_LOG_INFO("Editor", "Create entity");
            m_notifications.push(NotificationType::Success, "Entity created");
        }, "Create Entity", "Ctrl+Shift+N");

        m_commands.registerCommand("entity.delete", [this]() {
            if (!m_selection.hasSelection()) return;
            NF_LOG_INFO("Editor", "Delete selected entities");
            m_selection.clearSelection();
            m_notifications.push(NotificationType::Info, "Entity deleted");
        }, "Delete Entity", "Delete");
        m_commands.setEnabledCheck("entity.delete",
            [this]() { return m_selection.hasSelection(); });

        m_commands.registerCommand("entity.duplicate", [this]() {
            if (!m_selection.hasSelection()) return;
            NF_LOG_INFO("Editor", "Duplicate selected entity");
            m_notifications.push(NotificationType::Success, "Entity duplicated");
        }, "Duplicate Entity", "Ctrl+D");
        m_commands.setEnabledCheck("entity.duplicate",
            [this]() { return m_selection.hasSelection(); });

        // Gizmo mode commands
        m_commands.registerCommand("gizmo.translate", [this]() {
            m_gizmo.setMode(GizmoMode::Translate);
        }, "Translate Gizmo", "W");

        m_commands.registerCommand("gizmo.rotate", [this]() {
            m_gizmo.setMode(GizmoMode::Rotate);
        }, "Rotate Gizmo", "E");

        m_commands.registerCommand("gizmo.scale", [this]() {
            m_gizmo.setMode(GizmoMode::Scale);
        }, "Scale Gizmo", "R");

        // Toggle graph editor panel
        m_commands.registerCommand("view.toggle_graph_editor", [this]() {
            togglePanelVisibility("GraphEditor");
        }, "Toggle Graph Editor", "");

        // Toggle settings
        m_commands.registerCommand("view.toggle_dark_mode", [this]() {
            m_editorSettings.setDarkMode(!m_editorSettings.settings().darkMode);
            m_editorSettings.applyTheme(m_theme);
            NF_LOG_INFO("Editor", std::string("Dark mode: ") +
                (m_editorSettings.settings().darkMode ? "on" : "off"));
        }, "Toggle Dark Mode", "");

        // Create default panels
        {
            auto viewport = std::make_unique<ViewportPanel>();
            m_dockLayout.addPanel(viewport->name(), viewport->slot());
            m_editorPanels.push_back(std::move(viewport));
        }
        {
            auto inspector = std::make_unique<InspectorPanel>(&m_selection, nullptr);
            m_dockLayout.addPanel(inspector->name(), inspector->slot());
            m_editorPanels.push_back(std::move(inspector));
        }
        {
            auto hierarchy = std::make_unique<HierarchyPanel>(&m_selection);
            m_dockLayout.addPanel(hierarchy->name(), hierarchy->slot());
            m_editorPanels.push_back(std::move(hierarchy));
        }
        {
            auto console = std::make_unique<ConsolePanel>();
            m_dockLayout.addPanel(console->name(), console->slot());
            m_editorPanels.push_back(std::move(console));
        }
        {
            auto cb = std::make_unique<ContentBrowserPanel>(&m_contentBrowser);
            m_dockLayout.addPanel(cb->name(), cb->slot());
            m_editorPanels.push_back(std::move(cb));
        }
        {
            auto graphEd = std::make_unique<GraphEditorPanel>(m_graphVM);
            m_dockLayout.addPanel(graphEd->name(), graphEd->slot());
            m_dockLayout.setPanelVisible(graphEd->name(), false);  // hidden by default
            m_editorPanels.push_back(std::move(graphEd));
        }

        // Create default toolbar items
        m_toolbar.addItem("Select", "select", "Select tool", [this]() {
            m_gizmo.setMode(GizmoMode::Translate);
        });
        m_toolbar.addItem("Move", "move", "Move tool", [this]() {
            m_gizmo.setMode(GizmoMode::Translate);
        });
        m_toolbar.addItem("Rotate", "rotate", "Rotate tool", [this]() {
            m_gizmo.setMode(GizmoMode::Rotate);
        });
        m_toolbar.addItem("Scale", "scale", "Scale tool", [this]() {
            m_gizmo.setMode(GizmoMode::Scale);
        });
        m_toolbar.addSeparator();
        m_toolbar.addItem("Play", "play", "Play", []() {});
        m_toolbar.addItem("Pause", "pause", "Pause", []() {});
        m_toolbar.addItem("Stop", "stop", "Stop", []() {});

        // Build menu bar
        initMenuBar();

        // Load default hotkeys from registered commands
        m_hotkeyDispatcher.loadDefaults(m_commands);

        NF_LOG_INFO("Editor", "NovaForge Editor initialized");
        return true;
    }

    // Convenience overload: uses current working directory as the project root.
    bool init(int width, int height) { return init(width, height, "."); }

    void shutdown() {
        m_ideService.shutdown();
        m_editorPanels.clear();
        m_ui.shutdown();
        m_renderer.shutdown();
        NF_LOG_INFO("Editor", "NovaForge Editor shutdown");
    }

    void update() {}
    void render() {}

    // Per-frame update with input: routes right-click WASD fly-cam to the viewport,
    // dispatches hotkeys, ticks notifications, updates status bar and frame stats.
    void update(float dt, InputSystem& input) {
        input.update();
        if (auto* vp = viewportPanel())
            vp->updateCamera(dt, input);

        m_notifications.tick(dt);
        m_frameStats.beginFrame(dt);

        m_statusBar.update(
            "Editor",
            m_currentWorldPath,
            m_commandStack.isDirty(),
            static_cast<int>(m_selection.selectionCount()),
            m_frameStats.stats().fps);
    }

    // Process a hotkey string and dispatch matching commands.
    int processHotkey(const std::string& hotkey) {
        return m_hotkeyDispatcher.dispatch(hotkey, m_commands);
    }

    // Returns a pointer to the first ViewportPanel, or nullptr.
    [[nodiscard]] ViewportPanel* viewportPanel() {
        for (auto& p : m_editorPanels) {
            if (auto* vp = dynamic_cast<ViewportPanel*>(p.get())) return vp;
        }
        return nullptr;
    }
    [[nodiscard]] const ViewportPanel* viewportPanel() const {
        for (auto& p : m_editorPanels) {
            if (auto* vp = dynamic_cast<const ViewportPanel*>(p.get())) return vp;
        }
        return nullptr;
    }

    [[nodiscard]] GraphEditorPanel* graphEditorPanel() {
        for (auto& p : m_editorPanels) {
            if (auto* gep = dynamic_cast<GraphEditorPanel*>(p.get())) return gep;
        }
        return nullptr;
    }

    // Accessors for editor services
    EditorCommandRegistry& commands() { return m_commands; }
    CommandStack& commandStack() { return m_commandStack; }
    SelectionService& selection() { return m_selection; }
    ContentBrowser& contentBrowser() { return m_contentBrowser; }
    RecentFilesList& recentFiles() { return m_recentFiles; }
    LaunchService& launchService() { return m_launchService; }
    ProjectPathService& projectPaths() { return m_projectPaths; }
    const ProjectPathService& projectPaths() const { return m_projectPaths; }
    DockLayout& dockLayout() { return m_dockLayout; }
    EditorToolbar& toolbar() { return m_toolbar; }
    MenuBar& menuBar() { return m_menuBar; }
    EditorStatusBar& statusBar() { return m_statusBar; }
    NotificationQueue& notifications() { return m_notifications; }
    EditorCameraOrbit& editorCamera() { return m_editorCamera; }
    GizmoState& gizmo() { return m_gizmo; }
    EditorSettingsService& settingsService() { return m_editorSettings; }
    HotkeyDispatcher& hotkeyDispatcher() { return m_hotkeyDispatcher; }
    FrameStatsTracker& frameStatsTracker() { return m_frameStats; }
    EditorTheme& theme() { return m_theme; }

    [[nodiscard]] const std::string& currentWorldPath() const { return m_currentWorldPath; }
    void setCurrentWorldPath(const std::string& path) {
        m_currentWorldPath = path;
        m_recentFiles.addFile(path);
    }

    [[nodiscard]] bool isDirty() const { return m_commandStack.isDirty(); }

    void addPanel(std::unique_ptr<EditorPanel> panel) {
        m_dockLayout.addPanel(panel->name(), panel->slot());
        m_editorPanels.push_back(std::move(panel));
    }

    [[nodiscard]] const std::vector<std::unique_ptr<EditorPanel>>& editorPanels() const {
        return m_editorPanels;
    }

    void setGraphVM(GraphVM* vm) {
        m_graphVM = vm;
        if (auto* gep = graphEditorPanel()) gep->setGraphVM(vm);
    }
    [[nodiscard]] GraphVM* graphVM() const { return m_graphVM; }

    IDEService& ideService() { return m_ideService; }

private:
    void togglePanelVisibility(const std::string& name) {
        if (auto* p = m_dockLayout.findPanel(name)) {
            p->visible = !p->visible;
            NF_LOG_INFO("Editor", "Toggle panel '" + name + "' visible=" +
                        (p->visible ? "true" : "false"));
        }
    }

    void initMenuBar() {
        // File menu
        auto& file = m_menuBar.addCategory("File");
        file.addItem("New World",   "file.new",     "Ctrl+N");
        file.addItem("Open World",  "file.open",    "Ctrl+O");
        file.addItem("Save",        "file.save",    "Ctrl+S");
        file.addItem("Save As",     "file.save_as", "Ctrl+Shift+S");
        file.addSeparator();
        file.addItem("Exit",        "file.exit",    "Alt+F4");

        // Edit menu
        auto& edit = m_menuBar.addCategory("Edit");
        edit.addItem("Undo",        "edit.undo",    "Ctrl+Z");
        edit.addItem("Redo",        "edit.redo",    "Ctrl+Y");
        edit.addSeparator();
        edit.addItem("Select All",  "edit.select_all",  "Ctrl+A");
        edit.addItem("Deselect",    "edit.deselect",     "Ctrl+D");
        edit.addSeparator();
        edit.addItem("Create Entity",   "entity.create",    "Ctrl+Shift+N");
        edit.addItem("Delete Entity",   "entity.delete",    "Delete");
        edit.addItem("Duplicate Entity","entity.duplicate", "Ctrl+D");

        // View menu
        auto& view = m_menuBar.addCategory("View");
        view.addItem("Inspector",      "view.toggle_inspector",       "");
        view.addItem("Hierarchy",      "view.toggle_hierarchy",       "");
        view.addItem("Console",        "view.toggle_console",         "");
        view.addItem("Content Browser","view.toggle_content_browser", "");
        view.addItem("Graph Editor",   "view.toggle_graph_editor",    "");
        view.addSeparator();
        view.addItem("Reset Layout",   "view.reset_layout",           "");
        view.addItem("Dark Mode",      "view.toggle_dark_mode",       "");

        // Graph menu
        auto& graph = m_menuBar.addCategory("Graph");
        graph.addItem("New Graph",    "graph.new_graph",    "");
        graph.addItem("Open Graph",   "graph.open_graph",   "");
        graph.addSeparator();
        graph.addItem("Add Node",     "graph.add_node",     "");
        graph.addItem("Remove Node",  "graph.remove_node",  "");
        graph.addSeparator();
        graph.addItem("Compile",      "graph.compile",      "F7");

        // Code menu
        auto& code = m_menuBar.addCategory("Code");
        code.addItem("Go To Definition", "ide.go_to_definition", "F12");
        code.addItem("Find References",  "ide.find_references",  "Shift+F12");
        code.addItem("Go Back",          "ide.go_back",          "Alt+Left");
        code.addItem("Index Project",    "ide.index_project",    "");
    }

    Renderer m_renderer;
    UIRenderer m_ui;
    EditorCommandRegistry m_commands;
    CommandStack m_commandStack;
    SelectionService m_selection;
    ContentBrowser m_contentBrowser;
    RecentFilesList m_recentFiles;
    LaunchService m_launchService;
    ProjectPathService m_projectPaths;
    DockLayout m_dockLayout;
    EditorToolbar m_toolbar;
    std::vector<std::unique_ptr<EditorPanel>> m_editorPanels;
    std::string m_currentWorldPath;
    GraphVM* m_graphVM = nullptr;
    IDEService m_ideService;

    // New systems
    MenuBar m_menuBar;
    EditorStatusBar m_statusBar;
    NotificationQueue m_notifications;
    EditorCameraOrbit m_editorCamera;
    GizmoState m_gizmo;
    EditorSettingsService m_editorSettings;
    HotkeyDispatcher m_hotkeyDispatcher;
    FrameStatsTracker m_frameStats;
    EditorTheme m_theme;
};

// ── Property Editor ──────────────────────────────────────────────
// Reads and writes property values via reflection offsets.

class PropertyEditor {
public:
    // Read a property value from a struct via offset, returning a JSON-like representation
    static JsonValue readProperty(const void* obj, const PropertyInfo& prop) {
        const uint8_t* base = static_cast<const uint8_t*>(obj) + prop.offset;

        switch (prop.type) {
            case PropertyType::Bool:
                return JsonValue(*reinterpret_cast<const bool*>(base));
            case PropertyType::Int32:
                return JsonValue(*reinterpret_cast<const int32_t*>(base));
            case PropertyType::Float:
                return JsonValue(*reinterpret_cast<const float*>(base));
            case PropertyType::String:
                return JsonValue(*reinterpret_cast<const std::string*>(base));
            case PropertyType::Vec3: {
                const auto* v = reinterpret_cast<const Vec3*>(base);
                auto arr = JsonValue::array();
                arr.push(JsonValue(v->x));
                arr.push(JsonValue(v->y));
                arr.push(JsonValue(v->z));
                return arr;
            }
            case PropertyType::Color: {
                const auto* c = reinterpret_cast<const Color*>(base);
                auto arr = JsonValue::array();
                arr.push(JsonValue(c->r));
                arr.push(JsonValue(c->g));
                arr.push(JsonValue(c->b));
                arr.push(JsonValue(c->a));
                return arr;
            }
            default:
                return JsonValue();
        }
    }

    // Write a property value to a struct via offset
    static bool writeProperty(void* obj, const PropertyInfo& prop, const JsonValue& value) {
        uint8_t* base = static_cast<uint8_t*>(obj) + prop.offset;

        switch (prop.type) {
            case PropertyType::Bool:
                if (!value.isBool()) return false;
                *reinterpret_cast<bool*>(base) = value.asBool();
                return true;
            case PropertyType::Int32:
                if (!value.isNumber()) return false;
                *reinterpret_cast<int32_t*>(base) = value.asInt();
                return true;
            case PropertyType::Float:
                if (!value.isNumber()) return false;
                *reinterpret_cast<float*>(base) = value.asFloat();
                return true;
            case PropertyType::String:
                if (!value.isString()) return false;
                *reinterpret_cast<std::string*>(base) = value.asString();
                return true;
            case PropertyType::Vec3: {
                if (!value.isArray() || value.size() < 3) return false;
                auto* v = reinterpret_cast<Vec3*>(base);
                v->x = value[static_cast<size_t>(0)].asFloat();
                v->y = value[static_cast<size_t>(1)].asFloat();
                v->z = value[static_cast<size_t>(2)].asFloat();
                return true;
            }
            case PropertyType::Color: {
                if (!value.isArray() || value.size() < 4) return false;
                auto* c = reinterpret_cast<Color*>(base);
                c->r = value[static_cast<size_t>(0)].asFloat();
                c->g = value[static_cast<size_t>(1)].asFloat();
                c->b = value[static_cast<size_t>(2)].asFloat();
                c->a = value[static_cast<size_t>(3)].asFloat();
                return true;
            }
            default:
                return false;
        }
    }

    // Create an undo-safe property change for a float property
    static std::unique_ptr<ICommand> makeFloatChange(float* target, float newValue,
                                                      const std::string& propName) {
        return std::make_unique<PropertyChangeCommand<float>>(
            target, *target, newValue, "Change " + propName);
    }

    // Create an undo-safe property change for an int property
    static std::unique_ptr<ICommand> makeIntChange(int32_t* target, int32_t newValue,
                                                    const std::string& propName) {
        return std::make_unique<PropertyChangeCommand<int32_t>>(
            target, *target, newValue, "Change " + propName);
    }

    // Create an undo-safe property change for a bool property
    static std::unique_ptr<ICommand> makeBoolChange(bool* target, bool newValue,
                                                     const std::string& propName) {
        return std::make_unique<PropertyChangeCommand<bool>>(
            target, *target, newValue, "Change " + propName);
    }

    // Create an undo-safe property change for a Vec3 property
    static std::unique_ptr<ICommand> makeVec3Change(Vec3* target, Vec3 newValue,
                                                     const std::string& propName) {
        return std::make_unique<PropertyChangeCommand<Vec3>>(
            target, *target, newValue, "Change " + propName);
    }

    // Create an undo-safe property change for a Color property
    static std::unique_ptr<ICommand> makeColorChange(Color* target, Color newValue,
                                                      const std::string& propName) {
        return std::make_unique<PropertyChangeCommand<Color>>(
            target, *target, newValue, "Change " + propName);
    }

    // Create an undo-safe property change for a string property
    static std::unique_ptr<ICommand> makeStringChange(std::string* target,
                                                       const std::string& newValue,
                                                       const std::string& propName) {
        return std::make_unique<PropertyChangeCommand<std::string>>(
            target, *target, newValue, "Change " + propName);
    }
};

} // namespace NF
