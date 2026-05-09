#include "customcomponent.h"
#include "model/circuit.h"
#include "model/simulationengine.h"
#include "model/wire.h"
#include "components/inputswitch.h"
#include "components/outputprobe.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

CustomComponent::CustomComponent(const QString &name,
                                 const QJsonObject &circuitJson,
                                 const QStringList &inputNames,
                                 const QStringList &outputNames)
    : Component(QString("Custom: %1").arg(name))
    , m_customName(name)
    , m_circuitJson(circuitJson)
    , m_inputNames(inputNames)
    , m_outputNames(outputNames)
    , m_internalCircuit(nullptr)
    , m_internalEngine(nullptr)
{
    buildFromJson(circuitJson, inputNames, outputNames);
}

CustomComponent::~CustomComponent()
{
    delete m_internalEngine;
    delete m_internalCircuit;
}

void CustomComponent::buildFromJson(const QJsonObject &circuitJson,
                                     const QStringList &inputNames,
                                     const QStringList &outputNames)
{
    // Create the internal circuit from JSON
    m_internalCircuit = new Circuit();
    m_internalCircuit->fromJson(circuitJson);
    m_internalEngine = new SimulationEngine(m_internalCircuit);

    // Find InputSwitch components that match input names (by label)
    m_inputMappings.clear();
    for (const QString &inName : inputNames) {
        Component *found = nullptr;
        for (auto *comp : m_internalCircuit->components()) {
            auto *sw = dynamic_cast<InputSwitch*>(comp);
            if (sw && comp->label() == inName) {
                found = comp;
                break;
            }
        }
        // If not found by label, try matching by order among InputSwitches
        if (!found) {
            int idx = inputNames.indexOf(inName);
            int count = 0;
            for (auto *comp : m_internalCircuit->components()) {
                if (dynamic_cast<InputSwitch*>(comp)) {
                    if (count == idx) {
                        found = comp;
                        break;
                    }
                    count++;
                }
            }
        }
        m_inputMappings.append(found);
        addInputPin(inName);
    }

    // Find OutputProbe components that match output names (by label)
    m_outputMappings.clear();
    for (const QString &outName : outputNames) {
        Component *found = nullptr;
        for (auto *comp : m_internalCircuit->components()) {
            auto *probe = dynamic_cast<OutputProbe*>(comp);
            if (probe && comp->label() == outName) {
                found = comp;
                break;
            }
        }
        if (!found) {
            int idx = outputNames.indexOf(outName);
            int count = 0;
            for (auto *comp : m_internalCircuit->components()) {
                if (dynamic_cast<OutputProbe*>(comp)) {
                    if (count == idx) {
                        found = comp;
                        break;
                    }
                    count++;
                }
            }
        }
        m_outputMappings.append(found);
        addOutputPin(outName);
    }
}

void CustomComponent::evaluate()
{
    // 1. Copy external input states to internal InputSwitches
    for (int i = 0; i < m_inputs.size() && i < m_inputMappings.size(); ++i) {
        if (!m_inputMappings[i]) continue;
        auto *sw = dynamic_cast<InputSwitch*>(m_inputMappings[i]);
        if (!sw) continue;

        bool shouldBeOn = (m_inputs[i]->state() == Pin::High);
        if (sw->isOn() != shouldBeOn) {
            sw->toggle();
        } else {
            sw->evaluate();
        }
    }

    // 2. Run internal simulation with single-edge semantics:
    //    - First iteration: saves previous states, propagates, evaluates (edges fire)
    //    - Subsequent iterations: saves previous again (so edges don't re-fire),
    //      then settles combinational logic
    if (m_internalEngine)
        m_internalEngine->simulateSingleEdge();

    // 3. Read internal OutputProbe states to external output pins
    for (int i = 0; i < m_outputs.size() && i < m_outputMappings.size(); ++i) {
        if (!m_outputMappings[i]) continue;
        auto *probe = dynamic_cast<OutputProbe*>(m_outputMappings[i]);
        if (!probe) continue;

        m_outputs[i]->setState(probe->readState());
    }
}

