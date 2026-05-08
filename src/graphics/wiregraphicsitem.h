#ifndef WIREGRAPHICSITEM_H
#define WIREGRAPHICSITEM_H

#include <QGraphicsPathItem>

class Wire;
class PinGraphicsItem;
class ComponentGraphicsItem;

class WireGraphicsItem : public QGraphicsPathItem
{
public:
    WireGraphicsItem(Wire *wire, PinGraphicsItem *sourcePin, PinGraphicsItem *destPin);

    Wire* wire() const { return m_wire; }
    PinGraphicsItem* sourcePinItem() const { return m_sourcePinItem; }
    PinGraphicsItem* destPinItem() const { return m_destPinItem; }

    void updatePath();
    void updateColor();
    bool isConnectedTo(ComponentGraphicsItem *compItem) const;

private:
    Wire *m_wire;
    PinGraphicsItem *m_sourcePinItem;
    PinGraphicsItem *m_destPinItem;
};

#endif // WIREGRAPHICSITEM_H
