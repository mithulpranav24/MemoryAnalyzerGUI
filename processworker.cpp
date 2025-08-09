#include "processworker.h"
#include <QThread>
#include <QDebug>
#include <algorithm>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QTimer>

ProcessWorker::ProcessWorker(QObject *parent) : QObject(parent)
{
    m_timer = new QTimer(this);
}

void ProcessWorker::setThreshold(int percent) { this->memoryThreshold = percent; }

void ProcessWorker::startWork()
{
    fetchStaticInfo();
    connect(m_timer, &QTimer::timeout, this, &ProcessWorker::performScan);
    m_timer->start(2000);
    performScan();
}

void ProcessWorker::performScan()
{
    appData.memTotal = getMemInfo("MemTotal:");
    appData.memAvailable = getMemInfo("MemAvailable:");

    if (memoryThreshold > 0 && appData.memTotal > 0) {
        long memUsed = appData.memTotal - appData.memAvailable;
        int usagePercent = static_cast<int>(100.0 * memUsed / appData.memTotal);
        if (usagePercent > memoryThreshold) {
            QString message = QString("Warning: Memory usage is at %1%, exceeding threshold of %2%!")
                                  .arg(usagePercent).arg(memoryThreshold);
            emit thresholdExceeded(message);
        }
    }

    appData.processes.clear();
    // VVV FIX: Use `ls` and `QProcess` instead of `QDir` VVV
    QString ls_out = runCommand("ls /proc");
    for(const QString &entry : ls_out.split('\n')) {
        bool isNumber;
        pid_t pid = entry.toInt(&isNumber);
        if(isNumber) {
            long memory_kb = getVmRssFromPid(pid);
            if (memory_kb >= 0) {
                ProcessInfo info;
                info.pid = pid;
                info.memory = memory_kb;
                info.name = getNameFromPid(pid);
                appData.processes.append(info);
            }
        }
    }

    qDebug() << "Scan complete: Found" << appData.processes.count() << "processes. Total memory:" << appData.memTotal;

    std::sort(appData.processes.begin(), appData.processes.end(), [](const ProcessInfo &a, const ProcessInfo &b) {
        return a.memory > b.memory;
    });

    emit resultReady(appData);
}

QString ProcessWorker::runCommand(const QString &command)
{
    QProcess process;
    process.start("/bin/sh", QStringList() << "-c" << command);
    process.waitForFinished(-1);
    return process.readAllStandardOutput().trimmed();
}

void ProcessWorker::fetchStaticInfo()
{
    QString lscpu_out = runCommand("lscpu");
    for(const QString& line : lscpu_out.split('\n')) {
        if(line.startsWith("Model name:")) appData.cpuModel = line.section(':', 1).trimmed();
        if(line.startsWith("Core(s) per socket:")) appData.cpuCores = line.section(':', 1).trimmed();
        if(line.startsWith("CPU(s):")) appData.cpuThreads = line.section(':', 1).trimmed();
        if(line.startsWith("L1d cache:") || line.startsWith("L1 cache:")) appData.cpuL1Cache = line.section(':', 1).trimmed();
        if(line.startsWith("L2 cache:")) appData.cpuL2Cache = line.section(':', 1).trimmed();
        if(line.startsWith("L3 cache:")) appData.cpuL3Cache = line.section(':', 1).trimmed();
    }

    QStringList all_gpus_out = runCommand("lspci | grep -Ei 'VGA|3D|Display'").trimmed().split('\n');
    for (const QString &gpu_line : all_gpus_out) {
        appData.gpuModels.append(gpu_line.section(':', 2).trimmed());
    }

    QString dmidecode_out = runCommand("sudo dmidecode -t memory");
    if (dmidecode_out.isEmpty() || dmidecode_out.contains("permission denied")) {
        appData.memoryType = "N/A (run with sudo)";
        appData.memorySpeed = "N/A (run with sudo)";
        appData.memorySlots = "N/A (run with sudo)";
    } else {
        int deviceCount = 0;
        for(const QString& line : dmidecode_out.split('\n')) {
            QString trimmedLine = line.trimmed();
            if(trimmedLine.startsWith("Locator:") && !trimmedLine.contains("Not Specified")) deviceCount++;
            if(trimmedLine.startsWith("Type:") && appData.memoryType.isEmpty()) appData.memoryType = trimmedLine.section(':', 1).trimmed();
            if(trimmedLine.startsWith("Speed:") && !trimmedLine.contains("Unknown") && appData.memorySpeed.isEmpty()) {
                appData.memorySpeed = trimmedLine.section(':', 1).trimmed();
            }
        }
        appData.memorySlots = QString("%1 populated").arg(deviceCount);
    }
}


// --- VVV REWRITTEN HELPER FUNCTIONS USING QPROCESS VVV ---

long ProcessWorker::getMemInfo(const char *field)
{
    QString content = runCommand("cat /proc/meminfo");
    for(const QString &line : content.split('\n')) {
        if (line.startsWith(field)) {
            return line.section(':', 1).trimmed().split(' ').at(0).toLong();
        }
    }
    return -1;
}

long ProcessWorker::getVmRssFromPid(pid_t pid)
{
    QString content = runCommand(QString("cat /proc/%1/status").arg(pid));
    for(const QString &line : content.split('\n')) {
        if (line.startsWith("VmRSS:")) {
            return line.section(':', 1).trimmed().split(' ').at(0).toLong();
        }
    }
    return -1;
}

QString ProcessWorker::getNameFromPid(pid_t pid)
{
    return runCommand(QString("cat /proc/%1/comm").arg(pid));
}
