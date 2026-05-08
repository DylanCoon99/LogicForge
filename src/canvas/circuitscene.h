#ifndef CIRCUITSCENE_H
#define CIRCUITSCENE_H

#include <QGraphicsScene>
#include <QJsonObject>
#include <QStringList>
#include "model/circuit.h"

class ComponentGraphicsItem;
class WireGraphicsItem;
class PinGraphicsItem;
class SimulationEngine;
class QUndoStack;

class CircuitScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit CircuitScene(QObject *parent = nullptr);
    ~CircuitScene();

    Circuit* circuit() const { return m_circuit; }

    void setPendingComponentType(const QString &type);
    QString pendingComponentType() const { return m_pendingType; }
    void clearPendingComponent();

    // Wiring
    void startWiring(PinGraphicsItem *sourcePin);
    void completeWiring(PinGraphicsItem *destPin);
    void cancelWiring();
    bool isWiring() const { return m_wiringActive; }

    void removeComponentItem(ComponentGraphicsItem *item);
    void removeWireItem(WireGraphicsItem *item);

    void runSimulation();

    // Copy/Paste/Select
    void copySelection();
    void pasteClipboard();
    void cutSelection();
    void selectAll();

    // Load from JSON
    void loadCircuit(const QJsonObject &obj);

    // Undo
    void setUndoStack(QUndoStack *stack) { m_undoStack = stack; }
    QUndoStack *undoStack() const { return m_undoStack; }

    // Helpers to find graphics items by model pointer
    ComponentGraphicsItem* findComponentItem(Component *comp) const;
    PinGraphicsItem* findPinItem(Pin *pin) const;

    ComponentGraphicsItem* createComponentItem(const QString &type, const QPointF &pos);

    void createCustomFromSelection();

    static constexpr qreal GridSize = 20.0;

    static qreal snapToGrid(qreal val);
    static QPointF snapToGrid(const QPointF &pos);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private slots:
    void onSimulationComplete();

private:
    Circuit *m_circuit;
    SimulationEngine *m_engine;
    QString  m_pendingType;
    QUndoStack *m_undoStack;

    QJsonObject m_clipboard;

    // Wiring state
    bool m_wiringActive;
    PinGraphicsItem *m_wiringSourcePin;
    QGraphicsLineItem *m_wiringTempLine;
};

#endif // CIRCUITSCENE_H
