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

signals:
    void simulationComplete();
    void oscillationDetected();

private:
    Circuit *m_circuit;
};

#endif // SIMULATIONENGINE_H
