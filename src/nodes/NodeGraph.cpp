#include "nodes/NodeGraph.h"

#include <algorithm>
#include <unordered_set>

namespace cnode {

NodeId NodeGraph::addNode(std::unique_ptr<Node> node) {
    if (!node) return 0;
    NodeId id = node->id();
    if (id == 0 || m_nodes.contains(id)) {
        id = m_nextId;
        while (m_nodes.contains(id)) ++id;
        m_nextId = id + 1;
    }
    // Nodes own their IDs; callers should create nodes with the graph's next available ID.
    // To keep IDs consistent in this initial API, reject a conflicting non-zero ID.
    if (node->id() != id) return 0;
    m_nodes.emplace(id, std::move(node));
    if (id >= m_nextId) m_nextId = id + 1;
    return id;
}

bool NodeGraph::removeNode(NodeId id) {
    if (m_nodes.erase(id) == 0) return false;
    std::erase_if(m_connections, [id](const Connection& c) {
        return c.fromNode == id || c.toNode == id;
    });
    return true;
}

Node* NodeGraph::findNode(NodeId id) noexcept {
    const auto it = m_nodes.find(id);
    return it == m_nodes.end() ? nullptr : it->second.get();
}

const Node* NodeGraph::findNode(NodeId id) const noexcept {
    const auto it = m_nodes.find(id);
    return it == m_nodes.end() ? nullptr : it->second.get();
}

bool NodeGraph::connect(NodeId fromNode, PortId fromPort,
                        NodeId toNode, PortId toPort, std::string& error) {
    Node* source = findNode(fromNode);
    Node* target = findNode(toNode);
    if (!source || !target) {
        error = "Cannot connect: source or target node does not exist.";
        return false;
    }
    const NodePort* out = source->findPort(fromPort);
    const NodePort* in = target->findPort(toPort);
    if (!out || !in || out->direction != PortDirection::Output ||
        in->direction != PortDirection::Input) {
        error = "Connections must go from an output port to an input port.";
        return false;
    }
    if (out->type != in->type) {
        error = "Port types do not match.";
        return false;
    }
    for (const auto& c : m_connections) {
        if (c.toNode == toNode && c.toPort == toPort) {
            error = "This input already has a connection.";
            return false;
        }
        if (c.fromNode == fromNode && c.fromPort == fromPort &&
            c.toNode == toNode && c.toPort == toPort) {
            error = "This connection already exists.";
            return false;
        }
    }
    m_connections.push_back({fromNode, fromPort, toNode, toPort});
    error.clear();
    return true;
}

bool NodeGraph::disconnect(NodeId fromNode, PortId fromPort,
                           NodeId toNode, PortId toPort) {
    const auto oldSize = m_connections.size();
    std::erase_if(m_connections, [&](const Connection& c) {
        return c.fromNode == fromNode && c.fromPort == fromPort &&
               c.toNode == toNode && c.toPort == toPort;
    });
    return m_connections.size() != oldSize;
}

bool NodeGraph::evaluate(std::string& error) {
    // Basic topological evaluation. Cycles are rejected in this first milestone.
    std::unordered_map<NodeId, std::size_t> indegree;
    std::unordered_map<NodeId, std::vector<NodeId>> edges;
    for (const auto& [id, node] : m_nodes) {
        (void)node;
        indegree[id] = 0;
    }
    for (const auto& c : m_connections) {
        edges[c.fromNode].push_back(c.toNode);
        ++indegree[c.toNode];
    }

    std::vector<NodeId> ready;
    ready.reserve(m_nodes.size());
    for (const auto& [id, degree] : indegree) {
        if (degree == 0) ready.push_back(id);
    }

    std::size_t visited = 0;
    while (!ready.empty()) {
        const NodeId id = ready.back();
        ready.pop_back();
        Node* node = findNode(id);
        if (!node) continue;

        // Values are pulled by nodes through their graph in a later milestone.
        if (!node->evaluate(error)) return false;
        ++visited;

        for (NodeId next : edges[id]) {
            auto& degree = indegree[next];
            if (--degree == 0) ready.push_back(next);
        }
    }

    if (visited != m_nodes.size()) {
        error = "Graph contains a cycle. Cycles are not supported yet.";
        return false;
    }
    error.clear();
    return true;
}

} // namespace cnode
