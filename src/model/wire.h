#ifndef WIRE_H
#define WIRE_H

class Pin;

class Wire
{
public:
    Wire(Pin *source, Pin *dest);

    Pin* sourcePin() const { return m_source; }
    Pin* destPin() const { return m_dest; }

    void propagate();

private:
    Pin *m_source;
    Pin *m_dest;
};

#endif // WIRE_H
