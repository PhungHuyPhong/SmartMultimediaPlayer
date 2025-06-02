#include "mediaengine.h"

MediaEngine::MediaEngine(QObject *parent)
    : QObject(parent)
    , m_player(this)
    , m_videoSink(new QVideoSink(this))
    , m_audioOutput(new QAudioOutput(this))
{
    m_player.setAudioOutput(m_audioOutput);
    m_player.setVideoOutput(m_videoSink);
    // QQmlEngine::setObjectOwnership(m_videoSink, QQmlEngine::CppOwnership);
    connect(&m_player, &QMediaPlayer::positionChanged, this, &MediaEngine::onPositionChanged);
    connect(&m_player, &QMediaPlayer::durationChanged, this, &MediaEngine::onDurationChanged);
    connect(&m_player, &QMediaPlayer::playbackStateChanged, this, &MediaEngine::onStateChanged);
}

void MediaEngine::addToPlaylist(const QUrl &url) {
    m_sources.append(url);
    if (m_currentIndex < 0) {
        m_currentIndex = 0;
        m_player.setSource(m_sources[0]);
    }
    emit playlistChanged();
}

void MediaEngine::play() {
    if (m_currentIndex >= 0)
        m_player.play();
}

void MediaEngine::pause() {
    m_player.pause();
}

void MediaEngine::playPause() {
    if (m_player.playbackState() == QMediaPlayer::PlayingState)
        m_player.pause();
    else
        m_player.play();
}

void MediaEngine::next() {
    if (m_sources.isEmpty()) return;
    m_currentIndex = (m_currentIndex + 1) % m_sources.size();
    m_player.setSource(m_sources[m_currentIndex]);
    m_player.play();
    emit currentIndexChanged();
}

void MediaEngine::previous() {
    if (m_sources.isEmpty()) return;
    m_currentIndex = (m_currentIndex - 1 + m_sources.size()) % m_sources.size();
    m_player.setSource(m_sources[m_currentIndex]);
    m_player.play();
    emit currentIndexChanged();
}

QStringList MediaEngine::playlist() const {
    QStringList list;
    for (const auto &url : m_sources)
        list << url.toString();
    return list;
}

int MediaEngine::currentIndex() const {
    return m_currentIndex;
}

void MediaEngine::setCurrentIndex(int index) {
    if (index < 0 || index >= m_sources.size()) return;
    m_currentIndex = index;
    m_player.setSource(m_sources[m_currentIndex]);
    m_player.play();
    emit currentIndexChanged();
}

qint64 MediaEngine::duration() const {
    return m_player.duration();
}

qint64 MediaEngine::position() const {
    return m_player.position();
}

void MediaEngine::setPosition(qint64 pos) {
    m_player.setPosition(pos);
}

bool MediaEngine::isPlaying() const {
    return m_player.playbackState() == QMediaPlayer::PlayingState;
}

int MediaEngine::volume() const {
    return static_cast<int>(m_audioOutput->volume() * 100);
}

void MediaEngine::setVolume(int volume) {
    qreal v = qBound(qreal(0.0), volume / qreal(100.0), qreal(1.0));
    m_audioOutput->setVolume(v);
    emit volumeChanged();
}

void MediaEngine::onPositionChanged(qint64) {
    emit positionChanged();
}

void MediaEngine::onDurationChanged(qint64) {
    emit durationChanged();
}

void MediaEngine::onStateChanged(QMediaPlayer::PlaybackState) {
    emit playingChanged();
}

bool MediaEngine::isMuted() const {
    return m_audioOutput->isMuted();
}

void MediaEngine::setMuted(bool m) {
    if (m_audioOutput->isMuted() != m) {
        m_audioOutput->setMuted(m);
        emit mutedChanged();
    }
}

void MediaEngine::toggleMute() {
    setMuted(!isMuted());
}

QObject* MediaEngine::videoSink() const {
    return m_videoSink;
}
