#pragma once
// NF::Pipeline — Shared workspace pipeline for inter-tool communication
//
// All tools (editor, Blender bridge, SwissAgent, ArbiterAI, ContractScanner,
// ReplayMinimizer) communicate exclusively by reading and writing files under
// the workspace's .novaforge/pipeline/ directory tree.  No sockets, no RPC.
//
// Directory layout (relative to workspace root):
//   .novaforge/
//     pipeline/
//       changes/       ← .change.json events dropped here by any tool
//       assets/        ← imported/generated assets (meshes, textures)
//       worlds/        ← live world-state snapshots
//       scripts/       ← GraphVM bytecode / logic graph JSON
//       animations/    ← exported rigs, clips, IK configs
//       sessions/      ← AtlasAI session logs (future)
//     manifest.json    ← GUID → asset path registry
//     watch.log        ← append-only event log written by all tools

#include <cstdint>
#include <string>
#include <string_view>
#include <functional>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <filesystem>

namespace NF {

// ── ChangeEventType ──────────────────────────────────────────────

enum class ChangeEventType : uint8_t {
    Unknown,
    AssetImported,
    WorldChanged,
    ScriptUpdated,
    AnimationExported,
    ContractIssue,
    ReplayExported,
};

inline const char* changeEventTypeName(ChangeEventType t) noexcept {
    switch (t) {
        case ChangeEventType::AssetImported:     return "AssetImported";
        case ChangeEventType::WorldChanged:      return "WorldChanged";
        case ChangeEventType::ScriptUpdated:     return "ScriptUpdated";
        case ChangeEventType::AnimationExported: return "AnimationExported";
        case ChangeEventType::ContractIssue:     return "ContractIssue";
        case ChangeEventType::ReplayExported:    return "ReplayExported";
        default:                                  return "Unknown";
    }
}

inline ChangeEventType changeEventTypeFromString(std::string_view s) noexcept {
    if (s == "AssetImported")     return ChangeEventType::AssetImported;
    if (s == "WorldChanged")      return ChangeEventType::WorldChanged;
    if (s == "ScriptUpdated")     return ChangeEventType::ScriptUpdated;
    if (s == "AnimationExported") return ChangeEventType::AnimationExported;
    if (s == "ContractIssue")     return ChangeEventType::ContractIssue;
    if (s == "ReplayExported")    return ChangeEventType::ReplayExported;
    return ChangeEventType::Unknown;
}

// ── ChangeEvent ──────────────────────────────────────────────────
// Schema: { "tool": "...", "event_type": "...", "path": "...",
//           "timestamp": <ms>, "metadata": "..." }

struct ChangeEvent {
    std::string     tool;
    ChangeEventType eventType = ChangeEventType::Unknown;
    std::string     path;
    int64_t         timestamp = 0;   // milliseconds since Unix epoch
    std::string     metadata;        // arbitrary string (JSON-compatible)

    // Serialize to a JSON string.
    std::string toJson() const;

    // Parse from a JSON string; returns false on failure.
    static bool fromJson(const std::string& json, ChangeEvent& out);

    // Write to a .change.json file inside changesDir.
    // Filename: <tool>_<eventtype>_<timestamp>.change.json
    bool writeToFile(const std::filesystem::path& changesDir) const;

    // Read from a .change.json file.
    static bool readFromFile(const std::filesystem::path& filePath,
                             ChangeEvent& out);
};

// ── AssetRecord ──────────────────────────────────────────────────

struct AssetRecord {
    std::string guid;
    std::string type;         // "mesh", "texture", "animation", "script", …
    std::string path;         // relative path from workspace root
    int64_t     importDate = 0;
    std::string checksum;
};

// ── Manifest ─────────────────────────────────────────────────────
// Backed by manifest.json.  Stores the project-level GUID registry and
// module list.  All assets imported from any tool get an entry here.

class Manifest {
public:
    Manifest() = default;

    // Load from a manifest.json file.  Returns false if the file is missing
    // or cannot be parsed (leaves the object in a default-constructed state).
    bool load(const std::filesystem::path& manifestPath);

    // Persist to a manifest.json file.  Creates the file if it does not exist.
    bool save(const std::filesystem::path& manifestPath) const;

