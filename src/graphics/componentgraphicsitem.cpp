#include "componentgraphicsitem.h"
#include "pingraphicsitem.h"
#include "wiregraphicsitem.h"
#include "canvas/circuitscene.h"
#include "model/component.h"
#include "model/pin.h"
#include "components/gatecomponent.h"
#include "components/inputswitch.h"
#include "components/outputprobe.h"
#include "components/clocksource.h"
#include "components/flipflop.h"
#include "components/latch.h"
#include "components/registercomponent.h"
#include "components/counter.h"
#include "components/mux.h"
#include "components/demux.h"
#include "components/bussplitter.h"
#include "components/busjoiner.h"
#include "components/customcomponent.h"

#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <QPen>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsView>
#include <QMenu>
#include <QInputDialog>
#include <QApplication>

ComponentGraphicsItem::ComponentGraphicsItem(Component *component)
    : m_component(component)
{
    qreal h = computeHeight();
    setRect(0, 0, Width, h);

    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);

    setPen(QPen(QColor(200, 200, 200), 2));
    setBrush(QBrush(QColor(50, 50, 60)));

    createPinItems();

    // Apply any saved rotation
    if (m_component->rotation() != 0) {
        setTransformOriginPoint(rect().center());
        QGraphicsRectItem::setRotation(m_component->rotation());
    }
}

void ComponentGraphicsItem::createPinItems()
{
    const auto &inputs = m_component->inputPins();
    const auto &outputs = m_component->outputPins();

    qreal h = computeHeight();

    // Input pins on the left edge
    for (int i = 0; i < inputs.size(); ++i) {
        auto *pinItem = new PinGraphicsItem(inputs[i], this);
        qreal y = (h / (inputs.size() + 1)) * (i + 1);
        pinItem->setPos(0, y);
        m_inputPinItems.append(pinItem);
    }

    // Output pins on the right edge
    for (int i = 0; i < outputs.size(); ++i) {
        auto *pinItem = new PinGraphicsItem(outputs[i], this);
        qreal y = (h / (outputs.size() + 1)) * (i + 1);
        pinItem->setPos(Width, y);
        m_outputPinItems.append(pinItem);
    }
}

qreal ComponentGraphicsItem::computeHeight() const
{
    int maxPins = qMax(m_component->inputPins().size(), m_component->outputPins().size());
    return qMax(Height, (maxPins + 1) * PinSpacing);
}

bool ComponentGraphicsItem::isGate() const
{
    return dynamic_cast<GateComponent*>(m_component) != nullptr;
}

QRectF ComponentGraphicsItem::boundingRect() const
{
    // Expand to cover the selection highlight, pen width, gate bubbles, and label above
    const qreal margin = 10.0;
    QRectF r = rect().adjusted(-margin, -margin, margin, margin);
    // Extra space above for label
    if (!m_component->label().isEmpty()) {
        r.adjust(0, -18, 0, 0);
    }
    return r;
}

QVariant ComponentGraphicsItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange) {
        // Snap to grid
        QPointF newPos = value.toPointF();
        newPos = CircuitScene::snapToGrid(newPos);
        return newPos;
    }

    if (change == ItemPositionHasChanged) {
        // Update connected wires
        auto *circuitScene = dynamic_cast<CircuitScene*>(scene());
        if (circuitScene) {
            // Find all wire graphics items and update them
            for (auto *item : scene()->items()) {
                auto *wireItem = dynamic_cast<WireGraphicsItem*>(item);
                if (wireItem && wireItem->isConnectedTo(this)) {
                    wireItem->updatePath();
                }
            }
        }

        // Sync model position
        m_component->setPosition(pos());
    }

    return QGraphicsRectItem::itemChange(change, value);
}

// ============================================================================
// paint() - Main dispatcher
// ============================================================================

void ComponentGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    // Draw user label above the component if set
    if (!m_component->label().isEmpty()) {
        painter->setPen(QColor(180, 220, 255));
        QFont labelFont = painter->font();
        labelFont.setPointSize(8);
        labelFont.setItalic(true);
        painter->setFont(labelFont);
        QRectF labelRect(0, -16, rect().width(), 14);
        painter->drawText(labelRect, Qt::AlignCenter, m_component->label());
    }

    // Dispatch to the appropriate paint helper based on component type
    if (dynamic_cast<GateComponent*>(m_component)) {
        paintGate(painter);
    } else if (dynamic_cast<FlipFlop*>(m_component)) {
        paintFlipFlop(painter);
    } else if (dynamic_cast<Latch*>(m_component)) {
        paintLatch(painter);
    } else if (dynamic_cast<Mux*>(m_component)) {
        paintMux(painter);
    } else if (dynamic_cast<Demux*>(m_component)) {
        paintDemux(painter);
    } else if (dynamic_cast<InputSwitch*>(m_component)) {
        paintInputSwitch(painter);
    } else if (dynamic_cast<OutputProbe*>(m_component)) {
        paintOutputProbe(painter);
    } else if (dynamic_cast<ClockSource*>(m_component)) {
        paintClockSource(painter);
    } else if (dynamic_cast<RegisterComponent*>(m_component)) {
        paintRegister(painter);
    } else if (dynamic_cast<Counter*>(m_component)) {
        paintCounter(painter);
    } else if (dynamic_cast<BusSplitter*>(m_component) || dynamic_cast<BusJoiner*>(m_component)) {
        paintBus(painter);
    } else if (dynamic_cast<CustomComponent*>(m_component)) {
        paintCustomComponent(painter);
    } else {
        // Fallback: draw as rectangle with type label
        painter->setPen(QPen(QColor(200, 200, 200), 2));
        painter->setBrush(QBrush(QColor(50, 50, 60)));
        painter->drawRect(rect());

        painter->setPen(Qt::white);
        QFont font = painter->font();
        font.setPointSize(10);
        font.setBold(true);
        painter->setFont(font);
        painter->drawText(rect(), Qt::AlignCenter, m_component->type());
        paintPinLabels(painter);
    }

    // Draw selection highlight
    if (isSelected()) {
        painter->setPen(QPen(QColor(0, 150, 255), 2, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(rect().adjusted(-3, -3, 3, 3));
    }
}

// ============================================================================
// paintBubble / paintClkTriangle - Shared helpers
// ============================================================================

void ComponentGraphicsItem::paintBubble(QPainter *painter, qreal x, qreal y)
{
    painter->save();
    painter->setPen(QPen(QColor(200, 200, 200), 1.5));
    painter->setBrush(Qt::white);
    painter->drawEllipse(QPointF(x, y), 5.0, 5.0);
    painter->restore();
}

void ComponentGraphicsItem::paintClkTriangle(QPainter *painter, qreal x, qreal y)
{
    painter->save();
    QPainterPath tri;
    tri.moveTo(x, y - 5);
    tri.lineTo(x + 8, y);
    tri.lineTo(x, y + 5);
    tri.closeSubpath();
    painter->setPen(QPen(QColor(200, 200, 200), 1.5));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(tri);
    painter->restore();
}

// ============================================================================
// paintGate
// ============================================================================

void ComponentGraphicsItem::paintGate(QPainter *painter)
{
    auto *gate = dynamic_cast<GateComponent*>(m_component);
    if (!gate) return;

    const QRectF r = rect();
    const qreal w = r.width();
    const qreal h = r.height();
    const qreal x0 = r.x();
    const qreal y0 = r.y();

    painter->setPen(QPen(QColor(200, 200, 200), 2));
    painter->setBrush(QBrush(QColor(50, 50, 60)));

    GateComponent::GateType gt = gate->gateType();
    bool hasBubble = (gt == GateComponent::NAND || gt == GateComponent::NOR
                      || gt == GateComponent::XNOR || gt == GateComponent::NOT);
    const qreal bubbleR = 5.0;
    qreal bodyW = hasBubble ? (w - bubbleR * 2) : w;

    switch (gt) {
    case GateComponent::AND:
    case GateComponent::NAND: {
        // The arc's rightmost point is at arcLeft + h = bodyW*0.5 + h/2
        qreal arcRight = bodyW * 0.5 + h / 2.0;
        QPainterPath path;
        path.moveTo(x0, y0);
        path.lineTo(x0 + bodyW * 0.5, y0);
        path.arcTo(x0 + bodyW * 0.5 - h / 2.0, y0, h, h, 90, -180);
        path.lineTo(x0, y0 + h);
        path.closeSubpath();
        painter->drawPath(path);
        if (gt == GateComponent::NAND)
            paintBubble(painter, x0 + arcRight + bubbleR, y0 + h / 2.0);
        break;
    }
    case GateComponent::OR:
    case GateComponent::NOR: {
        QPainterPath path;
        path.moveTo(x0, y0);
        path.quadTo(x0 + bodyW * 0.6, y0, x0 + bodyW, y0 + h * 0.5);
        path.quadTo(x0 + bodyW * 0.6, y0 + h, x0, y0 + h);
        path.quadTo(x0 + bodyW * 0.25, y0 + h * 0.5, x0, y0);
        painter->drawPath(path);
        if (gt == GateComponent::NOR)
            paintBubble(painter, x0 + bodyW + bubbleR - 2, y0 + h / 2.0);
        break;
    }
    case GateComponent::XOR:
    case GateComponent::XNOR: {
        // OR-shaped body
        QPainterPath path;
        path.moveTo(x0, y0);
        path.quadTo(x0 + bodyW * 0.6, y0, x0 + bodyW, y0 + h * 0.5);
        path.quadTo(x0 + bodyW * 0.6, y0 + h, x0, y0 + h);
        path.quadTo(x0 + bodyW * 0.25, y0 + h * 0.5, x0, y0);
        painter->drawPath(path);
        // Extra curved input line (same curvature as left edge, offset 8px left)
        painter->save();
        painter->setBrush(Qt::NoBrush);
        QPainterPath extra;
        extra.moveTo(x0 - 8, y0);
        extra.quadTo(x0 - 8 + bodyW * 0.25, y0 + h * 0.5, x0 - 8, y0 + h);
        painter->drawPath(extra);
        painter->restore();
        if (gt == GateComponent::XNOR)
            paintBubble(painter, x0 + bodyW + bubbleR - 2, y0 + h / 2.0);
        break;
    }
    case GateComponent::NOT: {
        QPainterPath path;
        path.moveTo(x0, y0);
        path.lineTo(x0 + bodyW, y0 + h / 2.0);
        path.lineTo(x0, y0 + h);
        path.closeSubpath();
        painter->drawPath(path);
        paintBubble(painter, x0 + bodyW + bubbleR - 2, y0 + h / 2.0);
        break;
    }
    }
}

// ============================================================================
// paintFlipFlop
// ============================================================================

void ComponentGraphicsItem::paintFlipFlop(QPainter *painter)
{
    auto *ff = dynamic_cast<FlipFlop*>(m_component);
    if (!ff) return;

    const QRectF r = rect();

    // Body
    painter->setPen(QPen(QColor(200, 200, 200), 2));
    painter->setBrush(QBrush(QColor(50, 50, 60)));
    painter->drawRect(r);

    // Type label
    QString typeLabel;
    switch (ff->flipFlopType()) {
    case FlipFlop::D:  typeLabel = "D FF";  break;
    case FlipFlop::SR: typeLabel = "SR FF"; break;
    case FlipFlop::JK: typeLabel = "JK FF"; break;
    case FlipFlop::T:  typeLabel = "T FF";  break;
    }
    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setPointSize(10);
    font.setBold(true);
    font.setItalic(false);
    painter->setFont(font);
    painter->drawText(r, Qt::AlignCenter, typeLabel);

    // CLK triangle - find the CLK pin position
    const auto &inputs = m_component->inputPins();
    for (int i = 0; i < inputs.size(); ++i) {
        if (inputs[i]->name() == "CLK") {
            qreal y = (r.height() / (inputs.size() + 1)) * (i + 1);
            paintClkTriangle(painter, r.x(), r.y() + y);
            break;
        }
    }

    // Q state indicator circle at bottom-right
    QColor stateColor = ff->qState() ? QColor(0, 200, 0) : QColor(100, 100, 100);
    painter->setBrush(stateColor);
    painter->setPen(QPen(Qt::white, 1.5));
    painter->drawEllipse(QPointF(r.right() - 12, r.bottom() - 12), 6, 6);

    paintPinLabels(painter);
}

// ============================================================================
// paintLatch
// ============================================================================

void ComponentGraphicsItem::paintLatch(QPainter *painter)
{
    auto *latch = dynamic_cast<Latch*>(m_component);
    if (!latch) return;

    const QRectF r = rect();

    // Body
    painter->setPen(QPen(QColor(200, 200, 200), 2));
    painter->setBrush(QBrush(QColor(50, 50, 60)));
    painter->drawRect(r);

    // Type label
    QString typeLabel;
    switch (latch->latchType()) {
    case Latch::D:  typeLabel = "D Latch";  break;
    case Latch::SR: typeLabel = "SR Latch"; break;
    }
    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setPointSize(10);
    font.setBold(true);
    font.setItalic(false);
    painter->setFont(font);
    painter->drawText(r, Qt::AlignCenter, typeLabel);

    // Q state indicator circle at bottom-right
    QColor stateColor = latch->qState() ? QColor(0, 200, 0) : QColor(100, 100, 100);
    painter->setBrush(stateColor);
    painter->setPen(QPen(Qt::white, 1.5));
    painter->drawEllipse(QPointF(r.right() - 12, r.bottom() - 12), 6, 6);

    paintPinLabels(painter);
}

// ============================================================================
// paintMux
// ============================================================================

void ComponentGraphicsItem::paintMux(QPainter *painter)
{
    const QRectF r = rect();
    const qreal w = r.width();
    const qreal h = r.height();
    const qreal x0 = r.x();
    const qreal y0 = r.y();

    // Trapezoid: wider left, narrower right
    QPainterPath path;
    path.moveTo(x0, y0);
    path.lineTo(x0 + w, y0 + h * 0.2);
    path.lineTo(x0 + w, y0 + h * 0.8);
    path.lineTo(x0, y0 + h);
    path.closeSubpath();

    painter->setPen(QPen(QColor(200, 200, 200), 2));
    painter->setBrush(QBrush(QColor(50, 50, 60)));
    painter->drawPath(path);

    // "MUX" label
    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setPointSize(9);
    font.setBold(true);
    font.setItalic(false);
    painter->setFont(font);
    painter->drawText(r, Qt::AlignCenter, "MUX");

    paintPinLabels(painter);
}

// ============================================================================
// paintDemux
// ============================================================================

void ComponentGraphicsItem::paintDemux(QPainter *painter)
{
    const QRectF r = rect();
    const qreal w = r.width();
    const qreal h = r.height();
    const qreal x0 = r.x();
    const qreal y0 = r.y();

    // Trapezoid: narrow left, wide right
    QPainterPath path;
    path.moveTo(x0, y0 + h * 0.2);
    path.lineTo(x0 + w, y0);
    path.lineTo(x0 + w, y0 + h);
    path.lineTo(x0, y0 + h * 0.8);
    path.closeSubpath();

    painter->setPen(QPen(QColor(200, 200, 200), 2));
    painter->setBrush(QBrush(QColor(50, 50, 60)));
    painter->drawPath(path);

    // "DEMUX" label
    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setPointSize(9);
    font.setBold(true);
    font.setItalic(false);
    painter->setFont(font);
    painter->drawText(r, Qt::AlignCenter, "DEMUX");

    paintPinLabels(painter);
}

// ============================================================================
// paintInputSwitch
// ============================================================================

void ComponentGraphicsItem::paintInputSwitch(QPainter *painter)
{
    auto *sw = dynamic_cast<InputSwitch*>(m_component);
    if (!sw) return;

    const QRectF r = rect();

    // Rounded rectangle body
    painter->setPen(QPen(QColor(200, 200, 200), 2));
    painter->setBrush(QBrush(QColor(50, 50, 60)));
    painter->drawRoundedRect(r, 8, 8);

    // Large circle in center showing state
    QColor stateColor = sw->isOn() ? QColor(0, 200, 0) : QColor(100, 100, 100);
    QPointF center = r.center();
    painter->setBrush(stateColor);
    painter->setPen(QPen(Qt::white, 2));
    painter->drawEllipse(center, 14, 14);

    // "0" or "1" text in the circle
    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setPointSize(12);
    font.setBold(true);
    font.setItalic(false);
    painter->setFont(font);
    painter->drawText(QRectF(center.x() - 14, center.y() - 14, 28, 28),
                      Qt::AlignCenter, sw->isOn() ? "1" : "0");

    paintPinLabels(painter);
}

// ============================================================================
// paintOutputProbe
// ============================================================================

void ComponentGraphicsItem::paintOutputProbe(QPainter *painter)
{
    auto *probe = dynamic_cast<OutputProbe*>(m_component);
    if (!probe) return;

    const QRectF r = rect();

    // Rounded rectangle body
    painter->setPen(QPen(QColor(200, 200, 200), 2));
    painter->setBrush(QBrush(QColor(50, 50, 60)));
    painter->drawRoundedRect(r, 8, 8);

    // State color
    Pin::State st = probe->readState();
    QColor stateColor;
    switch (st) {
    case Pin::High:      stateColor = QColor(0, 200, 0);    break;
    case Pin::Low:       stateColor = QColor(100, 100, 100); break;
    case Pin::Undefined: stateColor = QColor(200, 0, 0);     break;
    case Pin::HighZ:     stateColor = QColor(150, 150, 150); break;
    }

    // Large circle showing state
    QPointF center = r.center();
    painter->setBrush(stateColor);
    painter->setPen(QPen(Qt::white, 2));
    painter->drawEllipse(center, 14, 14);

    // "LED" label below the circle
    painter->setPen(QColor(180, 180, 180));
    QFont font = painter->font();
    font.setPointSize(7);
    font.setBold(false);
    font.setItalic(false);
    painter->setFont(font);
    painter->drawText(QRectF(r.x(), center.y() + 16, r.width(), 14),
                      Qt::AlignHCenter | Qt::AlignTop, "LED");

    paintPinLabels(painter);
}

// ============================================================================
// paintClockSource
// ============================================================================

void ComponentGraphicsItem::paintClockSource(QPainter *painter)
{
    auto *clk = dynamic_cast<ClockSource*>(m_component);
    if (!clk) return;

    const QRectF r = rect();

    // Rectangle body
    painter->setPen(QPen(QColor(200, 200, 200), 2));
    painter->setBrush(QBrush(QColor(50, 50, 60)));
    painter->drawRect(r);

    // Draw a small square wave symbol inside
    painter->setPen(QPen(QColor(0, 200, 200), 1.5));
    qreal cx = r.center().x();
    qreal cy = r.center().y() - 4;
    qreal sw = 30; // total width of the wave
    qreal sh = 12; // amplitude
    qreal startX = cx - sw / 2.0;

    QPainterPath wave;
    wave.moveTo(startX, cy + sh / 2.0);
    wave.lineTo(startX, cy - sh / 2.0);
    wave.lineTo(startX + sw * 0.25, cy - sh / 2.0);
    wave.lineTo(startX + sw * 0.25, cy + sh / 2.0);
    wave.lineTo(startX + sw * 0.5, cy + sh / 2.0);
    wave.lineTo(startX + sw * 0.5, cy - sh / 2.0);
    wave.lineTo(startX + sw * 0.75, cy - sh / 2.0);
    wave.lineTo(startX + sw * 0.75, cy + sh / 2.0);
    wave.lineTo(startX + sw, cy + sh / 2.0);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(wave);

    // Running indicator at bottom-right
    QColor stateColor = clk->isRunning() ? QColor(0, 200, 0) : QColor(100, 100, 100);
    painter->setBrush(stateColor);
    painter->setPen(QPen(Qt::white, 1.5));
    painter->drawEllipse(QPointF(r.right() - 12, r.bottom() - 12), 6, 6);

    paintPinLabels(painter);
}

// ============================================================================
// paintRegister
// ============================================================================

void ComponentGraphicsItem::paintRegister(QPainter *painter)
{
    auto *reg = dynamic_cast<RegisterComponent*>(m_component);
    if (!reg) return;

    const QRectF r = rect();

    // Rectangle body
    painter->setPen(QPen(QColor(200, 200, 200), 2));
    painter->setBrush(QBrush(QColor(50, 50, 60)));
    painter->drawRect(r);

    // Type label at top
    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setPointSize(9);
    font.setBold(true);
    font.setItalic(false);
    painter->setFont(font);
    painter->drawText(QRectF(r.x(), r.y() + 4, r.width(), 16),
                      Qt::AlignHCenter | Qt::AlignTop, m_component->type());

    // CLK triangle
    const auto &inputs = m_component->inputPins();
    for (int i = 0; i < inputs.size(); ++i) {
        if (inputs[i]->name() == "CLK") {
            qreal y = (r.height() / (inputs.size() + 1)) * (i + 1);
            paintClkTriangle(painter, r.x(), r.y() + y);
            break;
        }
    }

    // Hex value display centered
    painter->setPen(QColor(0, 200, 0));
    QFont smallFont = painter->font();
    smallFont.setPointSize(8);
    smallFont.setBold(false);
    painter->setFont(smallFont);
    painter->drawText(r.adjusted(2, 0, -2, 0), Qt::AlignBottom | Qt::AlignHCenter,
                      QString("0x%1").arg(reg->storedValue(), 2, 16, QChar('0')).toUpper());

    paintPinLabels(painter);
}

// ============================================================================
// paintCounter
// ============================================================================

void ComponentGraphicsItem::paintCounter(QPainter *painter)
{
    auto *ctr = dynamic_cast<Counter*>(m_component);
    if (!ctr) return;

    const QRectF r = rect();

    // Rectangle body
    painter->setPen(QPen(QColor(200, 200, 200), 2));
    painter->setBrush(QBrush(QColor(50, 50, 60)));
    painter->drawRect(r);

    // Type label at top
    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setPointSize(9);
    font.setBold(true);
    font.setItalic(false);
    painter->setFont(font);
    painter->drawText(QRectF(r.x(), r.y() + 4, r.width(), 16),
                      Qt::AlignHCenter | Qt::AlignTop, m_component->type());

    // CLK triangle
    const auto &inputs = m_component->inputPins();
    for (int i = 0; i < inputs.size(); ++i) {
        if (inputs[i]->name() == "CLK") {
            qreal y = (r.height() / (inputs.size() + 1)) * (i + 1);
            paintClkTriangle(painter, r.x(), r.y() + y);
            break;
        }
    }

    // Count value display centered
    painter->setPen(QColor(0, 200, 0));
    QFont smallFont = painter->font();
    smallFont.setPointSize(8);
    smallFont.setBold(false);
    painter->setFont(smallFont);
    painter->drawText(r.adjusted(2, 0, -2, 0), Qt::AlignBottom | Qt::AlignHCenter,
                      QString::number(ctr->count()));

    paintPinLabels(painter);
}

// ============================================================================
// paintBus (BusSplitter / BusJoiner)
// ============================================================================

void ComponentGraphicsItem::paintBus(QPainter *painter)
{
    const QRectF r = rect();
    const qreal w = r.width();
    const qreal h = r.height();
    const qreal x0 = r.x();
    const qreal y0 = r.y();

    bool isSplitter = dynamic_cast<BusSplitter*>(m_component) != nullptr;

    QPainterPath path;
    if (isSplitter) {
        // Fan-out: narrow left, wide right
        path.moveTo(x0, y0 + h * 0.3);
        path.lineTo(x0 + w, y0);
        path.lineTo(x0 + w, y0 + h);
        path.lineTo(x0, y0 + h * 0.7);
        path.closeSubpath();
    } else {
        // Fan-in (BusJoiner): wide left, narrow right
        path.moveTo(x0, y0);
        path.lineTo(x0 + w, y0 + h * 0.3);
        path.lineTo(x0 + w, y0 + h * 0.7);
        path.lineTo(x0, y0 + h);
        path.closeSubpath();
    }

    painter->setPen(QPen(QColor(200, 200, 200), 2));
    painter->setBrush(QBrush(QColor(50, 50, 60)));
    painter->drawPath(path);

    paintPinLabels(painter);
}

// ============================================================================
// paintPinLabels
// ============================================================================

void ComponentGraphicsItem::paintCustomComponent(QPainter *painter)
{
    auto *custom = dynamic_cast<CustomComponent*>(m_component);
    if (!custom) return;

    const QRectF r = rect();

    // Distinctive teal border for custom components
    painter->setPen(QPen(QColor(0, 180, 180), 2));
    painter->setBrush(QBrush(QColor(40, 55, 65)));
    painter->drawRoundedRect(r, 6, 6);

    // Component name centered
    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setPointSize(9);
    font.setBold(true);
    font.setItalic(false);
    painter->setFont(font);
    painter->drawText(r, Qt::AlignCenter, custom->customName());

    // Pin labels
    paintPinLabels(painter);
}

void ComponentGraphicsItem::paintPinLabels(QPainter *painter)
{
    painter->setPen(QColor(180, 180, 180));
    QFont pinFont = painter->font();
    pinFont.setPointSize(7);
    pinFont.setBold(false);
    pinFont.setItalic(false);
    painter->setFont(pinFont);

    const QRectF r = rect();
    const auto &inputs = m_component->inputPins();
    const auto &outputs = m_component->outputPins();

    // Input pin labels (left side, offset 8px from left edge)
    for (int i = 0; i < inputs.size(); ++i) {
        qreal y = (r.height() / (inputs.size() + 1)) * (i + 1);
        QRectF textRect(r.x() + 8, r.y() + y - 7, r.width() / 2.0 - 10, 14);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, inputs[i]->name());
    }

    // Output pin labels (right side, offset 8px from right edge)
    for (int i = 0; i < outputs.size(); ++i) {
        qreal y = (r.height() / (outputs.size() + 1)) * (i + 1);
        QRectF textRect(r.x() + r.width() / 2.0 + 2, r.y() + y - 7, r.width() / 2.0 - 10, 14);
        painter->drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, outputs[i]->name());
    }
}

