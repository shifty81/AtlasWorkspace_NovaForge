#pragma once
// NF::Core — Math, memory, logging, events, reflection, serialization

#include <cstdint>
#include <cmath>
#include <cstring>
#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <vector>
#include <array>
#include <unordered_map>
#include <chrono>
#include <format>
#include <iostream>
#include <algorithm>
#include <cassert>

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
    LogLevel minLevel() const { return m_minLevel; }

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
#define NF_LOG_TRACE(cat, msg) NF_LOG(Trace, cat, msg)

// ── Math types ───────────────────────────────────────────────────

struct Vec2 {
    float x = 0.f, y = 0.f;
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    float dot(const Vec2& o) const { return x * o.x + y * o.y; }
    float lengthSq() const { return dot(*this); }
    float length() const { return std::sqrt(lengthSq()); }
    Vec2 normalized() const {
        float len = length();
        if (len < 1e-7f) return {0.f, 0.f};
        return {x / len, y / len};
    }
    bool operator==(const Vec2& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Vec2& o) const { return !(*this == o); }
};

struct Vec3 {
    float x = 0.f, y = 0.f, z = 0.f;
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3 operator-() const { return {-x, -y, -z}; }
    float dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vec3 cross(const Vec3& o) const {
        return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
    }
    float lengthSq() const { return dot(*this); }
    float length() const { return std::sqrt(lengthSq()); }
    Vec3 normalized() const {
        float len = length();
        if (len < 1e-7f) return {0.f, 0.f, 0.f};
        return {x / len, y / len, z / len};
    }
    bool operator==(const Vec3& o) const { return x == o.x && y == o.y && z == o.z; }
    bool operator!=(const Vec3& o) const { return !(*this == o); }

    static Vec3 lerp(const Vec3& a, const Vec3& b, float t) {
        return a + (b - a) * t;
    }
};

struct Vec4 {
    float x = 0.f, y = 0.f, z = 0.f, w = 0.f;
    Vec4 operator+(const Vec4& o) const { return {x + o.x, y + o.y, z + o.z, w + o.w}; }
    Vec4 operator-(const Vec4& o) const { return {x - o.x, y - o.y, z - o.z, w - o.w}; }
    Vec4 operator*(float s) const { return {x * s, y * s, z * s, w * s}; }
    float dot(const Vec4& o) const { return x * o.x + y * o.y + z * o.z + w * o.w; }
    bool operator==(const Vec4& o) const { return x == o.x && y == o.y && z == o.z && w == o.w; }
    bool operator!=(const Vec4& o) const { return !(*this == o); }
};

struct Rect {
    float x = 0.f, y = 0.f, w = 0.f, h = 0.f;
    bool contains(float px, float py) const {
        return px >= x && px <= x + w && py >= y && py <= y + h;
    }
    bool overlaps(const Rect& o) const {
        return x < o.x + o.w && x + w > o.x &&
               y < o.y + o.h && y + h > o.y;
    }
};

// ── Quaternion ───────────────────────────────────────────────────

struct Quat {
    float x = 0.f, y = 0.f, z = 0.f, w = 1.f;

    static Quat identity() { return {0.f, 0.f, 0.f, 1.f}; }

    static Quat fromAxisAngle(const Vec3& axis, float angleRadians) {
        float half = angleRadians * 0.5f;
        float s = std::sin(half);
        Vec3 n = axis.normalized();
        return {n.x * s, n.y * s, n.z * s, std::cos(half)};
    }

    static Quat fromEuler(float pitchRad, float yawRad, float rollRad) {
        float cp = std::cos(pitchRad * 0.5f), sp = std::sin(pitchRad * 0.5f);
        float cy = std::cos(yawRad   * 0.5f), sy = std::sin(yawRad   * 0.5f);
        float cr = std::cos(rollRad  * 0.5f), sr = std::sin(rollRad  * 0.5f);
        return {
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy,
            cr * cp * cy + sr * sp * sy
        };
    }

    Quat operator*(const Quat& o) const {
        return {
            w * o.x + x * o.w + y * o.z - z * o.y,
            w * o.y - x * o.z + y * o.w + z * o.x,
            w * o.z + x * o.y - y * o.x + z * o.w,
            w * o.w - x * o.x - y * o.y - z * o.z
        };
    }

    Quat conjugate() const { return {-x, -y, -z, w}; }

    float lengthSq() const { return x * x + y * y + z * z + w * w; }
    float length() const { return std::sqrt(lengthSq()); }

    Quat normalized() const {
        float len = length();
        if (len < 1e-7f) return identity();
        return {x / len, y / len, z / len, w / len};
    }

