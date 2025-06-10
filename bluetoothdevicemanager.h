#ifndef BLUETOOTHDEVICEMANAGER_H
#define BLUETOOTHDEVICEMANAGER_H

#include <QObject>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QBluetoothDeviceInfo>
#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtQml>
#include <QQmlEngine>
#include <QDebug>
#include <QProcess>
#include <QTimer>
#include <QRegularExpression>

class BluetoothDeviceManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList deviceNames READ deviceNames NOTIFY deviceListChanged)
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)

public:
    explicit BluetoothDeviceManager(QObject *parent = nullptr);

    Q_INVOKABLE void startScan();
    Q_INVOKABLE void stopScan();
    Q_INVOKABLE void connectToDevice(int index);

    QStringList deviceNames() const;
    bool scanning() const;

signals:
    void deviceListChanged();
    void scanningChanged();
    void connectedToDevice(const QString &name);
    void bluetoothError(const QString &error);

private slots:
    void onDeviceDiscovered(const QBluetoothDeviceInfo &info);
    void onScanFinished();

private:
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;
    QList<QBluetoothDeviceInfo> m_devices;
    QStringList m_deviceNames;
};

#endif // BLUETOOTHDEVICEMANAGER_H
