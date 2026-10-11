#include "scene/Scene.h"

#include <algorithm>
#include <utility>

namespace cnode {

EntityId Scene::createEntity(std::string name) {
    const EntityId id = m_nextId++;
    m_entities.push_back(Entity{id, std::move(name), {}});
    return id;
}

bool Scene::destroyEntity(EntityId id) {
    const auto oldSize = m_entities.size();
    std::erase_if(m_entities, [id](const Entity& entity) { return entity.id == id; });
    return m_entities.size() != oldSize;
}

Entity* Scene::findEntity(EntityId id) noexcept {
    for (auto& entity : m_entities) if (entity.id == id) return &entity;
    return nullptr;
}

const Entity* Scene::findEntity(EntityId id) const noexcept {
    for (const auto& entity : m_entities) if (entity.id == id) return &entity;
    return nullptr;
}

} // namespace cnode
