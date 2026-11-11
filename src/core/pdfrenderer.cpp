#include "renderer.h"

using namespace Qt::StringLiterals;

Renderer pdfRenderer()
{
    return {
        u"pdf"_s,
        u"PdfView"_s,
        [](const QMimeType &type) {
            return type.inherits(u"application/pdf"_s);
        },
    };
}
