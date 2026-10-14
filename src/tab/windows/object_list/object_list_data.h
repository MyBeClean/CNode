
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace CNode
{
    // Data item untuk Object List.
    // Dibedakan dari SceneObject milik node_graph_data.h.
    struct ObjectListItem
    {
        std::uint64_t id = 0;
        std::string name = "NewObject";
    };

    inline std::vector<ObjectListItem> sceneObjects;

    inline std::uint64_t nextObjectId = 1;
    inline std::uint64_t selectedObjectId = 0;

    inline bool isRenamingObject = false;
    inline std::uint64_t renamingObjectId = 0;

    inline char objectRenameBuffer[256] = {};
    inline std::string originalObjectName;
    inline bool focusObjectRename = false;

    inline ObjectListItem* FindSceneObject(std::uint64_t id)
    {
        for (auto& object : sceneObjects)
        {
            if (object.id == id)
            {
                return &object;
            }
        }

        return nullptr;
    }

    inline ObjectListItem* GetSelectedSceneObject()
    {
        return FindSceneObject(selectedObjectId);
    }

    inline std::uint64_t GetSelectedObjectId()
    {
        return selectedObjectId;
    }
}
