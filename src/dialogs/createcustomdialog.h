#ifndef CREATECUSTOMDIALOG_H
#define CREATECUSTOMDIALOG_H

#include <QDialog>
#include <QStringList>

class QLineEdit;
class QListWidget;

class CreateCustomDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateCustomDialog(const QStringList &inputSwitchLabels,
                                 const QStringList &outputProbeLabels,
                                 QWidget *parent = nullptr);

    QString componentName() const;
    QStringList selectedInputNames() const;
    QStringList selectedOutputNames() const;

private:
    QLineEdit   *m_nameEdit;
    QListWidget *m_inputList;
    QListWidget *m_outputList;
};

#endif // CREATECUSTOMDIALOG_H
