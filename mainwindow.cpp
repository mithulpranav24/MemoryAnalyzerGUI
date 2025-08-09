#include "mainwindow.h"
#include "processworker.h"

#include <QApplication>
#include <QIcon>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // --- Basic Window Setup ---
    this->setWindowTitle("Memory Analyzer");
    this->setMinimumSize(800, 600);
    this->setWindowIcon(QIcon(":/cpu.svg"));

    // --- Create Central Widget and Main Layout ---
    QWidget* centralWidget = new QWidget(this);
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    this->setCentralWidget(centralWidget);

    // --- Create and Configure Sidebar and StackedWidget ---
    m_sidebar = new QListWidget();
    m_mainStack = new QStackedWidget();
    m_sidebar->setMaximumWidth(220);
    m_sidebar->setIconSize(QSize(24, 24));
    mainLayout->addWidget(m_sidebar);
    mainLayout->addWidget(m_mainStack);

    // --- Populate Sidebar Menu with Icons ---
    m_sidebar->addItem(new QListWidgetItem(QIcon(":/cpu.svg"), "System Overview"));
    m_sidebar->addItem(new QListWidgetItem(QIcon(":/monitor.svg"), "Real-time Process Monitor"));
    m_sidebar->addItem(new QListWidgetItem(QIcon(":/search.svg"), "Process Inspector"));
    m_sidebar->addItem(new QListWidgetItem(QIcon(":/alert-triangle.svg"), "Threshold Alert"));
    m_sidebar->addItem(new QListWidgetItem(QIcon(":/save.svg"), "Save Report"));
    m_sidebar->setCurrentRow(0);

    // --- Create and add ALL feature pages to the StackedWidget ---
    m_mainStack->addWidget(createSystemOverviewPage());
    m_mainStack->addWidget(createRealTimeMonitorPage());
    m_mainStack->addWidget(createProcessInspectorPage());
    m_mainStack->addWidget(createThresholdAlertPage());
    m_mainStack->addWidget(createSaveReportPage());

    // --- Connect Signals and Slots ---
    connect(m_sidebar, &QListWidget::currentRowChanged, m_mainStack, &QStackedWidget::setCurrentIndex);
    connect(m_pidGetInfoButton, &QPushButton::clicked, this, &MainWindow::onGetInfoButtonClicked);
    connect(m_pidCompareButton, &QPushButton::clicked, this, &MainWindow::onCompareButtonClicked);
    connect(m_setAlertButton, &QPushButton::clicked, this, &MainWindow::onSetAlertButtonClicked);
    connect(m_saveReportButton, &QPushButton::clicked, this, &MainWindow::onSaveReportButtonClicked);
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);

    // --- Register Custom Type and Start Worker Thread ---
    qRegisterMetaType<AppData>("AppData");
    workerThread = new QThread();
    worker = new ProcessWorker();
    worker->moveToThread(workerThread);
    connect(workerThread, &QThread::started, worker, &ProcessWorker::startWork);
    connect(worker, &ProcessWorker::resultReady, this, &MainWindow::handleResults);
    connect(worker, &ProcessWorker::thresholdExceeded, this, &MainWindow::handleThresholdAlert);
    workerThread->start();
}

MainWindow::~MainWindow()
{
    workerThread->quit();
    workerThread->wait();
}

QWidget* MainWindow::createSystemOverviewPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(10, 10, 10, 10);
    pageLayout->setSpacing(15);

    QGroupBox* cpuGroup = new QGroupBox("CPU Information");
    QFormLayout* cpuLayout = new QFormLayout(cpuGroup);
    m_cpuModelLabel = new QLabel("retrieving...");
    m_cpuCoresThreadsLabel = new QLabel("retrieving...");
    m_cpuL1CacheLabel = new QLabel("retrieving...");
    m_cpuL2CacheLabel = new QLabel("retrieving...");
    m_cpuL3CacheLabel = new QLabel("retrieving...");
    cpuLayout->addRow("Model:", m_cpuModelLabel);
    cpuLayout->addRow("Cores / Threads:", m_cpuCoresThreadsLabel);
    cpuLayout->addRow("L1 Cache:", m_cpuL1CacheLabel);
    cpuLayout->addRow("L2 Cache:", m_cpuL2CacheLabel);
    cpuLayout->addRow("L3 Cache:", m_cpuL3CacheLabel);

    QGroupBox* memGroup = new QGroupBox("Memory (RAM) Information");
    QFormLayout* memLayout = new QFormLayout(memGroup);
    m_totalMemoryLabel = new QLabel("retrieving...");
    m_availableMemoryLabel = new QLabel("retrieving...");
    m_memoryTypeLabel = new QLabel("retrieving...");
    m_memorySpeedLabel = new QLabel("retrieving...");
    m_memorySlotsLabel = new QLabel("retrieving...");
    memLayout->addRow("Total Installed:", m_totalMemoryLabel);
    memLayout->addRow("Currently Available:", m_availableMemoryLabel);
    memLayout->addRow("Type:", m_memoryTypeLabel);
    memLayout->addRow("Speed:", m_memorySpeedLabel);
    memLayout->addRow("Slots:", m_memorySlotsLabel);

    QGroupBox* graphicsGroup = new QGroupBox("Graphics Controllers");
    QVBoxLayout* graphicsLayout = new QVBoxLayout(graphicsGroup);
    m_gpuListLabel = new QLabel("retrieving...");
    m_gpuListLabel->setWordWrap(true);
    graphicsLayout->addWidget(m_gpuListLabel);

    pageLayout->addWidget(cpuGroup);
    pageLayout->addWidget(memGroup);
    pageLayout->addWidget(graphicsGroup);
    pageLayout->addStretch();

    return page;
}

