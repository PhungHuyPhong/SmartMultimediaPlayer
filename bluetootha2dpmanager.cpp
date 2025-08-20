#include "bluetootha2dpmanager.h"
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusArgument>
#include <QDBusInterface>

BluetoothA2DPManager::BluetoothA2DPManager(QObject *parent)
    : QObject(parent), m_bus(QDBusConnection::systemBus())
{
    qDBusRegisterMetaType<QVariantMap>();
}

bool BluetoothA2DPManager::initialize()
{
    if (!m_bus.isConnected()) {
        qCritical() << "Can not connect to System DBus!";
        return false;
    }
    qInfo() << "Connected to DBus.";

    // 1. Đăng ký A2DP Profile (như giai đoạn 1)
    registerA2dpProfile();

    // 2. Kết nối với ObjectManager của BlueZ để lắng nghe các thay đổi
    // Tín hiệu "InterfacesAdded" sẽ được phát ra khi một thiết bị kết nối
    // và cung cấp các interface mới (như Media Player).
    bool success1 = m_bus.connect(
        m_bluezService,
        m_objectManagerPath,
        "org.freedesktop.DBus.ObjectManager",
        "InterfacesAdded",
        this,
        SLOT(onInterfacesAdded(QDBusObjectPath, QVariantMap))
        );
    bool success2 = m_bus.connect(
        m_bluezService,
        m_objectManagerPath,
        "org.freedesktop.DBus.ObjectManager",
        "InterfacesRemoved",
        this,
        SLOT(onInterfacesRemoved(QDBusObjectPath, QStringList))
        );

    if (success1 && success2) {
        qInfo() << "Connected to signal InterfacesAdded and InterfacesRemoved, ready to connect.";
    }
    else{
        qCritical() << "Can not connect to signal InterfacesAdded or InterfacesRemoved";
        return false;
    }

    // 3. Kiểm tra xem có thiết bị nào đã kết nối từ trước không
    initialDeviceCheck();

    return true;
}

void BluetoothA2DPManager::registerA2dpProfile()
{
    // ... (Giữ nguyên code từ Giai đoạn 1)
    qInfo() << "Registering A2DP Sink Profile...";
    QDBusInterface profileManager(m_bluezService, m_profileManagerPath, "org.bluez.ProfileManager1", m_bus);
    if (!profileManager.isValid()) {
        qCritical() << "Can not find BlueZ Profile Manager.";
        return;
    }
    QVariantMap profileOptions;
    profileOptions.insert("Name", "SmartMultimediaPlayer");
    profileOptions.insert("Service", m_a2dpSinkUUID);
    profileOptions.insert("Role", "sink");
    profileOptions.insert("RequireAuthentication", true);
    profileOptions.insert("RequireAuthorization", false);
    profileOptions.insert("AutoConnect", true);
    profileOptions.insert("Version", QVariant::fromValue<quint16>(0x0103));
    profileOptions.insert("Features", QVariant::fromValue<quint16>(0x0001));
    QDBusMessage reply = profileManager.call("RegisterProfile",
                                             QVariant::fromValue(QDBusObjectPath(m_a2dpSinkProfilePath)),
                                             m_a2dpSinkUUID,
                                             QVariant::fromValue(profileOptions));
    if (reply.type() == QDBusMessage::ReplyMessage) {
        qInfo() << "A2DP Sink Profile successfully registered!";
    } else if (reply.type() == QDBusMessage::ErrorMessage) {
        qCritical() << "Fail to register A2DP Sink Profile" << reply.errorMessage();
    }
}

// Slot này được gọi khi có một thiết bị kết nối
void BluetoothA2DPManager::onInterfacesAdded(const QDBusObjectPath &objectPath, const QVariantMap &interfaces)
{
    // Chúng ta chỉ quan tâm đến interface "org.bluez.MediaPlayer1"
    if (interfaces.contains(m_mediaPlayerInterfaceName)) {
        if (m_activePlayerPath.path().isEmpty() || m_activePlayerPath.path() == "/") {
            qInfo() << "New media player detected at " << objectPath.path();
            connectToMediaPlayer(objectPath);
        } else {
            disconnectMediaPlayer();
            qInfo() << "Ignoring new media player at " << objectPath.path() << " because a player at" << m_activePlayerPath.path() << " is already active.";
        }
    }
}

void BluetoothA2DPManager::onInterfacesRemoved(const QDBusObjectPath &objectPath, const QStringList &interfaces)
{
    if (interfaces.contains(m_mediaPlayerInterfaceName)) {
        if (!m_activePlayerPath.path().isEmpty() && m_activePlayerPath.path() == objectPath.path()) {
            qInfo() << "Active media player at " << objectPath.path() << " was disconnected.";
            disconnectMediaPlayer();
        }
    }
}

