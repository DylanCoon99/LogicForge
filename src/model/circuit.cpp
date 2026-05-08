#include "circuit.h"
#include <QJsonArray>

Circuit::Circuit() = default;

Circuit::~Circuit()
{
    clear();
}

void Circuit::addComponent(Component *comp)
{
    m_components.append(comp);
}

void Circuit::removeComponent(Component *comp)
{
    // Remove all wires connected to this component first
    auto connectedWires = wiresForComponent(comp);
    for (auto *w : connectedWires) {
        removeWire(w);
    }
    m_components.removeOne(comp);
    delete comp;
}

void Circuit::addWire(Wire *wire)
{
    m_wires.append(wire);
}

void Circuit::removeWire(Wire *wire)
{
    m_wires.removeOne(wire);
    delete wire;
}

QList<Wire*> Circuit::wiresForComponent(Component *comp) const
{
    QList<Wire*> result;
    for (auto *w : m_wires) {
        if (w->sourcePin()->parentComponent() == comp ||
            w->destPin()->parentComponent() == comp) {
            result.append(w);
        }
    }
    return result;
}

QList<Wire*> Circuit::wiresForPin(Pin *pin) const
{
    QList<Wire*> result;
    for (auto *w : m_wires) {
        if (w->sourcePin() == pin || w->destPin() == pin) {
            result.append(w);
        }
    }
    return result;
}

void Circuit::clear()
{
    qDeleteAll(m_wires);
    m_wires.clear();
    qDeleteAll(m_components);
    m_components.clear();
}

QJsonObject Circuit::toJson() const
{
    QJsonArray compsArray;
    for (auto *comp : m_components) {
        compsArray.append(comp->toJson());
    }

    QJsonArray wiresArray;
    for (auto *w : m_wires) {
        QJsonObject wireObj;
        wireObj["srcId"] = w->sourcePin()->parentComponent()->id().toString();
        wireObj["srcPin"] = w->sourcePin()->parentComponent()->outputPins().indexOf(w->sourcePin());
        wireObj["dstId"] = w->destPin()->parentComponent()->id().toString();
        wireObj["dstPin"] = w->destPin()->parentComponent()->inputPins().indexOf(w->destPin());
        wiresArray.append(wireObj);
    }

    QJsonObject obj;
    obj["version"] = "1.0";
    obj["components"] = compsArray;
    obj["wires"] = wiresArray;
    return obj;
}

void Circuit::fromJson(const QJsonObject &obj)
{
    clear();

    // Load components
    QJsonArray compsArray = obj["components"].toArray();
    for (const auto &val : compsArray) {
        Component *comp = Component::fromJson(val.toObject());
        if (comp)
            addComponent(comp);
    }

    // Load wires
    QJsonArray wiresArray = obj["wires"].toArray();
    for (const auto &val : wiresArray) {
        QJsonObject wireObj = val.toObject();
        QUuid srcId(wireObj["srcId"].toString());
        int srcPinIdx = wireObj["srcPin"].toInt();
        QUuid dstId(wireObj["dstId"].toString());
        int dstPinIdx = wireObj["dstPin"].toInt();

        // Find components by ID
        Component *srcComp = nullptr;
        Component *dstComp = nullptr;
        for (auto *comp : m_components) {
            if (comp->id() == srcId) srcComp = comp;
            if (comp->id() == dstId) dstComp = comp;
        }

        if (srcComp && dstComp &&
            srcPinIdx < srcComp->outputPins().size() &&
            dstPinIdx < dstComp->inputPins().size()) {
            auto *wire = new Wire(srcComp->outputPins()[srcPinIdx],
                                   dstComp->inputPins()[dstPinIdx]);
            addWire(wire);
        }
    }
}
