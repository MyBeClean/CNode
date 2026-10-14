#include "file_tab.h"
#include "menu/menu.h"
#include "scene/scene_objects.h"
#include "tab/windows/node_graph/node_graph_editor.h"

#include <Windows.h>
#include <shobjidl.h>
#include <objbase.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <exception>

#include "imgui.h"

namespace fs = std::filesystem;

namespace CNode
{
    namespace
    {
        fs::path g_scenePath;
        std::string g_sceneName = "Untitled";
        std::string g_sceneStatus = "No scene open.";

        bool g_sceneOpen = false;
        bool g_sceneDirty = false;

        std::string g_savedSignature;

        enum class PendingAction
        {
            None,
            NewScene,
            OpenScene
        };

        PendingAction g_pendingAction = PendingAction::None;
        fs::path g_pendingOpenPath;

        bool g_showUnsavedDialog = false;
        bool g_unsavedPopupOpened = false;

        bool g_showErrorDialog = false;
        bool g_errorPopupOpened = false;
        std::string g_errorMessage;

        bool g_saveThenContinue = false;

        struct SceneDocument
        {
            int id = 0;

            fs::path path;
            std::string name = "Untitled";
            std::string status = "New scene created.";
            std::string savedSignature;

            bool open = true;
            bool dirty = false;

            std::vector<SceneObjectEntry> objects;
            NodeGraph graph;
        };

        std::vector<SceneDocument> g_documents;

        int g_activeDocument = -1;
        int g_nextDocumentID = 1;

        void ShowError(const char* message)
        {
            g_errorMessage = message ? message : "Unknown scene error.";
            g_showErrorDialog = true;
        }

        void ShowError(const std::string& message)
        {
            g_errorMessage = message;
            g_showErrorDialog = true;
        }

        std::string BuildObjectSignature()
        {
            std::ostringstream stream;

            stream << std::setprecision(
                std::numeric_limits<float>::max_digits10);

            const auto& objects = SceneObjects::Get().GetAll();

            stream << objects.size() << '\n';

            for (const auto& object : objects)
            {
                stream
                    << object.id << '|'
                    << static_cast<int>(object.type) << '|'
                    << object.cube.position.x << ','
                    << object.cube.position.y << ','
                    << object.cube.position.z << '|'
                    << object.cube.rotation.x << ','
                    << object.cube.rotation.y << ','
                    << object.cube.rotation.z << '|'
                    << object.cube.scale.x << ','
                    << object.cube.scale.y << ','
                    << object.cube.scale.z << '|'
                    << (object.cube.visible ? 1 : 0)
                    << '\n';
            }

            return stream.str();
        }

        void UpdateDirtyState()
        {
            if (!g_sceneOpen)
                return;

            g_sceneDirty = BuildObjectSignature() != g_savedSignature;
        }

        bool ChooseSceneFile(fs::path& selectedPath)
        {
            HRESULT initResult = CoInitializeEx(
                nullptr,
                COINIT_APARTMENTTHREADED);

            const bool shouldUninitialize = SUCCEEDED(initResult);

            if (FAILED(initResult) &&
                initResult != RPC_E_CHANGED_MODE)
            {
                ShowError("Failed to initialize file dialog.");
                return false;
            }

            IFileOpenDialog* dialog = nullptr;

            HRESULT result = CoCreateInstance(
                CLSID_FileOpenDialog,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(&dialog));

            if (FAILED(result) || dialog == nullptr)
            {
                if (shouldUninitialize)
                    CoUninitialize();

                ShowError("Failed to create file dialog.");
                return false;
            }

            DWORD options = 0;

            if (SUCCEEDED(dialog->GetOptions(&options)))
            {
                dialog->SetOptions(
                    options |
                    FOS_FORCEFILESYSTEM |
                    FOS_FILEMUSTEXIST |
                    FOS_PATHMUSTEXIST);
            }

            const COMDLG_FILTERSPEC filters[] =
            {
                { L"CNode Scene (*.cns)", L"*.cns" },
                { L"All Files (*.*)", L"*.*" }
            };

            dialog->SetFileTypes(2, filters);

            result = dialog->Show(nullptr);

            if (FAILED(result))
            {
                dialog->Release();

                if (shouldUninitialize)
                    CoUninitialize();

                if (result != HRESULT_FROM_WIN32(ERROR_CANCELLED))
                    ShowError("Failed to open scene picker.");

                return false;
            }

            IShellItem* item = nullptr;
            result = dialog->GetResult(&item);

            bool success = false;

            if (SUCCEEDED(result) && item != nullptr)
            {
                PWSTR path = nullptr;

                if (SUCCEEDED(item->GetDisplayName(
                    SIGDN_FILESYSPATH, &path)) &&
                    path != nullptr)
                {
                    selectedPath = fs::path(path);
                    CoTaskMemFree(path);
                    success = true;
                }

                item->Release();
            }

            dialog->Release();

            if (shouldUninitialize)
                CoUninitialize();

            return success;
        }

