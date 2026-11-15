#pragma once

#include <QImage>
#include <QObject>
#include <QSize>

#include <memory>

class QQuickView;
class QQuickWindow;

class MainWindow : public QObject
{
    Q_OBJECT

public:
    explicit MainWindow(QObject *parent = nullptr);
    ~MainWindow() override;

    bool load();
    QQuickWindow *quickWindow() const;

    void show(const QString &activationToken);
    void close();

    bool isVisible() const;
    bool isActive() const;
    QSize size() const;
    QImage grab();

Q_SIGNALS:
    void closed();

private:
    std::unique_ptr<QQuickView> m_view;
};
