#include "wire.h"
#include "pin.h"

Wire::Wire(Pin *source, Pin *dest)
    : m_source(source)
    , m_dest(dest)
{
}

void Wire::propagate()
{
    if (m_source && m_dest) {
        m_dest->setState(m_source->state());
    }
}
