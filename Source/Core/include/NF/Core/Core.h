#pragma once
// NF::Core — Math, memory, logging, events, reflection, serialization

#include <cstdint>
#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <format>
#include <iostream>

namespace NF {

// ── Logging ──────────────────────────────────────────────────────

enum class LogLevel : uint8_t {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

class Logger {
public:
    static Logger& instance() {
        static Logger s;
        return s;
    }

    void log(LogLevel level, std::string_view category, std::string_view message) {
        if (level < m_minLevel) return;
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &time);
#else
        localtime_r(&time, &tm);
#endif
        std::cout << std::format("[{:02}:{:02}:{:02}] [{}] [{}] {}\n",
            tm.tm_hour, tm.tm_min, tm.tm_sec,
            levelString(level), category, message);
    }

    void setMinLevel(LogLevel level) { m_minLevel = level; }

private:
    Logger() = default;
    LogLevel m_minLevel = LogLevel::Info;

    static constexpr const char* levelString(LogLevel level) {
        switch (level) {
            case LogLevel::Trace: return "TRACE";
            case LogLevel::Debug: return "DEBUG";
            case LogLevel::Info:  return "INFO ";
            case LogLevel::Warn:  return "WARN ";
            case LogLevel::Error: return "ERROR";
            case LogLevel::Fatal: return "FATAL";
            default:              return "?????";
        }
    }
};

#define NF_LOG(level, category, msg) \
    ::NF::Logger::instance().log(::NF::LogLevel::level, category, msg)

#define NF_LOG_INFO(cat, msg)  NF_LOG(Info,  cat, msg)
#define NF_LOG_WARN(cat, msg)  NF_LOG(Warn,  cat, msg)
#define NF_LOG_ERROR(cat, msg) NF_LOG(Error, cat, msg)
#define NF_LOG_DEBUG(cat, msg) NF_LOG(Debug, cat, msg)

// ── Math types ───────────────────────────────────────────────────

struct Vec2 {
    float x = 0.f, y = 0.f;
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
};

struct Vec3 {
    float x = 0.f, y = 0.f, z = 0.f;
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    float dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vec3 cross(const Vec3& o) const {
        return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
    }
    float lengthSq() const { return dot(*this); }
};

struct Vec4 {
    float x = 0.f, y = 0.f, z = 0.f, w = 0.f;
};

struct Rect {
    float x = 0.f, y = 0.f, w = 0.f, h = 0.f;
    bool contains(float px, float py) const {
        return px >= x && px <= x + w && py >= y && py <= y + h;
    }
};

// ── Event bus ────────────────────────────────────────────────────

using EventID = uint32_t;

class EventBus {
public:
    using Handler = std::function<void(const void*)>;

    void subscribe(EventID id, Handler handler) {
        m_handlers[id].push_back(std::move(handler));
    }

    void publish(EventID id, const void* data = nullptr) {
        auto it = m_handlers.find(id);
        if (it != m_handlers.end()) {
            for (auto& h : it->second) h(data);
        }
    }

private:
    std::unordered_map<EventID, std::vector<Handler>> m_handlers;
};

// ── Version ──────────────────────────────────────────────────────

constexpr int NF_VERSION_MAJOR = 0;
constexpr int NF_VERSION_MINOR = 1;
constexpr int NF_VERSION_PATCH = 0;
constexpr const char* NF_VERSION_STRING = "0.1.0";

// ── Core lifecycle ───────────────────────────────────────────────

void coreInit();
void coreShutdown();

} // namespace NF
