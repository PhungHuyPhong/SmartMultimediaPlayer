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
    Q_PROPERTY(double temperature READ temperature NOTIFY temperatureChanged)
    //Q_PROPERTY(double ambientLight READ ambientLight NOTIFY ambientLightChanged)
    Q_PROPERTY(int gesture READ gesture NOTIFY gestureChanged)
public:
    explicit CanBusManager(QObject *parent = nullptr);
    ~CanBusManager();

    bool initialize(const QString &interfaceName = "can0");
    void sendData(quint32 id, const QByteArray &payload);
    int temperature() const { return m_temperature; }

    //double ambientLight() const { return m_ambientLight; }
    int gesture() const { return m_gesture; }
signals:
    void newSensorData(quint32 id, const QByteArray &data);
    void errorOccurred(const QString &errorString);
    void temperatureChanged();

    //void ambientLightChanged();
    void gestureChanged(int gesture);
private slots:
    void readCanMessages();
private:
    QCanBusDevice *m_canDevice = nullptr;

    static constexpr quint32 TemperatureHumidityFrameId = 0x01;
    //static constexpr quint32 AmbientLightFrameId       = 0x101  ;
    static constexpr quint32 GestureFrameId            = 0x02;

    double  m_temperature{0.0};

    //double m_ambientLight{0.0};
    int    m_gesture{0};
};

#endif // CANBUSMANAGER_H
