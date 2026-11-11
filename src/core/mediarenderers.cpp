#include "renderer.h"

using namespace Qt::StringLiterals;

namespace
{
bool hasTopLevelType(const QMimeType &type, QLatin1StringView prefix)
{
    if (type.name().startsWith(prefix)) {
        return true;
    }
    const QStringList ancestors = type.allAncestors();
    for (const QString &ancestor : ancestors) {
        if (ancestor.startsWith(prefix)) {
            return true;
        }
    }
    return false;
}
}

Renderer audioRenderer()
{
    return {
        u"audio"_s,
        u"AudioView"_s,
        [](const QMimeType &type) {
            return hasTopLevelType(type, "audio/"_L1);
        },
    };
}

Renderer videoRenderer()
{
    return {
        u"video"_s,
        u"VideoView"_s,
        [](const QMimeType &type) {
            return hasTopLevelType(type, "video/"_L1);
        },
    };
}
