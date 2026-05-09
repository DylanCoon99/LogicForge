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

        // 0. Save previous pin states every iteration
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

        // 2. Propagate all wires with bus resolution
        propagateWithBusResolution();

        // 3. Evaluate all components
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

    emit oscillationDetected();
    emit simulationComplete();
    return false;
}

bool SimulationEngine::simulateWithoutSavingPrevious()
{
    // Not used anymore, kept for API compatibility
    return simulate();
}

bool SimulationEngine::simulateSingleEdge()
{
    // Phase 1: Settle combinational logic with new input values.
    // Flip-flops will just hold their current state (no edges yet).
    // This ensures D inputs reach their correct values before latching.
    for (int iteration = 0; iteration < MaxIterations; ++iteration) {
        for (auto *comp : m_circuit->components()) {
            for (auto *pin : comp->inputPins())
                pin->savePreviousState();
            for (auto *pin : comp->outputPins())
                pin->savePreviousState();
        }

        QHash<Pin*, Pin::State> snapshot;
        for (auto *comp : m_circuit->components()) {
            for (auto *pin : comp->inputPins())
                snapshot[pin] = pin->state();
            for (auto *pin : comp->outputPins())
                snapshot[pin] = pin->state();
        }

        propagateWithBusResolution();
        for (auto *comp : m_circuit->components())
            comp->evaluate();

        bool changed = false;
        for (auto *comp : m_circuit->components()) {
            for (auto *pin : comp->inputPins()) {
                if (pin->state() != snapshot.value(pin)) { changed = true; break; }
            }
            if (changed) break;
            for (auto *pin : comp->outputPins()) {
                if (pin->state() != snapshot.value(pin)) { changed = true; break; }
            }
            if (changed) break;
        }
        if (!changed) break;
    }

    // Phase 2: Now do ONE pass with real edge detection.
    // Save previous states (captures the pre-edge settled state),
    // then propagate and evaluate. Flip-flops will see the real
    // clock edges and latch their D inputs.
    for (auto *comp : m_circuit->components()) {
        for (auto *pin : comp->inputPins())
            pin->savePreviousState();
        for (auto *pin : comp->outputPins())
            pin->savePreviousState();
    }
    propagateWithBusResolution();
    for (auto *comp : m_circuit->components())
        comp->evaluate();

    // Phase 3: Settle combinational logic again after flip-flop outputs changed.
    for (int iteration = 0; iteration < MaxIterations; ++iteration) {
        for (auto *comp : m_circuit->components()) {
            for (auto *pin : comp->inputPins())
                pin->savePreviousState();
            for (auto *pin : comp->outputPins())
                pin->savePreviousState();
        }

        QHash<Pin*, Pin::State> snapshot;
        for (auto *comp : m_circuit->components()) {
            for (auto *pin : comp->inputPins())
                snapshot[pin] = pin->state();
            for (auto *pin : comp->outputPins())
                snapshot[pin] = pin->state();
        }

        propagateWithBusResolution();
        for (auto *comp : m_circuit->components())
            comp->evaluate();

        bool changed = false;
        for (auto *comp : m_circuit->components()) {
            for (auto *pin : comp->inputPins()) {
                if (pin->state() != snapshot.value(pin)) { changed = true; break; }
            }
            if (changed) break;
            for (auto *pin : comp->outputPins()) {
                if (pin->state() != snapshot.value(pin)) { changed = true; break; }
            }
            if (changed) break;
        }
        if (!changed) {
            emit simulationComplete();
            return true;
        }
    }

    emit oscillationDetected();
    emit simulationComplete();
    return false;
}

Pin::State SimulationEngine::resolveBus(const QVector<Pin*> &sources)
{
    Pin::State driven = Pin::HighZ;
    for (auto *src : sources) {
        Pin::State s = src->state();
        if (s == Pin::HighZ) continue;
        if (s == Pin::Undefined) continue;
        if (driven == Pin::HighZ) {
            driven = s;
        } else if (driven != s) {
            return Pin::Undefined; // bus contention
        }
    }
    if (driven == Pin::HighZ) {
        for (auto *src : sources) {
            if (src->state() == Pin::Undefined)
                return Pin::Undefined;
        }
    }
    return driven;
}

void SimulationEngine::propagateWithBusResolution()
{
    // Collect all drivers for each destination pin
    QHash<Pin*, QVector<Pin*>> drivers;
    for (auto *wire : m_circuit->wires()) {
        drivers[wire->destPin()].append(wire->sourcePin());
    }
    // Resolve each destination pin
    for (auto it = drivers.begin(); it != drivers.end(); ++it) {
        it.key()->setState(resolveBus(it.value()));
    }
}
