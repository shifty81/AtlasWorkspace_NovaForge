#pragma once
// NF::Engine — ECS, world/level, behavior trees, asset system
#include "NF/Core/Core.h"
#include <any>
#include <typeindex>
#include <optional>
#include <set>

namespace NF {

// ── Entity IDs ───────────────────────────────────────────────────

using EntityID = uint32_t;
constexpr EntityID INVALID_ENTITY = 0;

// ── Entity Manager ───────────────────────────────────────────────

class EntityManager {
public:
    EntityID createEntity() {
        EntityID id;
        if (!m_freeList.empty()) {
            id = m_freeList.back();
            m_freeList.pop_back();
        } else {
            id = m_nextID++;
        }
        m_alive.push_back(id);
        return id;
    }

    void destroyEntity(EntityID id) {
        auto it = std::find(m_alive.begin(), m_alive.end(), id);
        if (it != m_alive.end()) {
            m_alive.erase(it);
            m_freeList.push_back(id);
        }
    }

    [[nodiscard]] bool isAlive(EntityID id) const {
        return std::find(m_alive.begin(), m_alive.end(), id) != m_alive.end();
    }

    [[nodiscard]] const std::vector<EntityID>& alive() const { return m_alive; }
    [[nodiscard]] size_t count() const { return m_alive.size(); }

    void clear() {
        m_alive.clear();
        m_freeList.clear();
        m_nextID = 1;
    }

private:
    EntityID m_nextID = 1;
    std::vector<EntityID> m_alive;
    std::vector<EntityID> m_freeList;
};

// ── Component Store ──────────────────────────────────────────────

class ComponentStore {
public:
    template<typename T>
    void add(EntityID entity, T component) {
        auto& pool = getPool<T>();
        pool[entity] = std::move(component);
    }

    template<typename T>
    void remove(EntityID entity) {
        auto& pool = getPool<T>();
        pool.erase(entity);
    }

    template<typename T>
    T* get(EntityID entity) {
        auto& pool = getPool<T>();
        auto it = pool.find(entity);
        return (it != pool.end()) ? &it->second : nullptr;
    }

    template<typename T>
    const T* get(EntityID entity) const {
        auto poolIt = m_pools.find(std::type_index(typeid(T)));
        if (poolIt == m_pools.end()) return nullptr;
        auto& pool = std::any_cast<const std::unordered_map<EntityID, T>&>(poolIt->second);
        auto it = pool.find(entity);
        return (it != pool.end()) ? &it->second : nullptr;
    }

    template<typename T>
    bool has(EntityID entity) const {
        auto poolIt = m_pools.find(std::type_index(typeid(T)));
        if (poolIt == m_pools.end()) return false;
        auto& pool = std::any_cast<const std::unordered_map<EntityID, T>&>(poolIt->second);
        return pool.count(entity) > 0;
    }

    template<typename T>
    std::unordered_map<EntityID, T>& getAll() {
        return getPool<T>();
    }

    void removeAll(EntityID entity) {
        for (auto& [typeIdx, pool] : m_pools) {
            auto& erasers = getErasers();
            auto it = erasers.find(typeIdx);
            if (it != erasers.end()) {
                it->second(pool, entity);
            }
        }
    }

private:
    using EraserFn = std::function<void(std::any&, EntityID)>;

    std::unordered_map<std::type_index, std::any> m_pools;

    static std::unordered_map<std::type_index, EraserFn>& getErasers() {
        static std::unordered_map<std::type_index, EraserFn> erasers;
        return erasers;
    }

    template<typename T>
    std::unordered_map<EntityID, T>& getPool() {
        auto key = std::type_index(typeid(T));
        auto it = m_pools.find(key);
        if (it == m_pools.end()) {
            m_pools[key] = std::unordered_map<EntityID, T>{};
            getErasers()[key] = [](std::any& pool, EntityID e) {
                auto& typed = std::any_cast<std::unordered_map<EntityID, T>&>(pool);
                typed.erase(e);
            };
        }
        return std::any_cast<std::unordered_map<EntityID, T>&>(m_pools[key]);
    }
};

// ── System Base ──────────────────────────────────────────────────

class SystemBase {
public:
    virtual ~SystemBase() = default;

