#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace cnode {

using NodeId = std::uint64_t;
using PortId = std::uint32_t;
using Value = std::variant<std::monostate, bool, int, float, double, std::string>;

enum class PortDirection { Input, Output };
enum class PortType { Flow, Boolean, Integer, Float, String };

struct NodePort {
    PortId id{};
    std::string name;
    PortDirection direction{PortDirection::Input};
    PortType type{PortType::Flow};
    Value defaultValue{};
};

class Node {
public:
    Node(NodeId id, std::string name);
    virtual ~Node() = default;

    NodeId id() const noexcept { return m_id; }
    const std::string& name() const noexcept { return m_name; }
    const std::vector<NodePort>& ports() const noexcept { return m_ports; }

    const NodePort* findPort(PortId portId) const noexcept;
    const NodePort* findPort(std::string const& name, PortDirection direction) const noexcept;

    virtual bool evaluate(std::string& error) = 0;
    virtual Value output(PortId portId) const;

protected:
    PortId addInput(std::string name, PortType type, Value defaultValue = {});
    PortId addOutput(std::string name, PortType type);
    void setOutput(PortId portId, Value value);

private:
    NodeId m_id;
    std::string m_name;
    PortId m_nextPortId{1};
    std::vector<NodePort> m_ports;
    std::unordered_map<PortId, Value> m_outputs;
};

} // namespace cnode
