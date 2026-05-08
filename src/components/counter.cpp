#include "counter.h"

Counter::Counter(int bitWidth)
    : Component(QString("Counter (%1-bit)").arg(bitWidth))
    , m_bitWidth(bitWidth)
    , m_count(0)
    , m_maxCount((1 << bitWidth) - 1)
    , m_clk(nullptr)
    , m_rst(nullptr)
{
    addInputPin("CLK");
    addInputPin("RST");
    m_clk = m_inputs[0];
    m_rst = m_inputs[1];

    for (int i = 0; i < m_bitWidth; ++i) {
        addOutputPin(QString("Q%1").arg(i));
        m_outputs[i]->setState(Pin::Low);
    }
}

QJsonObject Counter::toJson() const
{
    QJsonObject obj = Component::toJson();
    obj["bitWidth"] = m_bitWidth;
    return obj;
}

void Counter::evaluate()
{
    // Synchronous reset
    if (m_rst->state() == Pin::High) {
        m_count = 0;
    } else if (m_clk->hasRisingEdge()) {
        m_count = (m_count + 1) & m_maxCount;
    }

    // Output count bits
    for (int i = 0; i < m_bitWidth; ++i) {
        bool bit = (m_count >> i) & 1;
        m_outputs[i]->setState(bit ? Pin::High : Pin::Low);
    }
}
