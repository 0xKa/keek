#pragma once

#include <QAnyStringView>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QString>
#include <QTimeZone>

#include <chrono>
#include <filesystem>

inline QString fixture(QAnyStringView name)
{
    return QStringLiteral(KEEK_FIXTURES_DIR "/") + name.toString();
}

inline QDateTime fixtureTime()
{
    return QDateTime(QDate(2026, 1, 2), QTime(3, 4), QTimeZone::UTC);
}

inline bool copyFixtures(const QString &target)
{
    const QDir source(QStringLiteral(KEEK_FIXTURES_DIR));
    QStringList copied;
    QDirIterator it(source.path(), QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QFileInfo entry = it.nextFileInfo();
        const QString destination = target + u'/' + source.relativeFilePath(entry.filePath());
        if (entry.isDir() ? !QDir().mkpath(destination) : !QFile::copy(entry.filePath(), destination)) {
            return false;
        }
        copied.prepend(destination);
    }
    const std::chrono::sys_seconds time(std::chrono::seconds(fixtureTime().toSecsSinceEpoch()));
    for (const QString &path : std::as_const(copied)) {
        std::error_code error;
        std::filesystem::last_write_time(path.toStdString(), std::chrono::clock_cast<std::chrono::file_clock>(time), error);
        if (error) {
            return false;
        }
    }
    return true;
}
