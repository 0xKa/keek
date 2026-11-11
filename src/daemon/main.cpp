#include "controller.h"
#include "dbusservice.h"
#include "jsonlog.h"
#include "keek_config.h"
#include "logging.h"

#include <QCommandLineParser>
#include <QGuiApplication>

using namespace Qt::StringLiterals;

int main(int argc, char **argv)
{
    installJsonLog();

    QGuiApplication app(argc, argv);
    app.setApplicationName(u"keekd"_s);
    app.setApplicationVersion(QStringLiteral(KEEK_VERSION));
    app.setDesktopFileName(QStringLiteral(KEEK_APP_ID));
    app.setQuitOnLastWindowClosed(false);

    QCommandLineParser parser;
    parser.setApplicationDescription(u"Keek preview daemon"_s);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    Controller controller;
    if (!controller.isReady()) {
        qCCritical(lcKeek) << "The popup could not be loaded";
        return 1;
    }

    DBusService service(&controller);
    QString error;
    if (!service.registerOnBus(&error)) {
        qCCritical(lcKeek).noquote() << error;
        return 1;
    }

    qCInfo(lcKeek) << "ready";
    return app.exec();
}
