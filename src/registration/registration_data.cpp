
#include "registration/registration_data.h"
#include "tab/windows/scene_editor/scene_editor.h"

#include <string>
#include <unordered_set>

namespace CNode
{
    namespace
    {
        std::vector<WindowAttribute>& GetWindowStorage()
        {
            static std::vector<WindowAttribute> windows;
            return windows;
        }

        std::unordered_set<std::string>& GetWindowIDs()
        {
            static std::unordered_set<std::string> ids;
            return ids;
        }
    }

    bool RegisterWindow(const WindowAttribute& attribute)
    {
        if (!attribute.id || !attribute.name || !attribute.draw)
            return false;

        if (*attribute.id == '\0' || *attribute.name == '\0')
            return false;

        auto& ids = GetWindowIDs();

        if (!ids.insert(attribute.id).second)
            return false;

        WindowAttribute registered = attribute;
        registered.open = registered.defaultOpen;

        GetWindowStorage().push_back(registered);
        return true;
    }

    std::vector<WindowAttribute>& GetRegisteredWindows()
    {
        return GetWindowStorage();
    }

    void Windows()
    {
        RegisterWindow({
            "scene_editor",
            "Scene Editor",
            false,
            DrawSceneEditor
            });
    }
}
