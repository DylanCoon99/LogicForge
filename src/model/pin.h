#ifndef PIN_H
#define PIN_H

#include <QString>
#include <QObject>

class Component;

class Pin
{
public:
    enum Direction { Input, Output };
    enum State { Low = 0, High = 1, Undefined = -1, HighZ = -2 };

    Pin(const QString &name, Direction direction, Component *parent);

    const QString& name() const { return m_name; }
    Direction direction() const { return m_direction; }
    State state() const { return m_state; }
    void setState(State state) { m_state = state; }
    Component* parentComponent() const { return m_parent; }

    // Edge detection
    void savePreviousState() { m_previousState = m_state; }
    bool hasRisingEdge() const { return m_previousState == Low && m_state == High; }
    bool hasFallingEdge() const { return m_previousState == High && m_state == Low; }

private:
    QString    m_name;
    Direction  m_direction;
    State      m_state;
    State      m_previousState;
    Component *m_parent;
};

#endif // PIN_H
