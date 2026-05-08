#ifndef REGISTERCOMPONENT_H
#define REGISTERCOMPONENT_H

#include "model/component.h"
#include <QVector>

class RegisterComponent : public Component
{
public:
    RegisterComponent(int bitWidth = 8);

    int bitWidth() const { return m_bitWidth; }
    int storedValue() const;
    void evaluate() override;
    QJsonObject toJson() const override;

private:
    int m_bitWidth;
    QVector<bool> m_bits;
    Pin *m_clk;
};

#endif // REGISTERCOMPONENT_H
