#include "thumbnail.h"

#include <KConfigGroup>
#include <KFileItem>
#include <KIO/PreviewJob>
#include <KSharedConfig>

#include <QGuiApplication>
#include <QPainter>
#include <QPixmap>

using namespace Qt::StringLiterals;

namespace
{
QStringList enabledPlugins()
{
    const KConfigGroup group = KSharedConfig::openConfig(u"dolphinrc"_s)->group(u"PreviewSettings"_s);
    return group.readEntry("Plugins", KIO::PreviewJob::defaultPlugins());
}
}

Thumbnail::Thumbnail(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
}

Thumbnail::~Thumbnail()
{
    stop();
}

QUrl Thumbnail::source() const
{
    return m_source;
}

void Thumbnail::setSource(const QUrl &source)
{
    if (m_source == source) {
        return;
    }
    m_source = source;
    Q_EMIT sourceChanged();
    if (isComponentComplete()) {
        start();
    }
}

int Thumbnail::maximumSize() const
{
    return m_maximumSize;
}

void Thumbnail::setMaximumSize(int size)
{
    if (m_maximumSize == size) {
        return;
    }
    m_maximumSize = size;
    Q_EMIT maximumSizeChanged();
    if (isComponentComplete()) {
        start();
    }
}

bool Thumbnail::isFinished() const
{
    return m_finished;
}

bool Thumbnail::isAvailable() const
{
    return !m_image.isNull();
}

void Thumbnail::paint(QPainter *painter)
{
    if (m_image.isNull()) {
        return;
    }
    const QSizeF size = m_image.deviceIndependentSize().scaled(boundingRect().size(), Qt::KeepAspectRatio);
    const QRectF target((width() - size.width()) / 2, (height() - size.height()) / 2, size.width(), size.height());
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawImage(target, m_image);
}

void Thumbnail::componentComplete()
{
    QQuickPaintedItem::componentComplete();
    start();
}

void Thumbnail::start()
{
    stop();
    m_image = QImage();
    m_finished = false;
    setImplicitSize(0, 0);
    Q_EMIT stateChanged();
    update();

    if (!m_source.isLocalFile()) {
        finish({});
        return;
    }

    const qreal ratio = qApp->devicePixelRatio();
    const QStringList plugins = enabledPlugins();
    m_job = new KIO::PreviewJob(KFileItemList{KFileItem(m_source)}, QSize(m_maximumSize, m_maximumSize), &plugins);
    m_job->setDevicePixelRatio(ratio);
    connect(m_job, &KIO::PreviewJob::gotPreview, this, [this](const KFileItem &, const QPixmap &preview) {
        finish(preview.toImage());
    });
    connect(m_job, &KIO::PreviewJob::failed, this, [this](const KFileItem &) {
        finish({});
    });
    connect(m_job, &KJob::result, this, [this] {
        if (!m_finished) {
            finish({});
        }
    });
}

void Thumbnail::stop()
{
    if (m_job) {
        m_job->disconnect(this);
        m_job->kill();
    }
}

void Thumbnail::finish(const QImage &image)
{
    m_image = image;
    m_finished = true;
    const QSizeF size = image.deviceIndependentSize();
    setImplicitSize(size.width(), size.height());
    Q_EMIT stateChanged();
    update();
}
