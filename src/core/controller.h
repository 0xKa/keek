#pragma once

#include "rendererregistry.h"

#include <QJsonObject>
#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

#include <chrono>
#include <functional>
#include <memory>
#include <optional>

class MainWindow;
class PopupWindow;

class Controller : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Controller is created by the daemon")
    Q_PROPERTY(QString currentFile READ currentFile NOTIFY currentFileChanged FINAL)
    Q_PROPERTY(QString currentFileName READ currentFileName NOTIFY currentFileChanged FINAL)
    Q_PROPERTY(QUrl currentFileUrl READ currentFileUrl NOTIFY currentFileChanged FINAL)
    Q_PROPERTY(QString fileError READ fileError NOTIFY currentFileChanged FINAL)
    Q_PROPERTY(QString notice READ notice NOTIFY currentFileChanged FINAL)
    Q_PROPERTY(QString rendererComponent READ rendererComponent NOTIFY currentFileChanged FINAL)
    Q_PROPERTY(bool showing READ isShowing NOTIFY showingChanged FINAL)

public:
    using FileOpener = std::function<void(const QString &file, const QString &activationToken)>;

    explicit Controller(QObject *parent = nullptr);
    ~Controller() override;

    bool isReady() const;
    PopupWindow *window() const;
    MainWindow *mainWindow() const;

    QString currentFile() const;
    QString currentFileName() const;
    QUrl currentFileUrl() const;
    QString fileError() const;
    QString notice() const;
    QString rendererComponent() const;
    bool isShowing() const;

    void show(const QStringList &paths, const QString &activationToken = {});
    void toggle(const QStringList &paths, const QString &activationToken = {});
    Q_INVOKABLE void hide();
    Q_INVOKABLE bool next();
    Q_INVOKABLE bool previous();
    Q_INVOKABLE void openCurrentFile();
    Q_INVOKABLE void useFallback(const QString &reason = {});
    void setFileOpener(FileOpener opener);
    bool screenshot(const QString &file, QString *error = nullptr);
    bool openWindow(const QString &activationToken = {}, QString *error = nullptr);
    bool screenshotWindow(const QString &file, QString *error = nullptr);
    void setIdleInterval(std::chrono::milliseconds interval);
    QJsonObject state() const;

Q_SIGNALS:
    void currentFileChanged();
    void showingChanged();
    void stopRequested();

private:
    QString status() const;
    bool saveScreenshot(const QImage &image, const QString &what, const QString &file, QString *error);
    bool fail(const QString &problem, QString *error);
    void mainWindowClosed();
    void updateIdleTimer();
    void idleTimeout();
    void setShowing(bool showing);
    bool moveBy(int step);
    void selectCurrent();

    RendererRegistry m_registry;
    const Renderer *m_renderer;
    QMimeType m_mimeType;
    QStringList m_requested;
    QStringList m_files;
    qsizetype m_index = 0;
    FileOpener m_opener;
    QString m_fileError;
    QString m_notice;
    QString m_lastError;
    bool m_hadActivationToken = false;
    bool m_showing = false;
    QTimer m_idleTimer;
    std::optional<std::chrono::milliseconds> m_idleInterval;
    std::unique_ptr<MainWindow> m_mainWindow;
    std::unique_ptr<PopupWindow> m_window;
};
