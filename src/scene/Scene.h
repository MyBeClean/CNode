#pragma once

#include "scene/Transform.h"

#include <cstdint>
#include <string>
#include <vector>

namespace cnode {

using EntityId = std::uint64_t;

struct Entity {
    EntityId id{};
    std::string name;
    Transform transform{};
};

class Scene {
public:
    EntityId createEntity(std::string name);
    bool destroyEntity(EntityId id);
    Entity* findEntity(EntityId id) noexcept;
    const Entity* findEntity(EntityId id) const noexcept;
    const std::vector<Entity>& entities() const noexcept { return m_entities; }

private:
    EntityId m_nextId{1};
    std::vector<Entity> m_entities;
};

} // namespace cnode
