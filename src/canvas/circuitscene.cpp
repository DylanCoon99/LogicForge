#include "circuitscene.h"
#include "circuitview.h"
#include "graphics/componentgraphicsitem.h"
#include "graphics/pingraphicsitem.h"
#include "graphics/wiregraphicsitem.h"
#include "components/gatecomponent.h"
#include "components/inputswitch.h"
#include "components/outputprobe.h"
#include "components/clocksource.h"
#include "components/flipflop.h"
#include "components/latch.h"
#include "components/mux.h"
#include "components/demux.h"
#include "components/registercomponent.h"
#include "components/counter.h"
#include "components/bussplitter.h"
#include "components/busjoiner.h"
#include "components/customcomponent.h"
#include "components/tristatebuffer.h"
#include "dialogs/createcustomdialog.h"
#include "model/wire.h"
#include "model/simulationengine.h"

#include <QPainter>
#include <QMessageBox>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUndoStack>
#include <QDir>
#include <QFile>
#include <cmath>

CircuitScene::CircuitScene(QObject *parent)
    : QGraphicsScene(parent)
    , m_circuit(new Circuit())
    , m_engine(new SimulationEngine(m_circuit, this))
    , m_undoStack(nullptr)
    , m_wiringActive(false)
    , m_wiringSourcePin(nullptr)
    , m_wiringTempLine(nullptr)
{
    setSceneRect(-5000, -5000, 10000, 10000);

    connect(m_engine, &SimulationEngine::simulationComplete,
            this, &CircuitScene::onSimulationComplete);
    connect(m_engine, &SimulationEngine::oscillationDetected,
            this, []() {
                QMessageBox::warning(nullptr, "Oscillation Detected",
                    "The circuit did not settle — possible feedback loop.");
            });
}

CircuitScene::~CircuitScene()
{
    delete m_circuit;
}

qreal CircuitScene::snapToGrid(qreal val)
{
    return std::round(val / GridSize) * GridSize;
}

QPointF CircuitScene::snapToGrid(const QPointF &pos)
{
    return QPointF(snapToGrid(pos.x()), snapToGrid(pos.y()));
}

void CircuitScene::setPendingComponentType(const QString &type)
{
    m_pendingType = type;
    cancelWiring();
}

void CircuitScene::clearPendingComponent()
{
    m_pendingType.clear();
}

// ---- Wiring ----

void CircuitScene::startWiring(PinGraphicsItem *sourcePin)
{
    m_wiringActive = true;
    m_wiringSourcePin = sourcePin;

    QPointF start = sourcePin->sceneCenter();
    m_wiringTempLine = addLine(start.x(), start.y(), start.x(), start.y(),
                               QPen(QColor(255, 215, 0), 2, Qt::DashLine));
    m_wiringTempLine->setZValue(10);
}

void CircuitScene::completeWiring(PinGraphicsItem *destPin)
{
    if (!m_wiringActive || !m_wiringSourcePin || !destPin)
        return;

    Pin *srcPin = m_wiringSourcePin->pin();
    Pin *dstPin = destPin->pin();

    // Validate: output -> input, different components
    if (srcPin->direction() != Pin::Output || dstPin->direction() != Pin::Input)
        goto cancel;
    if (srcPin->parentComponent() == dstPin->parentComponent())
        goto cancel;

    {
        Wire *wire = new Wire(srcPin, dstPin);
        m_circuit->addWire(wire);

        auto *wireItem = new WireGraphicsItem(wire, m_wiringSourcePin, destPin);
        addItem(wireItem);
        cancelWiring();
        runSimulation();
        return;
    }

cancel:
    cancelWiring();
}

void CircuitScene::cancelWiring()
{
    if (m_wiringTempLine) {
        removeItem(m_wiringTempLine);
        delete m_wiringTempLine;
        m_wiringTempLine = nullptr;
    }
    m_wiringActive = false;
    m_wiringSourcePin = nullptr;
}

// ---- Component management ----

