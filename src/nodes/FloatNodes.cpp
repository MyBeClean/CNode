#include "nodes/FloatNodes.h"

#include <string>

namespace cnode {

FloatConstant::FloatConstant(NodeId id, float value)
    : Node(id, "Float Constant"), m_value(value) {
    m_valuePort = addOutput("Value", PortType::Float);
}

bool FloatConstant::evaluate(std::string& error) {
    setOutput(m_valuePort, m_value);
    error.clear();
    return true;
}

FloatAdd::FloatAdd(NodeId id) : Node(id, "Add") {
    m_aInput = addInput("A", PortType::Float, 0.0f);
    m_bInput = addInput("B", PortType::Float, 0.0f);
    m_result = addOutput("Result", PortType::Float);
}

bool FloatAdd::evaluate(std::string& error) {
    // The first milestone demonstrates graph structure and node evaluation.
    // Reading connected input values is the next core API to implement.
    setOutput(m_result, 0.0f);
    error.clear();
    return true;
}

} // namespace cnode
