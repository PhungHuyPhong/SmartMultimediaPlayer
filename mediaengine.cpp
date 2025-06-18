#include "mediaengine.h"

MediaEngine::MediaEngine(QObject *parent)
    : QObject(parent)
    , m_player(this)
    , m_videoSink(new QVideoSink(this))
    , m_audioOutput(new QAudioOutput(this))
{
    m_player.setAudioOutput(m_audioOutput);
    m_player.setVideoOutput(m_videoSink);
    connect(&m_player, &QMediaPlayer::positionChanged, this, &MediaEngine::onPositionChanged);
    connect(&m_player, &QMediaPlayer::durationChanged, this, &MediaEngine::onDurationChanged);
    connect(&m_player, &QMediaPlayer::playbackStateChanged, this, &MediaEngine::onStateChanged);
}

void MediaEngine::addToPlaylist(const QUrl &url) {
    QString path = url.toLocalFile();
    QFileInfo info(path);

    if (!info.exists())
        return;

    QStringList newSources;

    if (info.isDir()) {
        QDir dir(path);
        QStringList nameFilters;
        nameFilters << "*.mp3" << "*.wav" << "*.mp4" << "*.mkv" << "*.avi";

        QFileInfoList fileList = dir.entryInfoList(nameFilters, QDir::Files);
        for (const QFileInfo &fileInfo : fileList) {
            QUrl fileUrl = QUrl::fromLocalFile(fileInfo.absoluteFilePath());
            newSources << fileUrl.toString();
        }
    } else if (info.isFile()) {
        newSources << url.toString();
    }

    // Add to existing playlist
    for (const QString &file : newSources) {
        m_sources.append(QUrl(file));
    }

    // If playlist was empty or newly added files, play the first new file
    if (!newSources.isEmpty()) {
        m_currentIndex = m_sources.size() - newSources.size();
        m_player.setSource(m_sources[m_currentIndex]);
        m_hasVideo = m_player.hasVideo();
        emit titleChanged();
        emit hasVideoChanged();

        // Auto start playing
        m_player.play();
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
    if (m_shuffle) {
        m_currentIndex = QRandomGenerator::global()->bounded(m_sources.size());
    } else {
        m_currentIndex++;
        if (m_currentIndex >= m_sources.size()) {
            if (m_loopAll) {
                m_currentIndex = 0;
            } else {
                m_currentIndex = m_sources.size() - 1;
                return;
            }
        }
    }
    m_player.setSource(m_sources[m_currentIndex]);
    m_hasVideo = m_player.hasVideo();
    emit hasVideoChanged();
    m_player.play();
    emit currentIndexChanged();
    emit titleChanged();
}

void MediaEngine::previous() {
    if (m_sources.isEmpty()) return;
    m_currentIndex = (m_currentIndex - 1 + m_sources.size()) % m_sources.size();

    m_player.setSource(m_sources[m_currentIndex]);
    m_hasVideo = m_player.hasVideo();
    emit hasVideoChanged();
    m_player.play();
    emit currentIndexChanged();
    emit titleChanged();
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
    m_hasVideo = m_player.hasVideo();
    emit hasVideoChanged();
    m_player.play();
    emit currentIndexChanged();
    emit titleChanged();
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

void MediaEngine::onStateChanged(QMediaPlayer::PlaybackState state) {
    emit playingChanged();
    if (state == QMediaPlayer::StoppedState) {
        if (m_loopOne) {
            m_player.setSource(m_sources[m_currentIndex]);
            m_player.play();
        } else if (m_loopAll || m_shuffle) {
            next();
        }
    }
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

// QObject* MediaEngine::videoSink() const {
//     return m_videoSink;
// }

bool MediaEngine::hasVideo() const {
    return m_hasVideo;
}

QString MediaEngine::title() const {
    if (m_currentIndex >= 0 && m_currentIndex < m_sources.size()) {
        return m_sources[m_currentIndex].fileName();
    }
    return QString();
}
bool MediaEngine::isLoopOne() const { return m_loopOne; }
void MediaEngine::setLoopOne(bool l) {
    if (m_loopOne != l) {
        m_loopOne = l;
        emit loopOneChanged();
    }
}
bool MediaEngine::isLoopAll() const { return m_loopAll; }
void MediaEngine::setLoopAll(bool l) {
    if (m_loopAll != l) {
        m_loopAll = l;
        emit loopAllChanged();
    }
}
bool MediaEngine::isShuffle() const { return m_shuffle; }
void MediaEngine::setShuffle(bool s) {
    if (m_shuffle != s) {
        m_shuffle = s;
        emit shuffleChanged();
    }
}
void MediaEngine::setVideoSink(QVideoSink *sink) {
    if (m_videoSink == sink)
        return;
    m_videoSink = sink;
    m_player.setVideoOutput(m_videoSink);
    emit videoSinkChanged();
}
