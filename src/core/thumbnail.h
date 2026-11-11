#pragma once

#include <QImage>
#include <QPointer>
#include <QQuickPaintedItem>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

namespace KIO
{
class PreviewJob;
}

class Thumbnail : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(int maximumSize READ maximumSize WRITE setMaximumSize NOTIFY maximumSizeChanged FINAL)
    Q_PROPERTY(bool finished READ isFinished NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool available READ isAvailable NOTIFY stateChanged FINAL)

public:
    explicit Thumbnail(QQuickItem *parent = nullptr);
    ~Thumbnail() override;

    QUrl source() const;
    void setSource(const QUrl &source);
    int maximumSize() const;
    void setMaximumSize(int size);

    bool isFinished() const;
    bool isAvailable() const;

    void paint(QPainter *painter) override;

Q_SIGNALS:
    void sourceChanged();
    void maximumSizeChanged();
    void stateChanged();

protected:
    void componentComplete() override;

private:
    void start();
    void stop();
    void finish(const QImage &image);

    QUrl m_source;
    int m_maximumSize = 256;
    QImage m_image;
    bool m_finished = false;
    QPointer<KIO::PreviewJob> m_job;
};
