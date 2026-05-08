#ifndef COMPONENTGRAPHICSITEM_H
#define COMPONENTGRAPHICSITEM_H

#include <QGraphicsRectItem>
#include <QVector>

class Component;
class PinGraphicsItem;

class ComponentGraphicsItem : public QGraphicsRectItem
{
public:
    static constexpr qreal Width = 100.0;
    static constexpr qreal Height = 60.0;
    static constexpr qreal PinSpacing = 20.0;

    explicit ComponentGraphicsItem(Component *component);

    Component* component() const { return m_component; }
    const QVector<PinGraphicsItem*>& inputPinItems() const { return m_inputPinItems; }
    const QVector<PinGraphicsItem*>& outputPinItems() const { return m_outputPinItems; }

protected:
    QRectF boundingRect() const override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    void createPinItems();
    qreal computeHeight() const;
    bool isGate() const;

    void paintGate(QPainter *painter);
    void paintFlipFlop(QPainter *painter);
    void paintLatch(QPainter *painter);
    void paintMux(QPainter *painter);
    void paintDemux(QPainter *painter);
    void paintInputSwitch(QPainter *painter);
    void paintOutputProbe(QPainter *painter);
    void paintClockSource(QPainter *painter);
    void paintRegister(QPainter *painter);
    void paintCounter(QPainter *painter);
    void paintBus(QPainter *painter);
    void paintCustomComponent(QPainter *painter);
    void paintPinLabels(QPainter *painter);
    void paintClkTriangle(QPainter *painter, qreal x, qreal y);
    void paintBubble(QPainter *painter, qreal x, qreal y);

    Component *m_component;
    QVector<PinGraphicsItem*> m_inputPinItems;
    QVector<PinGraphicsItem*> m_outputPinItems;
};

#endif // COMPONENTGRAPHICSITEM_H
