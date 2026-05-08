#include "circuitview.h"
#include "circuitscene.h"

#include <QWheelEvent>
#include <QKeyEvent>
#include <QScrollBar>
#include <cmath>

CircuitView::CircuitView(CircuitScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
    , m_panning(false)
    , m_spaceHeld(false)
    , m_zoomFactor(1.0)
{
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(QGraphicsView::RubberBandDrag);
    setBackgroundBrush(QColor(30, 30, 30));
}

void CircuitView::wheelEvent(QWheelEvent *event)
{
    const double zoomIn = 1.15;
    const double zoomOut = 1.0 / zoomIn;
    const double minZoom = 0.1;
    const double maxZoom = 10.0;

    double factor = (event->angleDelta().y() > 0) ? zoomIn : zoomOut;
    double newZoom = m_zoomFactor * factor;

    if (newZoom < minZoom || newZoom > maxZoom)
        return;

    m_zoomFactor = newZoom;
    scale(factor, factor);
}

void CircuitView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton ||
        event->button() == Qt::RightButton ||
        (event->button() == Qt::LeftButton && event->modifiers() & Qt::AltModifier) ||
        (event->button() == Qt::LeftButton && m_spaceHeld)) {
        m_panning = true;
        m_panStart = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    QGraphicsView::mousePressEvent(event);
}

void CircuitView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_panning) {
        m_panning = false;
        setCursor(m_spaceHeld ? Qt::OpenHandCursor : Qt::ArrowCursor);
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void CircuitView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_panning) {
        QPoint delta = event->pos() - m_panStart;
        m_panStart = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        event->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void CircuitView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        m_spaceHeld = true;
        setCursor(Qt::OpenHandCursor);
        event->accept();
        return;
    }
    QGraphicsView::keyPressEvent(event);
}

void CircuitView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        m_spaceHeld = false;
        if (!m_panning)
            setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    QGraphicsView::keyReleaseEvent(event);
}
