#include "controller.h"
#include "fixtures.h"
#include "imagecompare.h"
#include "popupwindow.h"

#include <QDir>
#include <QJsonObject>
#include <QQuickWindow>
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

private Q_SLOTS:
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
        row("binary.bin", "placeholder", "ready");
        row("empty", "placeholder", "ready");
    }

    void fixtures()
    {
        QFETCH(QString, file);
        QFETCH(QString, renderer);
        QFETCH(QString, status);

        Controller controller;
        QVERIFY(controller.isReady());
        controller.show({fixture(file)});
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
        QCOMPARE(controller.state()["renderer"_L1].toString(), u"placeholder"_s);
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
        QCOMPARE(state["renderer"_L1].toString(), u"placeholder"_s);
        QVERIFY(!state["lastError"_L1].isNull());
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
};

QTEST_MAIN(PreviewsTest)

#include "tst_previews.moc"
