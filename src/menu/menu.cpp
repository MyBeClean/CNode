
#include "menu.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <cstring>

#include <Windows.h>
#include <shobjidl.h>
#include <objbase.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "node/3d_object/cube/cube_render.h"

namespace fs = std::filesystem;

namespace CNode
{
    namespace
    {
        struct ProjectInfo
        {
            fs::path root;
            std::string name;
            std::string version = "1.0";
        };

        ProjectInfo g_project;

        bool g_hasProject = false;
        bool g_showCreate = false;
        bool g_showDelete = false;
        bool g_showSettings = false;

        char g_projectName[128] = "MyGame";
        char g_projectLocation[1024] = "";
        char g_projectVersion[64] = "1.0";

        fs::path g_deletePath;
        std::string g_status = "No project open.";

        bool IsValidProject(const fs::path& path)
        {
            return fs::exists(path / "project.cnode");
        }

        bool WriteProjectFile(
            const fs::path& root,
            const std::string& name,
            const std::string& version)
        {
            std::ofstream file(root / "project.cnode");

            if (!file.is_open())
                return false;

            file << "name=" << name << '\n';
            file << "version=" << version << '\n';
            file << "scene=Scenes/Main.cns\n";

            return file.good();
        }

        bool ReadProjectFile(
            const fs::path& root,
            ProjectInfo& project)
        {
            std::ifstream file(root / "project.cnode");

            if (!file.is_open())
                return false;

            project.root = root;
            project.name.clear();
            project.version = "1.0";

            std::string line;

            while (std::getline(file, line))
            {
                const size_t separator = line.find('=');

                if (separator == std::string::npos)
                    continue;

                const std::string key = line.substr(0, separator);
                const std::string value = line.substr(separator + 1);

                if (key == "name")
                    project.name = value;
                else if (key == "version")
                    project.version = value;
            }

            return !project.name.empty();
        }

        bool ChooseFolder(fs::path& selectedPath)
        {
            HRESULT initResult = CoInitializeEx(
                nullptr,
                COINIT_APARTMENTTHREADED);

            const bool shouldUninitialize = SUCCEEDED(initResult);

            if (FAILED(initResult) &&
                initResult != RPC_E_CHANGED_MODE)
            {
                return false;
            }

            IFileDialog* dialog = nullptr;

            HRESULT result = CoCreateInstance(
                CLSID_FileOpenDialog,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(&dialog));

            if (FAILED(result) || dialog == nullptr)
            {
                if (shouldUninitialize)
                    CoUninitialize();

                return false;
            }

            DWORD options = 0;

            if (SUCCEEDED(dialog->GetOptions(&options)))
            {
                dialog->SetOptions(
                    options |
                    FOS_PICKFOLDERS |
                    FOS_FORCEFILESYSTEM);
            }

            result = dialog->Show(nullptr);

            if (FAILED(result))
            {
                dialog->Release();

                if (shouldUninitialize)
                    CoUninitialize();

                return false;
            }

            IShellItem* item = nullptr;
            result = dialog->GetResult(&item);

            if (FAILED(result) || item == nullptr)
            {
                dialog->Release();

                if (shouldUninitialize)
                    CoUninitialize();

                return false;
            }

            PWSTR path = nullptr;

            result = item->GetDisplayName(
                SIGDN_FILESYSPATH,
                &path);

            bool success = false;

            if (SUCCEEDED(result) && path != nullptr)
            {
                selectedPath = path;
                CoTaskMemFree(path);
                success = true;
            }

            item->Release();
            dialog->Release();

            if (shouldUninitialize)
                CoUninitialize();

            return success;
        }

        void ActivateProject(const ProjectInfo& project)
        {
            g_project = project;
            g_hasProject = true;
            g_status = "Opened project: " + project.name;
        }

        void CreateProject()
        {
            const std::string name = g_projectName;
            const std::string version = g_projectVersion;

            if (name.empty())
            {
                g_status = "Project name cannot be empty.";
                return;
            }

            if (g_projectLocation[0] == '\0')
            {
                g_status = "Choose a project location first.";
                return;
            }

            const fs::path parent = g_projectLocation;
            const fs::path root = parent / name;

            try
            {
                if (fs::exists(root))
                {
                    g_status = "The project folder already exists.";
                    return;
                }

                fs::create_directories(root / "Scenes");
                fs::create_directories(root / "Assets");
                fs::create_directories(root / "Scripts");

                if (!WriteProjectFile(root, name, version))
                {
                    g_status = "Failed to write project.cnode.";
                    return;
                }

                std::ofstream sceneFile(root / "Scenes" / "Main.cns");

                if (!sceneFile.is_open())
                {
                    g_status = "Failed to create Main.cns.";
                    return;
                }

                sceneFile << "# CNode Scene\n";
                sceneFile << "version=1\n";
                sceneFile << "name=Main\n";
                sceneFile.close();

                ProjectInfo project;
                project.root = root;
                project.name = name;
                project.version = version;

                ActivateProject(project);
                g_showCreate = false;
            }
            catch (const fs::filesystem_error& error)
            {
                g_status = std::string("Create project failed: ") +
                    error.what();
            }
        }

