#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class CircuitScene;
class CircuitView;
class QListWidget;
class QListWidgetItem;
class QUndoStack;
class TimingDiagramWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void newCircuit();
    void onPaletteItemClicked(int row);
    void saveCircuit();
    void saveCircuitAs();
    void openCircuit();
    void exportPng();
    void exportSvg();
    void showTruthTable();

private:
    void setupMenuBar();
    void setupSidebar();
    void setupCanvas();
    void populatePalette();
    void loadCustomComponents();
    void updateWindowTitle();
    QString defaultCircuitsDir() const;

    void addPaletteHeader(const QString &title);

    CircuitScene *m_scene;
    CircuitView  *m_view;
    QListWidget  *m_palette;
    QUndoStack   *m_undoStack;
    TimingDiagramWidget *m_timingDiagram;
    QString       m_currentFile;
};

#endif // MAINWINDOW_H
