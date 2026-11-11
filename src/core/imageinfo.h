#pragma once

#include <QObject>
#include <QSize>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

class ImageInfo : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(QSize size READ size NOTIFY sourceChanged FINAL)
    Q_PROPERTY(bool scalable READ isScalable NOTIFY sourceChanged FINAL)

public:
    explicit ImageInfo(QObject *parent = nullptr);

    QUrl source() const;
    void setSource(const QUrl &source);

    QSize size() const;
    bool isScalable() const;

Q_SIGNALS:
    void sourceChanged();

private:
    QUrl m_source;
    QSize m_size;
    bool m_scalable = false;
};
