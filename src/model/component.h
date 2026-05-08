#ifndef COMPONENT_H
#define COMPONENT_H

#include <QString>
#include <QUuid>
#include <QPointF>
#include <QVector>
#include <QJsonObject>
#include "pin.h"

class Component
{
public:
    Component(const QString &type);
    virtual ~Component();

    QUuid id() const { return m_id; }
    void setId(const QUuid &id) { m_id = id; }
    const QString& type() const { return m_type; }

    QPointF position() const { return m_position; }
    void setPosition(const QPointF &pos) { m_position = pos; }

    int rotation() const { return m_rotation; }
    void setRotation(int degrees) { m_rotation = degrees % 360; }

    const QString& label() const { return m_label; }
    void setLabel(const QString &label) { m_label = label; }

    const QVector<Pin*>& inputPins() const { return m_inputs; }
    const QVector<Pin*>& outputPins() const { return m_outputs; }

    virtual void evaluate() = 0;
    virtual QJsonObject toJson() const;
    static Component* fromJson(const QJsonObject &obj);

protected:
    void addInputPin(const QString &name);
    void addOutputPin(const QString &name);

    QUuid   m_id;
    QString m_type;
    QPointF m_position;
    int     m_rotation;
    QString m_label;
    QVector<Pin*> m_inputs;
    QVector<Pin*> m_outputs;
};

#endif // COMPONENT_H
