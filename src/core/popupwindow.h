#pragma once

#include <QImage>
#include <QObject>
#include <QSize>

#include <memory>

class QQuickView;
class QQuickWindow;

class PopupWindow : public QObject
{
    Q_OBJECT

public:
    explicit PopupWindow(QObject *controller, QObject *parent = nullptr);
    ~PopupWindow() override;

    bool isReady() const;
    QQuickWindow *quickWindow() const;

    void show(const QString &activationToken);
    void hide();

    bool isVisible() const;
    bool isActive() const;
    QSize size() const;
    QImage grab();

private:
    std::unique_ptr<QQuickView> m_view;
};
