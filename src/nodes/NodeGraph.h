#pragma once

#include "nodes/Node.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cnode {

struct Connection {
    NodeId fromNode{};
    PortId fromPort{};
    NodeId toNode{};
    PortId toPort{};
};

class NodeGraph {
public:
    NodeId addNode(std::unique_ptr<Node> node);
    bool removeNode(NodeId id);
    Node* findNode(NodeId id) noexcept;
    const Node* findNode(NodeId id) const noexcept;

    bool connect(NodeId fromNode, PortId fromPort,
                 NodeId toNode, PortId toPort, std::string& error);
    bool disconnect(NodeId fromNode, PortId fromPort,
                    NodeId toNode, PortId toPort);

    const std::vector<Connection>& connections() const noexcept { return m_connections; }
    bool evaluate(std::string& error);

private:
    std::unordered_map<NodeId, std::unique_ptr<Node>> m_nodes;
    std::vector<Connection> m_connections;
    NodeId m_nextId{1};
};

} // namespace cnode
