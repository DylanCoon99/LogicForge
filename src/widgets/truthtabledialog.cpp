#include "truthtabledialog.h"
#include "canvas/circuitscene.h"
#include "model/circuit.h"
#include "model/pin.h"
#include "components/inputswitch.h"
#include "components/outputprobe.h"
#include "model/simulationengine.h"

#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QClipboard>
#include <QApplication>
#include <QLabel>

TruthTableDialog::TruthTableDialog(CircuitScene *scene, QWidget *parent)
    : QDialog(parent)
    , m_table(new QTableWidget(this))
    , m_scene(scene)
{
    setWindowTitle(tr("Truth Table"));
    resize(500, 400);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_table);

    auto *btnLayout = new QHBoxLayout();
    auto *copyBtn = new QPushButton(tr("Copy to Clipboard"), this);
    connect(copyBtn, &QPushButton::clicked, this, &TruthTableDialog::copyToClipboard);
    btnLayout->addStretch();
    btnLayout->addWidget(copyBtn);
    auto *closeBtn = new QPushButton(tr("Close"), this);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    generate();
}

void TruthTableDialog::generate()
{
    Circuit *circuit = m_scene->circuit();

    // Collect input switches and output probes
    QVector<InputSwitch*> inputs;
    QVector<OutputProbe*> outputs;

    for (auto *comp : circuit->components()) {
        if (auto *sw = dynamic_cast<InputSwitch*>(comp))
            inputs.append(sw);
        else if (auto *probe = dynamic_cast<OutputProbe*>(comp))
            outputs.append(probe);
    }

    if (inputs.isEmpty() || outputs.isEmpty()) {
        m_table->setRowCount(1);
        m_table->setColumnCount(1);
        m_table->setItem(0, 0, new QTableWidgetItem(tr("Need at least 1 input and 1 output")));
        return;
    }

    int n = inputs.size();
    if (n > 10) {
        m_table->setRowCount(1);
        m_table->setColumnCount(1);
        m_table->setItem(0, 0, new QTableWidgetItem(tr("Too many inputs (max 10)")));
        return;
    }

    // Save original states
    QVector<bool> originalStates;
    for (auto *sw : inputs)
        originalStates.append(sw->isOn());

    int numRows = 1 << n;
    int numCols = n + outputs.size();

    m_table->setRowCount(numRows);
    m_table->setColumnCount(numCols);

    // Header labels
    QStringList headers;
    for (auto *sw : inputs) {
        QString name = sw->label().isEmpty() ? sw->type() : sw->label();
        headers << name;
    }
    for (auto *probe : outputs) {
        QString name = probe->label().isEmpty() ? probe->type() : probe->label();
        headers << name;
    }
    m_table->setHorizontalHeaderLabels(headers);

    // Create a temporary simulation engine
    SimulationEngine engine(circuit, nullptr);

    for (int row = 0; row < numRows; ++row) {
        // Set input switches to bit pattern
        for (int bit = 0; bit < n; ++bit) {
            bool val = (row >> (n - 1 - bit)) & 1;
            // Set the switch state directly
            if (inputs[bit]->isOn() != val)
                inputs[bit]->toggle();
        }

        // Run simulation
        engine.simulate();

        // Read input columns
        for (int bit = 0; bit < n; ++bit) {
            auto *item = new QTableWidgetItem(QString::number((row >> (n - 1 - bit)) & 1));
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            m_table->setItem(row, bit, item);
        }

        // Read output columns
        for (int o = 0; o < outputs.size(); ++o) {
            Pin::State st = outputs[o]->readState();
            QString text;
            switch (st) {
            case Pin::High:      text = "1"; break;
            case Pin::Low:       text = "0"; break;
            case Pin::Undefined: text = "X"; break;
            case Pin::HighZ:     text = "Z"; break;
            }
            auto *item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            m_table->setItem(row, n + o, item);
        }
    }

    // Restore original states
    for (int i = 0; i < inputs.size(); ++i) {
        if (inputs[i]->isOn() != originalStates[i])
            inputs[i]->toggle();
    }
    // Re-run simulation to restore circuit state
    m_scene->runSimulation();

    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->resizeColumnsToContents();
}

void TruthTableDialog::copyToClipboard()
{
    QString text;

    // Header
    for (int c = 0; c < m_table->columnCount(); ++c) {
        if (c > 0) text += '\t';
        text += m_table->horizontalHeaderItem(c)->text();
    }
    text += '\n';

    // Rows
    for (int r = 0; r < m_table->rowCount(); ++r) {
        for (int c = 0; c < m_table->columnCount(); ++c) {
            if (c > 0) text += '\t';
            auto *item = m_table->item(r, c);
            if (item) text += item->text();
        }
        text += '\n';
    }

    QApplication::clipboard()->setText(text);
}
