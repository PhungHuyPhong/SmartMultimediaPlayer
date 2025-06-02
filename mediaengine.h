// MediaEngine.h
#ifndef MEDIAENGINE_H
#define MEDIAENGINE_H

#include <QObject>
#include <QStringList>
#include <QUrl>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoSink>
#include <QDebug>

class MediaEngine : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList playlist READ playlist NOTIFY playlistChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(qint64 position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(bool playing READ isPlaying NOTIFY playingChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)

public:
    explicit MediaEngine(QObject *parent = nullptr);
    QObject* videoSink() const;
    Q_INVOKABLE void addToPlaylist(const QUrl &url);
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void playPause();
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void setVolume(int volume); // 0 - 100
    Q_INVOKABLE void toggleMute();
    Q_INVOKABLE void setPosition(qint64 pos);

    QStringList playlist() const;
    int currentIndex() const;
    void setCurrentIndex(int index);
    qint64 duration() const;
    qint64 position() const;
    bool isPlaying() const;
    int volume() const;
    bool isMuted() const;
    void setMuted(bool m);

signals:
    void playlistChanged();
    void currentIndexChanged();
    void durationChanged();
    void positionChanged();
    void playingChanged();
    void volumeChanged();
    void mutedChanged();

private slots:
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onStateChanged(QMediaPlayer::PlaybackState state);

private:
    QMediaPlayer   m_player;
    QAudioOutput*  m_audioOutput;
    QVideoSink*    m_videoSink;
    QList<QUrl>    m_sources;
    int            m_currentIndex{-1};
};

#endif // MEDIAENGINE_H