void BluetoothA2DPManager::connectToMediaPlayer(const QDBusObjectPath &path)
{
    qInfo() << "Connecting to player at" << path.path();
    m_activePlayerPath = path;

    // Ngắt kết nối với trình phát cũ nếu có
    // if (m_mediaPlayerInterface) {
    //     delete m_mediaPlayerInterface;
    //     m_mediaPlayerInterface = nullptr;
    // }

    // Kết nối tới tín hiệu "PropertiesChanged" của trình phát media này.
    // Tín hiệu này sẽ được phát ra mỗi khi có gì đó thay đổi (đổi bài, play/pause,...)
    bool success=m_bus.connect(
        m_bluezService,
        m_activePlayerPath.path(),
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged",
        this,
        SLOT(onPropertiesChanged(QString, QVariantMap, QStringList))
        );
    if (!success) {
        qCritical() << "Can not connect to signal PropertiesChanged.";
        return;
    }
    else qInfo() << "Connected to signal PropertiesChanged for "<<m_activePlayerPath.path();
    if (m_isConnected == false) {
        m_isConnected = true;
        emit connectionChanged();
        qInfo() << "Device connected.";
    }

    auto *iface = new QDBusInterface(m_bluezService, m_activePlayerPath.path(), m_mediaPlayerInterfaceName, m_bus);
    iface->setTimeout(2000);

    iface->deleteLater();
}

void BluetoothA2DPManager::disconnectMediaPlayer()
{
    qInfo() << "Diconnecting from player at "<<m_activePlayerPath.path();
    // if (m_mediaPlayerInterface) {
    //     delete m_mediaPlayerInterface;
    //     m_mediaPlayerInterface = nullptr;
    // }
    bool success=m_bus.disconnect(
        m_bluezService,
        m_activePlayerPath.path(),
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged",
        this,
        SLOT(onPropertiesChanged(QString, QVariantMap, QStringList))
        );
    if (!success) {
        qCritical() << "Can not disconnect to signal PropertiesChanged.";
        return;
    }
    else qInfo() << "Disconnected to signal PropertiesChanged.";
    m_activePlayerPath.setPath("");
    if(m_isConnected){
        m_isConnected = false;
        emit connectionChanged();
    }
    qInfo() << "Deleting track data...";
    if(!m_trackTitle.isEmpty() || !m_trackAlbum.isEmpty() || !m_trackArtist.isEmpty()){
        m_trackAlbum.clear();
        m_trackArtist.clear();
        m_trackTitle.clear();
        emit trackChanged();
        qInfo() << "Deleted track data.";
    }
    if(m_isPlaying){
        m_isPlaying = false;
        emit playbackStatusChanged();
    }
    qInfo() << "Disconnected";
}

// Slot này được gọi khi có sự thay đổi thuộc tính trên thiết bị (đổi bài, play/pause)
void BluetoothA2DPManager::onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties);
    const QDBusMessage &msg = message();

    if (interfaceName != m_mediaPlayerInterfaceName || m_activePlayerPath.path().isEmpty() || msg.path() != m_activePlayerPath.path()) {
        return;
    }

    qDebug() << "Media properties changed:" << changedProperties;
    updateTrackInfo(changedProperties);
    updatePlaybackStatus(changedProperties);
}

void BluetoothA2DPManager::updateTrackInfo(const QVariantMap &properties)
{
    if (properties.contains("Track")) {
        qInfo() <<"Retrieving Track data";
        // Thông tin bài hát được lồng trong một map khác
        QVariantMap trackMap = qdbus_cast<QVariantMap>(properties.value("Track").value<QDBusArgument>());
        bool changed = false;
        if (trackMap.contains("Title") && m_trackTitle != trackMap.value("Title").toString()) {
            m_trackTitle = trackMap.value("Title").toString();
            changed = true;
        }
        if (trackMap.contains("Artist") && m_trackArtist != trackMap.value("Artist").toString()) {
            m_trackArtist = trackMap.value("Artist").toString();
            changed = true;
        }
        if (trackMap.contains("Album") && m_trackAlbum != trackMap.value("Album").toString()) {
            m_trackAlbum = trackMap.value("Album").toString();
            changed = true;
        }

        if (changed) {
            qInfo() << "Song:" << m_trackTitle << "-" << m_trackArtist;
            emit trackChanged();
        }
        qInfo() <<"Retrieved Track data";
    }
    else qWarning() << "No Track property";
}

