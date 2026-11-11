#include "fixtures.h"
#include "imageinfo.h"
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
        QTest::newRow("binary") << fixture("binary.bin") << u"placeholder"_s;
        QTest::newRow("empty") << fixture("empty") << u"placeholder"_s;
        QTest::newRow("folder") << QStringLiteral(KEEK_FIXTURES_DIR) << u"placeholder"_s;
        QTest::newRow("pdf, by name") << u"/nonexistent/document.pdf"_s << u"placeholder"_s;
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

    void imageInfoHasNoSizeForOtherFiles()
    {
        ImageInfo info;
        info.setSource(QUrl::fromLocalFile(fixture("text-plain.txt")));
        QVERIFY(!info.size().isValid());
    }
};

QTEST_MAIN(RenderersTest)

#include "tst_renderers.moc"
