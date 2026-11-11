#include "keek_config.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusError>
#include <QDBusMessage>
#include <QDeadlineTimer>
#include <QDir>
#include <QTextStream>
#include <QThread>

using namespace Qt::StringLiterals;

namespace
{
enum ExitCode {
    Ok = 0,
    UsageError = 1,
    DaemonUnreachable = 2,
    DaemonError = 3,
};

constexpr int callTimeoutMs = 3000;

void printUsage(QTextStream &out)
{
    out << "Usage: keek <command> [arguments]\n"
           "\n"
           "Commands:\n"
           "  show <paths>        Show the popup for these files\n"
           "  toggle <paths>      Show the popup, or hide it if it shows that file\n"
           "  hide                Hide the popup\n"
           "  state               Print the current state as JSON\n"
           "  screenshot <file>   Save what the popup is drawing to an image file\n"
           "  quit                Stop the daemon\n"
           "\n"
           "Exit codes: 0 success, 1 usage error, 2 keekd not reachable, 3 keekd reported an error\n";
}

int usageError(const QString &message)
{
    QTextStream err(stderr);
    err << "keek: " << message << "\n\n";
    printUsage(err);
    return UsageError;
}

QStringList absolutePaths(const QStringList &paths)
{
    QStringList result;
    result.reserve(paths.size());
    for (const QString &path : paths) {
        result.append(QDir::cleanPath(QDir::current().absoluteFilePath(path)));
    }
    return result;
}

QDBusMessage methodCall(const QString &method, const QVariantList &arguments)
{
    QDBusMessage message =
        QDBusMessage::createMethodCall(QStringLiteral(KEEK_DBUS_SERVICE), QStringLiteral(KEEK_DBUS_PATH), QStringLiteral(KEEK_DBUS_INTERFACE), method);
    message.setArguments(arguments);
    return message;
}

int call(const QString &method, const QVariantList &arguments = {}, QString *reply = nullptr)
{
    const QDBusMessage response = QDBusConnection::sessionBus().call(methodCall(method, arguments), QDBus::Block, callTimeoutMs);
    if (response.type() != QDBusMessage::ErrorMessage) {
        if (reply && !response.arguments().isEmpty()) {
            *reply = response.arguments().constFirst().toString();
        }
        return Ok;
    }

    QTextStream err(stderr);
    switch (QDBusError(response).type()) {
    case QDBusError::ServiceUnknown:
    case QDBusError::Disconnected:
        err << "keek: keekd is not running\n";
        return DaemonUnreachable;
    case QDBusError::NoReply:
    case QDBusError::Timeout:
    case QDBusError::TimedOut:
        err << "keek: keekd did not answer\n";
        return DaemonUnreachable;
    default:
        err << "keek: " << response.errorMessage() << '\n';
        return DaemonError;
    }
}

int quitDaemon()
{
    QTextStream err(stderr);
    QDBusConnection bus = QDBusConnection::sessionBus();
    const QString service = QStringLiteral(KEEK_DBUS_SERVICE);
    if (!bus.isConnected() || !bus.interface()->isServiceRegistered(service)) {
        err << "keek: keekd is not running\n";
        return DaemonUnreachable;
    }

    bus.call(methodCall(u"Quit"_s, {}), QDBus::NoBlock);

    QDeadlineTimer deadline(callTimeoutMs);
    while (bus.interface()->isServiceRegistered(service)) {
        if (deadline.hasExpired()) {
            err << "keek: keekd did not stop\n";
            return DaemonError;
        }
        QThread::msleep(20);
    }
    return Ok;
}
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(u"keek"_s);
    app.setApplicationVersion(QStringLiteral(KEEK_VERSION));

    QStringList arguments = app.arguments().mid(1);
    if (arguments.isEmpty()) {
        return usageError(u"no command given"_s);
    }

    QTextStream out(stdout);
    const QString command = arguments.takeFirst();

    if (command == "--help"_L1 || command == "-h"_L1 || command == "help"_L1) {
        printUsage(out);
        return Ok;
    }
    if (command == "--version"_L1 || command == "-v"_L1) {
        out << "keek " KEEK_VERSION "\n";
        return Ok;
    }

    if (command == "show"_L1 || command == "toggle"_L1) {
        if (arguments.isEmpty()) {
            return usageError(u"%1 needs at least one path"_s.arg(command));
        }
        const QString token = qEnvironmentVariable("XDG_ACTIVATION_TOKEN");
        return call(command == "show"_L1 ? u"Show"_s : u"Toggle"_s, {absolutePaths(arguments), token});
    }

    if (command == "screenshot"_L1) {
        if (arguments.size() != 1) {
            return usageError(u"screenshot needs exactly one file"_s);
        }
        return call(u"Screenshot"_s, {absolutePaths(arguments).constFirst()});
    }

    if (command != "hide"_L1 && command != "state"_L1 && command != "quit"_L1) {
        return usageError(u"unknown command %1"_s.arg(command));
    }
    if (!arguments.isEmpty()) {
        return usageError(u"%1 takes no arguments"_s.arg(command));
    }

    if (command == "hide"_L1) {
        return call(u"Hide"_s);
    }
    if (command == "state"_L1) {
        QString json;
        const int result = call(u"State"_s, {}, &json);
        if (result == Ok) {
            out << json << '\n';
        }
        return result;
    }
    return quitDaemon();
}
