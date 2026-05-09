#ifndef WIREGRAPHICSITEM_H
#define WIREGRAPHICSITEM_H

#include <QGraphicsPathItem>
#include <QVector>
#include <QPointF>

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

    const QVector<QPointF>& bendPoints() const { return m_bendPoints; }

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    int bendPointAt(const QPointF &pos) const;
    int segmentAt(const QPointF &pos) const;

    Wire *m_wire;
    PinGraphicsItem *m_sourcePinItem;
    PinGraphicsItem *m_destPinItem;
    QVector<QPointF> m_bendPoints;

    int m_draggingBend;  // index of bend point being dragged, -1 if none
};

#endif // WIREGRAPHICSITEM_H
