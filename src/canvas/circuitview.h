#ifndef CIRCUITVIEW_H
#define CIRCUITVIEW_H

#include <QGraphicsView>

class CircuitScene;

class CircuitView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CircuitView(CircuitScene *scene, QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    bool m_panning;
    bool m_spaceHeld;
    QPoint m_panStart;
    double m_zoomFactor;
};

#endif // CIRCUITVIEW_H
