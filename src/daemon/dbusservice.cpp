#include "dbusservice.h"

#include "controller.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QJsonDocument>
#include <QTimer>

using namespace Qt::StringLiterals;

DBusService::DBusService(Controller *controller, QObject *parent)
    : QObject(parent)
    , m_controller(controller)
{
}

bool DBusService::registerOnBus(QString *error)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        *error = u"Could not connect to the session bus"_s;
        return false;
    }
    if (!bus.registerObject(QStringLiteral(KEEK_DBUS_PATH), this, QDBusConnection::ExportAllSlots)) {
        *error = u"Could not register the D-Bus object"_s;
        return false;
    }
    if (!bus.registerService(QStringLiteral(KEEK_DBUS_SERVICE))) {
        *error = u"keekd is already running"_s;
        return false;
    }
    return true;
}

void DBusService::Show(const QStringList &paths, const QString &activationToken)
{
    if (rejectEmpty(paths)) {
        return;
    }
    m_controller->show(paths, activationToken);
}

void DBusService::Toggle(const QStringList &paths, const QString &activationToken)
{
    if (rejectEmpty(paths)) {
        return;
    }
    m_controller->toggle(paths, activationToken);
}

void DBusService::Hide()
{
    m_controller->hide();
}

void DBusService::Next()
{
    move(true);
}

void DBusService::Previous()
{
    move(false);
}

void DBusService::move(bool forward)
{
    if (!m_controller->isShowing()) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::Failed, u"The popup is not visible"_s);
        }
        return;
    }
    if (forward) {
        m_controller->next();
    } else {
        m_controller->previous();
    }
}

QString DBusService::State()
{
    return QString::fromUtf8(QJsonDocument(m_controller->state()).toJson(QJsonDocument::Compact));
}

void DBusService::Screenshot(const QString &file)
{
    QString error;
    if (!m_controller->screenshot(file, &error) && calledFromDBus()) {
        sendErrorReply(QDBusError::Failed, error);
    }
}

void DBusService::Quit()
{
    QTimer::singleShot(0, qApp, &QCoreApplication::quit);
}

bool DBusService::rejectEmpty(const QStringList &paths)
{
    if (!paths.isEmpty()) {
        return false;
    }
    if (calledFromDBus()) {
        sendErrorReply(QDBusError::InvalidArgs, u"No files were given"_s);
    }
    return true;
}
