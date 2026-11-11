#include "folderinfo.h"

#include "folderlisting.h"

#include <KFileItem>
#include <KFormat>

#include <QDirListing>
#include <QElapsedTimer>
#include <QPromise>
#include <QtConcurrentRun>

using namespace Qt::StringLiterals;

namespace
{
constexpr qint64 progressIntervalMs = 200;

void countFolder(QPromise<FolderTotals> &promise, const QString &folder)
{
    using Flag = QDirListing::IteratorFlag;
    FolderTotals totals;
    QElapsedTimer sinceReport;
    sinceReport.start();

    for (const QDirListing::DirEntry &entry : QDirListing(folder, Flag::Recursive | Flag::IncludeHidden | Flag::ExcludeOther)) {
        if (promise.isCanceled()) {
            return;
        }
        if (entry.isFile() && !entry.isSymLink()) {
            totals.bytes += entry.size();
        }
        if (sinceReport.elapsed() >= progressIntervalMs) {
            promise.addResult(totals);
            sinceReport.restart();
        }
    }
    totals.complete = true;
    promise.addResult(totals);
}
}

FolderInfo::FolderInfo(QObject *parent)
    : QObject(parent)
{
    connect(&m_watcher, &QFutureWatcher<FolderTotals>::resultReadyAt, this, [this](int index) {
        m_totals = m_watcher.resultAt(index);
        Q_EMIT totalsChanged();
    });
}

FolderInfo::~FolderInfo()
{
    stopCounting();
}

QUrl FolderInfo::source() const
{
    return m_source;
}

void FolderInfo::setSource(const QUrl &source)
{
    if (m_source == source) {
        return;
    }
    m_source = source;
    load();
}

int FolderInfo::maximumEntries() const
{
    return m_maximumEntries;
}

void FolderInfo::setMaximumEntries(int count)
{
    if (m_maximumEntries == count) {
        return;
    }
    m_maximumEntries = count;
    load();
}

int FolderInfo::fileCount() const
{
    return m_fileCount;
}

int FolderInfo::folderCount() const
{
    return m_folderCount;
}

QVariantList FolderInfo::entries() const
{
    return m_entries;
}

QString FolderInfo::totalSize() const
{
    return KFormat().formatByteSize(static_cast<double>(m_totals.bytes));
}

bool FolderInfo::isCounting() const
{
    return !m_totals.complete;
}

void FolderInfo::load()
{
    stopCounting();
    m_fileCount = 0;
    m_folderCount = 0;
    m_entries.clear();
    m_totals = {};
    m_totals.complete = m_source.toLocalFile().isEmpty();

    const QString folder = m_source.toLocalFile();
    if (!folder.isEmpty()) {
        const QFileInfoList entries = sortedEntries(folder, false);
        for (const QFileInfo &entry : entries) {
            if (entry.isDir()) {
                ++m_folderCount;
            } else {
                ++m_fileCount;
            }
            if (m_entries.size() < m_maximumEntries) {
                const KFileItem item(QUrl::fromLocalFile(entry.absoluteFilePath()));
                m_entries.append(QVariantMap{{u"name"_s, entry.fileName()}, {u"iconName"_s, item.iconName()}});
            }
        }
        startCounting(folder);
    }

    Q_EMIT sourceChanged();
    Q_EMIT totalsChanged();
}

void FolderInfo::startCounting(const QString &folder)
{
    m_watcher.setFuture(QtConcurrent::run(countFolder, folder));
}

void FolderInfo::stopCounting()
{
    m_watcher.cancel();
    m_watcher.setFuture({});
}
