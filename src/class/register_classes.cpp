#include "class/register_classes.h"

#include "class/class_registry.h"
#include "class/transform/transform.h"
#include "class/visual/visual.h"

namespace CNode
{
    bool RegisterAllClasses()
    {
        // Daftarkan class Transform.
        const bool transformRegistered =
            RegisterTransformClass();

        // Daftarkan class Visual.
        const bool visualRegistered =
            RegisterVisualClass();

        // Berhasil jika seluruh registrasi berhasil.
        return transformRegistered &&
            visualRegistered;
    }
}