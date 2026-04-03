#pragma once
// NF::Editor — Editor app, docking panels, viewport, toolbar. Does not ship.
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/Game/Game.h"
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
    void init(const std::string& executablePath) {
        m_executablePath = executablePath;

        // Derive project root from executable path
        auto exePath = std::filesystem::path(executablePath);
        // Walk up from bin/ or Builds/ to find project root
        constexpr int kMaxProjectRootSearchDepth = 5;
        auto dir = exePath.parent_path();
        for (int i = 0; i < kMaxProjectRootSearchDepth; ++i) {
            if (std::filesystem::exists(dir / "Config" / "novaforge.project.json")) {
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

    // Accessors for editor services
    EditorCommandRegistry& commands() { return m_commands; }
    CommandStack& commandStack() { return m_commandStack; }
    SelectionService& selection() { return m_selection; }
    ContentBrowser& contentBrowser() { return m_contentBrowser; }
    RecentFilesList& recentFiles() { return m_recentFiles; }
    LaunchService& launchService() { return m_launchService; }

    [[nodiscard]] const std::string& currentWorldPath() const { return m_currentWorldPath; }
    void setCurrentWorldPath(const std::string& path) {
        m_currentWorldPath = path;
        m_recentFiles.addFile(path);
    }

    [[nodiscard]] bool isDirty() const { return m_commandStack.isDirty(); }

private:
    Renderer m_renderer;
    UIRenderer m_ui;
    EditorCommandRegistry m_commands;
    CommandStack m_commandStack;
    SelectionService m_selection;
    ContentBrowser m_contentBrowser;
    RecentFilesList m_recentFiles;
    LaunchService m_launchService;
    std::vector<DockPanel> m_panels;
    std::string m_currentWorldPath;
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
