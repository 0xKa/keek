#pragma once

#include <QFont>
#include <QObject>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

class TextFile : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(QString fileName READ fileName NOTIFY sourceChanged FINAL)
    Q_PROPERTY(QString text READ text NOTIFY contentChanged FINAL)
    Q_PROPERTY(bool ready READ isReady NOTIFY contentChanged FINAL)
    Q_PROPERTY(bool truncated READ isTruncated NOTIFY contentChanged FINAL)
    Q_PROPERTY(QString error READ error NOTIFY contentChanged FINAL)
    Q_PROPERTY(QFont fixedFont READ fixedFont CONSTANT FINAL)

public:
    static constexpr qint64 maxBytes = 512 * 1024;

    explicit TextFile(QObject *parent = nullptr);

    QUrl source() const;
    void setSource(const QUrl &source);

    QString fileName() const;
    QString text() const;
    bool isReady() const;
    bool isTruncated() const;
    QString error() const;
    QFont fixedFont() const;

Q_SIGNALS:
    void sourceChanged();
    void contentChanged();

private:
    void load();

    QUrl m_source;
    QString m_text;
    QString m_error;
    bool m_ready = false;
    bool m_truncated = false;
};
