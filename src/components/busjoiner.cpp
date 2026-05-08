#include "busjoiner.h"

BusJoiner::BusJoiner(int bitWidth)
    : Component(QString("Bus Join (%1)").arg(bitWidth))
    , m_bitWidth(bitWidth)
{
    for (int i = 0; i < m_bitWidth; ++i) {
        addInputPin(QString("BIT%1").arg(i));
        addOutputPin(QString("BUS%1").arg(i));
    }
}

QJsonObject BusJoiner::toJson() const
{
    QJsonObject obj = Component::toJson();
    obj["bitWidth"] = m_bitWidth;
    return obj;
}

void BusJoiner::evaluate()
{
    for (int i = 0; i < m_bitWidth; ++i) {
        m_outputs[i]->setState(m_inputs[i]->state());
    }
}
