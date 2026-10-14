
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace CNode
{
    using GraphID = std::uint64_t;
    using NodeID = std::uint64_t;
    using ClassID = std::uint64_t;
    using VariableID = std::uint64_t;

    // =========================================================
    // DATA TYPES
    // =========================================================

    enum class SocketType
    {
        Float,
        Integer,
        Boolean,
        String,
        Vector3,
        Material,
        TransformClass,
        VisualClass,
        Any
    };

    enum class SocketKind
    {
        ClassInput,
        ClassOutput,
        VariableInput
    };

    enum class VariableControl
    {
        InputField,
        Checkbox,
        Dropdown,
        Slot
    };

    struct Vector3
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    // =========================================================
    // SOCKET REFERENCES
    // =========================================================

    struct SocketRef
    {
        NodeID nodeID = 0;
        ClassID classID = 0;
        VariableID variableID = 0;

        SocketKind kind = SocketKind::ClassInput;
        SocketType type = SocketType::Any;
    };

    struct Connection
    {
        SocketRef source;
        SocketRef target;
    };

    // =========================================================
    // CLASS VARIABLES
    // =========================================================

    struct ClassVariable
    {
        VariableID id = 0;

        std::string name;
        SocketType type = SocketType::Any;

        VariableControl control =
            VariableControl::InputField;

        bool hasInputSocket = false;
        bool hasOutputSocket = false;

        bool boolValue = false;
        float floatValue = 0.0f;
        int intValue = 0;

        Vector3 vectorValue;

        std::string stringValue;
        std::string selectedResource;
    };

    // =========================================================
    // CLASS INSTANCE
    // =========================================================

    struct ClassInstance
    {
        ClassID id = 0;

        std::string className;

        // Apakah class boleh ditambahkan lebih dari sekali
        // ke node yang sama.
        bool canDuplicate = true;

        // Apakah class mempunyai variabel internal.
        bool hasVariables = true;

        // Status ekspansi tampilan class.
        bool expanded = true;

        // Status koneksi input class.
        bool classInputConnected = false;

        // Output class disembunyikan ketika input terhubung.
        bool classOutputHidden = false;

        std::vector<ClassVariable> variables;
    };

    // =========================================================
    // NODE
    // =========================================================

    struct GraphNode
    {
        NodeID id = 0;

        std::string name;

        // Posisi tampilan node pada graph.
        float x = 0.0f;
        float y = 0.0f;

        // Ukuran tampilan node.
        float width = 240.0f;
        float height = 160.0f;

        bool selected = false;

        std::vector<ClassInstance> classes;
    };

    // =========================================================
    // GRAPH
    // =========================================================

    struct NodeGraph
    {
        GraphID id = 0;

        // Setiap objek memiliki NodeGraph miliknya sendiri.
        std::vector<GraphNode> nodes;
        std::vector<Connection> connections;

        NodeID nextNodeID = 1;
        ClassID nextClassID = 1;
        VariableID nextVariableID = 1;
    };

    // =========================================================
    // SCENE OBJECT
    // =========================================================

    struct SceneObject
    {
        std::uint64_t id = 0;

        std::string name;

        // Graph tidak dibagikan dengan objek lain.
        NodeGraph graph;
    };

    // =========================================================
    // GRAPH HELPERS
    // =========================================================

    inline GraphNode* FindNode(
        NodeGraph& graph,
        NodeID nodeID)
    {
        for (auto& node : graph.nodes)
        {
            if (node.id == nodeID)
                return &node;
        }

        return nullptr;
    }

    inline const GraphNode* FindNode(
        const NodeGraph& graph,
        NodeID nodeID)
    {
        for (const auto& node : graph.nodes)
        {
            if (node.id == nodeID)
                return &node;
        }

        return nullptr;
    }

    inline ClassInstance* FindClass(
        GraphNode& node,
        ClassID classID)
    {
        for (auto& classInstance : node.classes)
        {
            if (classInstance.id == classID)
                return &classInstance;
        }

        return nullptr;
    }

    inline ClassVariable* FindVariable(
        ClassInstance& classInstance,
        VariableID variableID)
    {
        for (auto& variable : classInstance.variables)
        {
            if (variable.id == variableID)
                return &variable;
        }

        return nullptr;
    }

    // =========================================================
    // SOCKET COMPATIBILITY
    // =========================================================

    inline bool AreSocketTypesCompatible(
        SocketType source,
        SocketType target)
    {
        if (source == SocketType::Any ||
            target == SocketType::Any)
        {
            return true;
        }

        return source == target;
    }

    // =========================================================
    // CLASS CONNECTION RULES
    // =========================================================

    inline bool IsClassOutputAvailable(
        const ClassInstance& classInstance)
    {
        return !classInstance.classInputConnected &&
            !classInstance.classOutputHidden;
    }

    inline void DisconnectVariableInputs(
        NodeGraph& graph,
        NodeID nodeID,
        ClassID classID)
    {
        graph.connections.erase(
            std::remove_if(
                graph.connections.begin(),
                graph.connections.end(),
                [nodeID, classID](const Connection& connection)
                {
                    return
                        connection.target.nodeID == nodeID &&
                        connection.target.classID == classID &&
                        connection.target.kind ==
                        SocketKind::VariableInput;
                }),
            graph.connections.end()
        );
    }

    inline bool ConnectClassInput(
        NodeGraph& graph,
        NodeID targetNodeID,
        ClassID targetClassID)
    {
        GraphNode* node = FindNode(
            graph,
            targetNodeID
        );

        if (node == nullptr)
            return false;

        ClassInstance* classInstance = FindClass(
            *node,
            targetClassID
        );

        if (classInstance == nullptr)
            return false;

        // Jangan mengubah data jika class sudah
        // berada dalam class mode.
        if (classInstance->classInputConnected)
            return false;

        // Koneksi variabel benar-benar dihapus.
        DisconnectVariableInputs(
            graph,
            targetNodeID,
            targetClassID
        );

        // Beralih ke class mode.
        classInstance->classInputConnected = true;
        classInstance->classOutputHidden = true;

        // Variabel otomatis disembunyikan.
        classInstance->expanded = false;

        return true;
    }

    inline bool DisconnectClassInput(
        NodeGraph& graph,
        NodeID targetNodeID,
        ClassID targetClassID)
    {
        GraphNode* node = FindNode(
            graph,
            targetNodeID
        );

        if (node == nullptr)
            return false;

        ClassInstance* classInstance = FindClass(
            *node,
            targetClassID
        );

        if (classInstance == nullptr)
            return false;

        if (!classInstance->classInputConnected)
            return false;

        // Kembali ke variable mode.
        classInstance->classInputConnected = false;
        classInstance->classOutputHidden = false;

        // Variabel kembali terlihat.
        // Koneksi variabel yang sudah dihapus
        // tidak dipulihkan.
        classInstance->expanded = true;

        return true;
    }
}
