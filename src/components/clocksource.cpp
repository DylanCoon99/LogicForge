#include "clocksource.h"
#include "canvas/circuitscene.h"

ClockSource::ClockSource()
    : Component("Clock")
    , m_running(false)
    , m_state(false)
    , m_periodMs(500)
    , m_timer(new QTimer())
{
    addOutputPin("CLK");
    m_outputs[0]->setState(Pin::Low);
}

ClockSource::~ClockSource()
{
    m_timer->stop();
    delete m_timer;
}

void ClockSource::setPeriod(int ms)
{
    m_periodMs = ms;
    if (m_running) {
        m_timer->setInterval(m_periodMs);
    }
}

void ClockSource::start(CircuitScene *scene)
{
    if (m_running) return;
    m_running = true;

    QObject::disconnect(m_timer, nullptr, nullptr, nullptr);
    QObject::connect(m_timer, &QTimer::timeout, [this, scene]() {
        m_state = !m_state;
        evaluate();
        scene->runSimulation();
    });
    m_timer->start(m_periodMs);
}

void ClockSource::stop()
{
    m_running = false;
    m_timer->stop();
}

void ClockSource::toggle(CircuitScene *scene)
{
    if (m_running)
        stop();
    else
        start(scene);
}

void ClockSource::singleStep(CircuitScene *scene)
{
    // Toggle once (one half-cycle) without starting continuous mode
    if (m_running) stop();
    m_state = !m_state;
    evaluate();
    scene->runSimulation();
}

QJsonObject ClockSource::toJson() const
{
    QJsonObject obj = Component::toJson();
    obj["periodMs"] = m_periodMs;
    return obj;
}

void ClockSource::evaluate()
{
    m_outputs[0]->setState(m_state ? Pin::High : Pin::Low);
}
