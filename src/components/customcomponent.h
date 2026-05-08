#ifndef CUSTOMCOMPONENT_H
#define CUSTOMCOMPONENT_H

#include "model/component.h"
#include <QStringList>

class Circuit;
class SimulationEngine;

class CustomComponent : public Component
{
public:
    // Construct from an internal circuit definition
    CustomComponent(const QString &name,
                    const QJsonObject &circuitJson,
                    const QStringList &inputNames,
                    const QStringList &outputNames);
    ~CustomComponent();

    const QString& customName() const { return m_customName; }
    Circuit* internalCircuit() const { return m_internalCircuit; }

    void evaluate() override;
    QJsonObject toJson() const override;

    // Load from a .dccomp file on disk
    static CustomComponent* fromFile(const QString &filePath);

    // Save component definition to a .dccomp file
    void saveToFile(const QString &filePath) const;

    // Build a definition JSON from selected components
    static QJsonObject createDefinition(const QString &name,
                                         Circuit *sourceCircuit,
                                         const QList<Component*> &components,
                                         const QStringList &inputLabels,
                                         const QStringList &outputLabels);

private:
    void buildFromJson(const QJsonObject &circuitJson,
                       const QStringList &inputNames,
                       const QStringList &outputNames);

    QString m_customName;
    QJsonObject m_circuitJson;       // stored for serialization
    QStringList m_inputNames;
    QStringList m_outputNames;

    Circuit *m_internalCircuit;
    SimulationEngine *m_internalEngine;

    // Internal InputSwitch components mapped to external input pins
    QVector<Component*> m_inputMappings;
    // Internal OutputProbe components mapped to external output pins
    QVector<Component*> m_outputMappings;
};

#endif // CUSTOMCOMPONENT_H