        void OpenProject()
        {
            fs::path selectedPath;

            if (!ChooseFolder(selectedPath))
            {
                g_status = "Folder selection cancelled or failed.";
                return;
            }

            ProjectInfo project;

            if (!ReadProjectFile(selectedPath, project))
            {
                g_status =
                    "Invalid project. Select a folder containing project.cnode.";
                return;
            }

            ActivateProject(project);
        }

        void DrawCreateProjectDialog()
        {
            if (g_showCreate)
                ImGui::OpenPopup("Create Project");

            if (ImGui::BeginPopupModal(
                "Create Project",
                nullptr,
                ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::InputText(
                    "Project Name",
                    g_projectName,
                    sizeof(g_projectName));

                ImGui::InputText(
                    "Version",
                    g_projectVersion,
                    sizeof(g_projectVersion));

                ImGui::TextWrapped(
                    "Project Location: %s",
                    g_projectLocation[0]
                    ? g_projectLocation
                    : "(not selected)");

                if (ImGui::Button("Browse...", ImVec2(120, 0)))
                {
                    fs::path folder;

                    if (ChooseFolder(folder))
                    {
                        const std::string path = folder.string();

                        strncpy_s(
                            g_projectLocation,
                            sizeof(g_projectLocation),
                            path.c_str(),
                            _TRUNCATE);
                    }
                }

                ImGui::Spacing();

                if (ImGui::Button("Create", ImVec2(120, 0)))
                    CreateProject();

                ImGui::SameLine();

                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                {
                    g_showCreate = false;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
        }

        void DrawDeleteProjectDialog()
        {
            if (g_showDelete)
                ImGui::OpenPopup("Delete Project");

            if (ImGui::BeginPopupModal(
                "Delete Project",
                nullptr,
                ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::TextWrapped(
                    "Choose a project folder to delete. "
                    "This permanently removes that folder.");

                if (ImGui::Button("Choose Project Folder..."))
                {
                    fs::path folder;

                    if (ChooseFolder(folder))
                    {
                        if (IsValidProject(folder))
                        {
                            g_deletePath = folder;
                            g_status = "Selected: " + folder.string();
                        }
                        else
                        {
                            g_status =
                                "That folder is not a valid CNode project.";
                        }
                    }
                }

                ImGui::TextWrapped(
                    "%s",
                    g_deletePath.empty()
                    ? "No project selected."
                    : g_deletePath.string().c_str());

                ImGui::Spacing();

                if (ImGui::Button("Delete Permanently"))
                {
                    try
                    {
                        if (g_deletePath.empty() ||
                            !IsValidProject(g_deletePath))
                        {
                            g_status = "Select a valid project first.";
                        }
                        else if (
                            g_hasProject &&
                            fs::absolute(g_project.root).lexically_normal() ==
                            fs::absolute(g_deletePath).lexically_normal())
                        {
                            g_status =
                                "Close the active project before deleting it.";
                        }
                        else
                        {
                            fs::remove_all(g_deletePath);
                            g_status = "Project deleted.";
                            g_deletePath.clear();
                            g_showDelete = false;
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    catch (const fs::filesystem_error& error)
                    {
                        g_status = std::string("Delete failed: ") +
                            error.what();
                    }
                }

                ImGui::SameLine();

                if (ImGui::Button("Cancel"))
                {
                    g_showDelete = false;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
        }

        void DrawProjectSettingsDialog()
        {
            if (g_showSettings)
                ImGui::OpenPopup("Project Settings");

            if (ImGui::BeginPopupModal(
                "Project Settings",
                nullptr,
                ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Project: %s", g_project.name.c_str());

                ImGui::InputText(
                    "Version",
                    g_projectVersion,
                    sizeof(g_projectVersion));

                ImGui::Spacing();

                if (ImGui::Button("Save Settings"))
                {
                    if (WriteProjectFile(
                        g_project.root,
                        g_project.name,
                        g_projectVersion))
                    {
                        g_project.version = g_projectVersion;
                        g_status = "Project settings saved.";
                        g_showSettings = false;
                        ImGui::CloseCurrentPopup();
                    }
                    else
                    {
                        g_status = "Failed to save project settings.";
                    }
                }

                ImGui::SameLine();

                if (ImGui::Button("Cancel"))
                {
                    g_showSettings = false;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
        }

        void DrawProjectDialogsInternal()
        {
            DrawCreateProjectDialog();
            DrawDeleteProjectDialog();
            DrawProjectSettingsDialog();
        }
    }

    bool HasActiveProject()
    {
        return g_hasProject;
    }

    void DrawProjectManagerScreen()
    {
        if (g_hasProject)
            return;

        ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(
            viewport->WorkPos,
            ImGuiCond_Always);

        ImGui::SetNextWindowSize(
            viewport->WorkSize,
            ImGuiCond_Always);

        ImGui::Begin(
            "CNode Project Manager",
            nullptr,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoTitleBar);

        const float width = 260.0f;

        ImGui::Dummy(ImVec2(0.0f, 65.0f));

        ImGui::SetCursorPosX(
            (ImGui::GetWindowWidth() - width) * 0.5f);

        ImGui::TextUnformatted("CNode Engine");

        ImGui::SetCursorPosX(
            (ImGui::GetWindowWidth() - width) * 0.5f);

        ImGui::TextDisabled("Project Manager");

        ImGui::Dummy(ImVec2(0.0f, 35.0f));

        ImGui::SetCursorPosX(
            (ImGui::GetWindowWidth() - width) * 0.5f);

        if (ImGui::Button(
            "Create New Project",
            ImVec2(width, 42.0f)))
        {
            g_showCreate = true;
        }

        ImGui::SetCursorPosX(
            (ImGui::GetWindowWidth() - width) * 0.5f);

        if (ImGui::Button(
            "Open Existing Project",
            ImVec2(width, 42.0f)))
        {
            OpenProject();
        }

        ImGui::Dummy(ImVec2(0.0f, 12.0f));

        ImGui::SetCursorPosX(
            (ImGui::GetWindowWidth() - width) * 0.5f);

        if (ImGui::Button(
            "Delete Project",
            ImVec2(width, 36.0f)))
        {
            g_showDelete = true;
        }

        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        ImGui::SetCursorPosX(
            (ImGui::GetWindowWidth() - width) * 0.5f);

        ImGui::TextWrapped("%s", g_status.c_str());

        ImGui::End();

        DrawProjectDialogsInternal();
    }

    void DrawProjectMenu()
    {
        if (ImGui::BeginMenu("Project"))
        {
            if (ImGui::MenuItem("New Project..."))
                g_showCreate = true;

            if (ImGui::MenuItem("Open Project..."))
                OpenProject();

            ImGui::Separator();

            if (ImGui::MenuItem("Delete Project..."))
                g_showDelete = true;

            if (ImGui::MenuItem(
                "Project Settings...",
                nullptr,
                false,
                g_hasProject))
            {
                strncpy_s(
                    g_projectVersion,
                    sizeof(g_projectVersion),
                    g_project.version.c_str(),
                    _TRUNCATE);

                g_showSettings = true;
            }

            ImGui::EndMenu();
        }

        DrawProjectDialogsInternal();
    }

    void DrawProjectDialogs()
    {
        DrawProjectDialogsInternal();
    }

    void RequestCreateProject()
    {
        g_showCreate = true;
    }

    void RequestOpenProject()
    {
        OpenProject();
    }

    void RequestDeleteProject()
    {
        g_showDelete = true;
    }

    void RequestProjectSettings()
    {
        if (!g_hasProject)
            return;

        strncpy_s(
            g_projectVersion,
            sizeof(g_projectVersion),
            g_project.version.c_str(),
            _TRUNCATE);

        g_showSettings = true;
    }

    std::filesystem::path GetActiveProjectRoot()
    {
        if (!g_hasProject)
            return {};

        return g_project.root;
    }

    void DrawProjectTab()
    {
        if (ImGui::BeginMenu("Project"))
        {
            if (ImGui::MenuItem("New Project..."))
                g_showCreate = true;

            if (ImGui::MenuItem("Open Project..."))
                OpenProject();

            ImGui::Separator();

            if (ImGui::MenuItem("Delete Project..."))
                g_showDelete = true;

            if (ImGui::MenuItem(
                "Project Settings...",
                nullptr,
                false,
                g_hasProject))
            {
                strncpy_s(
                    g_projectVersion,
                    sizeof(g_projectVersion),
                    g_project.version.c_str(),
                    _TRUNCATE);

                g_showSettings = true;
            }

            ImGui::EndMenu();
        }

        DrawProjectDialogsInternal();
    }
}
