#include "fixtures.h"
#include "folderinfo.h"
#include "imageinfo.h"
#include "mediasettings.h"
#include "rendererregistry.h"
#include "textfile.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;

namespace
{
QString writeFile(const QTemporaryDir &dir, const QString &name, const QByteArray &data)
{
    const QString path = dir.filePath(name);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return {};
    }
    file.write(data);
    return path;
}
}

class RenderersTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void selection_data()
    {
        QTest::addColumn<QString>("path");
        QTest::addColumn<QString>("renderer");

        QTest::newRow("png") << fixture("image-landscape.png") << u"image"_s;
        QTest::newRow("jpeg") << fixture("image-portrait.jpg") << u"image"_s;
        QTest::newRow("webp") << fixture("image-wide.webp") << u"image"_s;
        QTest::newRow("svg") << fixture("image-vector.svg") << u"image"_s;
        QTest::newRow("broken png") << fixture("image-broken.png") << u"image"_s;
        QTest::newRow("plain text") << fixture("text-plain.txt") << u"text"_s;
        QTest::newRow("c++") << fixture("text-code.cpp") << u"text"_s;
        QTest::newRow("python") << fixture("text-script.py") << u"text"_s;
        QTest::newRow("json") << fixture("text-data.json") << u"text"_s;
        QTest::newRow("markdown") << fixture("text-notes.md") << u"text"_s;
        QTest::newRow("ini, by content") << fixture("text-settings.ini") << u"text"_s;
        QTest::newRow("arabic") << fixture("text-arabic.txt") << u"text"_s;
        QTest::newRow("webm") << fixture("video-clip.webm") << u"video"_s;
        QTest::newRow("broken mp4") << fixture("video-broken.mp4") << u"video"_s;
        QTest::newRow("flac") << fixture("audio-tone.flac") << u"audio"_s;
        QTest::newRow("binary") << fixture("binary.bin") << u"fallback"_s;
        QTest::newRow("empty") << fixture("empty") << u"fallback"_s;
        QTest::newRow("folder") << fixture("folder") << u"folder"_s;
        QTest::newRow("pdf") << fixture("pdf-document.pdf") << u"pdf"_s;
        QTest::newRow("broken pdf") << fixture("pdf-broken.pdf") << u"pdf"_s;
        QTest::newRow("password pdf") << fixture("pdf-password.pdf") << u"pdf"_s;
        QTest::newRow("pdf, by name") << u"/nonexistent/document.pdf"_s << u"pdf"_s;
        QTest::newRow("png, by name") << u"/nonexistent/picture.png"_s << u"image"_s;
    }

    void selection()
    {
        QFETCH(QString, path);
        QFETCH(QString, renderer);

        const RendererRegistry registry;
        const RendererRegistry::Match match = registry.select(path);
        QCOMPARE(match.renderer->id, renderer);
        QVERIFY(match.mimeType.isValid());
    }

    void textFileReadsUtf8()
    {
        TextFile file;
        file.setSource(QUrl::fromLocalFile(fixture("text-plain.txt")));

        QVERIFY(file.isReady());
        QVERIFY(file.error().isEmpty());
        QVERIFY(!file.isTruncated());
        QVERIFY(file.text().contains(u"café"_s));
        QVERIFY(file.text().contains(u"日本語"_s));
        QCOMPARE(file.fileName(), u"text-plain.txt"_s);
    }

    void textFileFallsBackToLatin1()
    {
        QTemporaryDir dir;
        const QString path = writeFile(dir, u"latin1.txt"_s, "caf\xe9\n");

        TextFile file;
        file.setSource(QUrl::fromLocalFile(path));
        QVERIFY(file.isReady());
        QCOMPARE(file.text(), u"café\n"_s);
    }

    void textFileReadsUtf16WithByteOrderMark()
    {
        QTemporaryDir dir;
        const QString path = writeFile(dir, u"utf16.txt"_s, QByteArray("\xff\xfeh\0i\0\n\0", 8));

        TextFile file;
        file.setSource(QUrl::fromLocalFile(path));
        QVERIFY(file.isReady());
        QCOMPARE(file.text(), u"hi\n"_s);
    }

    void textFileTruncatesAtALineEnd()
    {
        QTemporaryDir dir;
        QByteArray data;
        while (data.size() < TextFile::maxBytes + 4096) {
            data.append("a line of text that repeats\n");
        }
        const QString path = writeFile(dir, u"big.txt"_s, data);

        TextFile file;
        file.setSource(QUrl::fromLocalFile(path));
        QVERIFY(file.isReady());
        QVERIFY(file.isTruncated());
        QVERIFY(file.text().size() <= TextFile::maxBytes);
        QVERIFY(file.text().endsWith(u'\n'));
    }

    void textFileRejectsBinaryData()
    {
        TextFile file;
        file.setSource(QUrl::fromLocalFile(fixture("binary.bin")));
        QVERIFY(!file.isReady());
        QVERIFY(!file.error().isEmpty());
        QVERIFY(file.text().isEmpty());
    }

    void textFileReportsAMissingFile()
    {
        TextFile file;
        file.setSource(QUrl::fromLocalFile(u"/nonexistent/file.txt"_s));
        QVERIFY(!file.isReady());
        QVERIFY(!file.error().isEmpty());
    }

    void textFileResetsWhenTheSourceIsCleared()
    {
        TextFile file;
        file.setSource(QUrl::fromLocalFile(fixture("text-plain.txt")));
        file.setSource(QUrl());
        QVERIFY(!file.isReady());
        QVERIFY(file.text().isEmpty());
        QVERIFY(file.error().isEmpty());
    }

    void imageInfoReadsTheSize()
    {
        ImageInfo info;
        info.setSource(QUrl::fromLocalFile(fixture("image-landscape.png")));
        QCOMPARE(info.size(), QSize(640, 400));
        QVERIFY(!info.isScalable());
    }

    void imageInfoMarksVectorImagesScalable()
    {
        ImageInfo info;
        info.setSource(QUrl::fromLocalFile(fixture("image-vector.svg")));
        QCOMPARE(info.size(), QSize(200, 120));
        QVERIFY(info.isScalable());
    }

    void folderInfoCountsAndListsTheVisibleEntries()
    {
        FolderInfo info;
        info.setMaximumEntries(3);
        info.setSource(QUrl::fromLocalFile(fixture("folder")));

        QCOMPARE(info.fileCount(), 7);
        QCOMPARE(info.folderCount(), 2);
        QCOMPARE(info.entries().size(), 3);
        const QVariantMap first = info.entries().constFirst().toMap();
        QCOMPARE(first["name"_L1].toString(), u"Documents"_s);
        QVERIFY(!first["iconName"_L1].toString().isEmpty());

        QTRY_VERIFY(!info.isCounting());
        QCOMPARE(info.totalSize(), u"1.1 KiB"_s);
    }

    void folderInfoHandlesAnEmptyFolder()
    {
        QTemporaryDir dir;
        FolderInfo info;
        info.setSource(QUrl::fromLocalFile(dir.path()));
        QCOMPARE(info.fileCount(), 0);
        QCOMPARE(info.folderCount(), 0);
        QVERIFY(info.entries().isEmpty());
        QTRY_VERIFY(!info.isCounting());
        QCOMPARE(info.totalSize(), u"0 B"_s);
    }

    void folderInfoCanBeDestroyedWhileCounting()
    {
        for (int i = 0; i < 20; ++i) {
            FolderInfo info;
            info.setSource(QUrl::fromLocalFile(u"/usr"_s));
            info.setSource(QUrl::fromLocalFile(fixture("folder")));
        }
    }

    void mediaSettingsAreRemembered()
    {
        {
            MediaSettings settings;
            settings.setVideoMuted(false);
            settings.setAudioMuted(true);
        }
        MediaSettings settings;
        QVERIFY(!settings.isVideoMuted());
        QVERIFY(settings.isAudioMuted());
        settings.setVideoMuted(true);
        settings.setAudioMuted(false);
        QVERIFY(settings.isVideoMuted());
        QVERIFY(!settings.isAudioMuted());
        QVERIFY(!settings.autoplay());
    }

    void imageInfoHasNoSizeForOtherFiles()
    {
        ImageInfo info;
        info.setSource(QUrl::fromLocalFile(fixture("text-plain.txt")));
        QVERIFY(!info.size().isValid());
    }
};

QTEST_MAIN(RenderersTest)

#include "tst_renderers.moc"
