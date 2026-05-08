#include "demux.h"
#include <cmath>

Demux::Demux(int numOutputs)
    : Component(QString("Demux 1:%1").arg(numOutputs))
    , m_numOutputs(numOutputs)
    , m_numSelectors(static_cast<int>(std::log2(numOutputs)))
{
    // Data input
    addInputPin("D");
    // Selector inputs
    for (int i = 0; i < m_numSelectors; ++i) {
        addInputPin(QString("S%1").arg(i));
    }
    // Outputs
    for (int i = 0; i < m_numOutputs; ++i) {
        addOutputPin(QString("Y%1").arg(i));
    }
}

QJsonObject Demux::toJson() const
{
    QJsonObject obj = Component::toJson();
    obj["numOutputs"] = m_numOutputs;
    return obj;
}

void Demux::evaluate()
{
    // Read selector bits to form index
    int index = 0;
    for (int i = 0; i < m_numSelectors; ++i) {
        Pin *sel = m_inputs[1 + i]; // skip D input
        if (sel->state() == Pin::Undefined) {
            for (int j = 0; j < m_numOutputs; ++j)
                m_outputs[j]->setState(Pin::Undefined);
            return;
        }
        if (sel->state() == Pin::High) {
            index |= (1 << i);
        }
    }

    // Route input to selected output, others Low
    for (int i = 0; i < m_numOutputs; ++i) {
        if (i == index)
            m_outputs[i]->setState(m_inputs[0]->state());
        else
            m_outputs[i]->setState(Pin::Low);
    }
}