QJsonObject CustomComponent::toJson() const
{
    QJsonObject obj = Component::toJson();
    obj["customName"] = m_customName;

    QJsonArray inputs, outputs;
    for (const auto &n : m_inputNames) inputs.append(n);
    for (const auto &n : m_outputNames) outputs.append(n);
    obj["inputs"] = inputs;
    obj["outputs"] = outputs;
    obj["circuit"] = m_circuitJson;

    return obj;
}

CustomComponent* CustomComponent::fromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return nullptr;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError)
        return nullptr;

    QJsonObject obj = doc.object();
    QString name = obj["name"].toString();
    QJsonObject circuitJson = obj["circuit"].toObject();

    QStringList inputNames, outputNames;
    for (const auto &v : obj["inputs"].toArray())
        inputNames.append(v.toString());
    for (const auto &v : obj["outputs"].toArray())
        outputNames.append(v.toString());

    return new CustomComponent(name, circuitJson, inputNames, outputNames);
}

void CustomComponent::saveToFile(const QString &filePath) const
{
    QJsonObject obj;
    obj["name"] = m_customName;

    QJsonArray inputs, outputs;
    for (const auto &n : m_inputNames) inputs.append(n);
    for (const auto &n : m_outputNames) outputs.append(n);
    obj["inputs"] = inputs;
    obj["outputs"] = outputs;
    obj["circuit"] = m_circuitJson;

    QJsonDocument doc(obj);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
    }
}

QJsonObject CustomComponent::createDefinition(const QString &name,
                                               Circuit *sourceCircuit,
                                               const QList<Component*> &components,
                                               const QStringList &inputLabels,
                                               const QStringList &outputLabels)
{
    // Build a sub-circuit JSON from the selected components and their internal wires
    QSet<QUuid> compIds;
    for (auto *c : components)
        compIds.insert(c->id());

    QJsonArray compsArray;
    for (auto *c : components) {
        QJsonObject cj = c->toJson();
        // Label InputSwitches and OutputProbes with their external pin names
        auto *sw = dynamic_cast<InputSwitch*>(c);
        if (sw) {
            int idx = 0;
            for (auto *comp : components) {
                if (comp == c) break;
                if (dynamic_cast<InputSwitch*>(comp)) idx++;
            }
            if (idx < inputLabels.size())
                cj["label"] = inputLabels[idx];
        }
        auto *probe = dynamic_cast<OutputProbe*>(c);
        if (probe) {
            int idx = 0;
            for (auto *comp : components) {
                if (comp == c) break;
                if (dynamic_cast<OutputProbe*>(comp)) idx++;
            }
            if (idx < outputLabels.size())
                cj["label"] = outputLabels[idx];
        }
        compsArray.append(cj);
    }

    // Find wires where both endpoints are in the selection
    QJsonArray wiresArray;
    for (auto *w : sourceCircuit->wires()) {
        QUuid srcId = w->sourcePin()->parentComponent()->id();
        QUuid dstId = w->destPin()->parentComponent()->id();
        if (compIds.contains(srcId) && compIds.contains(dstId)) {
            QJsonObject wj;
            wj["srcId"] = srcId.toString();
            wj["srcPin"] = w->sourcePin()->parentComponent()->outputPins().indexOf(w->sourcePin());
            wj["dstId"] = dstId.toString();
            wj["dstPin"] = w->destPin()->parentComponent()->inputPins().indexOf(w->destPin());
            wiresArray.append(wj);
        }
    }

    QJsonObject circuitJson;
    circuitJson["version"] = "1.0";
    circuitJson["components"] = compsArray;
    circuitJson["wires"] = wiresArray;

    QJsonObject def;
    def["name"] = name;
    QJsonArray inputs, outputs;
    for (const auto &n : inputLabels) inputs.append(n);
    for (const auto &n : outputLabels) outputs.append(n);
    def["inputs"] = inputs;
    def["outputs"] = outputs;
    def["circuit"] = circuitJson;

    return def;
}
