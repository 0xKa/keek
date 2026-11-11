#include "renderer.h"

using namespace Qt::StringLiterals;

Renderer fallbackRenderer()
{
    return {
        u"fallback"_s,
        u"FallbackView"_s,
        [](const QMimeType &) {
            return true;
        },
    };
}
