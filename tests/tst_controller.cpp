#include "controller.h"
#include "fixtures.h"
#include "popupwindow.h"

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
        controller.show({file});
        QQuickWindow *window = controller.window()->quickWindow();
        window->requestActivate();
        return QTest::qWaitForWindowActive(window);
    }

private Q_SLOTS:
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