    Vec3 rotate(const Vec3& v) const {
        Quat p{v.x, v.y, v.z, 0.f};
        Quat result = *this * p * conjugate();
        return {result.x, result.y, result.z};
    }

    static Quat slerp(const Quat& a, const Quat& b, float t) {
        float cosTheta = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        Quat target = b;
        if (cosTheta < 0.f) {
            target = {-b.x, -b.y, -b.z, -b.w};
            cosTheta = -cosTheta;
        }
        if (cosTheta > 0.9995f) {
            return Quat{
                a.x + (target.x - a.x) * t,
                a.y + (target.y - a.y) * t,
                a.z + (target.z - a.z) * t,
                a.w + (target.w - a.w) * t
            }.normalized();
        }
        float theta = std::acos(cosTheta);
        float sinTheta = std::sin(theta);
        float wa = std::sin((1.f - t) * theta) / sinTheta;
        float wb = std::sin(t * theta) / sinTheta;
        return Quat{
            a.x * wa + target.x * wb,
            a.y * wa + target.y * wb,
            a.z * wa + target.z * wb,
            a.w * wa + target.w * wb
        };
    }
};

// ── 4x4 Matrix (column-major) ────────────────────────────────────

struct Mat4 {
    float m[16] = {};

    static Mat4 identity() {
        Mat4 r;
        r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.f;
        return r;
    }

    static Mat4 translation(float tx, float ty, float tz) {
        Mat4 r = identity();
        r.m[12] = tx; r.m[13] = ty; r.m[14] = tz;
        return r;
    }

    static Mat4 translation(const Vec3& v) { return translation(v.x, v.y, v.z); }

    static Mat4 scaling(float sx, float sy, float sz) {
        Mat4 r;
        r.m[0] = sx; r.m[5] = sy; r.m[10] = sz; r.m[15] = 1.f;
        return r;
    }

    static Mat4 scaling(const Vec3& v) { return scaling(v.x, v.y, v.z); }

    static Mat4 fromQuat(const Quat& q) {
        Mat4 r = identity();
        float xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
        float xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
        float wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;
        r.m[0]  = 1.f - 2.f * (yy + zz);
        r.m[1]  = 2.f * (xy + wz);
        r.m[2]  = 2.f * (xz - wy);
        r.m[4]  = 2.f * (xy - wz);
        r.m[5]  = 1.f - 2.f * (xx + zz);
        r.m[6]  = 2.f * (yz + wx);
        r.m[8]  = 2.f * (xz + wy);
        r.m[9]  = 2.f * (yz - wx);
        r.m[10] = 1.f - 2.f * (xx + yy);
        return r;
    }

    static Mat4 perspective(float fovRadians, float aspect, float nearPlane, float farPlane) {
        Mat4 r;
        float tanHalf = std::tan(fovRadians * 0.5f);
        r.m[0]  = 1.f / (aspect * tanHalf);
        r.m[5]  = 1.f / tanHalf;
        r.m[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
        r.m[11] = -1.f;
        r.m[14] = -(2.f * farPlane * nearPlane) / (farPlane - nearPlane);
        return r;
    }

    static Mat4 ortho(float left, float right, float bottom, float top,
                      float nearPlane, float farPlane) {
        Mat4 r;
        r.m[0]  =  2.f / (right - left);
        r.m[5]  =  2.f / (top - bottom);
        r.m[10] = -2.f / (farPlane - nearPlane);
        r.m[12] = -(right + left) / (right - left);
        r.m[13] = -(top + bottom) / (top - bottom);
        r.m[14] = -(farPlane + nearPlane) / (farPlane - nearPlane);
        r.m[15] = 1.f;
        return r;
    }

    static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
        Vec3 f = (target - eye).normalized();
        Vec3 r = f.cross(up).normalized();
        Vec3 u = r.cross(f);
        Mat4 res = identity();
        res.m[0] = r.x;  res.m[4] = r.y;  res.m[8]  = r.z;
        res.m[1] = u.x;  res.m[5] = u.y;  res.m[9]  = u.z;
        res.m[2] = -f.x; res.m[6] = -f.y; res.m[10] = -f.z;
        res.m[12] = -r.dot(eye);
        res.m[13] = -u.dot(eye);
        res.m[14] =  f.dot(eye);
        return res;
    }

