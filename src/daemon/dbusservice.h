#pragma once

#include "keek_config.h"

#include <QDBusContext>
#include <QObject>
#include <QStringList>

class Controller;

class DBusService : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", KEEK_DBUS_INTERFACE)

public:
    explicit DBusService(Controller *controller, QObject *parent = nullptr);

    bool registerOnBus(QString *error);

public Q_SLOTS:
    void Show(const QStringList &paths, const QString &activationToken);
    void Toggle(const QStringList &paths, const QString &activationToken);
    void Hide();
    void Next();
    void Previous();
    QString State();
    void Screenshot(const QString &file);
    void Quit();

private:
    bool rejectEmpty(const QStringList &paths);
    void move(bool forward);

    Controller *m_controller;
};
