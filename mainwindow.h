#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "datatypes.h"

// Forward-declare all the Qt classes we will use as pointers
class QListWidget;
class QStackedWidget;
class QLabel;
class QTableWidget;
class QLineEdit;
class QPushButton;
class QSpinBox;
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
    void onSearchTextChanged(const QString &text); // <<< NEW SLOT FOR THE SEARCH BAR

private:
    QWidget* createSystemOverviewPage();
    QWidget* createRealTimeMonitorPage();
    QWidget* createProcessInspectorPage();
    QWidget* createThresholdAlertPage();
    QWidget* createSaveReportPage();
    void formatMemory(QString& buffer, long kilobytes);

    // ... other member variables are the same ...

    // VVV NEW WIDGET POINTER AND FILTER STRING VVV
    QLineEdit* m_searchLineEdit;
    QString m_currentFilter;

    // Main layout
    QListWidget* m_sidebar;
    QStackedWidget* m_mainStack;
    QLabel* m_cpuModelLabel, *m_cpuCoresThreadsLabel, *m_cpuL1CacheLabel, *m_cpuL2CacheLabel, *m_cpuL3CacheLabel;
    QLabel* m_totalMemoryLabel, *m_availableMemoryLabel, *m_memoryTypeLabel, *m_memorySpeedLabel, *m_memorySlotsLabel;
    QLabel* m_gpuListLabel;
    QTableWidget* m_processTableWidget;
    QLineEdit* m_pidLineEdit, *m_pid1LineEdit, *m_pid2LineEdit;
    QPushButton* m_pidGetInfoButton, *m_pidCompareButton;
    QLabel* m_pidResultLabel, *m_compareResultLabel;
    QSpinBox* m_thresholdSpinBox;
    QPushButton* m_setAlertButton;
    QLabel* m_alertStatusLabel;
    QPushButton* m_saveReportButton;
    QLabel* m_reportStatusLabel;

    QThread* workerThread;
    ProcessWorker* worker;
    AppData lastData;
};
#endif // MAINWINDOW_H
