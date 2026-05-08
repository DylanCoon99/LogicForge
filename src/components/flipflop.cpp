#include "flipflop.h"

FlipFlop::FlipFlop(Type type)
    : Component(typeName(type) + " Flip-Flop")
    , m_type(type)
    , m_q(false)
    , m_clk(nullptr)
{
    switch (type) {
    case SR:
        addInputPin("S");
        addInputPin("R");
        addInputPin("CLK");
        m_clk = m_inputs[2];
        break;
    case D:
        addInputPin("D");
        addInputPin("CLK");
        m_clk = m_inputs[1];
        break;
    case JK:
        addInputPin("J");
        addInputPin("K");
        addInputPin("CLK");
        m_clk = m_inputs[2];
        break;
    case T:
        addInputPin("T");
        addInputPin("CLK");
        m_clk = m_inputs[1];
        break;
    }

    addOutputPin("Q");
    addOutputPin("Q'");

    // Initialize outputs
    m_outputs[0]->setState(Pin::Low);
    m_outputs[1]->setState(Pin::High);
}

QJsonObject FlipFlop::toJson() const
{
    QJsonObject obj = Component::toJson();
    obj["ffType"] = static_cast<int>(m_type);
    obj["q"] = m_q;
    return obj;
}

void FlipFlop::evaluate()
{
    // Only update on rising clock edge
    if (!m_clk->hasRisingEdge()) {
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
            // Invalid state
            m_outputs[0]->setState(Pin::Undefined);
            m_outputs[1]->setState(Pin::Undefined);
            return;
        }
        if (s) m_q = true;
        if (r) m_q = false;
        // s=0, r=0: hold
        break;
    }
    case D:
        m_q = (m_inputs[0]->state() == Pin::High);
        break;
    case JK: {
        bool j = (m_inputs[0]->state() == Pin::High);
        bool k = (m_inputs[1]->state() == Pin::High);
        if (j && k)      m_q = !m_q; // toggle
        else if (j)      m_q = true;
        else if (k)       m_q = false;
        // j=0, k=0: hold
        break;
    }
    case T:
        if (m_inputs[0]->state() == Pin::High)
            m_q = !m_q; // toggle
        break;
    }

    m_outputs[0]->setState(m_q ? Pin::High : Pin::Low);
    m_outputs[1]->setState(m_q ? Pin::Low : Pin::High);
}

QString FlipFlop::typeName(Type type)
{
    switch (type) {
    case SR: return "SR";
    case D:  return "D";
    case JK: return "JK";
    case T:  return "T";
    }
    return "Unknown";
}
