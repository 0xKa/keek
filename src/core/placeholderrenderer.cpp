#include "renderer.h"

using namespace Qt::StringLiterals;

Renderer placeholderRenderer()
{
    return {
        u"placeholder"_s,
        u"PlaceholderView"_s,
        [](const QMimeType &) {
            return true;
        },
    };
}
