#include "nodes/Node.h"

namespace cnode {

Node::Node(NodeId id, std::string name)
    : m_id(id), m_name(std::move(name)) {}

const NodePort* Node::findPort(PortId portId) const noexcept {
    for (const auto& port : m_ports) {
        if (port.id == portId) return &port;
    }
    return nullptr;
}

const NodePort* Node::findPort(std::string const& name, PortDirection direction) const noexcept {
    for (const auto& port : m_ports) {
        if (port.name == name && port.direction == direction) return &port;
    }
    return nullptr;
}

Value Node::output(PortId portId) const {
    const auto it = m_outputs.find(portId);
    return it == m_outputs.end() ? Value{} : it->second;
}

PortId Node::addInput(std::string name, PortType type, Value defaultValue) {
    const PortId id = m_nextPortId++;
    m_ports.push_back({id, std::move(name), PortDirection::Input, type, std::move(defaultValue)});
    return id;
}

PortId Node::addOutput(std::string name, PortType type) {
    const PortId id = m_nextPortId++;
    m_ports.push_back({id, std::move(name), PortDirection::Output, type, {}});
    return id;
}

void Node::setOutput(PortId portId, Value value) {
    m_outputs[portId] = std::move(value);
}

} // namespace cnode