QWidget* MainWindow::createRealTimeMonitorPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);

    m_searchLineEdit = new QLineEdit();
    m_searchLineEdit->setPlaceholderText("Filter by process name...");
    m_searchLineEdit->setClearButtonEnabled(true);
    layout->addWidget(m_searchLineEdit);

    m_processTableWidget = new QTableWidget();
    layout->addWidget(m_processTableWidget);

    m_processTableWidget->setColumnCount(3);
    m_processTableWidget->setHorizontalHeaderLabels({"Process Name", "PID", "Memory Usage"});
    m_processTableWidget->horizontalHeader()->setStretchLastSection(true);
    m_processTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_processTableWidget->setAlternatingRowColors(true);
    m_processTableWidget->setSortingEnabled(true);

    return page;
}

QWidget* MainWindow::createProcessInspectorPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* mainVLayout = new QVBoxLayout(page);
    QGroupBox* infoGroup = new QGroupBox("Get Info for a Single Process");
    QFormLayout* infoLayout = new QFormLayout(infoGroup);
    m_pidLineEdit = new QLineEdit();
    m_pidLineEdit->setPlaceholderText("Enter PID");
    m_pidGetInfoButton = new QPushButton("Get Info");
    m_pidResultLabel = new QLabel("Result will be shown here.");
    infoLayout->addRow(m_pidLineEdit, m_pidGetInfoButton);
    infoLayout->addRow(m_pidResultLabel);
    QGroupBox* compareGroup = new QGroupBox("Compare Two Processes");
    QFormLayout* compareLayout = new QFormLayout(compareGroup);
    m_pid1LineEdit = new QLineEdit();
    m_pid1LineEdit->setPlaceholderText("Enter first PID");
    m_pid2LineEdit = new QLineEdit();
    m_pid2LineEdit->setPlaceholderText("Enter second PID");
    m_pidCompareButton = new QPushButton("Compare");
    m_compareResultLabel = new QLabel("Comparison result will be shown here.");
    compareLayout->addRow("Process 1 PID:", m_pid1LineEdit);
    compareLayout->addRow("Process 2 PID:", m_pid2LineEdit);
    compareLayout->addWidget(m_pidCompareButton);
    compareLayout->addRow(m_compareResultLabel);
    mainVLayout->addWidget(infoGroup);
    mainVLayout->addWidget(compareGroup);
    mainVLayout->addStretch();
    return page;
}

QWidget* MainWindow::createThresholdAlertPage()
{
    QWidget* page = new QWidget();
    QFormLayout* layout = new QFormLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    m_thresholdSpinBox = new QSpinBox();
    m_thresholdSpinBox->setRange(1, 99);
    m_thresholdSpinBox->setValue(80);
    m_thresholdSpinBox->setSuffix("%");
    m_setAlertButton = new QPushButton("Set Alert");
    m_alertStatusLabel = new QLabel("Alert is not set.");
    layout->addRow("Set Memory Threshold:", m_thresholdSpinBox);
    layout->addRow(m_setAlertButton);
    layout->addRow(m_alertStatusLabel);
    return page;
}

QWidget* MainWindow::createSaveReportPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    m_saveReportButton = new QPushButton("Generate and Save Full Report");
    m_reportStatusLabel = new QLabel("Click button to save report.");
    layout->addWidget(m_saveReportButton);
    layout->addWidget(m_reportStatusLabel);
    layout->addStretch();
    return page;
}


void MainWindow::onSearchTextChanged(const QString &text)
{
    m_currentFilter = text;
    handleResults(lastData);
}

