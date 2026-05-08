#include "timingdiagramwidget.h"
#include "canvas/circuitscene.h"
#include "model/circuit.h"
#include "model/pin.h"
#include "components/inputswitch.h"
#include "components/outputprobe.h"
#include "components/flipflop.h"

#include <QPainter>
#include <QPaintEvent>

TimingDiagramWidget::TimingDiagramWidget(QWidget *parent)
    : QWidget(parent)
    , m_scene(nullptr)
    , m_maxSteps(100)
{
    setMinimumHeight(120);
}

void TimingDiagramWidget::recordStep()
{
    if (!m_scene) return;

    Circuit *circuit = m_scene->circuit();

    // Collect signals: inputs, outputs, flip-flop Q pins
    QVector<QPair<QString, Pin::State>> signalList;

    for (auto *comp : circuit->components()) {
        if (auto *sw = dynamic_cast<InputSwitch*>(comp)) {
            QString name = sw->label().isEmpty() ? sw->type() : sw->label();
            Pin::State st = sw->outputPins().isEmpty() ? Pin::Undefined : sw->outputPins()[0]->state();
            signalList.append({name, st});
        }
    }
    for (auto *comp : circuit->components()) {
        if (auto *probe = dynamic_cast<OutputProbe*>(comp)) {
            QString name = probe->label().isEmpty() ? probe->type() : probe->label();
            signalList.append({name, probe->readState()});
        }
    }
    for (auto *comp : circuit->components()) {
        if (auto *ff = dynamic_cast<FlipFlop*>(comp)) {
            QString name = (ff->label().isEmpty() ? ff->type() : ff->label()) + " Q";
            // Q output is first output pin
            Pin::State st = ff->outputPins().isEmpty() ? Pin::Undefined : ff->outputPins()[0]->state();
            signalList.append({name, st});
        }
    }

    // Match signalList to existing traces or create new ones
    for (auto &sig : signalList) {
        bool found = false;
        for (auto &trace : m_traces) {
            if (trace.name == sig.first) {
                if (trace.history.size() >= m_maxSteps)
                    trace.history.removeFirst();
                trace.history.append(sig.second);
                found = true;
                break;
            }
        }
        if (!found) {
            SignalTrace trace;
            trace.name = sig.first;
            trace.history.append(sig.second);
            m_traces.append(trace);
        }
    }

    update();
}

void TimingDiagramWidget::clearHistory()
{
    m_traces.clear();
    update();
}

void TimingDiagramWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(30, 30, 30));

    if (m_traces.isEmpty()) {
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, tr("No timing data. Run simulation steps to record."));
        return;
    }

    const int labelWidth = 100;
    const int traceHeight = 30;
    const int stepWidth = 8;
    const int margin = 5;

    int y = margin;

    for (const auto &trace : m_traces) {
        // Draw label
        painter.setPen(Qt::white);
        QFont f = painter.font();
        f.setPointSize(9);
        painter.setFont(f);
        painter.drawText(QRect(0, y, labelWidth - 4, traceHeight), Qt::AlignVCenter | Qt::AlignRight, trace.name);

        // Draw waveform
        int x = labelWidth;
        for (int i = 0; i < trace.history.size(); ++i) {
            Pin::State st = trace.history[i];

            QColor color;
            int yHigh = y + 4;
            int yLow = y + traceHeight - 4;
            int yPos;

            switch (st) {
            case Pin::High:
                color = QColor(0, 200, 0);
                yPos = yHigh;
                break;
            case Pin::Low:
                color = QColor(100, 100, 100);
                yPos = yLow;
                break;
            case Pin::Undefined:
                color = QColor(200, 0, 0);
                yPos = (yHigh + yLow) / 2;
                break;
            case Pin::HighZ:
                color = QColor(150, 150, 150);
                yPos = (yHigh + yLow) / 2;
                break;
            }

            painter.setPen(QPen(color, 2));

            // Transition line from previous state
            if (i > 0) {
                Pin::State prevSt = trace.history[i - 1];
                int prevY;
                switch (prevSt) {
                case Pin::High: prevY = yHigh; break;
                case Pin::Low:  prevY = yLow;  break;
                default:        prevY = (yHigh + yLow) / 2; break;
                }
                if (prevY != yPos) {
                    painter.drawLine(x, prevY, x, yPos);
                }
            }

            // Horizontal segment
            painter.drawLine(x, yPos, x + stepWidth, yPos);
            x += stepWidth;
        }

        // Separator line
        painter.setPen(QPen(QColor(60, 60, 60), 1));
        painter.drawLine(labelWidth, y + traceHeight, width(), y + traceHeight);

        y += traceHeight;
    }

    setMinimumHeight(qMax(120, y + margin));
}
