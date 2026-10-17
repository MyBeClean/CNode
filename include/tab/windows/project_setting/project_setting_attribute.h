#include "registration/registration_data.h"
#include "tab/windows/project_setting/project_setting.h"

namespace CNode
{
    namespace
    {
        const bool projectSettingAttributeRegistered =
            RegisterWindowAttribute(
                "project_setting",
                "Project Setting",
                &DrawProjectSetting,
                ImGuiDir_None
            );
    }
}