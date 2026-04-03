#pragma once
// NF::Renderer — RHI (OpenGL), forward pipeline, mesh, materials
#include "NF/Core/Core.h"

namespace NF {

// ── Vertex Data ──────────────────────────────────────────────────

struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec2 texCoord;
    Vec4 color{1.f, 1.f, 1.f, 1.f};
};

// ── Mesh ─────────────────────────────────────────────────────────

class Mesh {
public:
    Mesh() = default;
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
        : m_vertices(std::move(vertices)), m_indices(std::move(indices)) {}

    void setVertices(std::vector<Vertex> verts) { m_vertices = std::move(verts); m_dirty = true; }
    void setIndices(std::vector<uint32_t> idx) { m_indices = std::move(idx); m_dirty = true; }

    [[nodiscard]] const std::vector<Vertex>& vertices() const { return m_vertices; }
    [[nodiscard]] const std::vector<uint32_t>& indices() const { return m_indices; }
    [[nodiscard]] size_t vertexCount() const { return m_vertices.size(); }
    [[nodiscard]] size_t indexCount() const { return m_indices.size(); }
    [[nodiscard]] size_t triangleCount() const { return m_indices.size() / 3; }
    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void clearDirty() { m_dirty = false; }

    // Generate AABB from vertices
    [[nodiscard]] std::pair<Vec3, Vec3> bounds() const {
        if (m_vertices.empty()) return {{}, {}};
        Vec3 lo = m_vertices[0].position;
        Vec3 hi = m_vertices[0].position;
        for (auto& v : m_vertices) {
            lo.x = std::min(lo.x, v.position.x);
            lo.y = std::min(lo.y, v.position.y);
            lo.z = std::min(lo.z, v.position.z);
            hi.x = std::max(hi.x, v.position.x);
            hi.y = std::max(hi.y, v.position.y);
            hi.z = std::max(hi.z, v.position.z);
        }
        return {lo, hi};
    }

private:
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    bool m_dirty = true;
};

// ── Shader ───────────────────────────────────────────────────────

enum class ShaderStage : uint8_t {
    Vertex,
    Fragment,
    Geometry,
    Compute
};

class Shader {
public:
    Shader() = default;
    Shader(StringID name, const std::string& vertSrc, const std::string& fragSrc)
        : m_name(name), m_vertexSource(vertSrc), m_fragmentSource(fragSrc) {}

    [[nodiscard]] StringID name() const { return m_name; }
    [[nodiscard]] const std::string& vertexSource() const { return m_vertexSource; }
    [[nodiscard]] const std::string& fragmentSource() const { return m_fragmentSource; }
    [[nodiscard]] bool isCompiled() const { return m_compiled; }

    bool compile() {
        // Placeholder: real implementation would compile via OpenGL API
        m_compiled = !m_vertexSource.empty() && !m_fragmentSource.empty();
        return m_compiled;
    }

    void setUniformFloat(const std::string& name, float value) { m_floatUniforms[name] = value; }
    void setUniformVec3(const std::string& name, const Vec3& value) { m_vec3Uniforms[name] = value; }
    void setUniformMat4(const std::string& name, const Mat4& value) { m_mat4Uniforms[name] = value; }

    [[nodiscard]] const std::unordered_map<std::string, float>& floatUniforms() const { return m_floatUniforms; }
    [[nodiscard]] const std::unordered_map<std::string, Vec3>& vec3Uniforms() const { return m_vec3Uniforms; }

private:
    StringID m_name;
    std::string m_vertexSource;
    std::string m_fragmentSource;
    bool m_compiled = false;
    std::unordered_map<std::string, float> m_floatUniforms;
    std::unordered_map<std::string, Vec3> m_vec3Uniforms;
    std::unordered_map<std::string, Mat4> m_mat4Uniforms;
};

// ── Material ─────────────────────────────────────────────────────

class Material {
public:
    Material() = default;
    explicit Material(StringID name) : m_name(name) {}

