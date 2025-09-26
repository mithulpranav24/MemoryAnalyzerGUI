// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QRadioButton>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QComboBox>
#include "datatypes.h"

class ProcessWorker;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleResults(const AppData &data);
    void handleThresholdAlert(const QString& message);
    void onGetInfoButtonClicked();
    void onCompareButtonClicked();
    void onSetAlertButtonClicked();
    void onSaveReportButtonClicked();
    void onSearchTextChanged(const QString &text);
    void onGetTopNClicked();
    void onStartLoggingClicked();
    void onIgnoreAlert();

private:
    QWidget* createSystemOverviewPage();
    QWidget* createRealTimeMonitorPage();
    QWidget* createProcessInspectorPage();
    QWidget* createThresholdAlertPage();
    QWidget* createSaveReportPage();
    QWidget* createTopNPage();
    QWidget* createTrackMemoryPage();
    void formatMemory(QString& buffer, long kilobytes);
    void performLog();

    // Main layout
    QListWidget* m_sidebar;
    QStackedWidget* m_mainStack;

    // Page 0: System Overview
    QLabel* m_cpuModelLabel, *m_cpuCoresThreadsLabel, *m_cpuL1CacheLabel, *m_cpuL2CacheLabel, *m_cpuL3CacheLabel;
    QLabel* m_totalMemoryLabel, *m_availableMemoryLabel, *m_memoryTypeLabel, *m_memorySpeedLabel, *m_memorySlotsLabel;
    QLabel* m_gpuListLabel;

    // Page 1: Real-time Process Monitor
    QTableWidget* m_processTableWidget;
    QLineEdit* m_searchLineEdit;
    QString m_currentFilter;

    // Page 2: Process Inspector
    QLineEdit* m_pidLineEdit, *m_pid1LineEdit, *m_pid2LineEdit;
    QPushButton* m_pidGetInfoButton, *m_pidCompareButton;
    QLabel* m_pidResultLabel, *m_compareResultLabel;

    // Page 3: Threshold Alert
    QSpinBox* m_thresholdSpinBox;
    QPushButton* m_setAlertButton;
    QLabel* m_alertStatusLabel;
    QPushButton* m_ignoreButton;

    // Page 4: Save Report
    QPushButton* m_saveReportButton;
    QLabel* m_reportStatusLabel;

    // Page 5: Top N Processes
    QSpinBox* m_topNSpinBox;
    QPushButton* m_topNButton;
    QTableWidget* m_topNTableWidget;

    // Page 6: Track Memory Usage
    QSpinBox* m_intervalValueSpinBox;
    QComboBox* m_intervalUnitComboBox;
    QSpinBox* m_durationValueSpinBox;
    QComboBox* m_durationUnitComboBox;
    QRadioButton* m_allRadio;
    QRadioButton* m_specificRadio;
    QLineEdit* m_pidsLineEdit;
    QPushButton* m_startLoggingButton;
    QLabel* m_loggingStatusLabel;

    // Logging management
    QTimer* m_loggingTimer;
    int m_logCount;
    int m_totalLogs;
    QString m_logContent;
    QList<pid_t> m_specificPids;

    // Worker thread members
    QThread* workerThread;
    ProcessWorker* worker;
    AppData lastData;
    bool alertActive = false;
    int currentThreshold = -1; // To track the current threshold
};
#endif // MAINWINDOW_H
