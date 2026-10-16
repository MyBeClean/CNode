#pragma once

#include "imgui.h"

namespace CNode
{
    class EditorFont
    {
    public:
        bool Load();
        void Apply() const;
        void Reset();

    private:
        ImFont* font = nullptr;
    };
}