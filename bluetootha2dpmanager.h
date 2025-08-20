#ifndef BLUETOOTHA2DPMANAGER_H
#define BLUETOOTHA2DPMANAGER_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDebug>
#include <QVariantMap>
#include <QDBusObjectPath>
#include <QDBusContext>


// Forward declaration
class QDBusMessage;
class QDBusInterface;

class BluetoothA2DPManager : public QObject, protected QDBusContext
{
    Q_OBJECT
    // Các thuộc tính này sẽ được đưa lên QML để hiển thị trên giao diện
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(QString trackTitle READ trackTitle NOTIFY trackChanged)
    Q_PROPERTY(QString trackArtist READ trackArtist NOTIFY trackChanged)
    Q_PROPERTY(QString trackAlbum READ trackAlbum NOTIFY trackChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playbackStatusChanged)

public:
    explicit BluetoothA2DPManager(QObject *parent = nullptr);
    bool initialize();

    // Các hàm để QML có thể truy cập
    bool isConnected() const;
    QString trackTitle() const;
    QString trackArtist() const;
    QString trackAlbum() const;
    bool isPlaying() const;
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void playPause();
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();

signals:
    // Tín hiệu để thông báo cho UI khi có thay đổi
    void connectionChanged();
    void trackChanged();
    void playbackStatusChanged();

private slots:
    // Slot để xử lý các tín hiệu từ D-Bus
    void onInterfacesAdded(const QDBusObjectPath &objectPath, const QVariantMap &interfaces);
    void onInterfacesRemoved(const QDBusObjectPath &objectPath, const QStringList &interfaces);
    void onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

private:
    void registerA2dpProfile();
    void initialDeviceCheck();
    void connectToMediaPlayer(const QDBusObjectPath &path);
    void disconnectMediaPlayer();
    void updateTrackInfo(const QVariantMap &properties);
    void updatePlaybackStatus(const QVariantMap &properties);

    QDBusConnection m_bus;
    QDBusObjectPath m_activePlayerPath; // Interface để điều khiển media player

    // Các hằng số D-Bus
    const QString m_bluezService = "org.bluez";
    const QString m_objectManagerPath = "/";
    const QString m_profileManagerPath = "/org/bluez";
    const QString m_mediaPlayerInterfaceName = "org.bluez.MediaPlayer1";
    const QString m_a2dpSinkProfilePath = "/com/phunghuyphong/a2dpsink";
    const QString m_a2dpSinkUUID = "0000110B-0000-1000-8000-00805F9B34FB";

    // Biến lưu trạng thái
    bool m_isConnected = false;
    QString m_trackTitle;
    QString m_trackArtist;
    QString m_trackAlbum;
    bool m_isPlaying = false;

};

#endif // BLUETOOTHA2DPMANAGER_H
