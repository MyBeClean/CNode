
#pragma once

#include "tab/windows/node_graph/node_graph_data.h"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>

namespace CNode
{
    inline std::string SerializeNodeGraph(const NodeGraph& graph)
    {
        std::ostringstream out;

        out << std::setprecision(9);

        out << graph.id << ' '
            << graph.nextNodeID << ' '
            << graph.nextClassID << ' '
            << graph.nextVariableID << ' '
            << graph.nodes.size() << ' ';

        for (const auto& node : graph.nodes)
        {
            out << node.id << ' '
                << std::quoted(node.name) << ' '
                << node.x << ' ' << node.y << ' '
                << node.width << ' ' << node.height << ' '
                << node.selected << ' '
                << node.classes.size() << ' ';

            for (const auto& cls : node.classes)
            {
                out << cls.id << ' '
                    << std::quoted(cls.className) << ' '
                    << cls.canDuplicate << ' '
                    << cls.hasVariables << ' '
                    << cls.expanded << ' '
                    << cls.classInputConnected << ' '
                    << cls.classOutputHidden << ' '
                    << cls.variables.size() << ' ';

                for (const auto& var : cls.variables)
                {
                    out << var.id << ' '
                        << std::quoted(var.name) << ' '
                        << static_cast<int>(var.type) << ' '
                        << static_cast<int>(var.control) << ' '
                        << var.hasInputSocket << ' '
                        << var.hasOutputSocket << ' '
                        << var.boolValue << ' '
                        << var.floatValue << ' '
                        << var.intValue << ' '
                        << var.vectorValue.x << ' '
                        << var.vectorValue.y << ' '
                        << var.vectorValue.z << ' '
                        << std::quoted(var.stringValue) << ' '
                        << std::quoted(var.selectedResource) << ' ';
                }
            }
        }

        out << graph.connections.size() << ' ';

        for (const auto& connection : graph.connections)
        {
            const auto writeSocket = [&](const SocketRef& socket)
                {
                    out << socket.nodeID << ' '
                        << socket.classID << ' '
                        << socket.variableID << ' '
                        << static_cast<int>(socket.kind) << ' '
                        << static_cast<int>(socket.type) << ' ';
                };

            writeSocket(connection.source);
            writeSocket(connection.target);
        }

        return out.str();
    }

    inline bool IsValidSocketReference(
        const NodeGraph& graph,
        const SocketRef& socket)
    {
        const GraphNode* node = FindNode(graph, socket.nodeID);

        if (!node)
            return false;

        for (const auto& cls : node->classes)
        {
            if (cls.id != socket.classID)
                continue;

            if (socket.kind == SocketKind::ClassInput)
                return !cls.classInputConnected;

            if (socket.kind == SocketKind::ClassOutput)
                return IsClassOutputAvailable(cls);

            for (const auto& variable : cls.variables)
            {
                if (variable.id != socket.variableID)
                    continue;

                if (variable.type != socket.type)
                    return false;

                if (socket.kind == SocketKind::VariableInput)
                    return variable.hasInputSocket &&
                    !cls.classInputConnected;

                if (socket.kind == SocketKind::VariableOutput)
                    return variable.hasOutputSocket;
            }
        }

        return false;
    }

    inline bool ValidateNodeGraph(const NodeGraph& graph)
    {
        std::unordered_set<NodeID> nodeIDs;
        std::unordered_set<ClassID> classIDs;
        std::unordered_set<VariableID> variableIDs;

        NodeID maxNodeID = 0;
        ClassID maxClassID = 0;
        VariableID maxVariableID = 0;

        for (const auto& node : graph.nodes)
        {
            if (node.id == 0 ||
                !nodeIDs.insert(node.id).second ||
                !std::isfinite(node.x) ||
                !std::isfinite(node.y) ||
                !std::isfinite(node.width) ||
                !std::isfinite(node.height) ||
                node.width < 40.0f ||
                node.height < 40.0f)
            {
                return false;
            }

            maxNodeID = std::max(maxNodeID, node.id);

            for (const auto& cls : node.classes)
            {
                if (cls.id == 0 ||
                    !classIDs.insert(cls.id).second ||
                    cls.className.empty())
                {
                    return false;
                }

                maxClassID = std::max(maxClassID, cls.id);

                for (const auto& variable : cls.variables)
                {
                    if (variable.id == 0 ||
                        !variableIDs.insert(variable.id).second ||
                        !std::isfinite(variable.floatValue) ||
                        !std::isfinite(variable.vectorValue.x) ||
                        !std::isfinite(variable.vectorValue.y) ||
                        !std::isfinite(variable.vectorValue.z))
                    {
                        return false;
                    }

                    maxVariableID =
                        std::max(maxVariableID, variable.id);
                }
            }
        }

        if (graph.nextNodeID <= maxNodeID ||
            graph.nextClassID <= maxClassID ||
            graph.nextVariableID <= maxVariableID)
        {
            return false;
        }

        std::unordered_set<std::string> occupiedInputs;

        for (const auto& connection : graph.connections)
        {
            const SocketRef& source = connection.source;
            const SocketRef& target = connection.target;

            if (!IsValidSocketReference(graph, source) ||
                !IsValidSocketReference(graph, target))
            {
                return false;
            }

            if (!AreSocketTypesCompatible(source.type, target.type))
                return false;

            const bool sourceClass =
                source.kind == SocketKind::ClassOutput;

            const bool targetClass =
                target.kind == SocketKind::ClassInput;

            if (sourceClass != targetClass)
                return false;

            if (source.nodeID == target.nodeID &&
                source.classID == target.classID &&
                source.variableID == target.variableID)
            {
                return false;
            }

            std::ostringstream key;
            key << target.nodeID << ':'
                << target.classID << ':'
                << target.variableID << ':'
                << static_cast<int>(target.kind);

            if (!occupiedInputs.insert(key.str()).second)
                return false;
        }

        return true;
    }