        fs::path GetUniqueScenePath()
        {
            const fs::path root = GetActiveProjectRoot();

            if (root.empty())
                return {};

            const fs::path scenes = root / "Scenes";

            std::error_code ec;
            fs::create_directories(scenes, ec);

            if (ec)
                return {};

            fs::path candidate = scenes / "Untitled.cns";

            if (!fs::exists(candidate))
                return candidate;

            for (unsigned int index = 1; index < 100000; ++index)
            {
                candidate = scenes /
                    ("Untitled_" + std::to_string(index) + ".cns");

                if (!fs::exists(candidate))
                    return candidate;
            }

            return {};
        }

        bool WriteSceneFile(const fs::path& path)
        {
            std::ofstream file(
                path,
                std::ios::out | std::ios::trunc);

            if (!file.is_open())
            {
                ShowError(
                    "Cannot write scene file:\n" + path.string());
                return false;
            }

            const auto& objects = SceneObjects::Get().GetAll();

            file << std::setprecision(
                std::numeric_limits<float>::max_digits10);

            file << "# CNode Scene\n";
            file << "version=1\n";
            file << "name=" << g_sceneName << '\n';

            size_t cubeCount = 0;

            for (const auto& object : objects)
            {
                if (object.type == SceneObjectType::Cube)
                    ++cubeCount;
            }

            file << "object_count=" << cubeCount << '\n';

            for (const auto& object : objects)
            {
                if (object.type != SceneObjectType::Cube)
                    continue;

                file << "\n[Object]\n";
                file << "id=" << object.id << '\n';
                file << "type=Cube\n";

                file << "position="
                    << object.cube.position.x << ','
                    << object.cube.position.y << ','
                    << object.cube.position.z << '\n';

                file << "rotation="
                    << object.cube.rotation.x << ','
                    << object.cube.rotation.y << ','
                    << object.cube.rotation.z << '\n';

                file << "scale="
                    << object.cube.scale.x << ','
                    << object.cube.scale.y << ','
                    << object.cube.scale.z << '\n';

                file << "visible="
                    << (object.cube.visible ? 1 : 0) << '\n';
            }

            file.flush();

            if (!file.good())
            {
                ShowError("Failed to write scene file.");
                return false;
            }

            return true;
        }

        bool ParseVec3(
            const std::string& value,
            glm::vec3& result)
        {
            std::istringstream stream(value);

            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;

            char comma1 = '\0';
            char comma2 = '\0';

            if (!(stream >> x >> comma1 >> y >> comma2 >> z))
                return false;

            if (comma1 != ',' || comma2 != ',')
                return false;

            std::string extra;

            if (stream >> extra)
                return false;

            if (!std::isfinite(x) ||
                !std::isfinite(y) ||
                !std::isfinite(z))
                return false;

            result = glm::vec3(x, y, z);
            return true;
        }

