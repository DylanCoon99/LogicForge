#include "simulationengine.h"
#include "circuit.h"
#include "component.h"
#include "wire.h"
#include "pin.h"

#include <QHash>

SimulationEngine::SimulationEngine(Circuit *circuit, QObject *parent)
    : QObject(parent)
    , m_circuit(circuit)
{
}

bool SimulationEngine::simulate()
{
    for (int iteration = 0; iteration < MaxIterations; ++iteration) {

        // 0. Save previous pin states (for edge detection)
        for (auto *comp : m_circuit->components()) {
            for (auto *pin : comp->inputPins())
                pin->savePreviousState();
            for (auto *pin : comp->outputPins())
                pin->savePreviousState();
        }

        // 1. Snapshot all pin states
        QHash<Pin*, Pin::State> snapshot;
        for (auto *comp : m_circuit->components()) {
            for (auto *pin : comp->inputPins())
                snapshot[pin] = pin->state();
            for (auto *pin : comp->outputPins())
                snapshot[pin] = pin->state();
        }

        // 2. Propagate all wires (push output states to input pins)
        for (auto *wire : m_circuit->wires()) {
            wire->propagate();
        }

        // 3. Evaluate all components (compute outputs from inputs)
        for (auto *comp : m_circuit->components()) {
            comp->evaluate();
        }

        // 4. Check if anything changed
        bool changed = false;
        for (auto *comp : m_circuit->components()) {
            for (auto *pin : comp->inputPins()) {
                if (pin->state() != snapshot.value(pin)) {
                    changed = true;
                    break;
                }
            }
            if (changed) break;
            for (auto *pin : comp->outputPins()) {
                if (pin->state() != snapshot.value(pin)) {
                    changed = true;
                    break;
                }
            }
            if (changed) break;
        }

        if (!changed) {
            emit simulationComplete();
            return true;
        }
    }

    // Oscillation detected — didn't settle within MaxIterations
    emit oscillationDetected();
    emit simulationComplete();
    return false;
}
