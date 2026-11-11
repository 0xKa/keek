#include "renderer.h"

using namespace Qt::StringLiterals;

Renderer textRenderer()
{
    return {
        u"text"_s,
        u"TextView"_s,
        [](const QMimeType &type) {
            return type.inherits(u"text/plain"_s);
        },
    };
}
