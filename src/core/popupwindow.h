#pragma once

#include <QImage>
#include <QObject>
#include <QSize>

#include <functional>
#include <memory>

class QQuickView;
class QQuickWindow;

class PopupWindow : public QObject
{
    Q_OBJECT

public:
    explicit PopupWindow(QObject *controller, QObject *parent = nullptr);
    ~PopupWindow() override;

    void load();
    bool isReady() const;
    QQuickWindow *quickWindow() const;

    void show(const QString &activationToken);
    void hide();
    void requestActivationToken(std::function<void(const QString &)> callback);

    bool isVisible() const;
    bool isActive() const;
    QSize size() const;
    QImage grab();

    bool isContentReady() const;
    QString contentError() const;

Q_SIGNALS:
    void visibleChanged();

private:
    std::unique_ptr<QQuickView> m_view;
};
