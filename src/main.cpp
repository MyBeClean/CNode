#include "core/Logger.h"
#include "nodes/FloatNodes.h"
#include "scene/Scene.h"

#include <iostream>

int main() {
    using namespace cnode;

    Logger::info("CNode core starting.");
    Logger::info("C++20 build is running.");

    Scene scene;
    const EntityId playerId = scene.createEntity("Player");
    if (const Entity* player = scene.findEntity(playerId)) {
        std::cout << "Scene demo: entity '" << player->name
                  << "' at (" << player->transform.position.x << ", "
                  << player->transform.position.y << ", "
                  << player->transform.position.z << ")\n";
    }

    FloatConstant constant(1, 2.5f);
    std::string error;
    if (constant.evaluate(error)) {
        const Value value = constant.output(constant.valuePort());
        if (const auto* number = std::get_if<float>(&value)) {
            std::cout << "Node demo: Float Constant = " << *number << '\n';
        }
    } else {
        Logger::error(error);
        return 1;
    }

    Logger::info("CNode core demo finished.");
    return 0;
}