    inline bool DeserializeNodeGraph(
        const std::string& data,
        NodeGraph& result)
    {
        std::istringstream in(data);
        NodeGraph parsed;

        std::size_t nodeCount = 0;

        if (!(in >> parsed.id
            >> parsed.nextNodeID
            >> parsed.nextClassID
            >> parsed.nextVariableID
            >> nodeCount))
        {
            return false;
        }

        if (nodeCount > 100000)
            return false;

        for (std::size_t ni = 0; ni < nodeCount; ++ni)
        {
            GraphNode node;
            std::size_t classCount = 0;

            if (!(in >> node.id
                >> std::quoted(node.name)
                >> node.x >> node.y
                >> node.width >> node.height
                >> node.selected
                >> classCount))
            {
                return false;
            }

            if (classCount > 10000)
                return false;

            for (std::size_t ci = 0; ci < classCount; ++ci)
            {
                ClassInstance cls;
                std::size_t variableCount = 0;

                if (!(in >> cls.id
                    >> std::quoted(cls.className)
                    >> cls.canDuplicate
                    >> cls.hasVariables
                    >> cls.expanded
                    >> cls.classInputConnected
                    >> cls.classOutputHidden
                    >> variableCount))
                {
                    return false;
                }

                if (variableCount > 100000)
                    return false;

                for (std::size_t vi = 0; vi < variableCount; ++vi)
                {
                    ClassVariable variable;
                    int type = 0;
                    int control = 0;

                    if (!(in >> variable.id
                        >> std::quoted(variable.name)
                        >> type >> control
                        >> variable.hasInputSocket
                        >> variable.hasOutputSocket
                        >> variable.boolValue
                        >> variable.floatValue
                        >> variable.intValue
                        >> variable.vectorValue.x
                        >> variable.vectorValue.y
                        >> variable.vectorValue.z
                        >> std::quoted(variable.stringValue)
                        >> std::quoted(variable.selectedResource)))
                    {
                        return false;
                    }

                    if (type < 0 ||
                        type > static_cast<int>(SocketType::Any) ||
                        control < 0 ||
                        control > static_cast<int>(VariableControl::Slot))
                    {
                        return false;
                    }

                    variable.type = static_cast<SocketType>(type);
                    variable.control =
                        static_cast<VariableControl>(control);

                    cls.variables.push_back(std::move(variable));
                }

                node.classes.push_back(std::move(cls));
            }

            parsed.nodes.push_back(std::move(node));
        }

        std::size_t connectionCount = 0;

        if (!(in >> connectionCount) ||
            connectionCount > 1000000)
        {
            return false;
        }

        for (std::size_t i = 0; i < connectionCount; ++i)
        {
            Connection connection;

            const auto readSocket = [&](SocketRef& socket) -> bool
                {
                    int kind = 0;
                    int type = 0;

                    if (!(in >> socket.nodeID
                        >> socket.classID
                        >> socket.variableID
                        >> kind >> type))
                    {
                        return false;
                    }

                    if (kind < 0 ||
                        kind > static_cast<int>(SocketKind::VariableOutput) ||
                        type < 0 ||
                        type > static_cast<int>(SocketType::Any))
                    {
                        return false;
                    }

                    socket.kind = static_cast<SocketKind>(kind);
                    socket.type = static_cast<SocketType>(type);

                    return true;
                };

            if (!readSocket(connection.source) ||
                !readSocket(connection.target))
            {
                return false;
            }

            parsed.connections.push_back(connection);
        }

        std::string extra;

        if (in >> extra)
            return false;

        if (!ValidateNodeGraph(parsed))
            return false;

        result = std::move(parsed);
        return true;
    }
}
