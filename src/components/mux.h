#ifndef MUX_H
#define MUX_H

#include "model/component.h"

class Mux : public Component
{
public:
    Mux(int numInputs); // 2, 4, or 8

    int numDataInputs() const { return m_numInputs; }
    void evaluate() override;
    QJsonObject toJson() const override;

private:
    int m_numInputs;
    int m_numSelectors;
};

#endif // MUX_H
