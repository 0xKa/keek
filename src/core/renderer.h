#pragma once

#include <QMimeType>
#include <QString>

#include <functional>

struct Renderer {
    QString id;
    QString component;
    std::function<bool(const QMimeType &)> accepts;
};

Renderer imageRenderer();
Renderer pdfRenderer();
Renderer textRenderer();
Renderer folderRenderer();
Renderer fallbackRenderer();
