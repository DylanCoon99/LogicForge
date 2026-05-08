#ifndef CLOCKSOURCE_H
#define CLOCKSOURCE_H

#include "model/component.h"
#include <QTimer>

class CircuitScene;

class ClockSource : public Component
{
public:
    ClockSource();
    ~ClockSource();

    bool isRunning() const { return m_running; }
    int periodMs() const { return m_periodMs; }
    void setPeriod(int ms);

    void start(CircuitScene *scene);
    void stop();
    void toggle(CircuitScene *scene);
    void singleStep(CircuitScene *scene);

    void evaluate() override;
    QJsonObject toJson() const override;

private:
    bool m_running;
    bool m_state;
    int  m_periodMs;
    QTimer *m_timer;
};

#endif // CLOCKSOURCE_H