ComponentGraphicsItem* CircuitScene::createComponentItem(const QString &type, const QPointF &pos)
{
    Component *comp = nullptr;

    if (type.startsWith("Custom:")) {
        // Load custom component from .dccomp file — check FIRST to avoid
        // matching built-in type names like "Counter", "Register", etc.
        QString customName = type.mid(8); // strip "Custom: "
        QString dir = QDir::homePath() + "/Documents/digital_design_app/components";
        QString filePath = dir + "/" + customName + ".dccomp";
        auto *custom = CustomComponent::fromFile(filePath);
        if (custom) {
            comp = custom;
        }
    } else if (type.contains("Input Switch")) {
        comp = new InputSwitch();
    } else if (type.contains("Output Probe")) {
        comp = new OutputProbe();
    } else if (type.contains("Clock Source")) {
        comp = new ClockSource();
    } else if (type.contains("D Flip-Flop")) {
        comp = new FlipFlop(FlipFlop::D);
    } else if (type.contains("SR Flip-Flop")) {
        comp = new FlipFlop(FlipFlop::SR);
    } else if (type.contains("JK Flip-Flop")) {
        comp = new FlipFlop(FlipFlop::JK);
    } else if (type.contains("T Flip-Flop")) {
        comp = new FlipFlop(FlipFlop::T);
    } else if (type.contains("D Latch")) {
        comp = new Latch(Latch::D);
    } else if (type.contains("SR Latch")) {
        comp = new Latch(Latch::SR);
    } else if (type.contains("Mux 2:1")) {
        comp = new Mux(2);
    } else if (type.contains("Mux 4:1")) {
        comp = new Mux(4);
    } else if (type.contains("Demux 1:2")) {
        comp = new Demux(2);
    } else if (type.contains("Demux 1:4")) {
        comp = new Demux(4);
    } else if (type.contains("Register")) {
        comp = new RegisterComponent(8);
    } else if (type.contains("Counter")) {
        comp = new Counter(4);
    } else if (type.contains("Bus Splitter")) {
        comp = new BusSplitter(8);
    } else if (type.contains("Bus Joiner")) {
        comp = new BusJoiner(8);
    } else if (type.contains("Tri-State Buffer")) {
        comp = new TriStateBuffer(false);
    } else if (type.contains("Tri-State Inverter")) {
        comp = new TriStateBuffer(true);
    } else {
        // It's a gate
        auto gateType = GateComponent::gateTypeFromName(type);
        comp = new GateComponent(gateType);
    }

    comp->setPosition(pos);
    m_circuit->addComponent(comp);

    auto *item = new ComponentGraphicsItem(comp);
    item->setPos(pos);
    addItem(item);

    return item;
}

void CircuitScene::removeComponentItem(ComponentGraphicsItem *item)
{
    // Remove connected wire graphics items first
    QList<QGraphicsItem*> toRemove;
    for (auto *sceneItem : items()) {
        auto *wireItem = dynamic_cast<WireGraphicsItem*>(sceneItem);
        if (wireItem && wireItem->isConnectedTo(item)) {
            toRemove.append(wireItem);
        }
    }
    for (auto *ri : toRemove) {
        auto *wireItem = static_cast<WireGraphicsItem*>(ri);
        m_circuit->removeWire(wireItem->wire());
        removeItem(wireItem);
        delete wireItem;
    }

    // Remove component from model without touching wires
    // (we already removed the wires above)
    Component *comp = item->component();
    removeItem(item);
    delete item;
    m_circuit->removeComponentOnly(comp);
}

void CircuitScene::removeWireItem(WireGraphicsItem *item)
{
    m_circuit->removeWire(item->wire());
    removeItem(item);
    delete item;
}

// ---- Helpers ----

ComponentGraphicsItem* CircuitScene::findComponentItem(Component *comp) const
{
    for (auto *item : items()) {
        auto *compItem = dynamic_cast<ComponentGraphicsItem*>(item);
        if (compItem && compItem->component() == comp)
            return compItem;
    }
    return nullptr;
}

PinGraphicsItem* CircuitScene::findPinItem(Pin *pin) const
{
    for (auto *item : items()) {
        auto *compItem = dynamic_cast<ComponentGraphicsItem*>(item);
        if (!compItem) continue;
        for (auto *pi : compItem->inputPinItems()) {
            if (pi->pin() == pin) return pi;
        }
        for (auto *pi : compItem->outputPinItems()) {
            if (pi->pin() == pin) return pi;
        }
    }
    return nullptr;
}

