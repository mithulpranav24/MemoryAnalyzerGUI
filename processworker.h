#ifndef PROCESSWORKER_H
#define PROCESSWORKER_H

#include <QObject>
#include "datatypes.h"

class QTimer;

class ProcessWorker : public QObject
{
    Q_OBJECT
public:
    explicit ProcessWorker(QObject *parent = nullptr);

    // Public helper functions for one-off calls from MainWindow
    long getVmRssFromPid(pid_t pid);
    QString getNameFromPid(pid_t pid);

public slots:
    void startWork();
    void setThreshold(int percent);

private slots:
    void performScan();

signals:
    void resultReady(const AppData &data);
    void thresholdExceeded(const QString &message);

private:
    // Helpers used internally
    QString runCommand(const QString& command);
    void fetchStaticInfo();
    long getMemInfo(const char* field);

    int memoryThreshold = -1;
    AppData appData;
    QTimer* m_timer;
};

#endif // PROCESSWORKER_H
