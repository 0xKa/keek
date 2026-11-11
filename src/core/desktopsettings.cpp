#include "desktopsettings.h"

#include <KConfigGroup>

#include <algorithm>

using namespace Qt::StringLiterals;

DesktopSettings::DesktopSettings(QObject *parent)
    : QObject(parent)
    , m_config(KSharedConfig::openConfig(u"kdeglobals"_s, KConfig::NoGlobals))
    , m_watcher(KConfigWatcher::create(m_config))
{
    connect(m_watcher.data(), &KConfigWatcher::configChanged, this, [this](const KConfigGroup &group) {
        if (group.name() == "KDE"_L1) {
            read();
        }
    });
    read();
}

double DesktopSettings::animationFactor() const
{
    return m_animationFactor;
}

void DesktopSettings::read()
{
    m_config->reparseConfiguration();
    const double factor = std::max(0.0, m_config->group(u"KDE"_s).readEntry("AnimationDurationFactor", 1.0));
    if (qFuzzyCompare(factor + 1.0, m_animationFactor + 1.0)) {
        return;
    }
    m_animationFactor = factor;
    Q_EMIT animationFactorChanged();
}
