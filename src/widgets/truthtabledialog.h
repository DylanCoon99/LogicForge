#ifndef TRUTHTABLEDIALOG_H
#define TRUTHTABLEDIALOG_H

#include <QDialog>
class QTableWidget;
class CircuitScene;

class TruthTableDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TruthTableDialog(CircuitScene *scene, QWidget *parent = nullptr);
private:
    void generate();
    void copyToClipboard();
    QTableWidget *m_table;
    CircuitScene *m_scene;
};
#endif
