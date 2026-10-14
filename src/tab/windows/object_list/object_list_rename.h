#pragma once

#include <cstring>

#include "object_list_data.h"

namespace CNode
{
    inline void BeginRenameSceneObject(std::uint64_t id)
    {
        SceneObject* object = FindSceneObject(id);

        if (!object)
            return;

        selectedObjectId = id;
        renamingObjectId = id;
        originalObjectName = object->name;

        std::strncpy(
            objectRenameBuffer,
            object->name.c_str(),
            sizeof(objectRenameBuffer) - 1
        );

        objectRenameBuffer[sizeof(objectRenameBuffer) - 1] = '\0';

        isRenamingObject = true;
        focusObjectRename = true;
    }

    inline void CommitRenameSceneObject()
    {
        SceneObject* object = FindSceneObject(renamingObjectId);

        if (object && objectRenameBuffer[0] != '\0')
            object->name = objectRenameBuffer;

        isRenamingObject = false;
        focusObjectRename = false;
        renamingObjectId = 0;
    }

    inline void CancelRenameSceneObject()
    {
        SceneObject* object = FindSceneObject(renamingObjectId);

        if (object)
            object->name = originalObjectName;

        isRenamingObject = false;
        focusObjectRename = false;
        renamingObjectId = 0;
    }
}