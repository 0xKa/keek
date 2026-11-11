#include "textfile.h"

#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QStringDecoder>

using namespace Qt::StringLiterals;

namespace
{
QString decode(const QByteArray &data)
{
    const auto detected = QStringConverter::encodingForData(data);
    if (detected) {
        return QStringDecoder(*detected).decode(data);
    }

    QStringDecoder utf8(QStringConverter::Utf8);
    const QString text = utf8.decode(data);
    if (utf8.hasError()) {
        return QString::fromLatin1(data);
    }
    return text;
}
}

TextFile::TextFile(QObject *parent)
    : QObject(parent)
{
}

QUrl TextFile::source() const
{
    return m_source;
}

void TextFile::setSource(const QUrl &source)
{
    if (m_source == source) {
        return;
    }
    m_source = source;
    Q_EMIT sourceChanged();
    load();
}

QString TextFile::fileName() const
{
    return QFileInfo(m_source.toLocalFile()).fileName();
}

QString TextFile::text() const
{
    return m_text;
}

bool TextFile::isReady() const
{
    return m_ready;
}

bool TextFile::isTruncated() const
{
    return m_truncated;
}

QString TextFile::error() const
{
    return m_error;
}

QFont TextFile::fixedFont() const
{
    return QFontDatabase::systemFont(QFontDatabase::FixedFont);
}

void TextFile::load()
{
    m_text.clear();
    m_error.clear();
    m_ready = false;
    m_truncated = false;

    if (m_source.isEmpty()) {
        Q_EMIT contentChanged();
        return;
    }

    QFile file(m_source.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        m_error = tr("This file could not be read");
        Q_EMIT contentChanged();
        return;
    }

    QByteArray data = file.read(maxBytes + 1);
    if (data.size() > maxBytes) {
        m_truncated = true;
        data.truncate(maxBytes);
        const qsizetype lastLineEnd = data.lastIndexOf('\n');
        if (lastLineEnd > 0) {
            data.truncate(lastLineEnd + 1);
        }
    }

    const bool hasByteOrderMark = QStringConverter::encodingForData(data).has_value();
    if (!hasByteOrderMark && data.contains('\0')) {
        m_error = tr("This file is not text");
        m_truncated = false;
        Q_EMIT contentChanged();
        return;
    }

    m_text = decode(data);
    m_ready = true;
    Q_EMIT contentChanged();
}
