#include "pingraphicsitem.h"
#include "componentgraphicsitem.h"
#include "canvas/circuitscene.h"
#include "model/pin.h"

#include <QBrush>
#include <QPen>
#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

PinGraphicsItem::PinGraphicsItem(Pin *pin, ComponentGraphicsItem *parent)
    : QGraphicsEllipseItem(-PinRadius, -PinRadius, PinRadius * 2, PinRadius * 2, parent)
    , m_pin(pin)
{
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable, false);

    // Visual style
    setPen(QPen(Qt::white, 1.5));
    if (pin->direction() == Pin::Input) {
        setBrush(QBrush(QColor(100, 149, 237))); // cornflower blue
    } else {
        setBrush(QBrush(QColor(50, 205, 50)));    // lime green
    }
}

ComponentGraphicsItem* PinGraphicsItem::componentItem() const
{
    return static_cast<ComponentGraphicsItem*>(parentItem());
}

QPointF PinGraphicsItem::sceneCenter() const
{
    return mapToScene(QPointF(0, 0));
}

void PinGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    setBrush(QBrush(QColor(255, 215, 0))); // gold on hover
    setCursor(Qt::CrossCursor);
}

void PinGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    if (m_pin->direction() == Pin::Input) {
        setBrush(QBrush(QColor(100, 149, 237)));
    } else {
        setBrush(QBrush(QColor(50, 205, 50)));
    }
    setCursor(Qt::ArrowCursor);
}

void PinGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    auto *circuitScene = dynamic_cast<CircuitScene*>(scene());
    if (!circuitScene)
        return;

    if (!circuitScene->isWiring()) {
        // Start wiring from output pin
        if (m_pin->direction() == Pin::Output) {
            circuitScene->startWiring(this);
            event->accept();
            return;
        }
    } else {
        // Complete wiring to input pin
        if (m_pin->direction() == Pin::Input) {
            circuitScene->completeWiring(this);
            event->accept();
            return;
        }
    }

    QGraphicsEllipseItem::mousePressEvent(event);
}
