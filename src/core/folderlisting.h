#pragma once

#include <QFileInfoList>
#include <QString>
#include <QStringList>

QFileInfoList sortedEntries(const QString &folder, bool includeHidden);
QStringList browsableSiblings(const QString &path);
