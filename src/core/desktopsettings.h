#pragma once

#include <KConfigWatcher>

#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

class DesktopSettings : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(double animationFactor READ animationFactor NOTIFY animationFactorChanged FINAL)

public:
    explicit DesktopSettings(QObject *parent = nullptr);

    double animationFactor() const;

Q_SIGNALS:
    void animationFactorChanged();

private:
    void read();

    KSharedConfig::Ptr m_config;
    KConfigWatcher::Ptr m_watcher;
    double m_animationFactor = 1.0;
};
