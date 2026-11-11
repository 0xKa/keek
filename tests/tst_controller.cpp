#include "controller.h"
#include "fixtures.h"
#include "popupwindow.h"

#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QKeyEvent>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;

class ControllerTest : public QObject
{
    Q_OBJECT

private:
    static QString fileA()
    {
        return fixture("text-plain.txt");
    }

    static QString fileB()
    {
        return fixture("text-code.cpp");
    }

    static bool showAndActivate(Controller &controller, const QString &file)
    {
        return showAndActivate(controller, QStringList{file});
    }

    static bool showAndActivate(Controller &controller, const QStringList &files)
    {
        controller.show(files);
        QQuickWindow *window = controller.window()->quickWindow();
        window->requestActivate();
        return QTest::qWaitForWindowActive(window);
    }

    static QString current(const Controller &controller)
    {
        return QFileInfo(controller.currentFile()).fileName();
    }

    QTemporaryDir m_folder;

    QString inFolder(const QString &name) const
    {
        return m_folder.filePath(name);
    }

private Q_SLOTS:
    void initTestCase()
    {
        QVERIFY(m_folder.isValid());
        for (const QString &name : {u"a.txt"_s, u"b.txt"_s, u"c.txt"_s}) {
            QFile file(inFolder(name));
            QVERIFY(file.open(QIODevice::WriteOnly));
            file.write("keek\n");
        }
    }

    void startsHiddenWithNoFile()
    {
        Controller controller;
        QVERIFY(controller.isReady());

        const QJsonObject state = controller.state();
        QCOMPARE(state["visible"_L1].toBool(), false);
        QCOMPARE(state["status"_L1].toString(), u"hidden"_s);
        QVERIFY(state["file"_L1].isNull());
        QVERIFY(state["renderer"_L1].isNull());
        QVERIFY(state["lastError"_L1].isNull());
    }

    void showMakesThePopupVisible()
    {
        Controller controller;
        controller.show({fileA(), fileB()});

        const QJsonObject state = controller.state();
        QCOMPARE(state["visible"_L1].toBool(), true);
        QCOMPARE(state["file"_L1].toString(), fileA());
        QCOMPARE(state["files"_L1].toArray().size(), 2);
        QCOMPARE(state["activationToken"_L1].toBool(), false);
        QCOMPARE(state["mimeType"_L1].toString(), u"text/plain"_s);
        QCOMPARE(controller.currentFileName(), u"text-plain.txt"_s);
        QVERIFY(state["width"_L1].toInt() > 0);
        QVERIFY(state["height"_L1].toInt() > 0);
    }

    void showWithNoPathsDoesNothing()
    {
        Controller controller;
        controller.show({});
        QCOMPARE(controller.state()["visible"_L1].toBool(), false);
    }

    void showRecordsTheActivationToken()
    {
        Controller controller;
        controller.show({fileA()}, u"token"_s);
        QCOMPARE(controller.state()["activationToken"_L1].toBool(), true);

        controller.show({fileA()});
        QCOMPARE(controller.state()["activationToken"_L1].toBool(), false);
    }

    void toggleHidesTheSameFile()
    {
        Controller controller;
        controller.toggle({fileA()});
        QCOMPARE(controller.state()["visible"_L1].toBool(), true);

        controller.toggle({fileA()});
        QCOMPARE(controller.state()["visible"_L1].toBool(), false);

        controller.toggle({fileA()});
        QCOMPARE(controller.state()["visible"_L1].toBool(), true);
    }

    void toggleSwitchesToAnotherFile()
    {
        Controller controller;
        controller.toggle({fileA()});
        controller.toggle({fileB()});

        const QJsonObject state = controller.state();
        QCOMPARE(state["visible"_L1].toBool(), true);
        QCOMPARE(state["file"_L1].toString(), fileB());
    }

    void closeKeys_data()
    {
        QTest::addColumn<Qt::Key>("key");
        QTest::newRow("space") << Qt::Key_Space;
        QTest::newRow("escape") << Qt::Key_Escape;
    }

