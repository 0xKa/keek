#pragma once

#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

class MediaSettings : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool videoMuted READ isVideoMuted WRITE setVideoMuted NOTIFY changed FINAL)
    Q_PROPERTY(bool audioMuted READ isAudioMuted WRITE setAudioMuted NOTIFY changed FINAL)
    Q_PROPERTY(bool autoplay READ autoplay CONSTANT FINAL)

public:
    explicit MediaSettings(QObject *parent = nullptr);

    bool isVideoMuted() const;
    void setVideoMuted(bool muted);
    bool isAudioMuted() const;
    void setAudioMuted(bool muted);
    bool autoplay() const;

Q_SIGNALS:
    void changed();

private:
    void write(const char *key, bool value);
};
