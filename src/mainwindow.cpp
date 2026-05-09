#include "mainwindow.h"
#include "canvas/circuitscene.h"
#include "canvas/circuitview.h"
#include "widgets/truthtabledialog.h"
#include "widgets/timingdiagramwidget.h"

#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenuBar>
#include <QAction>
#include <QApplication>
#include <QSplitter>
#include <QLabel>
#include <QFileDialog>
#include <QJsonDocument>
#include <QUndoStack>
#include <QImage>
#include <QPainter>
#include <QSvgGenerator>
#include <QMessageBox>
#include <QDockWidget>
#include <QDir>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_scene(nullptr)
    , m_view(nullptr)
    , m_palette(nullptr)
    , m_undoStack(nullptr)
    , m_timingDiagram(nullptr)
{
    m_undoStack = new QUndoStack(this);

    setupCanvas();
    setupSidebar();
    setupMenuBar();

    // Timing diagram dock
    m_timingDiagram = new TimingDiagramWidget(this);
    m_timingDiagram->setScene(m_scene);
    auto *dock = new QDockWidget(tr("Timing Diagram"), this);
    dock->setWidget(m_timingDiagram);
    addDockWidget(Qt::BottomDockWidgetArea, dock);
    dock->hide();

    // Layout: splitter with sidebar on left, canvas on right
    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(m_palette);
    splitter->addWidget(m_view);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({200, 1000});

    setCentralWidget(splitter);
    updateWindowTitle();
}

MainWindow::~MainWindow() = default;

QString MainWindow::defaultCircuitsDir() const
{
    QString dir = QDir::homePath() + "/Documents/digital_design_app/circuits";
    QDir().mkpath(dir);
    return dir;
}

void MainWindow::updateWindowTitle()
{
    QString name = m_currentFile.isEmpty() ? tr("Untitled") : QFileInfo(m_currentFile).fileName();
    setWindowTitle(QString("%1 - Digital Design").arg(name));
}

void MainWindow::setupMenuBar()
{
    // --- File menu ---
    auto *fileMenu = menuBar()->addMenu(tr("&File"));

    auto *newAction = fileMenu->addAction(tr("&New"), this, &MainWindow::newCircuit);
    newAction->setShortcut(QKeySequence::New);

    auto *openAction = fileMenu->addAction(tr("&Open..."), this, &MainWindow::openCircuit);
    openAction->setShortcut(QKeySequence(tr("Ctrl+O")));

    fileMenu->addSeparator();

    auto *saveAction = fileMenu->addAction(tr("&Save"), this, &MainWindow::saveCircuit);
    saveAction->setShortcut(QKeySequence(tr("Ctrl+S")));

    auto *saveAsAction = fileMenu->addAction(tr("Save &As..."), this, &MainWindow::saveCircuitAs);
    saveAsAction->setShortcut(QKeySequence(tr("Ctrl+Shift+S")));

    fileMenu->addSeparator();

    fileMenu->addAction(tr("Export &PNG..."), this, &MainWindow::exportPng);
    fileMenu->addAction(tr("Export S&VG..."), this, &MainWindow::exportSvg);

    fileMenu->addSeparator();

    auto *quitAction = fileMenu->addAction(tr("&Quit"), qApp, &QApplication::quit);
    quitAction->setShortcut(QKeySequence::Quit);

    // --- Edit menu ---
    auto *editMenu = menuBar()->addMenu(tr("&Edit"));

    auto *undoAction = m_undoStack->createUndoAction(this, tr("&Undo"));
    undoAction->setShortcut(QKeySequence(tr("Ctrl+Z")));
    editMenu->addAction(undoAction);

    auto *redoAction = m_undoStack->createRedoAction(this, tr("&Redo"));
    redoAction->setShortcut(QKeySequence(tr("Ctrl+Y")));
    editMenu->addAction(redoAction);

    editMenu->addSeparator();

    auto *cutAction = editMenu->addAction(tr("Cu&t"), [this]() { m_scene->cutSelection(); });
    cutAction->setShortcut(QKeySequence(tr("Ctrl+X")));

    auto *copyAction = editMenu->addAction(tr("&Copy"), [this]() { m_scene->copySelection(); });
    copyAction->setShortcut(QKeySequence(tr("Ctrl+C")));

    auto *pasteAction = editMenu->addAction(tr("&Paste"), [this]() { m_scene->pasteClipboard(); });
    pasteAction->setShortcut(QKeySequence(tr("Ctrl+V")));

    editMenu->addSeparator();

    auto *selectAllAction = editMenu->addAction(tr("Select &All"), [this]() { m_scene->selectAll(); });
    selectAllAction->setShortcut(QKeySequence(tr("Ctrl+A")));

    // --- Tools menu ---
    auto *toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(tr("&Truth Table..."), this, &MainWindow::showTruthTable);
    toolsMenu->addAction(tr("&Refresh Components"), this, &MainWindow::loadCustomComponents);

    // Timing diagram visibility toggle
    auto *viewMenu = menuBar()->addMenu(tr("&View"));
    for (auto *d : findChildren<QDockWidget*>()) {
        viewMenu->addAction(d->toggleViewAction());
    }
}