    void closeKeys()
    {
        QFETCH(Qt::Key, key);

        Controller controller;
        QVERIFY(showAndActivate(controller, fileA()));

        QTest::keyClick(controller.window()->quickWindow(), key);
        QCOMPARE(controller.state()["visible"_L1].toBool(), false);
    }

    void otherKeysDoNotClose()
    {
        Controller controller;
        QVERIFY(showAndActivate(controller, fileA()));

        QTest::keyClick(controller.window()->quickWindow(), Qt::Key_A);
        QCOMPARE(controller.state()["visible"_L1].toBool(), true);
    }

    void heldSpaceDoesNotClose()
    {
        Controller controller;
        QVERIFY(showAndActivate(controller, fileA()));

        QKeyEvent repeat(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, u" "_s, true);
        QCoreApplication::sendEvent(controller.window()->quickWindow(), &repeat);
        QCOMPARE(controller.state()["visible"_L1].toBool(), true);
    }

    void oneFileBrowsesItsFolder()
    {
        Controller controller;
        controller.show({inFolder(u"b.txt"_s)});
        QCOMPARE(controller.state()["index"_L1].toInt(), 1);
        QCOMPARE(controller.state()["count"_L1].toInt(), 3);
        QCOMPARE(controller.state()["files"_L1].toArray().size(), 1);

        QVERIFY(controller.next());
        QCOMPARE(current(controller), u"c.txt"_s);
        QVERIFY(!controller.next());
        QCOMPARE(current(controller), u"c.txt"_s);

        QVERIFY(controller.previous());
        QVERIFY(controller.previous());
        QCOMPARE(current(controller), u"a.txt"_s);
        QVERIFY(!controller.previous());
        QCOMPARE(controller.state()["index"_L1].toInt(), 0);
    }

    void severalFilesBrowseOnlyThemInTheGivenOrder()
    {
        Controller controller;
        controller.show({inFolder(u"c.txt"_s), inFolder(u"a.txt"_s)});
        QCOMPARE(controller.state()["count"_L1].toInt(), 2);
        QCOMPARE(current(controller), u"c.txt"_s);

        QVERIFY(controller.next());
        QCOMPARE(current(controller), u"a.txt"_s);
        QVERIFY(!controller.next());
    }

    void movingNeedsAVisiblePopup()
    {
        Controller controller;
        QVERIFY(!controller.next());

        controller.show({inFolder(u"a.txt"_s)});
        controller.hide();
        QVERIFY(!controller.next());
        QCOMPARE(current(controller), u"a.txt"_s);
    }

    void hidingBeforeThePopupAppearsStopsThePreview()
    {
        Controller controller;
        controller.show({fixture("video-clip.webm")});
        QVERIFY(controller.isShowing());
        QVERIFY(!controller.window()->quickWindow()->isVisible());

        controller.hide();
        QVERIFY(!controller.isShowing());
        QCOMPARE(controller.state()["visible"_L1].toBool(), false);
        QTest::qWait(300);
        QVERIFY(!controller.window()->quickWindow()->isVisible());
    }

    void movingPicksTheRendererForTheNewFile()
    {
        Controller controller;
        controller.show({fixture("text-plain.txt"), fixture("image-small.png")});
        QCOMPARE(controller.state()["renderer"_L1].toString(), u"text"_s);
        QVERIFY(controller.next());
        QCOMPARE(controller.state()["renderer"_L1].toString(), u"image"_s);
        QCOMPARE(controller.state()["mimeType"_L1].toString(), u"image/png"_s);
    }

    void arrowKeysMove_data()
    {
        QTest::addColumn<Qt::Key>("key");
        QTest::addColumn<Qt::KeyboardModifiers>("modifiers");
        QTest::addColumn<QString>("expected");
        QTest::newRow("down") << Qt::Key_Down << Qt::KeyboardModifiers() << u"c.txt"_s;
        QTest::newRow("right") << Qt::Key_Right << Qt::KeyboardModifiers() << u"c.txt"_s;
        QTest::newRow("up") << Qt::Key_Up << Qt::KeyboardModifiers() << u"a.txt"_s;
        QTest::newRow("left") << Qt::Key_Left << Qt::KeyboardModifiers() << u"a.txt"_s;
        QTest::newRow("ctrl+down") << Qt::Key_Down << Qt::KeyboardModifiers(Qt::ControlModifier) << u"b.txt"_s;
        QTest::newRow("shift+right") << Qt::Key_Right << Qt::KeyboardModifiers(Qt::ShiftModifier) << u"b.txt"_s;
    }

