#include "bluetootha2dpmanager.h"


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
    QDBusInterface profileManager(m_bluezService, m_profileManagerPath, "org.bluez.ProfileManager1", m_bus);
    if (!profileManager.isValid()) {
        qCritical() << "Can not find BlueZ Profile Manager.";
        return;
    }
    QVariantMap profileOptions;
    profileOptions.insert("Name", "Smart Multimedia Player");
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
        qInfo() << "Detect Media Player at:" << objectPath.path();
        connectToMediaPlayer(objectPath);
    }
}

void BluetoothA2DPManager::onInterfacesRemoved(const QDBusObjectPath &objectPath, const QStringList &interfaces)
{
    if (interfaces.contains(m_mediaPlayerInterfaceName)) {
        qInfo() << "Remove Media Player at:" << objectPath.path();
        disconnectMediaPlayer(objectPath);
    }
}

void BluetoothA2DPManager::connectToMediaPlayer(const QDBusObjectPath &path)
{
    m_connectedDevicePath = path;

    // Ngắt kết nối với trình phát cũ nếu có
    if (m_mediaPlayerInterface) {
        delete m_mediaPlayerInterface;
        m_mediaPlayerInterface = nullptr;
    }

    // Tạo interface mới để giao tiếp với trình phát media của thiết bị vừa kết nối
    m_mediaPlayerInterface = new QDBusInterface(
        m_bluezService,
        path.path(),
        m_mediaPlayerInterfaceName,
        m_bus,
        this
        );

    if (!m_mediaPlayerInterface->isValid()) {
        qCritical() << "Cannot initialize interface for Media Player!";
        delete m_mediaPlayerInterface;
        m_mediaPlayerInterface = nullptr;
        return;
    }

    // Kết nối tới tín hiệu "PropertiesChanged" của trình phát media này.
    // Tín hiệu này sẽ được phát ra mỗi khi có gì đó thay đổi (đổi bài, play/pause,...)
    bool success=m_bus.connect(
        m_bluezService,
        path.path(),
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged",
        this,
        SLOT(onPropertiesChanged(QString, QVariantMap, QStringList))
        );
    if (!success) {
        qCritical() << "Can not connect to signal PropertiesChanged.";
        return;
    }
    else qInfo() << "Connected to signal PropertiesChanged.";
    if (m_isConnected == false) {
        m_isConnected = true;
        emit connectionChanged();
        qInfo() << "Device connected.";
    }

    // Lấy thông tin ban đầu của bài hát
    QDBusReply<QVariantMap> properties = m_mediaPlayerInterface->call("GetAll", m_mediaPlayerInterfaceName);
    if(properties.isValid()){
        qInfo() <<"Updating data";
        updateTrackInfo(properties.value());
        updatePlaybackStatus(properties.value());
        qInfo() <<"Updating done";
    }
    else qWarning()<<"Can not get metadata";
}

void BluetoothA2DPManager::disconnectMediaPlayer(const QDBusObjectPath &path)
{
    qInfo() << "Diconnecting....";
    if (m_mediaPlayerInterface) {
        delete m_mediaPlayerInterface;
        m_mediaPlayerInterface = nullptr;
    }
    bool success=m_bus.disconnect(
        m_bluezService,
        path.path(),
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
    qInfo() << "Deleting track data...";
    m_trackAlbum.clear();
    m_trackArtist.clear();
    m_trackTitle.clear();
    qInfo() << "Deleted track data.";
    m_isConnected = false;
    m_isPlaying = false;
    qInfo() << "Disconnected";
}

// Slot này được gọi khi có sự thay đổi thuộc tính trên thiết bị (đổi bài, play/pause)
void BluetoothA2DPManager::onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties);
    if (interfaceName != m_mediaPlayerInterfaceName) {
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

void BluetoothA2DPManager::initialDeviceCheck()
{
    QDBusInterface objectManager(m_bluezService, m_objectManagerPath, "org.freedesktop.DBus.ObjectManager", m_bus);
    QDBusReply<QMap<QDBusObjectPath, QMap<QString, QVariantMap>>> reply = objectManager.call("GetManagedObjects");

    if (!reply.isValid()) {
        qWarning() << "Can not get list of objects from BlueZ.";
        return;
    }

    auto managedObjects = reply.value();
    for (auto it = managedObjects.constBegin(); it != managedObjects.constEnd(); ++it) {
        const QDBusObjectPath &path = it.key();
        const QMap<QString, QVariantMap> &interfaces = it.value();
        if (interfaces.contains(m_mediaPlayerInterfaceName)) {
            qInfo() << "Detect connected device at:" << path.path();
            connectToMediaPlayer(path);
            return; // Chỉ xử lý thiết bị đầu tiên tìm thấy
        }
    }
}