void MainWindow::setupSidebar()
{
    m_palette = new QListWidget();
    m_palette->setDragEnabled(false);
    m_palette->setMaximumWidth(220);
    m_palette->setMinimumWidth(150);

    populatePalette();

    connect(m_palette, &QListWidget::currentRowChanged,
            this, &MainWindow::onPaletteItemClicked);
}

void MainWindow::setupCanvas()
{
    m_scene = new CircuitScene(this);
    m_scene->setUndoStack(m_undoStack);
    m_view = new CircuitView(m_scene, this);
}

void MainWindow::addPaletteHeader(const QString &title)
{
    auto *item = new QListWidgetItem(title);
    item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
    QFont font = item->font();
    font.setBold(true);
    font.setPointSize(9);
    item->setFont(font);
    item->setForeground(QColor(120, 180, 255));
    item->setBackground(QColor(40, 40, 50));
    m_palette->addItem(item);
}

void MainWindow::populatePalette()
{
    addPaletteHeader("Logic Gates");
    m_palette->addItem("AND Gate");
    m_palette->addItem("OR Gate");
    m_palette->addItem("NOT Gate");
    m_palette->addItem("NAND Gate");
    m_palette->addItem("NOR Gate");
    m_palette->addItem("XOR Gate");
    m_palette->addItem("XNOR Gate");

    addPaletteHeader("I/O");
    m_palette->addItem("Input Switch");
    m_palette->addItem("Output Probe");
    m_palette->addItem("Clock Source");

    addPaletteHeader("Flip-Flops");
    m_palette->addItem("D Flip-Flop");
    m_palette->addItem("SR Flip-Flop");
    m_palette->addItem("JK Flip-Flop");
    m_palette->addItem("T Flip-Flop");

    addPaletteHeader("Latches");
    m_palette->addItem("D Latch");
    m_palette->addItem("SR Latch");

    addPaletteHeader("Mux / Demux");
    m_palette->addItem("Mux 2:1");
    m_palette->addItem("Mux 4:1");
    m_palette->addItem("Demux 1:2");
    m_palette->addItem("Demux 1:4");

    addPaletteHeader("Registers / Counters");
    m_palette->addItem("Register (8-bit)");
    m_palette->addItem("Counter (4-bit)");

    addPaletteHeader("Bus");
    m_palette->addItem("Bus Splitter (8)");
    m_palette->addItem("Bus Joiner (8)");

    loadCustomComponents();
}

