#include "rendererregistry.h"

RendererRegistry::RendererRegistry()
{
    m_renderers.push_back(imageRenderer());
    m_renderers.push_back(textRenderer());
    m_renderers.push_back(folderRenderer());
    m_renderers.push_back(fallbackRenderer());
}

RendererRegistry::Match RendererRegistry::select(const QString &path) const
{
    const QMimeType mimeType = m_mimeDatabase.mimeTypeForFile(path);
    for (const Renderer &renderer : m_renderers) {
        if (renderer.accepts(mimeType)) {
            return {&renderer, mimeType};
        }
    }
    return {&fallback(), mimeType};
}

const Renderer &RendererRegistry::fallback() const
{
    return m_renderers.back();
}
