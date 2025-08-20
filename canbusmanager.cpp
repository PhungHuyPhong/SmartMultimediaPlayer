#include "canbusmanager.h"
#include <QDebug>

CanBusManager::CanBusManager(QObject *parent)
    : QObject(parent)
{
}

CanBusManager::~CanBusManager()
{
    if (m_canDevice) {
        m_canDevice->disconnectDevice();
        delete m_canDevice;
    }
}

bool CanBusManager::initialize(const QString &interfaceName)
{
    const QString plugin = "socketcan"; // Dùng cho Linux + MCP2515
    QString errorString;

    m_canDevice = QCanBus::instance()->createDevice(plugin, interfaceName, &errorString);

    if (!m_canDevice) {
        qCritical()<<"Failed to create CAN device: " + errorString;
        return false;
    }

    connect(m_canDevice, &QCanBusDevice::framesReceived,
            this, &CanBusManager::readCanMessages);

    if (!m_canDevice->connectDevice()) {
        qCritical()<<"Failed to connect CAN device: " + m_canDevice->errorString();
        delete m_canDevice;
        m_canDevice = nullptr;
        return false;
    }

    qDebug() << "CAN device connected on interface:" << interfaceName;
    return true;
}

void CanBusManager::sendData(quint32 id, const QByteArray &payload)
{
    if (!m_canDevice) {
        qCritical()<<"CAN device not connected.";
        return;
    }

    QCanBusFrame frame(id, payload);
    frame.setFrameType(QCanBusFrame::DataFrame);

    if (!m_canDevice->writeFrame(frame)) {
        qCritical()<<"Failed to send CAN frame: " + m_canDevice->errorString();
    }
}

void CanBusManager::readCanMessages()
{
    if (!m_canDevice) return;

    while (m_canDevice->framesAvailable()) {
        const QCanBusFrame frame = m_canDevice->readFrame();
        QCanBusFrame::FrameId frameId = frame.frameId();
        QByteArray payload = frame.payload();
        qDebug() << "ID: " << QString::number(frameId, 16).toUpper()
                 << " Data: " << payload.toHex().toUpper();
    }
}