void MainWindow::loadCustomComponents()
{
    // Remove existing custom items
    for (int i = m_palette->count() - 1; i >= 0; --i) {
        auto *item = m_palette->item(i);
        if (item->text().startsWith("Custom:") || item->text() == "Custom ICs")
            delete m_palette->takeItem(i);
    }

    // Scan components directory
    QString dir = QDir::homePath() + "/Documents/digital_design_app/components";
    QDir compDir(dir);
    if (!compDir.exists()) return;

    QStringList files = compDir.entryList(QStringList() << "*.dccomp", QDir::Files);
    if (files.isEmpty()) return;

    addPaletteHeader("Custom ICs");
    for (const QString &f : files) {
        QString name = QFileInfo(f).baseName();
        m_palette->addItem("Custom: " + name);
    }
}

void MainWindow::newCircuit()
{
    m_scene->clear();
    m_currentFile.clear();
    m_undoStack->clear();
    updateWindowTitle();
}

void MainWindow::onPaletteItemClicked(int row)
{
    if (row < 0) return;
    auto *item = m_palette->item(row);
    if (!item || !(item->flags() & Qt::ItemIsEnabled)) return;
    m_scene->setPendingComponentType(item->text());
}

// --- Save / Load ---

void MainWindow::saveCircuit()
{
    if (m_currentFile.isEmpty()) {
        saveCircuitAs();
        return;
    }

    QJsonDocument doc(m_scene->circuit()->toJson());
    QFile file(m_currentFile);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot write file: %1").arg(file.errorString()));
        return;
    }
    file.write(doc.toJson());
    updateWindowTitle();
}

void MainWindow::saveCircuitAs()
{
    QString startDir = m_currentFile.isEmpty() ? defaultCircuitsDir() : QFileInfo(m_currentFile).absolutePath();
    QString path = QFileDialog::getSaveFileName(this, tr("Save Circuit"),
                                                 startDir,
                                                 tr("Digital Design Circuit (*.ddc)"));
    if (path.isEmpty()) return;
    if (!path.endsWith(".ddc", Qt::CaseInsensitive))
        path += ".ddc";
    m_currentFile = path;
    saveCircuit();
}

void MainWindow::openCircuit()
{
    QString startDir = m_currentFile.isEmpty() ? defaultCircuitsDir() : QFileInfo(m_currentFile).absolutePath();
    QString path = QFileDialog::getOpenFileName(this, tr("Open Circuit"),
                                                 startDir,
                                                 tr("Digital Design Circuit (*.ddc)"));
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot read file: %1").arg(file.errorString()));
        return;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid JSON: %1").arg(err.errorString()));
        return;
    }

    m_scene->loadCircuit(doc.object());
    m_currentFile = path;
    m_undoStack->clear();
    updateWindowTitle();
}

// --- Export ---

void MainWindow::exportPng()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Export PNG"),
                                                 defaultCircuitsDir(),
                                                 tr("PNG Images (*.png)"));
    if (path.isEmpty()) return;
    if (!path.endsWith(".png", Qt::CaseInsensitive))
        path += ".png";

    QRectF sceneRect = m_scene->itemsBoundingRect().adjusted(-20, -20, 20, 20);
    QImage image(sceneRect.size().toSize() * 2, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    m_scene->render(&painter, QRectF(), sceneRect);
    painter.end();

    if (!image.save(path)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to save PNG."));
    }
}

void MainWindow::exportSvg()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Export SVG"),
                                                 defaultCircuitsDir(),
                                                 tr("SVG Files (*.svg)"));
    if (path.isEmpty()) return;
    if (!path.endsWith(".svg", Qt::CaseInsensitive))
        path += ".svg";

    QRectF sceneRect = m_scene->itemsBoundingRect().adjusted(-20, -20, 20, 20);

    QSvgGenerator generator;
    generator.setFileName(path);
    generator.setSize(sceneRect.size().toSize());
    generator.setViewBox(QRectF(QPointF(0, 0), sceneRect.size()));
    generator.setTitle(tr("Digital Design Circuit"));

    QPainter painter(&generator);
    painter.setRenderHint(QPainter::Antialiasing);
    m_scene->render(&painter, QRectF(), sceneRect);
    painter.end();
}

// --- Tools ---

void MainWindow::showTruthTable()
{
    auto *dlg = new TruthTableDialog(m_scene, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}
