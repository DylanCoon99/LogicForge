#include "mux.h"
#include <cmath>

Mux::Mux(int numInputs)
    : Component(QString("Mux %1:1").arg(numInputs))
    , m_numInputs(numInputs)
    , m_numSelectors(static_cast<int>(std::log2(numInputs)))
{
    // Data inputs
    for (int i = 0; i < m_numInputs; ++i) {
        addInputPin(QString("D%1").arg(i));
    }
    // Selector inputs
    for (int i = 0; i < m_numSelectors; ++i) {
        addInputPin(QString("S%1").arg(i));
    }
    // Output
    addOutputPin("Y");
}

QJsonObject Mux::toJson() const
{
    QJsonObject obj = Component::toJson();
    obj["numInputs"] = m_numInputs;
    return obj;
}

void Mux::evaluate()
{
    // Read selector bits to form index
    int index = 0;
    for (int i = 0; i < m_numSelectors; ++i) {
        Pin *sel = m_inputs[m_numInputs + i];
        if (sel->state() == Pin::Undefined) {
            m_outputs[0]->setState(Pin::Undefined);
            return;
        }
        if (sel->state() == Pin::High) {
            index |= (1 << i);
        }
    }

    if (index >= m_numInputs) {
        m_outputs[0]->setState(Pin::Undefined);
        return;
    }

    m_outputs[0]->setState(m_inputs[index]->state());
}
