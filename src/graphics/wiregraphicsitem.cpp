#include "wiregraphicsitem.h"
#include "pingraphicsitem.h"
#include "componentgraphicsitem.h"
#include "canvas/circuitscene.h"
#include "model/wire.h"
#include "model/pin.h"

#include <QPen>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QCursor>
#include <cmath>

static constexpr qreal BendPointRadius = 5.0;
static constexpr qreal HitDistance = 8.0;

WireGraphicsItem::WireGraphicsItem(Wire *wire, PinGraphicsItem *sourcePin, PinGraphicsItem *destPin)
    : m_wire(wire)
    , m_sourcePinItem(sourcePin)
    , m_destPinItem(destPin)
    , m_draggingBend(-1)
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
    setPen(QPen(QColor(150, 150, 150), 2.5));
    setZValue(-1); // draw behind components
    updatePath();
}

void WireGraphicsItem::updatePath()
{
    QPointF start = m_sourcePinItem->sceneCenter();
    QPointF end = m_destPinItem->sceneCenter();

    QPainterPath path;
    path.moveTo(start);

    if (m_bendPoints.isEmpty()) {
        // Default orthogonal routing
        qreal midX = (start.x() + end.x()) / 2.0;
        path.lineTo(midX, start.y());
        path.lineTo(midX, end.y());
        path.lineTo(end);
    } else {
        // Route through bend points
        for (const QPointF &bp : m_bendPoints) {
            path.lineTo(bp);
        }
        path.lineTo(end);
    }

    setPath(path);
    updateColor();
}

void WireGraphicsItem::updateColor()
{
    Pin::State state = m_wire->sourcePin()->state();
    QColor color;

    switch (state) {
    case Pin::High:      color = QColor(0, 200, 0);   break;
    case Pin::Low:       color = QColor(100, 100, 100); break;
    case Pin::Undefined: color = QColor(200, 0, 0);    break;
    case Pin::HighZ:     color = QColor(150, 150, 150); break;
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

void WireGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsPathItem::paint(painter, option, widget);

    // Draw bend point handles when selected
    if (isSelected() && !m_bendPoints.isEmpty()) {
        painter->setPen(QPen(Qt::white, 1.5));
        painter->setBrush(QColor(0, 150, 255, 200));
        for (const QPointF &bp : m_bendPoints) {
            painter->drawEllipse(bp, BendPointRadius, BendPointRadius);
        }
    }
}

int WireGraphicsItem::bendPointAt(const QPointF &pos) const
{
    for (int i = 0; i < m_bendPoints.size(); ++i) {
        qreal dx = pos.x() - m_bendPoints[i].x();
        qreal dy = pos.y() - m_bendPoints[i].y();
        if (std::sqrt(dx * dx + dy * dy) < HitDistance)
            return i;
    }
    return -1;
}

// Find which segment of the wire path the point is near
int WireGraphicsItem::segmentAt(const QPointF &pos) const
{
    // Build the list of points along the path
    QVector<QPointF> points;
    points.append(m_sourcePinItem->sceneCenter());
    if (m_bendPoints.isEmpty()) {
        QPointF start = points[0];
        QPointF end = m_destPinItem->sceneCenter();
        qreal midX = (start.x() + end.x()) / 2.0;
        points.append(QPointF(midX, start.y()));
        points.append(QPointF(midX, end.y()));
    } else {
        points.append(m_bendPoints);
    }
    points.append(m_destPinItem->sceneCenter());

    // Check distance to each segment
    for (int i = 0; i < points.size() - 1; ++i) {
        QPointF a = points[i];
        QPointF b = points[i + 1];

        // Point-to-line-segment distance
        qreal dx = b.x() - a.x();
        qreal dy = b.y() - a.y();
        qreal lenSq = dx * dx + dy * dy;
        if (lenSq < 0.001) continue;

        qreal t = ((pos.x() - a.x()) * dx + (pos.y() - a.y()) * dy) / lenSq;
        t = qBound(0.0, t, 1.0);

        QPointF closest(a.x() + t * dx, a.y() + t * dy);
        qreal dist = std::sqrt(std::pow(pos.x() - closest.x(), 2) +
                               std::pow(pos.y() - closest.y(), 2));
        if (dist < HitDistance)
            return i;
    }
    return -1;
}

void WireGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int idx = bendPointAt(event->scenePos());
        if (idx >= 0) {
            m_draggingBend = idx;
            event->accept();
            return;
        }
    }
    QGraphicsPathItem::mousePressEvent(event);
}

void WireGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_draggingBend >= 0 && m_draggingBend < m_bendPoints.size()) {
        m_bendPoints[m_draggingBend] = CircuitScene::snapToGrid(event->scenePos());
        updatePath();
        event->accept();
        return;
    }
    QGraphicsPathItem::mouseMoveEvent(event);
}

void WireGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_draggingBend >= 0) {
        m_draggingBend = -1;
        event->accept();
        return;
    }
    QGraphicsPathItem::mouseReleaseEvent(event);
}

void WireGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    // Double-click on a bend point to remove it
    int idx = bendPointAt(event->scenePos());
    if (idx >= 0) {
        m_bendPoints.removeAt(idx);
        updatePath();
        event->accept();
        return;
    }

    // Double-click on a wire segment to add a bend point
    int seg = segmentAt(event->scenePos());
    if (seg >= 0) {
        QPointF newPoint = CircuitScene::snapToGrid(event->scenePos());
        // Insert after the corresponding bend point index
        // seg 0 is before first bend point, seg 1 is after first, etc.
        int insertIdx;
        if (m_bendPoints.isEmpty()) {
            insertIdx = 0;
        } else {
            insertIdx = qMin(seg, m_bendPoints.size());
        }
        m_bendPoints.insert(insertIdx, newPoint);
        updatePath();
        event->accept();
        return;
    }

    QGraphicsPathItem::mouseDoubleClickEvent(event);
}

void WireGraphicsItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    int idx = bendPointAt(event->scenePos());
    if (idx >= 0) {
        setCursor(Qt::SizeAllCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
    QGraphicsPathItem::hoverMoveEvent(event);
}
