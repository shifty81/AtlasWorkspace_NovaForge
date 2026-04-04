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

// ── Editor application ───────────────────────────────────────────

class EditorApp {
public:
    bool init(int width, int height) {
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
            NF_LOG_INFO("Editor", "New graph");
        }, "New Graph", "");

        m_commands.registerCommand("graph.open_graph", [this]() {
            NF_LOG_INFO("Editor", "Open graph");
        }, "Open Graph", "");

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
            NF_LOG_INFO("IDE", "Index project");
        }, "Index Project", "");

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

        // Create default toolbar items
        m_toolbar.addItem("Select", "select", "Select tool", []() {});
        m_toolbar.addItem("Move", "move", "Move tool", []() {});
        m_toolbar.addItem("Rotate", "rotate", "Rotate tool", []() {});
        m_toolbar.addItem("Scale", "scale", "Scale tool", []() {});
        m_toolbar.addSeparator();
        m_toolbar.addItem("Play", "play", "Play", []() {});
        m_toolbar.addItem("Pause", "pause", "Pause", []() {});
        m_toolbar.addItem("Stop", "stop", "Stop", []() {});

        NF_LOG_INFO("Editor", "NovaForge Editor initialized");
        return true;
    }

    void shutdown() {
        m_ideService.shutdown();
        m_editorPanels.clear();
        m_ui.shutdown();
        m_renderer.shutdown();
        NF_LOG_INFO("Editor", "NovaForge Editor shutdown");
    }

    void update() {}
    void render() {}

    // Per-frame update with input: routes right-click WASD fly-cam to the viewport.
    void update(float dt, InputSystem& input) {
        input.update();
        if (auto* vp = viewportPanel())
            vp->updateCamera(dt, input);
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

    // Accessors for editor services
    EditorCommandRegistry& commands() { return m_commands; }
    CommandStack& commandStack() { return m_commandStack; }
    SelectionService& selection() { return m_selection; }
    ContentBrowser& contentBrowser() { return m_contentBrowser; }
    RecentFilesList& recentFiles() { return m_recentFiles; }
    LaunchService& launchService() { return m_launchService; }
    DockLayout& dockLayout() { return m_dockLayout; }
    EditorToolbar& toolbar() { return m_toolbar; }

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

    void setGraphVM(GraphVM* vm) { m_graphVM = vm; }
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

    Renderer m_renderer;
    UIRenderer m_ui;
    EditorCommandRegistry m_commands;
    CommandStack m_commandStack;
    SelectionService m_selection;
    ContentBrowser m_contentBrowser;
    RecentFilesList m_recentFiles;
    LaunchService m_launchService;
    DockLayout m_dockLayout;
    EditorToolbar m_toolbar;
    std::vector<std::unique_ptr<EditorPanel>> m_editorPanels;
    std::string m_currentWorldPath;
    GraphVM* m_graphVM = nullptr;
    IDEService m_ideService;
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
