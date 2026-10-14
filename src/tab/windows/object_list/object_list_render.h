#pragma once

#include "imgui.h"

#include "object_list_data.h"
#include "object_list_creation.h"
#include "object_list_selection.h"
#include "object_list_rename.h"

namespace CNode
{
    inline void DrawObjectListContents()
    {
        if (ImGui::Button("+", ImVec2(28.0f, 0.0f)))
        {
            CreateSceneObject();
        }

        ImGui::SameLine();
        ImGui::TextUnformatted("objects");

        ImGui::Separator();

        // Shortcut F2 untuk rename objek aktif.
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)
            && ImGui::IsKeyPressed(ImGuiKey_F2)
            && !isRenamingObject
            && selectedObjectId != 0)
        {
            BeginRenameSceneObject(selectedObjectId);
        }

        for (auto& object : sceneObjects)
        {
            ImGui::PushID(static_cast<int>(object.id));

            const bool selected = object.id == selectedObjectId;
            const bool editing =
                isRenamingObject && object.id == renamingObjectId;

            if (editing)
            {
                if (focusObjectRename)
                {
                    ImGui::SetKeyboardFocusHere();
                    focusObjectRename = false;
                }

                const bool submitted = ImGui::InputText(
                    "##ObjectName",
                    objectRenameBuffer,
                    sizeof(objectRenameBuffer),
                    ImGuiInputTextFlags_EnterReturnsTrue |
                    ImGuiInputTextFlags_AutoSelectAll
                );

                // Tampilkan nama yang sedang diketik secara langsung.
                if (objectRenameBuffer[0] != '\0')
                    object.name = objectRenameBuffer;

                if (submitted)
                {
                    CommitRenameSceneObject();
                }
                else if (ImGui::IsKeyPressed(ImGuiKey_Escape))
                {
                    CancelRenameSceneObject();
                }
                else if (!ImGui::IsItemActive() &&
                    ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    CommitRenameSceneObject();
                }
            }
            else
            {
                if (ImGui::Selectable(
                    object.name.c_str(),
                    selected,
                    ImGuiSelectableFlags_AllowDoubleClick))
                {
                    SelectSceneObject(object.id);

                    if (ImGui::IsMouseDoubleClicked(
                        ImGuiMouseButton_Left))
                    {
                        BeginRenameSceneObject(object.id);
                    }
                }
            }

            ImGui::PopID();
        }
    }
}