#include "component.h"
#include "components/gatecomponent.h"
#include "components/customcomponent.h"
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
#include "components/tristatebuffer.h"

#include <QJsonArray>

Component::Component(const QString &type)
    : m_id(QUuid::createUuid())
    , m_type(type)
    , m_rotation(0)
{
}

Component::~Component()
{
    qDeleteAll(m_inputs);
    qDeleteAll(m_outputs);
}

void Component::addInputPin(const QString &name)
{
    m_inputs.append(new Pin(name, Pin::Input, this));
}

void Component::addOutputPin(const QString &name)
{
    m_outputs.append(new Pin(name, Pin::Output, this));
}

QJsonObject Component::toJson() const
{
    QJsonObject obj;
    obj["id"] = m_id.toString();
    obj["type"] = m_type;
    obj["x"] = m_position.x();
    obj["y"] = m_position.y();
    obj["rotation"] = m_rotation;
    if (!m_label.isEmpty())
        obj["label"] = m_label;
    return obj;
}

Component* Component::fromJson(const QJsonObject &obj)
{
    QString type = obj["type"].toString();
    Component *comp = nullptr;

    // Gates
    if (type == "AND" || type == "OR" || type == "NOT" ||
        type == "NAND" || type == "NOR" || type == "XOR" || type == "XNOR") {
        auto gateType = GateComponent::gateTypeFromName(type);
        int numInputs = obj.contains("numInputs") ? obj["numInputs"].toInt() : 2;
        comp = new GateComponent(gateType, numInputs);
    }
    // I/O
    else if (type == "Input Switch") {
        auto *sw = new InputSwitch();
        if (obj.contains("on") && obj["on"].toBool())
            sw->toggle();
        comp = sw;
    }
    else if (type == "Output Probe") {
        comp = new OutputProbe();
    }
    else if (type == "Clock") {
        auto *clk = new ClockSource();
        if (obj.contains("periodMs"))
            clk->setPeriod(obj["periodMs"].toInt());
        comp = clk;
    }
    // Flip-Flops
    else if (type.contains("Flip-Flop")) {
        FlipFlop::Type ffType = FlipFlop::D;
        if (type.startsWith("SR")) ffType = FlipFlop::SR;
        else if (type.startsWith("JK")) ffType = FlipFlop::JK;
        else if (type.startsWith("T ") || type == "T Flip-Flop") ffType = FlipFlop::T;
        comp = new FlipFlop(ffType);
    }
    // Latches
    else if (type.contains("Latch")) {
        Latch::Type lType = Latch::D;
        if (type.startsWith("SR")) lType = Latch::SR;
        comp = new Latch(lType);
    }
    // Mux/Demux
    else if (type.startsWith("Mux")) {
        int n = obj.contains("numInputs") ? obj["numInputs"].toInt() : 2;
        comp = new Mux(n);
    }
    else if (type.startsWith("Demux")) {
        int n = obj.contains("numOutputs") ? obj["numOutputs"].toInt() : 2;
        comp = new Demux(n);
    }
    // Register/Counter
    else if (type.contains("Register")) {
        int bits = obj.contains("bitWidth") ? obj["bitWidth"].toInt() : 8;
        comp = new RegisterComponent(bits);
    }
    else if (type.contains("Counter")) {
        int bits = obj.contains("bitWidth") ? obj["bitWidth"].toInt() : 4;
        comp = new Counter(bits);
    }
    // Bus
    else if (type.contains("Bus Split")) {
        int bits = obj.contains("bitWidth") ? obj["bitWidth"].toInt() : 8;
        comp = new BusSplitter(bits);
    }
    else if (type.contains("Bus Join")) {
        int bits = obj.contains("bitWidth") ? obj["bitWidth"].toInt() : 8;
        comp = new BusJoiner(bits);
    }
    else if (type == "Tri-State Buffer" || type == "Tri-State Inverter") {
        bool inv = obj.contains("inverting") ? obj["inverting"].toBool() : type.contains("Inverter");
        comp = new TriStateBuffer(inv);
    }
    else if (type.startsWith("Custom:")) {
        QString customName = obj["customName"].toString();
        QJsonObject circuitJson = obj["circuit"].toObject();
        QStringList inputNames, outputNames;
        for (const auto &v : obj["inputs"].toArray())
            inputNames.append(v.toString());
        for (const auto &v : obj["outputs"].toArray())
            outputNames.append(v.toString());
        comp = new CustomComponent(customName, circuitJson, inputNames, outputNames);
    }

    if (!comp) return nullptr;

    if (obj.contains("id"))
        comp->setId(QUuid(obj["id"].toString()));
    comp->setPosition(QPointF(obj["x"].toDouble(), obj["y"].toDouble()));
    comp->setRotation(obj["rotation"].toInt(0));
    if (obj.contains("label"))
        comp->setLabel(obj["label"].toString());

    return comp;
}
