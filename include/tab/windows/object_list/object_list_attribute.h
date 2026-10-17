#include "registration/registration_data.h"
#include "tab/windows/object_list/object_list.h"

namespace CNode
{
    namespace
    {
        const bool objectListAttributeRegistered =
            RegisterWindowAttribute(
                "object_list",
                "Object List",
                &DrawObjectList,
                ImGuiDir_None
            );
    }
}