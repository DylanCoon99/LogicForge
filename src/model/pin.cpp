#include "pin.h"

Pin::Pin(const QString &name, Direction direction, Component *parent)
    : m_name(name)
    , m_direction(direction)
    , m_state(Undefined)
    , m_previousState(Undefined)
    , m_parent(parent)
{
}
