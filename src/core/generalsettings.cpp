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
constexpr int defaultStopAfter = 10;
constexpr int maximumCount = 999;

constexpr std::array whenClosedNames = {
    std::pair{GeneralSettings::WhenClosed::KeepReady, "keep"},
    std::pair{GeneralSettings::WhenClosed::FreeMemory, "free"},
    std::pair{GeneralSettings::WhenClosed::Stop, "stop"},
};

constexpr std::array unitNames = {
    std::pair{GeneralSettings::Unit::Seconds, "seconds"},
    std::pair{GeneralSettings::Unit::Minutes, "minutes"},
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

template<typename Names, typename Value>
Value readName(const Names &names, const char *key, Value fallback)
{
    const QString name = generalGroup().readEntry(key, QString());
    for (const auto &[value, text] : names) {
        if (name == QLatin1StringView(text)) {
            return value;
        }
    }
    return fallback;
}

template<typename Names, typename Value>
void writeName(const Names &names, const char *key, Value wanted)
{
    for (const auto &[value, text] : names) {
        if (value == wanted) {
            write(key, QString::fromLatin1(text));
        }
    }
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
    return readName(whenClosedNames, "WhenClosed", WhenClosed::KeepReady);
}

void GeneralSettings::setWhenClosed(WhenClosed whenClosed)
{
    if (whenClosed == this->whenClosed()) {
        return;
    }
    writeName(whenClosedNames, "WhenClosed", whenClosed);
    Q_EMIT changed();
}

int GeneralSettings::stopAfter() const
{
    return std::clamp(generalGroup().readEntry("StopAfter", defaultStopAfter), 0, maximumCount);
}

void GeneralSettings::setStopAfter(int count)
{
    count = std::clamp(count, 0, maximumCount);
    if (count == stopAfter()) {
        return;
    }
    write("StopAfter", count);
    Q_EMIT changed();
}

GeneralSettings::Unit GeneralSettings::stopAfterUnit() const
{
    return readName(unitNames, "StopAfterUnit", Unit::Minutes);
}

void GeneralSettings::setStopAfterUnit(Unit unit)
{
    if (unit == stopAfterUnit()) {
        return;
    }
    writeName(unitNames, "StopAfterUnit", unit);
    Q_EMIT changed();
}

int GeneralSettings::maximumStopAfter() const
{
    return maximumCount;
}

std::chrono::seconds GeneralSettings::stopDelay() const
{
    const std::chrono::seconds count(stopAfter());
    return stopAfterUnit() == Unit::Minutes ? count * 60 : count;
}
