#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;

class CliTest : public QObject
{
    Q_OBJECT

private:
    struct Result {
        int exitCode = -1;
        QString out;
        QString err;
    };

    Result keek(const QStringList &arguments, const QString &activationToken = {}, const QString &workingDirectory = {})
    {
        QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
        environment.remove(u"XDG_ACTIVATION_TOKEN"_s);
        if (!activationToken.isEmpty()) {
            environment.insert(u"XDG_ACTIVATION_TOKEN"_s, activationToken);
        }

        QProcess process;
        process.setProcessEnvironment(environment);
        if (!workingDirectory.isEmpty()) {
            process.setWorkingDirectory(workingDirectory);
        }
        process.start(m_keek, arguments);

        Result result;
        if (!process.waitForFinished(10000)) {
            process.kill();
            process.waitForFinished();
            return result;
        }
        result.exitCode = process.exitStatus() == QProcess::NormalExit ? process.exitCode() : -1;
        result.out = QString::fromUtf8(process.readAllStandardOutput());
        result.err = QString::fromUtf8(process.readAllStandardError());
        return result;
    }

    QJsonObject state()
    {
        const Result result = keek({u"state"_s});
        if (result.exitCode != 0) {
            return {};
        }
        return QJsonDocument::fromJson(result.out.toUtf8()).object();
    }

    QString m_keek;
    QString m_keekd;
    QTemporaryDir m_dir;
    QString m_fileA;
    QString m_fileB;
    QProcess m_daemon;

private Q_SLOTS:
    void initTestCase()
    {
        if (qEnvironmentVariable("KEEK_TEST_PRIVATE_BUS") != "1"_L1) {
            QSKIP("Run through ctest, which gives this test a private session bus");
        }
        m_keek = qEnvironmentVariable("KEEK_BIN");
        m_keekd = qEnvironmentVariable("KEEKD_BIN");
        QVERIFY(QFile::exists(m_keek));
        QVERIFY(QFile::exists(m_keekd));

        QVERIFY(m_dir.isValid());
        m_fileA = m_dir.filePath(u"a.txt"_s);
        m_fileB = m_dir.filePath(u"b.txt"_s);
        for (const QString &path : {m_fileA, m_fileB}) {
            QFile file(path);
            QVERIFY(file.open(QIODevice::WriteOnly));
            file.write("keek\n");
        }
    }

    void usageErrors_data()
    {
        QTest::addColumn<QStringList>("arguments");
        QTest::newRow("no command") << QStringList{};
        QTest::newRow("unknown command") << QStringList{u"bogus"_s};
        QTest::newRow("unknown command with argument") << QStringList{u"bogus"_s, u"x"_s};
        QTest::newRow("show without paths") << QStringList{u"show"_s};
        QTest::newRow("toggle without paths") << QStringList{u"toggle"_s};
        QTest::newRow("screenshot without file") << QStringList{u"screenshot"_s};
        QTest::newRow("hide with argument") << QStringList{u"hide"_s, u"x"_s};
    }

    void usageErrors()
    {
        QFETCH(QStringList, arguments);
        const Result result = keek(arguments);
        QCOMPARE(result.exitCode, 1);
        QVERIFY(result.err.contains("Usage:"_L1));
    }

    void helpAndVersion()
    {
        const Result help = keek({u"--help"_s});
        QCOMPARE(help.exitCode, 0);
        QVERIFY(help.out.contains("Usage:"_L1));

        const Result version = keek({u"--version"_s});
        QCOMPARE(version.exitCode, 0);
        QVERIFY(version.out.startsWith("keek "_L1));
    }

    void commandsFailWithoutTheDaemon_data()
    {
        QTest::addColumn<QStringList>("arguments");
        QTest::newRow("state") << QStringList{u"state"_s};
        QTest::newRow("show") << QStringList{u"show"_s, u"x"_s};
        QTest::newRow("hide") << QStringList{u"hide"_s};
        QTest::newRow("quit") << QStringList{u"quit"_s};
    }

