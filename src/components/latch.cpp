#include "latch.h"

Latch::Latch(Type type)
    : Component(typeName(type) + " Latch")
    , m_type(type)
    , m_q(false)
{
    switch (type) {
    case SR:
        addInputPin("S");
        addInputPin("R");
        addInputPin("EN");
        break;
    case D:
        addInputPin("D");
        addInputPin("EN");
        break;
    }

    addOutputPin("Q");
    addOutputPin("Q'");

    m_outputs[0]->setState(Pin::Low);
    m_outputs[1]->setState(Pin::High);
}

QJsonObject Latch::toJson() const
{
    QJsonObject obj = Component::toJson();
    obj["latchType"] = static_cast<int>(m_type);
    obj["q"] = m_q;
    return obj;
}

void Latch::evaluate()
{
    // Level-triggered: update when enable is high
    Pin *enablePin = (m_type == SR) ? m_inputs[2] : m_inputs[1];

    if (enablePin->state() != Pin::High) {
        // Hold current state
        m_outputs[0]->setState(m_q ? Pin::High : Pin::Low);
        m_outputs[1]->setState(m_q ? Pin::Low : Pin::High);
        return;
    }

    switch (m_type) {
    case SR: {
        bool s = (m_inputs[0]->state() == Pin::High);
        bool r = (m_inputs[1]->state() == Pin::High);
        if (s && r) {
            m_outputs[0]->setState(Pin::Undefined);
            m_outputs[1]->setState(Pin::Undefined);
            return;
        }
        if (s) m_q = true;
        if (r) m_q = false;
        break;
    }
    case D:
        m_q = (m_inputs[0]->state() == Pin::High);
        break;
    }

    m_outputs[0]->setState(m_q ? Pin::High : Pin::Low);
    m_outputs[1]->setState(m_q ? Pin::Low : Pin::High);
}

QString Latch::typeName(Type type)
{
    switch (type) {
    case SR: return "SR";
    case D:  return "D";
    }
    return "Unknown";
}