    virtual void init(EntityManager& em, ComponentStore& cs) {
        m_entities = &em;
        m_components = &cs;
    }
    virtual void update(float dt) = 0;
    virtual void shutdown() {}

    [[nodiscard]] virtual std::string_view name() const = 0;

protected:
    EntityManager* m_entities = nullptr;
    ComponentStore* m_components = nullptr;
};

// ── World / Level ────────────────────────────────────────────────

class Level {
public:
    explicit Level(std::string name) : m_name(std::move(name)) {}

    EntityID spawnEntity() { return m_entities.createEntity(); }
    void despawnEntity(EntityID e) {
        m_components.removeAll(e);
        m_entities.destroyEntity(e);
    }

    EntityManager&  entities()   { return m_entities; }
    ComponentStore& components() { return m_components; }

    const std::string& name() const { return m_name; }

    template<typename T>
    void addComponent(EntityID e, T comp) { m_components.add(e, std::move(comp)); }

    template<typename T>
    T* getComponent(EntityID e) { return m_components.get<T>(e); }

private:
    std::string m_name;
    EntityManager m_entities;
    ComponentStore m_components;
};

// ── Behavior Tree ────────────────────────────────────────────────

enum class BTStatus : uint8_t {
    Running,
    Success,
    Failure
};

class BTNode {
public:
    virtual ~BTNode() = default;
    virtual BTStatus tick(float dt) = 0;
    virtual void reset() {}
};

class BTAction : public BTNode {
public:
    using ActionFn = std::function<BTStatus(float)>;

    explicit BTAction(ActionFn fn) : m_fn(std::move(fn)) {}
    BTStatus tick(float dt) override { return m_fn(dt); }

private:
    ActionFn m_fn;
};

class BTSequence : public BTNode {
public:
    void addChild(std::unique_ptr<BTNode> child) {
        m_children.push_back(std::move(child));
    }

    BTStatus tick(float dt) override {
        for (size_t i = m_current; i < m_children.size(); ++i) {
            auto status = m_children[i]->tick(dt);
            if (status == BTStatus::Running) {
                m_current = i;
                return BTStatus::Running;
            }
            if (status == BTStatus::Failure) {
                reset();
                return BTStatus::Failure;
            }
        }
        reset();
        return BTStatus::Success;
    }

    void reset() override {
        m_current = 0;
        for (auto& c : m_children) c->reset();
    }

private:
    std::vector<std::unique_ptr<BTNode>> m_children;
    size_t m_current = 0;
};

class BTSelector : public BTNode {
public:
    void addChild(std::unique_ptr<BTNode> child) {
        m_children.push_back(std::move(child));
    }

    BTStatus tick(float dt) override {
        for (size_t i = m_current; i < m_children.size(); ++i) {
            auto status = m_children[i]->tick(dt);
            if (status == BTStatus::Running) {
                m_current = i;
                return BTStatus::Running;
            }
            if (status == BTStatus::Success) {
                reset();
                return BTStatus::Success;
            }
        }
        reset();
        return BTStatus::Failure;
    }

    void reset() override {
        m_current = 0;
        for (auto& c : m_children) c->reset();
    }

private:
    std::vector<std::unique_ptr<BTNode>> m_children;
    size_t m_current = 0;
};

// ── Asset Handle ─────────────────────────────────────────────────

enum class AssetState : uint8_t {
    Unloaded,
    Loading,
    Loaded,
    Failed
};

template<typename T>
class AssetHandle {
public:
    AssetHandle() = default;
    explicit AssetHandle(StringID id) : m_id(id) {}

    [[nodiscard]] StringID id() const { return m_id; }
    [[nodiscard]] AssetState state() const { return m_state; }
    [[nodiscard]] bool isLoaded() const { return m_state == AssetState::Loaded; }

    T* get() { return m_data.get(); }
    const T* get() const { return m_data.get(); }

    void setData(std::unique_ptr<T> data) {
        m_data = std::move(data);
        m_state = m_data ? AssetState::Loaded : AssetState::Failed;
    }

    void setState(AssetState state) { m_state = state; }

private:
    StringID m_id;
    AssetState m_state = AssetState::Unloaded;
    std::unique_ptr<T> m_data;
};

} // namespace NF