    // Register an asset.  If record.guid is empty a unique GUID is generated.
    // If an asset with the same path already exists it is updated.
    // Returns the final GUID.
    std::string registerAsset(AssetRecord record);

    const AssetRecord* findByGuid(const std::string& guid) const;
    const AssetRecord* findByPath(const std::string& path) const;

    // Remove by GUID.  Returns true if an entry was found and removed.
    bool removeAsset(const std::string& guid);

    const std::vector<AssetRecord>& records() const { return m_records; }
    size_t recordCount() const { return m_records.size(); }

    // Project-level metadata.
    std::string              projectName;
    std::vector<std::string> modules;

private:
    std::vector<AssetRecord>                  m_records;
    std::unordered_map<std::string, size_t>   m_guidIndex;  // guid  → index
    std::unordered_map<std::string, size_t>   m_pathIndex;  // path  → index

    static std::string generateGuid();
    void rebuildIndex();
};

// ── WatchLog ─────────────────────────────────────────────────────
// Append-only event log.  Every tool appends its ChangeEvents here so that
// AtlasAI (S9) can replay the full development timeline.

class WatchLog {
public:
    explicit WatchLog(std::filesystem::path logPath);
    ~WatchLog() = default;

    WatchLog(const WatchLog&)            = delete;
    WatchLog& operator=(const WatchLog&) = delete;

    // Append a ChangeEvent line (thread-safe).
    void append(const ChangeEvent& event);

    // Append a raw text line (thread-safe).
    void appendLine(std::string_view message);

    // Flush buffered writes to disk.
    void flush();

    const std::filesystem::path& path() const { return m_logPath; }

private:
    std::filesystem::path m_logPath;
    std::mutex            m_mutex;
};

// ── PipelineWatcher ──────────────────────────────────────────────
// Watches a directory for new .change.json files and dispatches typed
// ChangeEvents to registered callbacks.
//
// Two modes of use:
//  1. Background thread: call start() — the watcher polls every 100 ms.
//  2. Single-thread / test: call poll() manually.

using PipelineEventCallback = std::function<void(const ChangeEvent&)>;

class PipelineWatcher {
public:
    explicit PipelineWatcher(std::filesystem::path watchDir);
    ~PipelineWatcher();

    PipelineWatcher(const PipelineWatcher&)            = delete;
    PipelineWatcher& operator=(const PipelineWatcher&) = delete;

    // Register a callback to be invoked when a new event is detected.
    void subscribe(PipelineEventCallback cb);

    // Start the background polling thread.  Returns false if already running.
    bool start();

    // Stop the background thread and join it.
    void stop();

    bool isRunning() const noexcept { return m_running.load(); }

    // Manually scan watchDir for unprocessed .change.json files.
    // Dispatches callbacks synchronously.  Returns the count of new events.
    int poll();

    const std::filesystem::path& watchDir() const { return m_watchDir; }

private:
    void watchLoop();
    void processFile(const std::filesystem::path& filePath);

    std::filesystem::path              m_watchDir;
    std::vector<PipelineEventCallback> m_callbacks;
    std::atomic<bool>                  m_running{false};
    std::thread                        m_thread;
    std::mutex                         m_callbackMutex;

    // Tracks processed files: filename → last-write-time in ms.
    std::unordered_map<std::string, int64_t> m_seen;
    std::mutex                               m_seenMutex;
};

// ── PipelineDirectories ──────────────────────────────────────────
// Utility: derive all pipeline paths from a workspace root and optionally
// create the directory tree.

struct PipelineDirectories {
    std::filesystem::path root;
    std::filesystem::path dotNovaForge;
    std::filesystem::path pipeline;
    std::filesystem::path changes;
    std::filesystem::path assets;
    std::filesystem::path worlds;
    std::filesystem::path scripts;
    std::filesystem::path animations;
    std::filesystem::path sessions;
    std::filesystem::path manifestFile;
    std::filesystem::path watchLogFile;

    static PipelineDirectories fromRoot(const std::filesystem::path& workspaceRoot);

    // Create all directories on disk.  Returns false if any creation fails.
    bool ensureCreated() const;
};

} // namespace NF
