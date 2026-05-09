#ifndef SIMULATIONENGINE_H
#define SIMULATIONENGINE_H

#include <QObject>

class Circuit;

class SimulationEngine : public QObject
{
    Q_OBJECT

public:
    explicit SimulationEngine(Circuit *circuit, QObject *parent = nullptr);

    static constexpr int MaxIterations = 100;

    // Run simulation until all signals settle or MaxIterations reached.
    // Returns true if circuit settled, false if oscillation detected.
    bool simulate();

    // Run simulation without saving previous pin states first.
    // Used by CustomComponent which manages edge detection itself.
    bool simulateWithoutSavingPrevious();

    // Run a single-shot simulation: only the first iteration detects edges,
    // subsequent iterations settle combinational logic without re-triggering edges.
    bool simulateSingleEdge();

signals:
    void simulationComplete();
    void oscillationDetected();

private:
    Circuit *m_circuit;
};

#endif // SIMULATIONENGINE_H
