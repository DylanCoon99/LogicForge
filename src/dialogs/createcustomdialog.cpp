#include "createcustomdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>

CreateCustomDialog::CreateCustomDialog(const QStringList &inputSwitchLabels,
                                       const QStringList &outputProbeLabels,
                                       QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Create Custom Component"));
    setMinimumWidth(400);

    auto *layout = new QVBoxLayout(this);

    // Name
    layout->addWidget(new QLabel(tr("Component Name:")));
    m_nameEdit = new QLineEdit();
    m_nameEdit->setPlaceholderText(tr("e.g. Half Adder"));
    layout->addWidget(m_nameEdit);

    // Inputs
    layout->addWidget(new QLabel(tr("Inputs (from InputSwitches — check to include, edit to rename):")));
    m_inputList = new QListWidget();
    for (const QString &label : inputSwitchLabels) {
        auto *item = new QListWidgetItem(label.isEmpty() ? tr("Input") : label);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEditable);
        item->setCheckState(Qt::Checked);
        m_inputList->addItem(item);
    }
    layout->addWidget(m_inputList);

    // Outputs
    layout->addWidget(new QLabel(tr("Outputs (from OutputProbes — check to include, edit to rename):")));
    m_outputList = new QListWidget();
    for (const QString &label : outputProbeLabels) {
        auto *item = new QListWidgetItem(label.isEmpty() ? tr("Output") : label);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEditable);
        item->setCheckState(Qt::Checked);
        m_outputList->addItem(item);
    }
    layout->addWidget(m_outputList);

    // Buttons
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        if (m_nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Please enter a component name."));
            return;
        }
        if (selectedInputNames().isEmpty() && selectedOutputNames().isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Select at least one input or output."));
            return;
        }
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

QString CreateCustomDialog::componentName() const
{
    return m_nameEdit->text().trimmed();
}

QStringList CreateCustomDialog::selectedInputNames() const
{
    QStringList names;
    for (int i = 0; i < m_inputList->count(); ++i) {
        auto *item = m_inputList->item(i);
        if (item->checkState() == Qt::Checked)
            names.append(item->text());
    }
    return names;
}

QStringList CreateCustomDialog::selectedOutputNames() const
{
    QStringList names;
    for (int i = 0; i < m_outputList->count(); ++i) {
        auto *item = m_outputList->item(i);
        if (item->checkState() == Qt::Checked)
            names.append(item->text());
    }
    return names;
}
