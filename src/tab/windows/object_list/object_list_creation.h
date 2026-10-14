#pragma once

#include <cstring>

#include "object_list_data.h"

namespace CNode
{
    inline void CreateSceneObject()
    {
        SceneObject object;
        object.id = nextObjectId++;
        object.name = "NewObject";

        selectedObjectId = object.id;
        renamingObjectId = object.id;
        originalObjectName = object.name;

        std::strncpy(
            objectRenameBuffer,
            object.name.c_str(),
            sizeof(objectRenameBuffer) - 1
        );

        objectRenameBuffer[sizeof(objectRenameBuffer) - 1] = '\0';

        sceneObjects.push_back(object);

        isRenamingObject = true;
        focusObjectRename = true;
    }
}