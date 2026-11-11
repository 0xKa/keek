#pragma once

#include "renderer.h"

#include <QMimeDatabase>

#include <vector>

class RendererRegistry
{
public:
    struct Match {
        const Renderer *renderer;
        QMimeType mimeType;
    };

    RendererRegistry();

    Match select(const QString &path) const;
    const Renderer &fallback() const;

private:
    std::vector<Renderer> m_renderers;
    QMimeDatabase m_mimeDatabase;
};
