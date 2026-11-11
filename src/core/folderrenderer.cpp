#include "renderer.h"

using namespace Qt::StringLiterals;

Renderer folderRenderer()
{
    return {
        u"folder"_s,
        u"FolderView"_s,
        [](const QMimeType &type) {
            return type.inherits(u"inode/directory"_s);
        },
    };
}
