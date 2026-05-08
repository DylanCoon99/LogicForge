#include "gatecomponent.h"

GateComponent::GateComponent(GateType gateType, int numInputs)
    : Component(gateTypeName(gateType))
    , m_gateType(gateType)
{
    int inputs = (gateType == NOT) ? 1 : numInputs;
    for (int i = 0; i < inputs; ++i) {
        addInputPin(QString("IN%1").arg(i));
    }
    addOutputPin("OUT");
}

void GateComponent::evaluate()
{
    // If any input is undefined, output is undefined
    for (auto *pin : m_inputs) {
        if (pin->state() == Pin::Undefined) {
            m_outputs[0]->setState(Pin::Undefined);
            return;
        }
    }

    bool result = false;

    switch (m_gateType) {
    case AND:
    case NAND:
        result = true;
        for (auto *pin : m_inputs)
            result = result && (pin->state() == Pin::High);
        if (m_gateType == NAND) result = !result;
        break;

    case OR:
    case NOR:
        result = false;
        for (auto *pin : m_inputs)
            result = result || (pin->state() == Pin::High);
        if (m_gateType == NOR) result = !result;
        break;

    case XOR:
    case XNOR: {
        int highCount = 0;
        for (auto *pin : m_inputs)
            if (pin->state() == Pin::High) highCount++;
        result = (highCount % 2) == 1;
        if (m_gateType == XNOR) result = !result;
        break;
    }

    case NOT:
        result = (m_inputs[0]->state() != Pin::High);
        break;
    }

    m_outputs[0]->setState(result ? Pin::High : Pin::Low);
}

QJsonObject GateComponent::toJson() const
{
    QJsonObject obj = Component::toJson();
    obj["numInputs"] = m_inputs.size();
    return obj;
}

QString GateComponent::gateTypeName(GateType type)
{
    switch (type) {
    case AND:  return "AND";
    case OR:   return "OR";
    case NOT:  return "NOT";
    case NAND: return "NAND";
    case NOR:  return "NOR";
    case XOR:  return "XOR";
    case XNOR: return "XNOR";
    }
    return "UNKNOWN";
}

GateComponent::GateType GateComponent::gateTypeFromName(const QString &name)
{
    if (name.contains("NAND")) return NAND;
    if (name.contains("NOR"))  return NOR;
    if (name.contains("XNOR")) return XNOR;
    if (name.contains("XOR")) return XOR;
    if (name.contains("AND")) return AND;
    if (name.contains("OR"))  return OR;
    if (name.contains("NOT")) return NOT;
    return AND; // fallback
}