// ============================================================================
// mouseDoubleClickEvent / contextMenuEvent - unchanged
// ============================================================================

void ComponentGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    // Toggle input switch on double-click
    auto *sw = dynamic_cast<InputSwitch*>(m_component);
    if (sw) {
        sw->toggle();
        auto *circuitScene = dynamic_cast<CircuitScene*>(scene());
        if (circuitScene)
            circuitScene->runSimulation();
        event->accept();
        return;
    }
    // Toggle clock source on double-click
    auto *clock = dynamic_cast<ClockSource*>(m_component);
    if (clock) {
        auto *circuitScene = dynamic_cast<CircuitScene*>(scene());
        if (circuitScene)
            clock->toggle(circuitScene);
        update();
        event->accept();
        return;
    }

    QGraphicsRectItem::mouseDoubleClickEvent(event);
}

void ComponentGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;

    QAction *labelAction = menu.addAction("Edit Label...");

    QAction *rotateAction = menu.addAction("Rotate 90");

    QAction *deleteAction = menu.addAction("Delete");

    // Clock source controls
    QAction *clockStartStopAction = nullptr;
    QAction *clockStepAction = nullptr;
    QAction *clockFreqAction = nullptr;
    auto *clockComp = dynamic_cast<ClockSource*>(m_component);
    if (clockComp) {
        menu.addSeparator();
        clockStartStopAction = menu.addAction(clockComp->isRunning() ? "Stop Clock" : "Start Clock");
        clockStepAction = menu.addAction("Single Step");
        clockFreqAction = menu.addAction(QString("Set Frequency... (%1 ms)").arg(clockComp->periodMs()));
    }

    QAction *createCustomAction = nullptr;
    // Only show this if there are multiple selected items
    if (scene() && scene()->selectedItems().size() > 1) {
        menu.addSeparator();
        createCustomAction = menu.addAction("Create Custom Component...");
    }

    QAction *chosen = menu.exec(event->screenPos());
    if (!chosen) return;

    if (chosen == labelAction) {
        // Need a parent widget for QInputDialog
        QWidget *parentWidget = nullptr;
        if (scene() && !scene()->views().isEmpty())
            parentWidget = qobject_cast<QWidget*>(scene()->views().first());

        bool ok = false;
        QString text = QInputDialog::getText(parentWidget,
                                              "Edit Label",
                                              "Label:",
                                              QLineEdit::Normal,
                                              m_component->label(),
                                              &ok);
        if (ok) {
            m_component->setLabel(text);
            update();
        }
    } else if (chosen == rotateAction) {
        int newRot = (m_component->rotation() + 90) % 360;
        m_component->setRotation(newRot);
        setTransformOriginPoint(rect().center());
        QGraphicsRectItem::setRotation(newRot);
    } else if (chosen == deleteAction) {
        auto *circuitScene = dynamic_cast<CircuitScene*>(scene());
        if (circuitScene)
            circuitScene->removeComponentItem(this);
    } else if (chosen == createCustomAction && createCustomAction) {
        auto *circuitScene = dynamic_cast<CircuitScene*>(scene());
        if (circuitScene)
            circuitScene->createCustomFromSelection();
    } else if (chosen == clockStartStopAction && clockComp) {
        auto *circuitScene = dynamic_cast<CircuitScene*>(scene());
        if (circuitScene)
            clockComp->toggle(circuitScene);
        update();
    } else if (chosen == clockStepAction && clockComp) {
        auto *circuitScene = dynamic_cast<CircuitScene*>(scene());
        if (circuitScene)
            clockComp->singleStep(circuitScene);
        update();
    } else if (chosen == clockFreqAction && clockComp) {
        QWidget *parentWidget = nullptr;
        if (scene() && !scene()->views().isEmpty())
            parentWidget = scene()->views().first();

        bool ok = false;
        int period = QInputDialog::getInt(parentWidget,
                                           "Clock Period",
                                           "Period (ms):",
                                           clockComp->periodMs(),
                                           10, 10000, 10, &ok);
        if (ok) {
            clockComp->setPeriod(period);
            update();
        }
    }
}
