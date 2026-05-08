#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include "component.h"
#include "wire.h"

class Circuit
{
public:
    Circuit();
    ~Circuit();

    void addComponent(Component *comp);
    void removeComponent(Component *comp);

    void addWire(Wire *wire);
    void removeWire(Wire *wire);

    // Remove all wires connected to any pin of the given component
    QList<Wire*> wiresForComponent(Component *comp) const;
    QList<Wire*> wiresForPin(Pin *pin) const;

    const QList<Component*>& components() const { return m_components; }
    const QList<Wire*>& wires() const { return m_wires; }

    void clear();

    QJsonObject toJson() const;
    void fromJson(const QJsonObject &obj);

private:
    QList<Component*> m_components;
    QList<Wire*> m_wires;
};

#endif // CIRCUIT_H
