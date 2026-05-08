#include "wiregraphicsitem.h"
#include "pingraphicsitem.h"
#include "componentgraphicsitem.h"
#include "model/wire.h"
#include "model/pin.h"

#include <QPen>
#include <QPainterPath>

WireGraphicsItem::WireGraphicsItem(Wire *wire, PinGraphicsItem *sourcePin, PinGraphicsItem *destPin)
    : m_wire(wire)
    , m_sourcePinItem(sourcePin)
    , m_destPinItem(destPin)
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    setPen(QPen(QColor(150, 150, 150), 2.5));
    setZValue(-1); // draw behind components
    updatePath();
}

void WireGraphicsItem::updatePath()
{
    QPointF start = m_sourcePinItem->sceneCenter();
    QPointF end = m_destPinItem->sceneCenter();

    // Orthogonal routing: go right from source, then vertical, then right to dest
    qreal midX = (start.x() + end.x()) / 2.0;

    QPainterPath path;
    path.moveTo(start);
    path.lineTo(midX, start.y());
    path.lineTo(midX, end.y());
    path.lineTo(end);

    setPath(path);
    updateColor();
}

void WireGraphicsItem::updateColor()
{
    Pin::State state = m_wire->sourcePin()->state();
    QColor color;

    switch (state) {
    case Pin::High:      color = QColor(0, 200, 0);   break;  // green
    case Pin::Low:       color = QColor(100, 100, 100); break; // dark gray
    case Pin::Undefined: color = QColor(200, 0, 0);    break;  // red
    case Pin::HighZ:     color = QColor(150, 150, 150); break; // gray
    }

    QPen p = pen();
    p.setColor(color);
    setPen(p);
}

bool WireGraphicsItem::isConnectedTo(ComponentGraphicsItem *compItem) const
{
    return m_sourcePinItem->componentItem() == compItem ||
           m_destPinItem->componentItem() == compItem;
}
