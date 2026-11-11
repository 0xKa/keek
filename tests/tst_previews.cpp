#include "controller.h"
#include "fixtures.h"
#include "imagecompare.h"
#include "mediasettings.h"
#include "popupwindow.h"
#include "thumbnail.h"

#include <KIO/PreviewJob>

#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;

namespace
{
constexpr double maximumDifferingFraction = 0.001;

QString status(const Controller &controller)
{
    return controller.state()["status"_L1].toString();
}

bool settled(const Controller &controller)
{
    const QString current = status(controller);
    return current == "ready"_L1 || current == "error"_L1;
}
}

class PreviewsTest : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir m_copies;

private Q_SLOTS:
    void initTestCase()
    {
        QVERIFY(m_copies.isValid());
        QVERIFY(QDir(m_copies.path()).mkdir(u"locked"_s));
        QVERIFY(QFile::copy(fixture("text-plain.txt"), m_copies.filePath(u"unreadable.txt"_s)));
        QVERIFY(copyFixtures(m_copies.path()));
        for (const QString &name : {u"unreadable.txt"_s, u"locked"_s}) {
            QVERIFY(setFixtureTime(m_copies.filePath(name)));
            QVERIFY(QFile::setPermissions(m_copies.filePath(name), QFile::Permissions()));
        }
    }

    void cleanupTestCase()
    {
        QFile::setPermissions(m_copies.filePath(u"locked"_s), QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
    }

    void fixtures_data()
    {
        QTest::addColumn<QString>("file");
        QTest::addColumn<QString>("renderer");
        QTest::addColumn<QString>("status");

        const auto row = [](const char *file, const char *renderer, const char *status) {
            QTest::newRow(file) << QString::fromUtf8(file) << QString::fromUtf8(renderer) << QString::fromUtf8(status);
        };
        row("image-landscape.png", "image", "ready");
        row("image-portrait.jpg", "image", "ready");
        row("image-small.png", "image", "ready");
        row("image-wide.webp", "image", "ready");
        row("image-vector.svg", "image", "ready");
        row("image-broken.png", "image", "error");
        row("text-plain.txt", "text", "ready");
        row("text-code.cpp", "text", "ready");
        row("text-script.py", "text", "ready");
        row("text-data.json", "text", "ready");
        row("text-notes.md", "text", "ready");
        row("text-settings.ini", "text", "ready");
        row("text-long.txt", "text", "ready");
        row("pdf-document.pdf", "pdf", "ready");
        row("pdf-broken.pdf", "pdf", "error");
        row("pdf-password.pdf", "pdf", "error");
        row("video-clip.webm", "video", "ready");
        row("video-broken.mp4", "fallback", "ready");
        row("audio-tone.flac", "audio", "ready");
        row("binary.bin", "fallback", "ready");
        row("empty", "fallback", "ready");
        row("folder", "folder", "ready");
        row("missing.png", "fallback", "error");
        row("unreadable.txt", "fallback", "error");
        row("locked", "fallback", "error");
    }

    void fixtures()
    {
        QFETCH(QString, file);
        QFETCH(QString, renderer);
        QFETCH(QString, status);

        if ((file == "unreadable.txt"_L1 || file == "locked"_L1) && QFileInfo(m_copies.filePath(file)).isReadable()) {
            QSKIP("Permissions are not enforced for this user");
        }

        Controller controller;
        QVERIFY(controller.isReady());
        controller.show({m_copies.filePath(file)});
        QTRY_VERIFY(settled(controller));

        const QJsonObject state = controller.state();
        QCOMPARE(state["renderer"_L1].toString(), renderer);
        QCOMPARE(state["status"_L1].toString(), status);
        QCOMPARE(state["lastError"_L1].isNull(), status == "ready"_L1);

        const QImage actual = stableGrab(controller.window()->quickWindow());
        const QString referencePath = QStringLiteral(KEEK_REFERENCE_DIR "/") + file + ".png"_L1;

        if (qEnvironmentVariableIsSet("KEEK_UPDATE_REFERENCES")) {
            QVERIFY(QDir().mkpath(QStringLiteral(KEEK_REFERENCE_DIR)));
            QVERIFY(actual.save(referencePath));
            return;
        }

        const QImage expected(referencePath);
        QVERIFY2(!expected.isNull(), qPrintable(u"No reference image at %1"_s.arg(referencePath)));

        const double difference = differingFraction(actual, expected);
        if (difference > maximumDifferingFraction) {
            QVERIFY(QDir().mkpath(QStringLiteral(KEEK_TEST_OUTPUT_DIR)));
            const QString actualPath = QStringLiteral(KEEK_TEST_OUTPUT_DIR "/") + file + ".actual.png"_L1;
            actual.save(actualPath);
            QFAIL(qPrintable(u"%1% of the pixels differ from the reference; the popup as drawn is in %2"_s.arg(difference * 100, 0, 'f', 2).arg(actualPath)));
        }
    }

    void thePopupFitsTheFile_data()
    {
        QTest::addColumn<QString>("file");
        QTest::addColumn<QSize>("size");
        QTest::newRow("wide image at its own size") << fixture(u"image-wide.webp"_s) << QSize(1600, 340);
        QTest::newRow("small image at the minimum") << fixture(u"image-small.png"_s) << QSize(480, 320);
        QTest::newRow("portrait image") << fixture(u"image-portrait.jpg"_s) << QSize(480, 640);
        QTest::newRow("first PDF page whole") << fixture(u"pdf-document.pdf"_s) << QSize(608, 865);
        QTest::newRow("large image shrunk to the screen") << m_copies.filePath(u"large.png"_s) << QSize(1171, 918);
        QTest::newRow("error with its message") << fixture(u"image-broken.png"_s) << QSize(496, 356);
    }

    void thePopupFitsTheFile()
    {
        QFETCH(QString, file);
        QFETCH(QSize, size);

        QImage large(4000, 3000, QImage::Format_RGB32);
        large.fill(Qt::darkCyan);
        QVERIFY(QFileInfo::exists(m_copies.filePath(u"large.png"_s)) || large.save(m_copies.filePath(u"large.png"_s)));

        Controller controller;
        controller.show({file});
        QTRY_VERIFY(settled(controller));
        QTRY_COMPARE(controller.window()->size(), size);
    }

    void movingResizesThePopup()
    {
        Controller controller;
        controller.show({fixture(u"image-small.png"_s), fixture(u"image-wide.webp"_s)});
        QTRY_COMPARE(controller.window()->size(), QSize(480, 320));

        QVERIFY(controller.next());
        QTRY_COMPARE(controller.window()->size(), QSize(1600, 340));

        QVERIFY(controller.previous());
        QTRY_COMPARE(controller.window()->size(), QSize(480, 320));
    }

    void switchingFilesSwitchesTheRenderer()
    {
        Controller controller;

        controller.show({fixture(u"image-landscape.png"_s)});
        QTRY_COMPARE(status(controller), u"ready"_s);
        QCOMPARE(controller.state()["renderer"_L1].toString(), u"image"_s);

        controller.show({fixture(u"text-code.cpp"_s)});
        QTRY_COMPARE(status(controller), u"ready"_s);
        QCOMPARE(controller.state()["renderer"_L1].toString(), u"text"_s);

        controller.show({fixture(u"image-broken.png"_s)});
        QTRY_COMPARE(status(controller), u"error"_s);

        controller.show({fixture(u"binary.bin"_s)});
        QTRY_COMPARE(status(controller), u"ready"_s);
        QCOMPARE(controller.state()["renderer"_L1].toString(), u"fallback"_s);
        QVERIFY(controller.state()["lastError"_L1].isNull());
    }

    void hidingAndShowingAgainReloads()
    {
        Controller controller;
        controller.show({fixture(u"image-landscape.png"_s)});
        QTRY_COMPARE(status(controller), u"ready"_s);

        controller.hide();
        QCOMPARE(status(controller), u"hidden"_s);

        controller.show({fixture(u"image-landscape.png"_s)});
        QTRY_COMPARE(status(controller), u"ready"_s);
    }

    void aMissingFileIsAnError()
    {
        Controller controller;
        controller.show({u"/nonexistent/picture.png"_s});

        const QJsonObject state = controller.state();
        QCOMPARE(state["status"_L1].toString(), u"error"_s);
        QCOMPARE(state["renderer"_L1].toString(), u"fallback"_s);
        QVERIFY(!state["lastError"_L1].isNull());
    }

    void aVideoThatCannotBePlayedSaysWhy()
    {
        Controller controller;
        controller.show({fixture(u"video-broken.mp4"_s)});
        QTRY_COMPARE(controller.state()["renderer"_L1].toString(), u"fallback"_s);
        QVERIFY(!controller.state()["notice"_L1].isNull());

        controller.show({fixture(u"video-clip.webm"_s)});
        QVERIFY(controller.state()["notice"_L1].isNull());
    }

    void pageDownScrollsLongText()
    {
        Controller controller;
        controller.show({fixture(u"text-long.txt"_s)});
        QTRY_COMPARE(status(controller), u"ready"_s);

        QQuickWindow *window = controller.window()->quickWindow();
        window->requestActivate();
        QVERIFY(QTest::qWaitForWindowActive(window));

        const QImage before = stableGrab(window);
        QTest::keyClick(window, Qt::Key_PageDown);
        const QImage after = stableGrab(window);
        QVERIFY(differingFraction(before, after, 0) > 0.001);

        QTest::keyClick(window, Qt::Key_Home);
        QCOMPARE(stableGrab(window), before);
        QCOMPARE(controller.state()["visible"_L1].toBool(), true);
    }

    void pageDownScrollsAPdf()
    {
        Controller controller;
        controller.show({fixture(u"pdf-document.pdf"_s)});
        QTRY_COMPARE(status(controller), u"ready"_s);

        QQuickWindow *window = controller.window()->quickWindow();
        window->requestActivate();
        QVERIFY(QTest::qWaitForWindowActive(window));

        const QImage before = stableGrab(window);
        QTest::keyClick(window, Qt::Key_PageDown);
        QVERIFY(differingFraction(before, stableGrab(window), 0) > 0.001);
        QTest::keyClick(window, Qt::Key_Down, Qt::ControlModifier);
        QTest::keyClick(window, Qt::Key_Home);
        QCOMPARE(stableGrab(window), before);
        QCOMPARE(controller.currentFile(), fixture(u"pdf-document.pdf"_s));
    }

    void mediaKeys_data()
    {
        QTest::addColumn<QString>("file");
        QTest::addColumn<bool>("startsMuted");
        QTest::newRow("video") << u"video-clip.webm"_s << true;
        QTest::newRow("audio") << u"audio-tone.flac"_s << false;
    }

    void mediaKeys()
    {
        QFETCH(QString, file);
        QFETCH(bool, startsMuted);

        MediaSettings settings;
        settings.setVideoMuted(true);
        settings.setAudioMuted(false);

        Controller controller;
        controller.show({fixture(file)});
        QTRY_COMPARE(status(controller), u"ready"_s);

        QQuickWindow *window = controller.window()->quickWindow();
        window->requestActivate();
        QVERIFY(QTest::qWaitForWindowActive(window));

        QObject *player = window->contentItem()->findChild<QObject *>(u"player"_s);
        QVERIFY(player);
        QObject *output = player->property("audioOutput").value<QObject *>();
        QVERIFY(output);
        QCOMPARE(output->property("muted").toBool(), startsMuted);
        QCOMPARE(player->property("playbackState").toInt(), 2);

        QTest::keyClick(window, Qt::Key_K);
        QTRY_COMPARE(player->property("playbackState").toInt(), 1);
        QTest::keyClick(window, Qt::Key_K);
        QTRY_COMPARE(player->property("playbackState").toInt(), 2);

        QTest::keyClick(window, Qt::Key_Right, Qt::ShiftModifier);
        QTRY_VERIFY(player->property("position").toLongLong() > 0);
        QTest::keyClick(window, Qt::Key_Left, Qt::ShiftModifier);
        QTRY_COMPARE(player->property("position").toLongLong(), 0);

        QTest::keyClick(window, Qt::Key_M);
        QCOMPARE(output->property("muted").toBool(), !startsMuted);
        QCOMPARE(file.startsWith("video"_L1) ? settings.isVideoMuted() : settings.isAudioMuted(), !startsMuted);

        QCOMPARE(controller.currentFile(), fixture(file));
        QCOMPARE(controller.state()["visible"_L1].toBool(), true);
        settings.setVideoMuted(true);
        settings.setAudioMuted(false);
    }

    void aThumbnailReplacesTheIcon()
    {
        if (!KIO::PreviewJob::defaultPlugins().contains(u"comicbookthumbnail"_s)) {
            QSKIP("The comic book thumbnailer is not installed");
        }

        Controller controller;
        controller.show({fixture(u"comic.cbz"_s)});
        QTRY_COMPARE_WITH_TIMEOUT(status(controller), u"ready"_s, 10000);
        QCOMPARE(controller.state()["renderer"_L1].toString(), u"fallback"_s);

        const auto *thumbnail = controller.window()->quickWindow()->contentItem()->findChild<Thumbnail *>();
        QVERIFY(thumbnail);
        QVERIFY(thumbnail->isAvailable());
        QVERIFY(thumbnail->implicitWidth() > 0);
    }

    void controlDownScrollsTextByALine()
    {
        Controller controller;
        controller.show({fixture(u"text-long.txt"_s)});
        QTRY_COMPARE(status(controller), u"ready"_s);

        QQuickWindow *window = controller.window()->quickWindow();
        window->requestActivate();
        QVERIFY(QTest::qWaitForWindowActive(window));

        const QImage before = stableGrab(window);
        QTest::keyClick(window, Qt::Key_Down, Qt::ControlModifier);
        QVERIFY(differingFraction(before, stableGrab(window), 0) > 0.001);
        QCOMPARE(controller.currentFile(), fixture(u"text-long.txt"_s));

        QTest::keyClick(window, Qt::Key_Up, Qt::ControlModifier);
        QCOMPARE(stableGrab(window), before);
    }
};

QTEST_MAIN(PreviewsTest)

#include "tst_previews.moc"
