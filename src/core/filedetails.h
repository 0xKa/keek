#pragma once

#include <QObject>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

class FileDetails : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(bool exists READ exists NOTIFY sourceChanged FINAL)
    Q_PROPERTY(QString iconName READ iconName NOTIFY sourceChanged FINAL)
    Q_PROPERTY(QString typeName READ typeName NOTIFY sourceChanged FINAL)
    Q_PROPERTY(QString size READ size NOTIFY sourceChanged FINAL)
    Q_PROPERTY(QString modified READ modified NOTIFY sourceChanged FINAL)

public:
    explicit FileDetails(QObject *parent = nullptr);

    QUrl source() const;
    void setSource(const QUrl &source);

    bool exists() const;
    QString iconName() const;
    QString typeName() const;
    QString size() const;
    QString modified() const;

Q_SIGNALS:
    void sourceChanged();

private:
    QUrl m_source;
    bool m_exists = false;
    QString m_iconName;
    QString m_typeName;
    QString m_size;
    QString m_modified;
};
