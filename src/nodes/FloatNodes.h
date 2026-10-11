#pragma once

#include "nodes/Node.h"

namespace cnode {

class FloatConstant final : public Node {
public:
    FloatConstant(NodeId id, float value);
    bool evaluate(std::string& error) override;
    PortId valuePort() const noexcept { return m_valuePort; }

private:
    float m_value;
    PortId m_valuePort{};
};

class FloatAdd final : public Node {
public:
    FloatAdd(NodeId id);
    bool evaluate(std::string& error) override;
    PortId aInput() const noexcept { return m_aInput; }
    PortId bInput() const noexcept { return m_bInput; }
    PortId resultPort() const noexcept { return m_result; }

private:
    PortId m_aInput{};
    PortId m_bInput{};
    PortId m_result{};
};

} // namespace cnode
