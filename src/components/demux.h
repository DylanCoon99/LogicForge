#ifndef DEMUX_H
#define DEMUX_H

#include "model/component.h"

class Demux : public Component
{
public:
    Demux(int numOutputs); // 2, 4, or 8

    int numDataOutputs() const { return m_numOutputs; }
    void evaluate() override;
    QJsonObject toJson() const override;

private:
    int m_numOutputs;
    int m_numSelectors;
};

#endif // DEMUX_H
