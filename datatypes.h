#ifndef DATATYPES_H
#define DATATYPES_H

#include <QList>
#include <QString>
#include <QMetaType>
#include <QStringList>

// Struct for a single process
struct ProcessInfo {
    pid_t pid;
    long memory; // in Kilobytes
    QString name;
};

// Struct to hold all data passed from worker to main thread
struct AppData {
    // Dynamic data
    long memTotal = 0;
    long memAvailable = 0;
    QList<ProcessInfo> processes;

    // Static hardware data
    QString cpuModel;
    QString cpuCores;
    QString cpuThreads;
    QString cpuL1Cache;
    QString cpuL2Cache;
    QString cpuL3Cache;
    QStringList gpuModels;
    QString memoryType;
    QString memorySpeed;
    QString memorySlots;
};

// Required for using these custom structs in Qt's signal/slot system
Q_DECLARE_METATYPE(AppData)

#endif // DATATYPES_H