    void setShader(Shader* shader) { m_shader = shader; }
    void setColor(const Vec4& color) { m_color = color; }
    void setMetallic(float v) { m_metallic = v; }
    void setRoughness(float v) { m_roughness = v; }

    [[nodiscard]] StringID name() const { return m_name; }
    [[nodiscard]] Shader* shader() const { return m_shader; }
    [[nodiscard]] const Vec4& color() const { return m_color; }
    [[nodiscard]] float metallic() const { return m_metallic; }
    [[nodiscard]] float roughness() const { return m_roughness; }

private:
    StringID m_name;
    Shader* m_shader = nullptr;
    Vec4 m_color{1.f, 1.f, 1.f, 1.f};
    float m_metallic = 0.f;
    float m_roughness = 0.5f;
};

// ── Render Command ───────────────────────────────────────────────

struct RenderCommand {
    const Mesh* mesh = nullptr;
    const Material* material = nullptr;
    Mat4 transform = Mat4::identity();
    float sortKey = 0.f;
};

// ── Render Queue ─────────────────────────────────────────────────

class RenderQueue {
public:
    void submit(RenderCommand cmd) {
        m_commands.push_back(std::move(cmd));
    }

    void sort() {
        std::sort(m_commands.begin(), m_commands.end(),
                  [](const RenderCommand& a, const RenderCommand& b) {
                      return a.sortKey < b.sortKey;
                  });
    }

    void clear() { m_commands.clear(); }

    [[nodiscard]] const std::vector<RenderCommand>& commands() const { return m_commands; }
    [[nodiscard]] size_t size() const { return m_commands.size(); }
    [[nodiscard]] bool empty() const { return m_commands.empty(); }

private:
    std::vector<RenderCommand> m_commands;
};

// ── Camera ───────────────────────────────────────────────────────

struct Camera {
    Vec3 position{0.f, 0.f, 5.f};
    Vec3 target{0.f, 0.f, 0.f};
    Vec3 up{0.f, 1.f, 0.f};
    float fov = 60.f;
    float nearPlane = 0.1f;
    float farPlane = 1000.f;

    [[nodiscard]] Mat4 viewMatrix() const {
        return Mat4::lookAt(position, target, up);
    }

    [[nodiscard]] Mat4 projectionMatrix(float aspect) const {
        constexpr float DEG2RAD = 3.14159265358979f / 180.f;
        return Mat4::perspective(fov * DEG2RAD, aspect, nearPlane, farPlane);
    }

    [[nodiscard]] Vec3 forward() const { return (target - position).normalized(); }
};

// ── Renderer ─────────────────────────────────────────────────────

class Renderer {
public:
    bool init(int width, int height) {
        m_width = width;
        m_height = height;
        NF_LOG_INFO("Renderer", "Renderer initialized");
        return true;
    }

    void shutdown() {
        m_queue.clear();
        NF_LOG_INFO("Renderer", "Renderer shutdown");
    }

    void beginFrame() {
        m_queue.clear();
        m_drawCallCount = 0;
    }

    void submit(RenderCommand cmd) { m_queue.submit(std::move(cmd)); }

    void endFrame() {
        m_queue.sort();
        m_drawCallCount = m_queue.size();
        // Placeholder: real implementation would issue OpenGL draw calls
    }

    void setCamera(const Camera& cam) { m_camera = cam; }
    [[nodiscard]] const Camera& camera() const { return m_camera; }

    [[nodiscard]] int width() const { return m_width; }
    [[nodiscard]] int height() const { return m_height; }
    [[nodiscard]] float aspect() const {
        return m_height > 0 ? static_cast<float>(m_width) / static_cast<float>(m_height) : 1.f;
    }
    [[nodiscard]] size_t drawCallCount() const { return m_drawCallCount; }

private:
    int m_width = 0;
    int m_height = 0;
    Camera m_camera;
    RenderQueue m_queue;
    size_t m_drawCallCount = 0;
};

} // namespace NF
