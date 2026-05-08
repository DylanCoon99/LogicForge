#ifndef INPUTSWITCH_H
#define INPUTSWITCH_H

#include "model/component.h"

class InputSwitch : public Component
{
public:
    InputSwitch();

    bool isOn() const { return m_on; }
    void toggle();
    void evaluate() override;
    QJsonObject toJson() const override;

private:
    bool m_on;
};

#endif // INPUTSWITCH_H
