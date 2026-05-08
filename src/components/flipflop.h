#ifndef FLIPFLOP_H
#define FLIPFLOP_H

#include "model/component.h"

class FlipFlop : public Component
{
public:
    enum Type { SR, D, JK, T };

    FlipFlop(Type type);

    Type flipFlopType() const { return m_type; }
    bool qState() const { return m_q; }
    void evaluate() override;
    QJsonObject toJson() const override;

    static QString typeName(Type type);

private:
    Type m_type;
    bool m_q;
    Pin *m_clk;
};

#endif // FLIPFLOP_H
