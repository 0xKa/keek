#pragma once

#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

#include <chrono>

class QJSEngine;
class QQmlEngine;

class GeneralSettings : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(WhenClosed whenClosed READ whenClosed WRITE setWhenClosed NOTIFY changed FINAL)
    Q_PROPERTY(int stopAfter READ stopAfter WRITE setStopAfter NOTIFY changed FINAL)
    Q_PROPERTY(Unit stopAfterUnit READ stopAfterUnit WRITE setStopAfterUnit NOTIFY changed FINAL)
    Q_PROPERTY(int maximumStopAfter READ maximumStopAfter CONSTANT FINAL)

public:
    enum class WhenClosed {
        KeepReady,
        FreeMemory,
        Stop,
    };
    Q_ENUM(WhenClosed)

    enum class Unit {
        Seconds,
        Minutes,
    };
    Q_ENUM(Unit)

    static GeneralSettings *instance();
    static GeneralSettings *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);

    WhenClosed whenClosed() const;
    void setWhenClosed(WhenClosed whenClosed);
    int stopAfter() const;
    void setStopAfter(int count);
    Unit stopAfterUnit() const;
    void setStopAfterUnit(Unit unit);
    int maximumStopAfter() const;
    std::chrono::seconds stopDelay() const;

Q_SIGNALS:
    void changed();

private:
    explicit GeneralSettings(QObject *parent = nullptr);
};
