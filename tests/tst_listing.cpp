#include "folderlisting.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;

class ListingTest : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir m_dir;

    QString path(const QString &name) const
    {
        return m_dir.filePath(name);
    }

    static QStringList names(const QStringList &paths)
    {
        QStringList result;
        for (const QString &path : paths) {
            result.append(QFileInfo(path).fileName());
        }
        return result;
    }

private Q_SLOTS:
    void initTestCase()
    {
        QVERIFY(m_dir.isValid());
        for (const QString &name : {u"file10.txt"_s, u"file2.txt"_s, u"File3.txt"_s, u"zeta"_s, u".hidden"_s}) {
            QFile file(path(name));
            QVERIFY(file.open(QIODevice::WriteOnly));
        }
        QVERIFY(QDir(m_dir.path()).mkdir(u"b-folder"_s));
        QVERIFY(QDir(m_dir.path()).mkdir(u"A-folder"_s));
    }

    void foldersFirstThenNaturalOrder()
    {
        const QStringList expected = {u"A-folder"_s, u"b-folder"_s, u"file2.txt"_s, u"File3.txt"_s, u"file10.txt"_s, u"zeta"_s};
        QCOMPARE(names(browsableSiblings(path(u"file2.txt"_s))), expected);
        QCOMPARE(names(browsableSiblings(path(u"A-folder"_s))), expected);
    }

    void hiddenFilesOnlyFromAHiddenFile()
    {
        const QStringList siblings = names(browsableSiblings(path(u".hidden"_s)));
        QCOMPARE(siblings.size(), 7);
        QVERIFY(siblings.contains(u".hidden"_s));
        QVERIFY(!names(browsableSiblings(path(u"zeta"_s))).contains(u".hidden"_s));
    }

    void aMissingFileStandsAlone()
    {
        const QString missing = path(u"missing.txt"_s);
        QCOMPARE(browsableSiblings(missing), QStringList{missing});
    }

    void theRootStandsAlone()
    {
        QCOMPARE(browsableSiblings(u"/"_s), QStringList{u"/"_s});
    }

    void sortedEntriesListsAFolder()
    {
        const QFileInfoList entries = sortedEntries(m_dir.path(), false);
        QCOMPARE(entries.size(), 6);
        QVERIFY(entries.constFirst().isDir());
        QCOMPARE(entries.constLast().fileName(), u"zeta"_s);
    }
};

QTEST_GUILESS_MAIN(ListingTest)

#include "tst_listing.moc"
