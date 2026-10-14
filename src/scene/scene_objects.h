#pragma once

#include <cstdint>
#include <vector>

#include "node/3d_object/cube/cube.h"

namespace CNode
{
    enum class SceneObjectType
    {
        Cube
    };

    struct SceneObjectEntry
    {
        std::uint64_t id = 0;
        SceneObjectType type = SceneObjectType::Cube;
        CubeObject cube;
    };

    class SceneObjects
    {
    public:
        static SceneObjects& Get();

        SceneObjectEntry& AddCube(const CubeObject& cube);

        bool RemoveObject(std::uint64_t id);

        SceneObjectEntry* FindObject(std::uint64_t id);
        const SceneObjectEntry* FindObject(std::uint64_t id) const;

        std::vector<SceneObjectEntry>& GetAll();
        const std::vector<SceneObjectEntry>& GetAll() const;

        void Clear();

        // Mengganti seluruh objek dengan data hasil pemuatan scene.
        void ReplaceAll(
            const std::vector<SceneObjectEntry>& loadedObjects
        );

    private:
        SceneObjects() = default;

        std::vector<SceneObjectEntry> objects;
        std::uint64_t nextObjectID = 1;
    };
}