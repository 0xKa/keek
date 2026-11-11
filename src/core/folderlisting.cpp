#include "folderlisting.h"

#include <QCollator>
#include <QDir>

#include <algorithm>

namespace
{
QCollator naturalCollator()
{
    QLocale locale;
    if (locale.language() == QLocale::C) {
        locale = QLocale(QLocale::English);
    }
    QCollator collator(locale);
    collator.setNumericMode(true);
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    return collator;
}
}

QFileInfoList sortedEntries(const QString &folder, bool includeHidden)
{
    QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System;
    if (includeHidden) {
        filters |= QDir::Hidden;
    }
    QFileInfoList entries = QDir(folder).entryInfoList(filters, QDir::NoSort);

    const QCollator collator = naturalCollator();
    std::sort(entries.begin(), entries.end(), [&collator](const QFileInfo &a, const QFileInfo &b) {
        if (a.isDir() != b.isDir()) {
            return a.isDir();
        }
        const int order = collator.compare(a.fileName(), b.fileName());
        return order != 0 ? order < 0 : a.fileName() < b.fileName();
    });
    return entries;
}

QStringList browsableSiblings(const QString &path)
{
    const QFileInfo file(path);
    if (!file.exists() || file.isRoot()) {
        return {path};
    }

    const QFileInfoList entries = sortedEntries(file.absolutePath(), file.isHidden());
    QStringList paths;
    paths.reserve(entries.size());
    for (const QFileInfo &entry : entries) {
        paths.append(entry.absoluteFilePath());
    }
    if (!paths.contains(path)) {
        return {path};
    }
    return paths;
}
