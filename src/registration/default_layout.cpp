
#include "registration/default_layout.h"
#include "registration/registration_data.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <filesystem>
#include <fstream>

namespace CNode
{
    namespace
    {
        constexpr const char* kLayoutMarker =
            ".cnode_default_layout_v1";

        WindowInstance* FindWindow(const char* typeID)
        {
            auto& instances = GetWindowInstances();

            for (WindowInstance& instance : instances)
            {
                if (instance.typeID == typeID)
                    return &instance;
            }

            return nullptr;
        }

        void EnsureDefaultWindows()
        {
            constexpr const char* defaultWindows[] =
            {
                "scene_editor",
                "object_list",
                "node_graph",
                "inspector"
            };

            for (const char* typeID : defaultWindows)
            {
                if (!FindWindow(typeID))
                    AddWindow(typeID);
            }
        }

        void DockWindow(
            const char* typeID,
            ImGuiID dockID)
        {
            WindowInstance* instance = FindWindow(typeID);

            if (!instance)
                return;

            ImGui::DockBuilderDockWindow(
                instance->title.c_str(),
                dockID
            );
        }


        void BuildDefaultLayout(ImGuiID dockspaceID)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();

            ImGui::DockBuilderRemoveNode(dockspaceID);

            ImGui::DockBuilderAddNode(
                dockspaceID,
                ImGuiDockNodeFlags_DockSpace
            );

            ImGui::DockBuilderSetNodeSize(
                dockspaceID,
                viewport->WorkSize
            );

            ImGuiID dockRight = 0;
            ImGuiID dockLeft = 0;

            // area kanan untuk Inspector dan Object List.
            ImGui::DockBuilderSplitNode(
                dockspaceID,
                ImGuiDir_Right,
                0.28f,
                &dockRight,
                &dockLeft
            );

            ImGuiID dockBottom = 0;
            ImGuiID dockTop = 0;

            // node graph di bawah scene editor.
            ImGui::DockBuilderSplitNode(
                dockLeft,
                ImGuiDir_Down,
                0.35f,
                &dockBottom,
                &dockTop
            );

            // kiri atas.
            DockWindow("scene_editor", dockTop);

            // kiri bawah.
            DockWindow("node_graph", dockBottom);

            // dua window ini berbagi satu dock node,
            // sehingga Dear ImGui membuat tab otomatis.
            DockWindow("inspector", dockRight);
            DockWindow("object_list", dockRight);

            ImGui::DockBuilderFinish(dockspaceID);
        }

    }

    void SetupDefaultDockLayout()
    {
        static bool initialized = false;

        if (initialized)
            return;

        initialized = true;

        // Registry harus sudah dijalankan sebelum fungsi ini.
        EnsureDefaultWindows();

        // Bangun layout default hanya sekali untuk versi ini.
        // Perubahan layout berikutnya disimpan oleh Dear ImGui.
        if (std::filesystem::exists(kLayoutMarker))
            return;

        const ImGuiID dockspaceID =
            ImGui::GetID("CNodeDockspace");

        BuildDefaultLayout(dockspaceID);

        std::ofstream marker(kLayoutMarker);

        if (marker)
            marker << "1\n";
    }
}
