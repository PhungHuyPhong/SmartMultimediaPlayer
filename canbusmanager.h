#ifndef CANBUSMANAGER_H
#define CANBUSMANAGER_H

#include <QObject>
#include <QCanBus>
#include <QCanBusDevice>
#include <QCanBusFrame>
#include <QTimer>

class CanBusManager : public QObject
{
    Q_OBJECT
public:
    explicit CanBusManager(QObject *parent = nullptr);
    ~CanBusManager();

    bool initialize(const QString &interfaceName = "can0");
    void sendData(quint32 id, const QByteArray &payload);
signals:
    void newSensorData(quint32 id, const QByteArray &data);
    void errorOccurred(const QString &errorString);
private slots:
    void readCanMessages();
private:
    QCanBusDevice *m_canDevice = nullptr;
};

#endif // CANBUSMANAGER_H