// ---- Copy / Paste / Select ----

void CircuitScene::copySelection()
{
    QList<ComponentGraphicsItem*> selectedComps;
    for (auto *item : selectedItems()) {
        auto *compItem = dynamic_cast<ComponentGraphicsItem*>(item);
        if (compItem)
            selectedComps.append(compItem);
    }
    if (selectedComps.isEmpty()) return;

    // Collect component IDs
    QSet<QUuid> compIds;
    for (auto *ci : selectedComps)
        compIds.insert(ci->component()->id());

    // Serialize selected components
    QJsonArray compsArray;
    for (auto *ci : selectedComps)
        compsArray.append(ci->component()->toJson());

    // Serialize wires that connect two selected components
    QJsonArray wiresArray;
    for (auto *w : m_circuit->wires()) {
        QUuid srcId = w->sourcePin()->parentComponent()->id();
        QUuid dstId = w->destPin()->parentComponent()->id();
        if (compIds.contains(srcId) && compIds.contains(dstId)) {
            QJsonObject wireObj;
            wireObj["srcId"] = srcId.toString();
            wireObj["srcPin"] = w->sourcePin()->parentComponent()->outputPins().indexOf(w->sourcePin());
            wireObj["dstId"] = dstId.toString();
            wireObj["dstPin"] = w->destPin()->parentComponent()->inputPins().indexOf(w->destPin());
            wiresArray.append(wireObj);
        }
    }

    m_clipboard = QJsonObject();
    m_clipboard["components"] = compsArray;
    m_clipboard["wires"] = wiresArray;
}

void CircuitScene::pasteClipboard()
{
    if (m_clipboard.isEmpty()) return;

    QJsonArray compsArray = m_clipboard["components"].toArray();
    QJsonArray wiresArray = m_clipboard["wires"].toArray();

    // Map old IDs to new components
    QHash<QString, Component*> idMap;

    // Clear selection
    clearSelection();

    for (const auto &val : compsArray) {
        QJsonObject obj = val.toObject();
        QString oldId = obj["id"].toString();

        // Offset position
        obj["x"] = obj["x"].toDouble() + 40.0;
        obj["y"] = obj["y"].toDouble() + 40.0;

        // Assign new UUID
        QUuid newId = QUuid::createUuid();
        obj["id"] = newId.toString();

        Component *comp = Component::fromJson(obj);
        if (!comp) continue;

        m_circuit->addComponent(comp);
        idMap[oldId] = comp;

        auto *item = new ComponentGraphicsItem(comp);
        item->setPos(comp->position());
        addItem(item);
        item->setSelected(true);
    }

    // Recreate internal wires
    for (const auto &val : wiresArray) {
        QJsonObject wireObj = val.toObject();
        QString srcOldId = wireObj["srcId"].toString();
        int srcPinIdx = wireObj["srcPin"].toInt();
        QString dstOldId = wireObj["dstId"].toString();
        int dstPinIdx = wireObj["dstPin"].toInt();

        Component *srcComp = idMap.value(srcOldId);
        Component *dstComp = idMap.value(dstOldId);
        if (!srcComp || !dstComp) continue;
        if (srcPinIdx >= srcComp->outputPins().size()) continue;
        if (dstPinIdx >= dstComp->inputPins().size()) continue;

        Pin *srcPin = srcComp->outputPins()[srcPinIdx];
        Pin *dstPin = dstComp->inputPins()[dstPinIdx];

        Wire *wire = new Wire(srcPin, dstPin);
        m_circuit->addWire(wire);

        PinGraphicsItem *srcPinItem = findPinItem(srcPin);
        PinGraphicsItem *dstPinItem = findPinItem(dstPin);
        if (srcPinItem && dstPinItem) {
            auto *wireItem = new WireGraphicsItem(wire, srcPinItem, dstPinItem);
            addItem(wireItem);
        }
    }

    // Update clipboard with offset for next paste
    QJsonArray updatedComps;
    for (const auto &val : compsArray) {
        QJsonObject obj = val.toObject();
        obj["x"] = obj["x"].toDouble() + 40.0;
        obj["y"] = obj["y"].toDouble() + 40.0;
        updatedComps.append(obj);
    }
    m_clipboard["components"] = updatedComps;

    runSimulation();
}

