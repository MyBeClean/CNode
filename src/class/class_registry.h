#pragma once

#include "tab/windows/node_graph/node_graph_data.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace CNode
{
    // =========================================================
    // CLASS DEFINITION
    // =========================================================

    struct ClassDefinition
    {
        std::string name;
        std::string category;

        bool canDuplicate = true;
        bool hasVariables = true;

        // Menginisialisasi variabel dan properti default class.
        std::function<void(ClassInstance&)> initialize;
    };

    // =========================================================
    // ADD CLASS RESULT
    // =========================================================

    enum class AddClassResult
    {
        Success,
        NodeNotFound,
        ClassNotRegistered,
        DuplicateNotAllowed
    };

    // =========================================================
    // CLASS REGISTRY
    // =========================================================

    class ClassRegistry
    {
    public:

        static ClassRegistry& Get()
        {
            static ClassRegistry instance;
            return instance;
        }

        // Mendaftarkan definisi class baru.
        bool Register(const ClassDefinition& definition)
        {
            if (definition.name.empty())
                return false;

            // Nama class harus unik.
            if (Find(definition.name) != nullptr)
                return false;

            definitions.push_back(definition);
            return true;
        }

        // Mencari class berdasarkan nama.
        const ClassDefinition* Find(
            const std::string& name) const
        {
            for (const auto& definition : definitions)
            {
                if (definition.name == name)
                    return &definition;
            }

            return nullptr;
        }

        // Mencari class berdasarkan nama.
        // Hasil pencarian diurutkan secara alfabetis.
        std::vector<const ClassDefinition*> Search(
            const std::string& query = "") const
        {
            std::vector<const ClassDefinition*> results;

            const std::string normalizedQuery =
                ToLower(query);

            for (const auto& definition : definitions)
            {
                const std::string normalizedName =
                    ToLower(definition.name);

                if (normalizedQuery.empty() ||
                    normalizedName.find(normalizedQuery)
                    != std::string::npos)
                {
                    results.push_back(&definition);
                }
            }

            std::sort(
                results.begin(),
                results.end(),
                [](const ClassDefinition* a,
                    const ClassDefinition* b)
                {
                    return ToLower(a->name) <
                        ToLower(b->name);
                }
            );

            return results;
        }

        // Mengambil semua definisi class yang terdaftar.
        const std::vector<ClassDefinition>& GetAll() const
        {
            return definitions;
        }

    private:

        std::vector<ClassDefinition> definitions;

        ClassRegistry() = default;

        // Mengubah string menjadi huruf kecil.
        static std::string ToLower(std::string value)
        {
            std::transform(
                value.begin(),
                value.end(),
                value.begin(),
                [](unsigned char character)
                {
                    return static_cast<char>(
                        std::tolower(character)
                        );
                }
            );

            return value;
        }
    };

    // =========================================================
    // CLASS REGISTRATION HELPER
    // =========================================================

    inline bool RegisterClass(
        const ClassDefinition& definition)
    {
        return ClassRegistry::Get().Register(definition);
    }

    // =========================================================
    // ADD CLASS TO NODE
    // =========================================================

    inline AddClassResult AddClassToNode(
        NodeGraph& graph,
        NodeID nodeID,
        const std::string& className)
    {
        // Cari node tujuan.
        GraphNode* node = FindNode(graph, nodeID);

        if (node == nullptr)
            return AddClassResult::NodeNotFound;

        // Cari definisi class di registry.
        const ClassDefinition* definition =
            ClassRegistry::Get().Find(className);

        if (definition == nullptr)
            return AddClassResult::ClassNotRegistered;

        // Periksa aturan duplikasi sebelum mengubah graph.
        if (!definition->canDuplicate)
        {
            for (const auto& existingClass : node->classes)
            {
                if (existingClass.className ==
                    definition->name)
                {
                    return AddClassResult::DuplicateNotAllowed;
                }
            }
        }

        // Buat instance class baru.
        ClassInstance newClass;

        newClass.id = graph.nextClassID++;
        newClass.className = definition->name;

        newClass.canDuplicate =
            definition->canDuplicate;

        newClass.hasVariables =
            definition->hasVariables;

        newClass.expanded = true;
        newClass.classInputConnected = false;
        newClass.classOutputHidden = false;

        // Buat variabel default milik class.
        if (definition->initialize)
        {
            definition->initialize(newClass);
        }

        // Pastikan setiap variabel memiliki ID unik.
        for (auto& variable : newClass.variables)
        {
            variable.id = graph.nextVariableID++;
        }

        // Masukkan class ke node tujuan.
        node->classes.push_back(std::move(newClass));

        return AddClassResult::Success;
    }
}