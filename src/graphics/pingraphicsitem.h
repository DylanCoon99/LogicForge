#ifndef PINGRAPHICSITEM_H
#define PINGRAPHICSITEM_H

#include <QGraphicsEllipseItem>

class Pin;
class ComponentGraphicsItem;
class CircuitScene;

class PinGraphicsItem : public QGraphicsEllipseItem
{
public:
    static constexpr qreal PinRadius = 5.0;

    PinGraphicsItem(Pin *pin, ComponentGraphicsItem *parent);

    Pin* pin() const { return m_pin; }
    ComponentGraphicsItem* componentItem() const;

    QPointF sceneCenter() const;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    Pin *m_pin;
};

#endif // PINGRAPHICSITEM_H
