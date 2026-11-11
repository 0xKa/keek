#include "jsonlog.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include <cstdio>
#include <cstdlib>

using namespace Qt::StringLiterals;

namespace
{
QString levelName(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return u"debug"_s;
    case QtInfoMsg:
        return u"info"_s;
    case QtWarningMsg:
        return u"warning"_s;
    case QtCriticalMsg:
        return u"critical"_s;
    case QtFatalMsg:
        return u"fatal"_s;
    }
    return u"unknown"_s;
}

void writeJsonLine(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    const QJsonObject entry{
        {u"time"_s, QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
        {u"level"_s, levelName(type)},
        {u"category"_s, QString::fromUtf8(context.category ? context.category : "default")},
        {u"message"_s, message},
    };
    const QByteArray line = QJsonDocument(entry).toJson(QJsonDocument::Compact) + '\n';
    std::fwrite(line.constData(), 1, static_cast<size_t>(line.size()), stderr);
    std::fflush(stderr);

    if (type == QtFatalMsg) {
        std::abort();
    }
}
}

void installJsonLog()
{
    qInstallMessageHandler(writeJsonLine);
}
