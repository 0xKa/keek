#pragma once

#include "rendererregistry.h"

#include <QJsonObject>
#include <QObject>
#include <QStringList>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

#include <memory>

class PopupWindow;

class Controller : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Controller is created by the daemon")
    Q_PROPERTY(QString currentFile READ currentFile NOTIFY currentFileChanged FINAL)
    Q_PROPERTY(QString currentFileName READ currentFileName NOTIFY currentFileChanged FINAL)
    Q_PROPERTY(QUrl currentFileUrl READ currentFileUrl NOTIFY currentFileChanged FINAL)
    Q_PROPERTY(QString fileDescription READ fileDescription NOTIFY currentFileChanged FINAL)
    Q_PROPERTY(QString rendererComponent READ rendererComponent NOTIFY currentFileChanged FINAL)
    Q_PROPERTY(bool showing READ isShowing NOTIFY showingChanged FINAL)

public:
    explicit Controller(QObject *parent = nullptr);
    ~Controller() override;

    bool isReady() const;
    PopupWindow *window() const;

    QString currentFile() const;
    QString currentFileName() const;
    QUrl currentFileUrl() const;
    QString fileDescription() const;
    QString rendererComponent() const;
    bool isShowing() const;

    void show(const QStringList &paths, const QString &activationToken = {});
    void toggle(const QStringList &paths, const QString &activationToken = {});
    Q_INVOKABLE void hide();
    bool screenshot(const QString &file, QString *error = nullptr);
    QJsonObject state() const;

Q_SIGNALS:
    void currentFileChanged();
    void showingChanged();

private:
    QString status() const;

    RendererRegistry m_registry;
    const Renderer *m_renderer;
    QMimeType m_mimeType;
    QStringList m_files;
    QString m_fileError;
    QString m_lastError;
    bool m_hadActivationToken = false;
    std::unique_ptr<PopupWindow> m_window;
};
