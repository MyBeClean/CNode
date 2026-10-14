
#include "environment_settings.h"

namespace CNode
{
    EnvironmentSettings& GetEnvironmentSettings()
    {
        static EnvironmentSettings settings;
        return settings;
    }
}