        bool ReadSceneFile(
            const fs::path& path,
            std::string& sceneName,
            std::vector<SceneObjectEntry>& loadedObjects)
        {
            std::ifstream file(path);

            if (!file.is_open())
            {
                ShowError(
                    "Cannot open scene file:\n" + path.string());
                return false;
            }

            std::string line;
            std::string loadedName = path.stem().string();

            bool validHeader = false;
            bool inObject = false;
            bool objectHasID = false;
            bool objectHasType = false;

            SceneObjectEntry currentObject;
            std::vector<SceneObjectEntry> parsedObjects;

            auto finishObject = [&]() -> bool
                {
                    if (!inObject)
                        return true;

                    if (!objectHasID || !objectHasType)
                        return false;

                    parsedObjects.push_back(currentObject);

                    inObject = false;
                    objectHasID = false;
                    objectHasType = false;
                    currentObject = SceneObjectEntry{};

                    return true;
                };

            try
            {
                while (std::getline(file, line))
                {
                    if (!line.empty() && line.back() == '\r')
                        line.pop_back();

                    if (line == "# CNode Scene")
                    {
                        validHeader = true;
                        continue;
                    }

                    if (line.empty() || line[0] == '#')
                        continue;

                    if (line == "[Object]")
                    {
                        if (!finishObject())
                        {
                            ShowError(
                                "Invalid object data in scene file.");
                            return false;
                        }

                        inObject = true;
                        currentObject = SceneObjectEntry{};
                        currentObject.cube =
                            CreateCubeObject(glm::vec3(0.0f));

                        continue;
                    }

                    const size_t separator = line.find('=');

                    if (separator == std::string::npos)
                        continue;

                    const std::string key =
                        line.substr(0, separator);

                    const std::string value =
                        line.substr(separator + 1);

                    if (!inObject)
                    {
                        if (key == "name")
                            loadedName = value;

                        continue;
                    }

                    if (key == "id")
                    {
                        size_t parsedLength = 0;

                        const unsigned long long id =
                            std::stoull(value, &parsedLength);

                        if (parsedLength != value.size() || id == 0)
                        {
                            ShowError(
                                "Invalid object ID in scene file.");
                            return false;
                        }

                        currentObject.id =
                            static_cast<std::uint64_t>(id);

                        objectHasID = true;
                    }
                    else if (key == "type")
                    {
                        if (value != "Cube")
                        {
                            ShowError(
                                "Unsupported object type: " + value);
                            return false;
                        }

                        currentObject.type = SceneObjectType::Cube;
                        objectHasType = true;
                    }
                    else if (key == "position")
                    {
                        if (!ParseVec3(
                            value, currentObject.cube.position))
                        {
                            ShowError("Invalid Cube position.");
                            return false;
                        }
                    }
                    else if (key == "rotation")
                    {
                        if (!ParseVec3(
                            value, currentObject.cube.rotation))
                        {
                            ShowError("Invalid Cube rotation.");
                            return false;
                        }
                    }
                    else if (key == "scale")
                    {
                        if (!ParseVec3(
                            value, currentObject.cube.scale))
                        {
                            ShowError("Invalid Cube scale.");
                            return false;
                        }
                    }
                    else if (key == "visible")
                    {
                        if (value != "0" && value != "1")
                        {
                            ShowError("Invalid Cube visibility.");
                            return false;
                        }

                        currentObject.cube.visible = value == "1";
                    }
                }

                if (!finishObject())
                {
                    ShowError("Invalid object data in scene file.");
                    return false;
                }
            }
            catch (const std::exception&)
            {
                ShowError(
                    "Could not parse scene file. Check its object data.");
                return false;
            }

            if (!validHeader)
            {
                ShowError("This is not a valid CNode scene.");
                return false;
            }

            for (size_t i = 0; i < parsedObjects.size(); ++i)
            {
                for (size_t j = i + 1; j < parsedObjects.size(); ++j)
                {
                    if (parsedObjects[i].id == parsedObjects[j].id)
                    {
                        ShowError("Duplicate object ID in scene file.");
                        return false;
                    }
                }
            }

            sceneName = loadedName;
            loadedObjects = std::move(parsedObjects);

            return true;
        }

        void CaptureActiveDocument()
        {
            if (g_activeDocument < 0 ||
                g_activeDocument >=
                static_cast<int>(g_documents.size()))
            {
                return;
            }

            UpdateDirtyState();

            SceneDocument& document =
                g_documents[g_activeDocument];

            document.path = g_scenePath;
            document.name = g_sceneName;
            document.status = g_sceneStatus;
            document.savedSignature = g_savedSignature;
            document.open = g_sceneOpen;
            document.dirty = g_sceneDirty;

            document.objects = SceneObjects::Get().GetAll();
            document.graph = GetActiveNodeGraph();
        }

        void ActivateDocument(int index)
        {
            if (index < 0 ||
                index >= static_cast<int>(g_documents.size()))
            {
                return;
            }

            if (index == g_activeDocument)
                return;

            CaptureActiveDocument();

            g_activeDocument = index;

            const SceneDocument& document =
                g_documents[index];

            SceneObjects::Get().ReplaceAll(
                document.objects
            );

            ResetNodeGraph();

            GetActiveNodeGraph() = document.graph;

            g_scenePath = document.path;
            g_sceneName = document.name;
            g_sceneStatus = document.status;
            g_savedSignature = document.savedSignature;

            g_sceneOpen = document.open;
            g_sceneDirty = document.dirty;

            g_pendingAction = PendingAction::None;
            g_pendingOpenPath.clear();
            g_saveThenContinue = false;
        }

