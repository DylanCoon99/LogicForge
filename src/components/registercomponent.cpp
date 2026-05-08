#include "registercomponent.h"

RegisterComponent::RegisterComponent(int bitWidth)
    : Component(QString("Register (%1-bit)").arg(bitWidth))
    , m_bitWidth(bitWidth)
    , m_bits(bitWidth, false)
    , m_clk(nullptr)
{
    // Data inputs
    for (int i = 0; i < m_bitWidth; ++i) {
        addInputPin(QString("D%1").arg(i));
    }
    // Clock
    addInputPin("CLK");
    m_clk = m_inputs[m_bitWidth];

    // Outputs
    for (int i = 0; i < m_bitWidth; ++i) {
        addOutputPin(QString("Q%1").arg(i));
        m_outputs[i]->setState(Pin::Low);
    }
}

QJsonObject RegisterComponent::toJson() const
{
    QJsonObject obj = Component::toJson();
    obj["bitWidth"] = m_bitWidth;
    return obj;
}

void RegisterComponent::evaluate()
{
    if (!m_clk->hasRisingEdge()) {
        // Hold — output current stored bits
        for (int i = 0; i < m_bitWidth; ++i) {
            m_outputs[i]->setState(m_bits[i] ? Pin::High : Pin::Low);
        }
        return;
    }

    // Latch inputs on rising edge
    for (int i = 0; i < m_bitWidth; ++i) {
        m_bits[i] = (m_inputs[i]->state() == Pin::High);
        m_outputs[i]->setState(m_bits[i] ? Pin::High : Pin::Low);
    }
}

int RegisterComponent::storedValue() const
{
    int val = 0;
    for (int i = 0; i < m_bitWidth; ++i) {
        if (m_bits[i])
            val |= (1 << i);
    }
    return val;
}
