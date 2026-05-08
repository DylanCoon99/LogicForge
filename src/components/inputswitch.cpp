#include "inputswitch.h"

InputSwitch::InputSwitch()
    : Component("Input Switch")
    , m_on(false)
{
    addOutputPin("OUT");
}

void InputSwitch::toggle()
{
    m_on = !m_on;
    evaluate();
}

QJsonObject InputSwitch::toJson() const
{
    QJsonObject obj = Component::toJson();
    obj["on"] = m_on;
    return obj;
}

void InputSwitch::evaluate()
{
    m_outputs[0]->setState(m_on ? Pin::High : Pin::Low);
}
