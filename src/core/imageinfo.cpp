#include "imageinfo.h"

#include <QImageReader>

ImageInfo::ImageInfo(QObject *parent)
    : QObject(parent)
{
}

QUrl ImageInfo::source() const
{
    return m_source;
}

void ImageInfo::setSource(const QUrl &source)
{
    if (m_source == source) {
        return;
    }
    m_source = source;
    m_size = QSize();
    m_scalable = false;

    if (!m_source.isEmpty()) {
        QImageReader reader(m_source.toLocalFile());
        reader.setAutoTransform(true);
        m_size = reader.size();
        if (m_size.isValid() && reader.transformation().testFlag(QImageIOHandler::TransformationRotate90)) {
            m_size.transpose();
        }
        const QByteArray format = reader.format();
        m_scalable = format == "svg" || format == "svgz";
    }
    Q_EMIT sourceChanged();
}

QSize ImageInfo::size() const
{
    return m_size;
}

bool ImageInfo::isScalable() const
{
    return m_scalable;
}
