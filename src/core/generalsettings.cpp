#include "generalsettings.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QJSEngine>

#include <algorithm>
#include <array>
#include <utility>

using namespace Qt::StringLiterals;

namespace
{
constexpr int defaultStopAfterMinutes = 10;
constexpr int minimumMinutes = 1;
constexpr int maximumMinutes = 120;

constexpr std::array whenClosedNames = {
    std::pair{GeneralSettings::WhenClosed::KeepReady, "keep"},
    std::pair{GeneralSettings::WhenClosed::FreeMemory, "free"},
    std::pair{GeneralSettings::WhenClosed::Stop, "stop"},
};

KConfigGroup generalGroup()
{
    return KSharedConfig::openConfig(u"keekrc"_s, KConfig::SimpleConfig)->group(u"General"_s);
}

void write(const char *key, const QVariant &value)
{
    KConfigGroup group = generalGroup();
    group.writeEntry(key, value);
    group.sync();
}
}

GeneralSettings::GeneralSettings(QObject *parent)
    : QObject(parent)
{
}

GeneralSettings *GeneralSettings::instance()
{
    static GeneralSettings settings;
    return &settings;
}

GeneralSettings *GeneralSettings::create(QQmlEngine *, QJSEngine *)
{
    GeneralSettings *settings = instance();
    QJSEngine::setObjectOwnership(settings, QJSEngine::CppOwnership);
    return settings;
}

GeneralSettings::WhenClosed GeneralSettings::whenClosed() const
{
    const QString name = generalGroup().readEntry("WhenClosed", QString());
    for (const auto &[value, key] : whenClosedNames) {
        if (name == QLatin1StringView(key)) {
            return value;
        }
    }
    return WhenClosed::KeepReady;
}

void GeneralSettings::setWhenClosed(WhenClosed whenClosed)
{
    if (whenClosed == this->whenClosed()) {
        return;
    }
    for (const auto &[value, key] : whenClosedNames) {
        if (value == whenClosed) {
            write("WhenClosed", QString::fromLatin1(key));
        }
    }
    Q_EMIT changed();
}

int GeneralSettings::stopAfterMinutes() const
{
    return std::clamp(generalGroup().readEntry("StopAfterMinutes", defaultStopAfterMinutes), minimumMinutes, maximumMinutes);
}

void GeneralSettings::setStopAfterMinutes(int minutes)
{
    minutes = std::clamp(minutes, minimumMinutes, maximumMinutes);
    if (minutes == stopAfterMinutes()) {
        return;
    }
    write("StopAfterMinutes", minutes);
    Q_EMIT changed();
}

int GeneralSettings::minimumStopAfterMinutes() const
{
    return minimumMinutes;
}

int GeneralSettings::maximumStopAfterMinutes() const
{
    return maximumMinutes;
}
