#pragma once

#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

class QJSEngine;
class QQmlEngine;

class GeneralSettings : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(WhenClosed whenClosed READ whenClosed WRITE setWhenClosed NOTIFY changed FINAL)
    Q_PROPERTY(int stopAfterMinutes READ stopAfterMinutes WRITE setStopAfterMinutes NOTIFY changed FINAL)
    Q_PROPERTY(int minimumStopAfterMinutes READ minimumStopAfterMinutes CONSTANT FINAL)
    Q_PROPERTY(int maximumStopAfterMinutes READ maximumStopAfterMinutes CONSTANT FINAL)

public:
    enum class WhenClosed {
        KeepReady,
        FreeMemory,
        Stop,
    };
    Q_ENUM(WhenClosed)

    static GeneralSettings *instance();
    static GeneralSettings *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);

    WhenClosed whenClosed() const;
    void setWhenClosed(WhenClosed whenClosed);
    int stopAfterMinutes() const;
    void setStopAfterMinutes(int minutes);
    int minimumStopAfterMinutes() const;
    int maximumStopAfterMinutes() const;

Q_SIGNALS:
    void changed();

private:
    explicit GeneralSettings(QObject *parent = nullptr);
};
