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
Renderer textRenderer();
Renderer placeholderRenderer();
