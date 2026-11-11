#include "mediasettings.h"

#include <KConfigGroup>
#include <KSharedConfig>

using namespace Qt::StringLiterals;

namespace
{
KConfigGroup mediaGroup()
{
    return KSharedConfig::openConfig(u"keekrc"_s, KConfig::SimpleConfig)->group(u"Media"_s);
}
}

MediaSettings::MediaSettings(QObject *parent)
    : QObject(parent)
{
}

bool MediaSettings::isVideoMuted() const
{
    return mediaGroup().readEntry("VideoMuted", true);
}

void MediaSettings::setVideoMuted(bool muted)
{
    if (muted != isVideoMuted()) {
        write("VideoMuted", muted);
    }
}

bool MediaSettings::isAudioMuted() const
{
    return mediaGroup().readEntry("AudioMuted", false);
}

void MediaSettings::setAudioMuted(bool muted)
{
    if (muted != isAudioMuted()) {
        write("AudioMuted", muted);
    }
}

bool MediaSettings::autoplay() const
{
    return qEnvironmentVariable("KEEK_AUTOPLAY") != "0"_L1;
}

void MediaSettings::write(const char *key, bool value)
{
    KConfigGroup group = mediaGroup();
    group.writeEntry(key, value);
    group.sync();
    Q_EMIT changed();
}
