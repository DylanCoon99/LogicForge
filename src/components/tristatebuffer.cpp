#include "tristatebuffer.h"

TriStateBuffer::TriStateBuffer(bool inverting)
    : Component(inverting ? "Tri-State Inverter" : "Tri-State Buffer")
    , m_inverting(inverting)
{
    addInputPin("IN");
    addInputPin("EN");
    addOutputPin("OUT");
}

void TriStateBuffer::evaluate()
{
    if (m_inputs[1]->state() == Pin::High) {
        // Enabled - pass through (or invert)
        Pin::State in = m_inputs[0]->state();
        if (m_inverting && in == Pin::High)
            m_outputs[0]->setState(Pin::Low);
        else if (m_inverting && in == Pin::Low)
            m_outputs[0]->setState(Pin::High);
        else
            m_outputs[0]->setState(in);
    } else {
        // Disabled - high impedance
        m_outputs[0]->setState(Pin::HighZ);
    }
}

QJsonObject TriStateBuffer::toJson() const
{
    QJsonObject obj = Component::toJson();
    obj["inverting"] = m_inverting;
    return obj;
}
