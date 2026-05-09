#ifndef GATECOMPONENT_H
#define GATECOMPONENT_H

#include "model/component.h"

class GateComponent : public Component
{
public:
    enum GateType { AND, OR, NOT, NAND, NOR, XOR, XNOR };

    GateComponent(GateType gateType, int numInputs = 2);

    GateType gateType() const { return m_gateType; }
    int numInputs() const { return m_inputs.size(); }
    void addInput();
    void removeInput();
    void evaluate() override;
    QJsonObject toJson() const override;

    static QString gateTypeName(GateType type);
    static GateType gateTypeFromName(const QString &name);

private:
    GateType m_gateType;
};

#endif // GATECOMPONENT_H
