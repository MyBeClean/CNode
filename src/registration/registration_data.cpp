#include "registration/registration_data.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <string>
#include <unordered_set>
#include <vector>

namespace CNode
{
    namespace
    {
        std::vector<WindowType>& GetWindowTypeStorage()
        {
            static std::vector<WindowType> windows;
            return windows;
        }

        std::vector<WindowType>& GetWindowAttributeStorage()
        {
            static std::vector<WindowType> attributes;
            return attributes;
        }

        std::vector<WindowInstance>& GetInstanceStorage()
        {
            static std::vector<WindowInstance> instances;
            return instances;
        }

        std::unordered_set<std::string>& GetWindowTypeIDs()
        {
            static std::unordered_set<std::string> ids;
            return ids;
        }

        std::unordered_set<std::string>& GetWindowAttributeIDs()
        {
            static std::unordered_set<std::string> ids;
            return ids;
        }

        unsigned long long& GetNextInstanceID()
        {
            static unsigned long long id = 1;
            return id;
        }

        void InitializeRandomSeed()
        {
            static const bool initialized = []
                {
                    std::srand(
                        static_cast<unsigned int>(
                            std::time(nullptr)
                            )
                    );

                    return true;
                }();

            (void)initialized;
        }
    }

    bool RegisterWindowType(
        const char* id,
        const char* name,
        WindowDrawFunction draw,
        ImGuiDir defaultDockDirection)
    {
        if (!id || !name || !draw || *id == '\0' || *name == '\0')
            return false;

        auto& ids = GetWindowTypeIDs();

        if (!ids.insert(id).second)
            return false;

        GetWindowTypeStorage().push_back({
            id,
            name,
            draw,
            defaultDockDirection
            });

        return true;
    }

    bool RegisterWindowAttribute(
        const char* id,
        const char* name,
        WindowDrawFunction draw,
        ImGuiDir defaultDockDirection)
    {
        if (!id || !name || !draw || *id == '\0' || *name == '\0')
            return false;

        auto& ids = GetWindowAttributeIDs();

        if (!ids.insert(id).second)
            return false;

        GetWindowAttributeStorage().push_back({
            id,
            name,
            draw,
            defaultDockDirection
            });

        return true;
    }

    void RegisterWindowAttributes()
    {
        static bool registered = false;

        if (registered)
            return;

        registered = true;

        const auto& attributes = GetWindowAttributeStorage();

        for (const WindowType& attribute : attributes)
        {
            RegisterWindowType(
                attribute.id.c_str(),
                attribute.name.c_str(),
                attribute.draw,
                attribute.defaultDockDirection
            );
        }
    }

    WindowInstance* AddWindow(const char* typeID)
    {
        if (!typeID)
            return nullptr;

        auto& types = GetWindowTypeStorage();

        auto type = std::find_if(
            types.begin(),
            types.end(),
            [typeID](const WindowType& item)
            {
                return item.id == typeID;
            }
        );

        if (type == types.end())
            return nullptr;

        InitializeRandomSeed();

        const unsigned long long id = GetNextInstanceID()++;

        std::string uniqueTitle =
            type->name +
            "###" +
            type->id +
            "_" +
            std::to_string(id);

        auto& instances = GetInstanceStorage();

        instances.push_back({
            id,
            type->id,
            uniqueTitle,
            true
            });

        return &instances.back();
    }

    const std::vector<WindowType>& GetRegisteredWindows()
    {
        return GetWindowTypeStorage();
    }

    std::vector<WindowInstance>& GetWindowInstances()
    {
        return GetInstanceStorage();
    }

    void Windows()
    {
        RegisterWindowAttributes();
    }
}