    Mat4 operator*(const Mat4& o) const {
        Mat4 r;
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                float sum = 0.f;
                for (int k = 0; k < 4; ++k)
                    sum += m[k * 4 + row] * o.m[col * 4 + k];
                r.m[col * 4 + row] = sum;
            }
        }
        return r;
    }

    Vec4 operator*(const Vec4& v) const {
        return {
            m[0] * v.x + m[4] * v.y + m[8]  * v.z + m[12] * v.w,
            m[1] * v.x + m[5] * v.y + m[9]  * v.z + m[13] * v.w,
            m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w,
            m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w
        };
    }

    Vec3 transformPoint(const Vec3& p) const {
        Vec4 r = *this * Vec4{p.x, p.y, p.z, 1.f};
        return {r.x, r.y, r.z};
    }

    Vec3 transformDir(const Vec3& d) const {
        Vec4 r = *this * Vec4{d.x, d.y, d.z, 0.f};
        return {r.x, r.y, r.z};
    }

    float& at(int row, int col) { return m[col * 4 + row]; }
    float  at(int row, int col) const { return m[col * 4 + row]; }
};

// ── Transform ────────────────────────────────────────────────────

struct Transform {
    Vec3 position{0.f, 0.f, 0.f};
    Quat rotation = Quat::identity();
    Vec3 scale{1.f, 1.f, 1.f};

    Mat4 toMatrix() const {
        return Mat4::translation(position)
             * Mat4::fromQuat(rotation)
             * Mat4::scaling(scale);
    }

    Vec3 forward() const { return rotation.rotate({0.f, 0.f, -1.f}); }
    Vec3 right()   const { return rotation.rotate({1.f, 0.f,  0.f}); }
    Vec3 up()      const { return rotation.rotate({0.f, 1.f,  0.f}); }
};

// ── StringID (hashed string identifier) ──────────────────────────

class StringID {
public:
    constexpr StringID() = default;
    explicit constexpr StringID(uint32_t hash) : m_hash(hash) {}

    static constexpr uint32_t fnv1a(const char* str, uint32_t hash = 2166136261u) {
        return (str[0] == '\0') ? hash : fnv1a(str + 1, (hash ^ static_cast<uint32_t>(str[0])) * 16777619u);
    }

    explicit constexpr StringID(const char* str) : m_hash(fnv1a(str)) {}
    explicit StringID(std::string_view str) : m_hash(computeHash(str)) {}

    constexpr uint32_t hash() const { return m_hash; }
    constexpr bool isValid() const { return m_hash != 0; }

    constexpr bool operator==(const StringID& o) const { return m_hash == o.m_hash; }
    constexpr bool operator!=(const StringID& o) const { return m_hash != o.m_hash; }
    constexpr bool operator<(const StringID& o) const { return m_hash < o.m_hash; }

private:
    uint32_t m_hash = 0;

    static uint32_t computeHash(std::string_view str) {
        uint32_t hash = 2166136261u;
        for (char c : str)
            hash = (hash ^ static_cast<uint32_t>(c)) * 16777619u;
        return hash;
    }
};

} // namespace NF

template<>
struct std::hash<NF::StringID> {
    size_t operator()(const NF::StringID& id) const noexcept {
        return static_cast<size_t>(id.hash());
    }
};

namespace NF {

// ── Timer ────────────────────────────────────────────────────────

class Timer {
public:
    using Clock = std::chrono::high_resolution_clock;

    void reset() { m_start = Clock::now(); m_last = m_start; }

    float elapsed() const {
        return std::chrono::duration<float>(Clock::now() - m_start).count();
    }

    float delta() {
        auto now = Clock::now();
        float dt = std::chrono::duration<float>(now - m_last).count();
        m_last = now;
        return dt;
    }

private:
    Clock::time_point m_start = Clock::now();
    Clock::time_point m_last  = Clock::now();
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

    void clear() { m_handlers.clear(); }

    void clear(EventID id) {
        auto it = m_handlers.find(id);
        if (it != m_handlers.end()) it->second.clear();
    }

    size_t subscriberCount(EventID id) const {
        auto it = m_handlers.find(id);
        return (it != m_handlers.end()) ? it->second.size() : 0;
    }

private:
    std::unordered_map<EventID, std::vector<Handler>> m_handlers;
};

// ── Type ID (compile-time type identification) ───────────────────

class TypeID {
public:
    template<typename T>
    static uint32_t get() {
        static const uint32_t id = s_nextID++;
        return id;
    }
private:
    static inline uint32_t s_nextID = 1;
};

// ── Version ──────────────────────────────────────────────────────

constexpr int NF_VERSION_MAJOR = 0;
constexpr int NF_VERSION_MINOR = 1;
constexpr int NF_VERSION_PATCH = 0;
constexpr const char* NF_VERSION_STRING = "0.1.0";

// ── Core lifecycle ───────────────────────────────────────────────

void coreInit();
void coreShutdown();
bool isCoreInitialized();

} // namespace NF