    void commandsFailWithoutTheDaemon()
    {
        QFETCH(QStringList, arguments);
        const Result result = keek(arguments);
        QCOMPARE(result.exitCode, 2);
        QVERIFY(result.err.contains("not running"_L1));
    }

    void daemonStarts()
    {
        m_daemon.setProcessChannelMode(QProcess::ForwardedChannels);
        m_daemon.start(m_keekd, QStringList{});
        QVERIFY(m_daemon.waitForStarted());
        QTRY_COMPARE_WITH_TIMEOUT(keek({u"state"_s}).exitCode, 0, 10000);

        const QJsonObject initial = state();
        QCOMPARE(initial["visible"_L1].toBool(), false);
        QVERIFY(initial["file"_L1].isNull());
    }

    void showReportsTheFile()
    {
        QCOMPARE(keek({u"show"_s, m_fileA, m_fileB}).exitCode, 0);

        const QJsonObject current = state();
        QCOMPARE(current["visible"_L1].toBool(), true);
        QCOMPARE(current["file"_L1].toString(), m_fileA);
        QCOMPARE(current["files"_L1].toArray().size(), 2);
        QCOMPARE(current["activationToken"_L1].toBool(), false);
    }

    void relativePathsBecomeAbsolute()
    {
        QCOMPARE(keek({u"show"_s, u"b.txt"_s}, {}, m_dir.path()).exitCode, 0);
        QCOMPARE(state()["file"_L1].toString(), m_fileB);
    }

    void activationTokenIsForwarded()
    {
        QCOMPARE(keek({u"show"_s, m_fileA}, u"test-token"_s).exitCode, 0);
        QCOMPARE(state()["activationToken"_L1].toBool(), true);
    }

    void toggleHidesAndShows()
    {
        QCOMPARE(keek({u"toggle"_s, m_fileA}).exitCode, 0);
        QCOMPARE(state()["visible"_L1].toBool(), false);

        QCOMPARE(keek({u"toggle"_s, m_fileA}).exitCode, 0);
        QCOMPARE(state()["visible"_L1].toBool(), true);
    }

    void screenshotWritesAnImage()
    {
        const QString file = m_dir.filePath(u"popup.png"_s);
        QCOMPARE(keek({u"screenshot"_s, file}).exitCode, 0);

        const QImage image(file);
        QVERIFY(!image.isNull());
        const QJsonObject current = state();
        QCOMPARE(image.width(), current["width"_L1].toInt());
        QCOMPARE(image.height(), current["height"_L1].toInt());
    }

    void hideThenScreenshotFails()
    {
        QCOMPARE(keek({u"hide"_s}).exitCode, 0);
        QCOMPARE(state()["visible"_L1].toBool(), false);

        const Result result = keek({u"screenshot"_s, m_dir.filePath(u"hidden.png"_s)});
        QCOMPARE(result.exitCode, 3);
        QVERIFY(result.err.contains("not visible"_L1));
        QVERIFY(!state()["lastError"_L1].isNull());
    }

    void secondDaemonRefusesToStart()
    {
        QProcess second;
        second.start(m_keekd, QStringList{});
        QVERIFY(second.waitForFinished(10000));
        QCOMPARE(second.exitCode(), 1);
        QVERIFY(second.readAllStandardError().contains("already running"));
        QCOMPARE(keek({u"state"_s}).exitCode, 0);
    }

    void quitStopsTheDaemon()
    {
        QCOMPARE(keek({u"quit"_s}).exitCode, 0);
        QVERIFY(m_daemon.waitForFinished(5000));
        QCOMPARE(m_daemon.exitStatus(), QProcess::NormalExit);
        QCOMPARE(m_daemon.exitCode(), 0);
        QCOMPARE(keek({u"state"_s}).exitCode, 2);
    }

    void cleanupTestCase()
    {
        if (m_daemon.state() != QProcess::NotRunning) {
            m_daemon.kill();
            m_daemon.waitForFinished();
        }
    }
};

QTEST_GUILESS_MAIN(CliTest)

#include "tst_cli.moc"
