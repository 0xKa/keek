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
    Q_PROPERTY(int tabWidth READ tabWidth WRITE setTabWidth NOTIFY tabWidthChanged FINAL)
    Q_PROPERTY(int columns READ columns NOTIFY columnsChanged FINAL)
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

    int tabWidth() const;
    void setTabWidth(int tabWidth);
    int columns() const;

Q_SIGNALS:
    void sourceChanged();
    void contentChanged();
    void tabWidthChanged();
    void columnsChanged();

private:
    void load();
    void measure();

    QUrl m_source;
    QString m_text;
    QString m_error;
    bool m_ready = false;
    bool m_truncated = false;
    int m_tabWidth = 4;
    int m_columns = 0;
};
