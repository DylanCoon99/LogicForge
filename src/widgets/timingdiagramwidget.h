#ifndef TIMINGDIAGRAMWIDGET_H
#define TIMINGDIAGRAMWIDGET_H

#include <QWidget>
#include <QVector>
#include <QHash>
#include <QString>
#include "model/pin.h"

class CircuitScene;

class TimingDiagramWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TimingDiagramWidget(QWidget *parent = nullptr);

    void setScene(CircuitScene *scene) { m_scene = scene; }

public slots:
    void recordStep();
    void clearHistory();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    CircuitScene *m_scene;
    struct SignalTrace {
        QString name;
        QVector<Pin::State> history;
    };
    QVector<SignalTrace> m_traces;
    int m_maxSteps;
};
#endif