        void AddCurrentDocument()
        {
            SceneDocument document;

            document.id = g_nextDocumentID++;
            document.path = g_scenePath;
            document.name = g_sceneName;
            document.status = g_sceneStatus;
            document.savedSignature = g_savedSignature;

            document.open = g_sceneOpen;
            document.dirty = g_sceneDirty;

            document.objects = SceneObjects::Get().GetAll();
            document.graph = GetActiveNodeGraph();

            g_documents.push_back(std::move(document));

            g_activeDocument =
                static_cast<int>(g_documents.size()) - 1;
        }

        void CreateEmptyScene()
        {
            CaptureActiveDocument();

            SceneObjects::Get().Clear();
            ResetNodeGraph();

            g_scenePath.clear();
            g_sceneName = "Untitled";
            g_sceneOpen = true;

            g_savedSignature = BuildObjectSignature();
            g_sceneDirty = false;

            g_sceneStatus = "New scene created.";

            g_pendingAction = PendingAction::None;
            g_pendingOpenPath.clear();
            g_saveThenContinue = false;

            AddCurrentDocument();
        }

        void OpenSceneFromPath(const fs::path& path)
        {
            std::string loadedName;
            std::vector<SceneObjectEntry> loadedObjects;

            if (!ReadSceneFile(path, loadedName, loadedObjects))
                return;

            CaptureActiveDocument();

            SceneObjects::Get().ReplaceAll(loadedObjects);
            ResetNodeGraph();

            g_scenePath = path;
            g_sceneName = loadedName;
            g_sceneOpen = true;
            g_sceneDirty = false;
            g_savedSignature = BuildObjectSignature();

            g_sceneStatus =
                "Opened scene: " + path.filename().string();

            g_pendingAction = PendingAction::None;
            g_pendingOpenPath.clear();
            g_saveThenContinue = false;
            AddCurrentDocument();
        }

        void ContinuePendingAction()
        {
            const PendingAction action = g_pendingAction;
            const fs::path path = g_pendingOpenPath;

            g_pendingAction = PendingAction::None;
            g_pendingOpenPath.clear();

            switch (action)
            {
            case PendingAction::NewScene:
                CreateEmptyScene();
                break;

            case PendingAction::OpenScene:
                OpenSceneFromPath(path);
                break;

            case PendingAction::None:
            default:
                break;
            }
        }

        void RequestUnsavedAction(
            PendingAction action,
            const fs::path& path = {})
        {
            g_pendingAction = action;
            g_pendingOpenPath = path;
            g_showUnsavedDialog = true;
        }

        void EnsureProjectScene()
        {
            if (!HasActiveProject() || g_sceneOpen)
                return;

            const fs::path root = GetActiveProjectRoot();

            if (root.empty())
                return;

            const fs::path mainScene =
                root / "Scenes" / "Main.cns";

            try
            {
                if (!fs::exists(mainScene))
                {
                    fs::create_directories(mainScene.parent_path());

                    std::ofstream file(mainScene);

                    if (!file.is_open())
                    {
                        ShowError("Could not create Scenes/Main.cns.");
                        return;
                    }

                    file << "# CNode Scene\n";
                    file << "version=1\n";
                    file << "name=Main\n";
                    file << "object_count=0\n";
                }

                OpenSceneFromPath(mainScene);
            }
            catch (const fs::filesystem_error& error)
            {
                ShowError(
                    std::string("Could not initialize project scene: ") +
                    error.what());
            }
        }

