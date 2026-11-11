#pragma once

#include <QFutureWatcher>
#include <QObject>
#include <QUrl>
#include <QVariantList>
#include <QtQmlIntegration/qqmlintegration.h>

struct FolderTotals {
    qint64 bytes = 0;
    bool complete = false;
};

class FolderInfo : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(int maximumEntries READ maximumEntries WRITE setMaximumEntries NOTIFY sourceChanged FINAL)
    Q_PROPERTY(int fileCount READ fileCount NOTIFY sourceChanged FINAL)
    Q_PROPERTY(int folderCount READ folderCount NOTIFY sourceChanged FINAL)
    Q_PROPERTY(QVariantList entries READ entries NOTIFY sourceChanged FINAL)
    Q_PROPERTY(QString totalSize READ totalSize NOTIFY totalsChanged FINAL)
    Q_PROPERTY(bool counting READ isCounting NOTIFY totalsChanged FINAL)

public:
    explicit FolderInfo(QObject *parent = nullptr);
    ~FolderInfo() override;

    QUrl source() const;
    void setSource(const QUrl &source);
    int maximumEntries() const;
    void setMaximumEntries(int count);

    int fileCount() const;
    int folderCount() const;
    QVariantList entries() const;
    QString totalSize() const;
    bool isCounting() const;

Q_SIGNALS:
    void sourceChanged();
    void totalsChanged();

private:
    void load();
    void startCounting(const QString &folder);
    void stopCounting();

    QUrl m_source;
    int m_maximumEntries = 8;
    int m_fileCount = 0;
    int m_folderCount = 0;
    QVariantList m_entries;
    FolderTotals m_totals;
    QFutureWatcher<FolderTotals> m_watcher;
};
