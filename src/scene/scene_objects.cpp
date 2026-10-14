#include "scene_objects.h"

#include <algorithm>
#include <limits>

namespace CNode
{
    SceneObjects& SceneObjects::Get()
    {
        static SceneObjects instance;
        return instance;
    }

    SceneObjectEntry& SceneObjects::AddCube(
        const CubeObject& cube
    )
    {
        SceneObjectEntry entry;
        entry.id = nextObjectID++;
        entry.type = SceneObjectType::Cube;
        entry.cube = cube;

        objects.push_back(entry);
        return objects.back();
    }

    bool SceneObjects::RemoveObject(std::uint64_t id)
    {
        const auto it = std::find_if(
            objects.begin(),
            objects.end(),
            [id](const SceneObjectEntry& object)
            {
                return object.id == id;
            }
        );

        if (it == objects.end())
            return false;

        objects.erase(it);
        return true;
    }

    SceneObjectEntry* SceneObjects::FindObject(std::uint64_t id)
    {
        for (auto& object : objects)
        {
            if (object.id == id)
                return &object;
        }

        return nullptr;
    }

    const SceneObjectEntry* SceneObjects::FindObject(
        std::uint64_t id
    ) const
    {
        for (const auto& object : objects)
        {
            if (object.id == id)
                return &object;
        }

        return nullptr;
    }

    std::vector<SceneObjectEntry>& SceneObjects::GetAll()
    {
        return objects;
    }

    const std::vector<SceneObjectEntry>& SceneObjects::GetAll() const
    {
        return objects;
    }

    void SceneObjects::Clear()
    {
        objects.clear();
        nextObjectID = 1;
    }

    void SceneObjects::ReplaceAll(
        const std::vector<SceneObjectEntry>& loadedObjects
    )
    {
        objects = loadedObjects;

        std::uint64_t maxID = 0;

        for (const auto& object : objects)
        {
            maxID = std::max(maxID, object.id);
        }

        if (maxID == std::numeric_limits<std::uint64_t>::max())
            nextObjectID = maxID;
        else
            nextObjectID = maxID + 1;

        if (nextObjectID == 0)
            nextObjectID = 1;
    }
}