    void arrowKeysMove()
    {
        QFETCH(Qt::Key, key);
        QFETCH(Qt::KeyboardModifiers, modifiers);
        QFETCH(QString, expected);

        Controller controller;
        QVERIFY(showAndActivate(controller, inFolder(u"b.txt"_s)));
        QTest::keyClick(controller.window()->quickWindow(), key, modifiers);
        QCOMPARE(current(controller), expected);
        QCOMPARE(controller.state()["visible"_L1].toBool(), true);
    }

    void heldArrowKeepsMoving()
    {
        Controller controller;
        QVERIFY(showAndActivate(controller, inFolder(u"a.txt"_s)));

        QKeyEvent repeat(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier, QString(), true);
        QCoreApplication::sendEvent(controller.window()->quickWindow(), &repeat);
        QCOMPARE(current(controller), u"b.txt"_s);
    }

    void enterOpensTheCurrentFileAndCloses_data()
    {
        QTest::addColumn<Qt::Key>("key");
        QTest::newRow("return") << Qt::Key_Return;
        QTest::newRow("enter") << Qt::Key_Enter;
    }

    void enterOpensTheCurrentFileAndCloses()
    {
        QFETCH(Qt::Key, key);

        Controller controller;
        QStringList opened;
        controller.setFileOpener([&opened](const QString &file, const QString &) {
            opened.append(file);
        });
        QVERIFY(showAndActivate(controller, inFolder(u"a.txt"_s)));
        QVERIFY(controller.next());

        QTest::keyClick(controller.window()->quickWindow(), key);
        QTRY_COMPARE(opened, QStringList{inFolder(u"b.txt"_s)});
        QCOMPARE(controller.state()["visible"_L1].toBool(), false);
    }

    void openingNeedsAVisiblePopup()
    {
        Controller controller;
        bool opened = false;
        controller.setFileOpener([&opened](const QString &, const QString &) {
            opened = true;
        });
        controller.openCurrentFile();
        QTest::qWait(10);
        QVERIFY(!opened);
    }

    void useFallbackSwitchesTheRenderer()
    {
        Controller controller;
        controller.show({fileA()});
        QCOMPARE(controller.state()["renderer"_L1].toString(), u"text"_s);

        controller.useFallback();
        QCOMPARE(controller.state()["renderer"_L1].toString(), u"fallback"_s);
        QCOMPARE(controller.rendererComponent(), u"FallbackView"_s);

        QVERIFY(controller.next());
        QCOMPARE(controller.state()["renderer"_L1].toString(), u"text"_s);
    }

    void screenshotSavesWhatThePopupDraws()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString file = dir.filePath(u"popup.png"_s);

        Controller controller;
        controller.show({fileA()});
        QVERIFY(controller.screenshot(file));

        const QImage image(file);
        QVERIFY(!image.isNull());
        const QJsonObject state = controller.state();
        QCOMPARE(image.width(), state["width"_L1].toInt());
        QCOMPARE(image.height(), state["height"_L1].toInt());

        bool hasContent = false;
        const QRgb background = image.pixel(0, 0);
        for (int y = 0; y < image.height() && !hasContent; ++y) {
            for (int x = 0; x < image.width(); ++x) {
                if (image.pixel(x, y) != background) {
                    hasContent = true;
                    break;
                }
            }
        }
        QVERIFY2(hasContent, "the screenshot is a single flat colour");
    }

    void screenshotFailsWhenHidden()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString file = dir.filePath(u"popup.png"_s);

        Controller controller;
        QString error;
        QVERIFY(!controller.screenshot(file, &error));
        QVERIFY(!error.isEmpty());
        QVERIFY(!QFile::exists(file));
        QCOMPARE(controller.state()["lastError"_L1].toString(), error);

        controller.show({fileA()});
        QVERIFY(controller.state()["lastError"_L1].isNull());
    }
};

QTEST_MAIN(ControllerTest)

#include "tst_controller.moc"
