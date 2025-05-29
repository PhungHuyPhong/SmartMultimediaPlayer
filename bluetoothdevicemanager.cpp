#include "bluetoothdevicemanager.h"
#include <QDebug>
#include <QProcess>
#include <QTimer>
#include <QRegularExpression>

BluetoothDeviceManager::BluetoothDeviceManager(QObject *parent)
    : QObject(parent),
    m_discoveryAgent(new QBluetoothDeviceDiscoveryAgent(this))
{
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BluetoothDeviceManager::onDeviceDiscovered);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BluetoothDeviceManager::onScanFinished);
}

void BluetoothDeviceManager::startScan()
{
    m_deviceNames.clear();
    m_devices.clear();
    emit deviceListChanged();

    m_discoveryAgent->start();
    emit scanningChanged();
}

void BluetoothDeviceManager::stopScan()
{
    m_discoveryAgent->stop();
    emit scanningChanged();
}

void BluetoothDeviceManager::onDeviceDiscovered(const QBluetoothDeviceInfo &info)
{
    m_devices.append(info);
    m_deviceNames.append(info.name());
    emit deviceListChanged();
}

void BluetoothDeviceManager::onScanFinished()
{
    emit scanningChanged();
}

void BluetoothDeviceManager::connectToDevice(int index)
{
    if (index < 0 || index >= m_devices.size())
        return;

    const QBluetoothDeviceInfo &device = m_devices.at(index);
    const QString &address = device.address().toString();
    const QString &name = device.name();

    qDebug() << " Connecting to device:" << name << address;

    // Tạo process
    QProcess *process = new QProcess(this);

    // Kết nối xử lý kết quả đầu ra
    connect(process, &QProcess::readyReadStandardOutput, this, [=]() {
        QString output = process->readAllStandardOutput();
        qDebug().noquote() << output;

        if (output.contains("Connection successful", Qt::CaseInsensitive)) {
            emit connectedToDevice(name);
            process->close();
            process->deleteLater();
        } else if (output.contains("Failed to connect", Qt::CaseInsensitive) ||
                   output.contains("not available", Qt::CaseInsensitive)) {
            emit bluetoothError(" Failed to connect to " + name);
            process->close();
            process->deleteLater();
        }
    });

    // Khởi động bluetoothctl
    process->start("bluetoothctl");

    if (!process->waitForStarted()) {
        emit bluetoothError(" Failed to start bluetoothctl");
        process->deleteLater();
        return;
    }

    // Gửi lệnh tuần tự: pair -> trust -> connect
    auto sendCommand = [=](const QString &cmd) {
        process->write(cmd.toUtf8() + "\n");
        process->waitForBytesWritten(100);
    };

    sendCommand("pair " + address);
    QTimer::singleShot(1000, [=]() { sendCommand("trust " + address); });
    QTimer::singleShot(2000, [=]() { sendCommand("connect " + address); });

    // Timeout xử lý
    QTimer::singleShot(8000, [=]() {
        if (process->state() == QProcess::Running) {
            emit bluetoothError(" Connection timeout with " + name);
            process->close();
            process->deleteLater();
        }
    });
}


QStringList BluetoothDeviceManager::deviceNames() const
{
    return m_deviceNames;
}

bool BluetoothDeviceManager::scanning() const
{
    return m_discoveryAgent->isActive();
}
