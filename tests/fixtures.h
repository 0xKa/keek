#pragma once

#include <QAnyStringView>
#include <QString>

inline QString fixture(QAnyStringView name)
{
    return QStringLiteral(KEEK_FIXTURES_DIR "/") + name.toString();
}