void BluetoothA2DPManager::updatePlaybackStatus(const QVariantMap &properties)
{
    if (properties.contains("Status")) {
        qInfo() <<"Retrieving Status data";
        QString status = properties.value("Status").toString();
        bool newPlayingStatus = (status == "playing");

        if (m_isPlaying != newPlayingStatus) {
            m_isPlaying = newPlayingStatus;
            qInfo() << "Playback status:" << status;
            emit playbackStatusChanged();
        }
        qInfo() <<"Retrieved Status data";
    }
    else qWarning() <<"No Status property";
}

// Các hàm getter đơn giản
bool BluetoothA2DPManager::isConnected() const { return m_isConnected; }
QString BluetoothA2DPManager::trackTitle() const { return m_trackTitle; }
QString BluetoothA2DPManager::trackArtist() const { return m_trackArtist; }
QString BluetoothA2DPManager::trackAlbum() const { return m_trackAlbum; }
bool BluetoothA2DPManager::isPlaying() const { return m_isPlaying; }

void BluetoothA2DPManager::play()
{
    if (m_activePlayerPath.path().isEmpty() || m_activePlayerPath.path() == "/") {
        qWarning() <<"Control ignored: No active player to send PLAY command.";
        return;
    }

    qInfo() << "Sending PLAY command to" << m_activePlayerPath.path();
    // Tạo một interface tạm thời chỉ để gọi lệnh
    auto *iface = new QDBusInterface(m_bluezService, m_activePlayerPath.path(), m_mediaPlayerInterfaceName, m_bus);
    iface->call("Play");
    iface->deleteLater(); // Dọn dẹp ngay sau khi gọi
}

void BluetoothA2DPManager::pause()
{
    if (m_activePlayerPath.path().isEmpty() || m_activePlayerPath.path() == "/") {
        qWarning() << "Control ignored: No active player to send PAUSE command.";
        return;
    }

    qInfo() << "Sending PAUSE command to" << m_activePlayerPath.path();
    // Tạo một interface tạm thời chỉ để gọi lệnh
    auto *iface = new QDBusInterface(m_bluezService, m_activePlayerPath.path(), m_mediaPlayerInterfaceName, m_bus);
    iface->call("Pause");
    iface->deleteLater(); // Dọn dẹp ngay sau khi gọi
}

void BluetoothA2DPManager::playPause()
{
    if (m_isPlaying) {
        pause();
    } else {
        play();
    }
}

void BluetoothA2DPManager::next()
{
    if (m_activePlayerPath.path().isEmpty() || m_activePlayerPath.path() == "/") {
        qWarning() << "Control ignored: No active player to send NEXT command.";
        return;
    }

    qInfo() << "Sending NEXT command to" << m_activePlayerPath.path();
    // Tạo một interface tạm thời chỉ để gọi lệnh
    auto *iface = new QDBusInterface(m_bluezService, m_activePlayerPath.path(), m_mediaPlayerInterfaceName, m_bus);
    iface->call("Next");
    iface->deleteLater(); // Dọn dẹp ngay sau khi gọi
}

void BluetoothA2DPManager::previous()
{
    if (m_activePlayerPath.path().isEmpty() || m_activePlayerPath.path() == "/") {
        qWarning() << "Control ignored: No active player to send PREVIOUS command.";
        return;
    }

    qInfo() << "Sending PREVIOUS command to" << m_activePlayerPath.path();
    // Tạo một interface tạm thời chỉ để gọi lệnh
    auto *iface = new QDBusInterface(m_bluezService, m_activePlayerPath.path(), m_mediaPlayerInterfaceName, m_bus);
    iface->call("Previous");
    iface->deleteLater(); // Dọn dẹp ngay sau khi gọi
}

void BluetoothA2DPManager::initialDeviceCheck()
{
    QDBusInterface objectManager(m_bluezService, m_objectManagerPath, "org.freedesktop.DBus.ObjectManager", m_bus);
    QDBusReply<QMap<QDBusObjectPath, QMap<QString, QVariantMap>>> reply = objectManager.call("GetManagedObjects");

    if (!reply.isValid()) {
        qWarning() << "Could not get managed objects from BlueZ on startup.";
        return;
    }

    auto managedObjects = reply.value();
    for (auto it = managedObjects.constBegin(); it != managedObjects.constEnd(); ++it) {
        const QDBusObjectPath &path = it.key();
        const QMap<QString, QVariantMap> &interfaces = it.value();
        if (interfaces.contains(m_mediaPlayerInterfaceName)) {
            qInfo() << "Found pre-connected device at" << path.path();
            connectToMediaPlayer(path);
            return; // Chỉ xử lý thiết bị đầu tiên tìm thấy
        }
    }
}
