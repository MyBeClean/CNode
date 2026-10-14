#pragma once

#include "object_list_data.h"

namespace CNode
{
    inline void SelectSceneObject(std::uint64_t id)
    {
        selectedObjectId = id;
    }
}