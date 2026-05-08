#include "bussplitter.h"

BusSplitter::BusSplitter(int bitWidth)
    : Component(QString("Bus Split (%1)").arg(bitWidth))
    , m_bitWidth(bitWidth)
{
    for (int i = 0; i < m_bitWidth; ++i) {
        addInputPin(QString("BUS%1").arg(i));
        addOutputPin(QString("BIT%1").arg(i));
    }
}

QJsonObject BusSplitter::toJson() const
{
    QJsonObject obj = Component::toJson();
    obj["bitWidth"] = m_bitWidth;
    return obj;
}

void BusSplitter::evaluate()
{
    for (int i = 0; i < m_bitWidth; ++i) {
        m_outputs[i]->setState(m_inputs[i]->state());
    }
}