void CircuitScene::cutSelection()
{
    copySelection();

    // Delete selected items
    QList<QGraphicsItem*> selected = selectedItems();
    for (auto *item : selected) {
        auto *compItem = dynamic_cast<ComponentGraphicsItem*>(item);
        if (compItem) {
            removeComponentItem(compItem);
            continue;
        }
        auto *wireItem = dynamic_cast<WireGraphicsItem*>(item);
        if (wireItem) {
            removeWireItem(wireItem);
        }
    }
}

void CircuitScene::selectAll()
{
    for (auto *item : items()) {
        item->setSelected(true);
    }
}

// ---- Load Circuit ----

void CircuitScene::loadCircuit(const QJsonObject &obj)
{
    // Clear scene graphics (removes all QGraphicsItems)
    QGraphicsScene::clear();
    m_wiringTempLine = nullptr;
    m_wiringActive = false;
    m_wiringSourcePin = nullptr;

    // Clear and reload model
    m_circuit->fromJson(obj);

    // Recreate graphics items for all components
    for (auto *comp : m_circuit->components()) {
        auto *item = new ComponentGraphicsItem(comp);
        item->setPos(comp->position());
        addItem(item);
    }

    // Recreate graphics items for all wires
    for (auto *wire : m_circuit->wires()) {
        PinGraphicsItem *srcPinItem = findPinItem(wire->sourcePin());
        PinGraphicsItem *dstPinItem = findPinItem(wire->destPin());
        if (srcPinItem && dstPinItem) {
            auto *wireItem = new WireGraphicsItem(wire, srcPinItem, dstPinItem);
            addItem(wireItem);
        }
    }

    runSimulation();
}

// ---- Simulation ----

void CircuitScene::runSimulation()
{
    m_engine->simulate();
}

void CircuitScene::onSimulationComplete()
{
    for (auto *item : items()) {
        auto *wireItem = dynamic_cast<WireGraphicsItem*>(item);
        if (wireItem) {
            wireItem->updateColor();
            continue;
        }
        auto *compItem = dynamic_cast<ComponentGraphicsItem*>(item);
        if (compItem) {
            compItem->update();
        }
    }
}

// ---- Events ----

void CircuitScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !m_pendingType.isEmpty()) {
        QPointF pos = snapToGrid(event->scenePos());

        // Check if we clicked on an existing item
        QGraphicsItem *clickedItem = itemAt(event->scenePos(), QTransform());
        if (!clickedItem) {
            createComponentItem(m_pendingType, pos);
            event->accept();
            return;
        }
    }

    if (event->button() == Qt::RightButton && m_wiringActive) {
        cancelWiring();
        event->accept();
        return;
    }

    QGraphicsScene::mousePressEvent(event);
}

void CircuitScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_wiringActive && m_wiringTempLine) {
        QLineF line = m_wiringTempLine->line();
        m_wiringTempLine->setLine(line.x1(), line.y1(),
                                   event->scenePos().x(), event->scenePos().y());
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void CircuitScene::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (m_wiringActive) {
            cancelWiring();
        }
        clearPendingComponent();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        // Collect components to delete
        QSet<ComponentGraphicsItem*> compsToDelete;
        for (auto *item : selectedItems()) {
            auto *compItem = dynamic_cast<ComponentGraphicsItem*>(item);
            if (compItem)
                compsToDelete.insert(compItem);
        }

        // Find ALL wires that connect to any component being deleted,
        // plus any wires that are directly selected
        QSet<WireGraphicsItem*> wiresToDelete;
        for (auto *item : selectedItems()) {
            auto *wireItem = dynamic_cast<WireGraphicsItem*>(item);
            if (wireItem)
                wiresToDelete.insert(wireItem);
        }
        for (auto *sceneItem : items()) {
            auto *wireItem = dynamic_cast<WireGraphicsItem*>(sceneItem);
            if (wireItem) {
                if (compsToDelete.contains(wireItem->sourcePinItem()->componentItem()) ||
                    compsToDelete.contains(wireItem->destPinItem()->componentItem())) {
                    wiresToDelete.insert(wireItem);
                }
            }
        }

        // Remove all wires first (from model and scene)
        for (auto *wireItem : wiresToDelete) {
            m_circuit->removeWire(wireItem->wire());
            removeItem(wireItem);
            delete wireItem;
        }

        // Remove all components (from model and scene)
        for (auto *compItem : compsToDelete) {
            Component *comp = compItem->component();
            removeItem(compItem);
            delete compItem;
            m_circuit->removeComponentOnly(comp);
        }

        runSimulation();
        event->accept();
        return;
    }

    // R key: rotate selected components
    if (event->key() == Qt::Key_R && !(event->modifiers() & Qt::ControlModifier)) {
        QList<QGraphicsItem*> selected = selectedItems();
        for (auto *item : selected) {
            auto *compItem = dynamic_cast<ComponentGraphicsItem*>(item);
            if (compItem) {
                int newRot = (compItem->component()->rotation() + 90) % 360;
                compItem->component()->setRotation(newRot);
                compItem->setTransformOriginPoint(compItem->rect().center());
                compItem->setRotation(newRot);
            }
        }
        event->accept();
        return;
    }

    QGraphicsScene::keyPressEvent(event);
}

void CircuitScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    // Fill background
    painter->fillRect(rect, QColor(30, 30, 30));

    // Draw grid
    qreal left = std::floor(rect.left() / GridSize) * GridSize;
    qreal top = std::floor(rect.top() / GridSize) * GridSize;

    // Fine grid lines
    painter->setPen(QPen(QColor(50, 50, 50), 0.5));
    for (qreal x = left; x < rect.right(); x += GridSize) {
        painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
    }
    for (qreal y = top; y < rect.bottom(); y += GridSize) {
        painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
    }

    // Major grid lines (every 5 cells)
    qreal majorGrid = GridSize * 5;
    qreal majorLeft = std::floor(rect.left() / majorGrid) * majorGrid;
    qreal majorTop = std::floor(rect.top() / majorGrid) * majorGrid;

    painter->setPen(QPen(QColor(70, 70, 70), 1.0));
    for (qreal x = majorLeft; x < rect.right(); x += majorGrid) {
        painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
    }
    for (qreal y = majorTop; y < rect.bottom(); y += majorGrid) {
        painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
    }
}

void CircuitScene::createCustomFromSelection()
{
    // Gather selected components
    QList<Component*> selectedComps;
    for (auto *item : selectedItems()) {
        auto *compItem = dynamic_cast<ComponentGraphicsItem*>(item);
        if (compItem)
            selectedComps.append(compItem->component());
    }
    if (selectedComps.isEmpty()) return;

    // Find InputSwitches and OutputProbes
    QStringList inputLabels, outputLabels;
    for (auto *comp : selectedComps) {
        if (dynamic_cast<InputSwitch*>(comp))
            inputLabels.append(comp->label().isEmpty() ? "Input" : comp->label());
        if (dynamic_cast<OutputProbe*>(comp))
            outputLabels.append(comp->label().isEmpty() ? "Output" : comp->label());
    }

    if (inputLabels.isEmpty() && outputLabels.isEmpty()) {
        QMessageBox::warning(nullptr, "Error", "Selection must contain at least one Input Switch or Output Probe.");
        return;
    }

    // Show dialog
    QWidget *parentWidget = nullptr;
    if (!views().isEmpty())
        parentWidget = views().first();

    CreateCustomDialog dialog(inputLabels, outputLabels, parentWidget);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QString name = dialog.componentName();
    QStringList inputs = dialog.selectedInputNames();
    QStringList outputs = dialog.selectedOutputNames();

    // Create definition
    QJsonObject def = CustomComponent::createDefinition(name, m_circuit, selectedComps, inputs, outputs);

    // Save to file
    QString dir = QDir::homePath() + "/Documents/digital_design_app/components";
    QDir().mkpath(dir);
    QString filePath = dir + "/" + name + ".dccomp";

    QJsonDocument doc(def);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
    }

    QMessageBox::information(nullptr, "Success",
        QString("Custom component '%1' saved.\nRefresh the palette to use it.").arg(name));
}