void MainWindow::handleResults(const AppData &data)
{
    lastData = data;
    QString memStr;

    m_cpuModelLabel->setText(data.cpuModel);
    m_cpuCoresThreadsLabel->setText(QString("%1 Cores / %2 Threads").arg(data.cpuCores, data.cpuThreads));
    m_cpuL1CacheLabel->setText(data.cpuL1Cache);
    m_cpuL2CacheLabel->setText(data.cpuL2Cache);
    m_cpuL3CacheLabel->setText(data.cpuL3Cache);
    formatMemory(memStr, data.memTotal);
    m_totalMemoryLabel->setText(memStr);
    formatMemory(memStr, data.memAvailable);
    m_availableMemoryLabel->setText(memStr);
    m_memoryTypeLabel->setText(data.memoryType);
    m_memorySpeedLabel->setText(data.memorySpeed);
    m_memorySlotsLabel->setText(data.memorySlots);
    m_gpuListLabel->setText(data.gpuModels.join("\n"));

    QList<ProcessInfo> filteredList;
    if (m_currentFilter.isEmpty()) {
        filteredList = data.processes;
    } else {
        for (const auto& process : data.processes) {
            if (process.name.contains(m_currentFilter, Qt::CaseInsensitive)) {
                filteredList.append(process);
            }
        }
    }

    m_processTableWidget->setSortingEnabled(false);
    m_processTableWidget->setRowCount(filteredList.count());
    for(int i = 0; i < filteredList.count(); ++i) {
        const auto& process = filteredList.at(i);
        formatMemory(memStr, process.memory);
        QTableWidgetItem *nameItem = new QTableWidgetItem(process.name);
        QTableWidgetItem *pidItem = new QTableWidgetItem(QString::number(process.pid));
        QTableWidgetItem *memItem = new QTableWidgetItem(memStr);
        m_processTableWidget->setItem(i, 0, nameItem);
        m_processTableWidget->setItem(i, 1, pidItem);
        m_processTableWidget->setItem(i, 2, memItem);
    }
    m_processTableWidget->setSortingEnabled(true);
}

void MainWindow::handleThresholdAlert(const QString& message)
{
    QMessageBox::warning(this, "Memory Alert", message);
    m_alertStatusLabel->setText(QString("<font color='red'>%1</font>").arg(message));
}

void MainWindow::onGetInfoButtonClicked()
{
    pid_t pid = m_pidLineEdit->text().toInt();
    ProcessWorker tempWorker;
    long mem_kb = tempWorker.getVmRssFromPid(pid);
    QString result;
    if (mem_kb != -1) {
        QString memStr;
        formatMemory(memStr, mem_kb);
        result = QString("PID %1 is using %2.").arg(pid).arg(memStr);
    } else {
        result = QString("Could not find process with PID %1.").arg(pid);
    }
    m_pidResultLabel->setText(result);
}

void MainWindow::onCompareButtonClicked()
{
    pid_t pid1 = m_pid1LineEdit->text().toInt();
    pid_t pid2 = m_pid2LineEdit->text().toInt();
    ProcessWorker tempWorker;
    long mem1 = tempWorker.getVmRssFromPid(pid1);
    long mem2 = tempWorker.getVmRssFromPid(pid2);
    QString result;
    if(mem1 == -1 || mem2 == -1) {
        result = "Could not find one or both PIDs.";
    } else {
        QString memStr1, memStr2, diffStr;
        formatMemory(memStr1, mem1);
        formatMemory(memStr2, mem2);
        result = QString("PID %1: %2 | PID %3: %4. ").arg(pid1).arg(memStr1).arg(pid2).arg(memStr2);
        if (mem1 > mem2) {
            formatMemory(diffStr, mem1 - mem2);
            result += QString("PID %1 uses %2 more.").arg(pid1).arg(diffStr);
        } else if (mem2 > mem1) {
            formatMemory(diffStr, mem2 - mem1);
            result += QString("PID %1 uses %2 more.").arg(pid2).arg(diffStr);
        } else {
            result += "They use the same amount.";
        }
    }
    m_compareResultLabel->setText(result);
}

void MainWindow::onSetAlertButtonClicked()
{
    int threshold = m_thresholdSpinBox->value();
    worker->setThreshold(threshold);
    m_alertStatusLabel->setText(QString("Alert threshold set to %1%").arg(threshold));
}

void MainWindow::onSaveReportButtonClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Report", "memory_report.txt", "Text Files (*.txt)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not save file.");
        return;
    }
    QTextStream out(&file);
    QString memStr;
    out << "--- System Memory Report ---\n";
    formatMemory(memStr, lastData.memTotal);
    out << "Total Memory: " << memStr << "\n";
    formatMemory(memStr, lastData.memAvailable);
    out << "Available Memory: " << memStr << "\n";
    out << "Memory Type: " << lastData.memoryType << "\n";
    out << "Memory Speed: " << lastData.memorySpeed << "\n";
    out << "\n--- All Running Processes ---\n";
    out << QString("%1; %2; %3\n").arg("Name", -30).arg("PID", -10).arg("Memory");
    out << "--------------------------------------------------------------\n";
    for(const auto& process : lastData.processes) {
        formatMemory(memStr, process.memory);
        out << QString("%1; %2; %3\n").arg(process.name, -30).arg(process.pid, -10).arg(memStr);
    }
    file.close();
    m_reportStatusLabel->setText(QString("Report saved to %1").arg(fileName));
}

void MainWindow::formatMemory(QString& buffer, long kilobytes)
{
    if (kilobytes < 0) {
        buffer = "N/A";
    } else if (kilobytes < 1024) {
        buffer = QString("%1 KB").arg(kilobytes);
    } else if (kilobytes < 1024 * 1024) {
        buffer = QString::asprintf("%.2f MB", kilobytes / 1024.0);
    } else {
        buffer = QString::asprintf("%.2f GB", kilobytes / (1024.0 * 1024.0));
    }
}
