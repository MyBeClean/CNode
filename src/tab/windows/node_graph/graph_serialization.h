
#pragma once

#include "tab/windows/node_graph/node_graph_data.h"

#include <iomanip>
#include <sstream>
#include <string>

namespace CNode
{
    inline std::string SerializeNodeGraph(const NodeGraph& graph)
    {
        std::ostringstream out;

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
            const auto writeSocket = [&](const SocketRef& s)
                {
                    out << s.nodeID << ' '
                        << s.classID << ' '
                        << s.variableID << ' '
                        << static_cast<int>(s.kind) << ' '
                        << static_cast<int>(s.type) << ' ';
                };

            writeSocket(connection.source);
            writeSocket(connection.target);
        }

        return out.str();
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

        // Batas defensif untuk mencegah file rusak
        // meminta alokasi memori yang tidak masuk akal.
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
                    ClassVariable var;
                    int type = 0;
                    int control = 0;

                    if (!(in >> var.id
                        >> std::quoted(var.name)
                        >> type >> control
                        >> var.hasInputSocket
                        >> var.hasOutputSocket
                        >> var.boolValue
                        >> var.floatValue
                        >> var.intValue
                        >> var.vectorValue.x
                        >> var.vectorValue.y
                        >> var.vectorValue.z
                        >> std::quoted(var.stringValue)
                        >> std::quoted(var.selectedResource)))
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

                    var.type = static_cast<SocketType>(type);
                    var.control = static_cast<VariableControl>(control);

                    cls.variables.push_back(std::move(var));
                }

                node.classes.push_back(std::move(cls));
            }

            parsed.nodes.push_back(std::move(node));
        }

        std::size_t connectionCount = 0;

        if (!(in >> connectionCount) || connectionCount > 1000000)
            return false;

        for (std::size_t i = 0; i < connectionCount; ++i)
        {
            Connection connection;

            auto readSocket = [&](SocketRef& s) -> bool
                {
                    int kind = 0;
                    int type = 0;

                    if (!(in >> s.nodeID
                        >> s.classID
                        >> s.variableID
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

                    s.kind = static_cast<SocketKind>(kind);
                    s.type = static_cast<SocketType>(type);
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

        result = std::move(parsed);
        return true;
    }
}