        void DrawSceneDialogsInternal()
        {
            if (g_saveThenContinue)
            {
                g_saveThenContinue = false;

                if (SaveScene())
                {
                    ContinuePendingAction();
                }
                else
                {
                    g_showUnsavedDialog = true;
                }
            }

            if (g_showUnsavedDialog && !g_unsavedPopupOpened)
            {
                ImGui::OpenPopup("Unsaved Scene");
                g_unsavedPopupOpened = true;
            }

            if (ImGui::BeginPopupModal(
                "Unsaved Scene",
                nullptr,
                ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::TextWrapped(
                    "This scene has unsaved changes.");

                ImGui::TextWrapped(
                    "Save your changes before continuing?");

                if (ImGui::Button("Save", ImVec2(120, 0)))
                {
                    g_showUnsavedDialog = false;
                    g_unsavedPopupOpened = false;
                    g_saveThenContinue = true;

                    ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine();

                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                {
                    g_showUnsavedDialog = false;
                    g_unsavedPopupOpened = false;
                    g_pendingAction = PendingAction::None;
                    g_pendingOpenPath.clear();
                    g_saveThenContinue = false;

                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
            else if (!g_showUnsavedDialog)
            {
                g_unsavedPopupOpened = false;
            }

            if (g_showErrorDialog && !g_errorPopupOpened)
            {
                ImGui::OpenPopup("Scene Error");
                g_errorPopupOpened = true;
            }

            if (ImGui::BeginPopupModal(
                "Scene Error",
                nullptr,
                ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::TextWrapped("%s", g_errorMessage.c_str());

                if (ImGui::Button("OK", ImVec2(120, 0)))
                {
                    g_showErrorDialog = false;
                    g_errorPopupOpened = false;

                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
            else if (!g_showErrorDialog)
            {
                g_errorPopupOpened = false;
            }
        }
    }

    void NewScene()
    {
        UpdateDirtyState();

        if (g_sceneOpen && g_sceneDirty)
        {
            RequestUnsavedAction(PendingAction::NewScene);
            return;
        }

        CreateEmptyScene();
    }

    void OpenScene()
    {
        fs::path selectedPath;

        if (!ChooseSceneFile(selectedPath))
            return;

        UpdateDirtyState();

        if (g_sceneOpen && g_sceneDirty)
        {
            RequestUnsavedAction(
                PendingAction::OpenScene,
                selectedPath);
            return;
        }

        OpenSceneFromPath(selectedPath);
    }

    bool SaveScene()
    {
        if (!g_sceneOpen)
        {
            ShowError("There is no active scene to save.");
            return false;
        }

        fs::path targetPath = g_scenePath;

        if (targetPath.empty())
        {
            targetPath = GetUniqueScenePath();

            if (targetPath.empty())
            {
                ShowError(
                    "Could not create a unique scene path in the project.");
                return false;
            }
        }

        try
        {
            fs::create_directories(targetPath.parent_path());

            if (!WriteSceneFile(targetPath))
                return false;

            g_scenePath = targetPath;
            g_sceneDirty = false;
            g_savedSignature = BuildObjectSignature();

            g_sceneStatus =
                "Saved scene: " + g_scenePath.filename().string();

            return true;
        }
        catch (const fs::filesystem_error& error)
        {
            ShowError(
                std::string("Save failed: ") + error.what());
            return false;
        }
    }

    std::string GetActiveSceneDisplayName()
    {
        if (!g_sceneOpen)
            return "Untitled";

        if (!g_scenePath.empty())
            return g_scenePath.filename().string();

        return g_sceneName + ".cns";
    }

    void DrawSceneDialogs()
    {
        DrawSceneDialogsInternal();
    }

    void DrawSceneDocumentTabs()
    {
        if (g_documents.empty())
            return;

        UpdateDirtyState();
        CaptureActiveDocument();

        if (ImGui::BeginTabBar(
            "##CNodeSceneDocuments",
            ImGuiTabBarFlags_Reorderable))
        {
            for (int i = 0;
                i < static_cast<int>(g_documents.size());
                ++i)
            {
                SceneDocument& document = g_documents[i];

                std::string label =
                    document.path.empty()
                    ? document.name
                    : document.path.filename().string();

                if (document.dirty)
                    label += " *";

                label += "###SceneDocument_";
                label += std::to_string(document.id);

                if (ImGui::BeginTabItem(label.c_str()))
                {
                    ActivateDocument(i);

                    ImGui::EndTabItem();
                }
            }

            ImGui::EndTabBar();
        }
    }

    void DrawFileTab()
    {
        EnsureProjectScene();
        UpdateDirtyState();

        ImGuiIO& io = ImGui::GetIO();

        const bool canUseShortcuts =
            !io.WantTextInput &&
            !ImGui::IsAnyItemActive() &&
            !ImGui::IsPopupOpen(
                "", ImGuiPopupFlags_AnyPopupId);

        if (canUseShortcuts && io.KeyCtrl)
        {
            if (ImGui::IsKeyPressed(ImGuiKey_S, false))
                SaveScene();
            else if (ImGui::IsKeyPressed(ImGuiKey_N, false))
                NewScene();
            else if (ImGui::IsKeyPressed(ImGuiKey_O, false))
                OpenScene();
        }

        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene", "Ctrl+N"))
                NewScene();

            if (ImGui::MenuItem("Open Scene", "Ctrl+O"))
                OpenScene();

            if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
                SaveScene();

            ImGui::EndMenu();
        }
    }
}