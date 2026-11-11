#include "renderer.h"

#include <QImageReader>
#include <QSet>

using namespace Qt::StringLiterals;

namespace
{
QSet<QString> readableImageTypes()
{
    QSet<QString> types;
    const QList<QByteArray> supported = QImageReader::supportedMimeTypes();
    for (const QByteArray &type : supported) {
        types.insert(QString::fromLatin1(type));
    }
    types.remove(u"application/pdf"_s);
    return types;
}
}

Renderer imageRenderer()
{
    return {
        u"image"_s,
        u"ImageView"_s,
        [types = readableImageTypes()](const QMimeType &type) {
            if (types.contains(type.name())) {
                return true;
            }
            const QStringList ancestors = type.allAncestors();
            for (const QString &ancestor : ancestors) {
                if (ancestor.startsWith("image/"_L1) && types.contains(ancestor)) {
                    return true;
                }
            }
            return false;
        },
    };
}
