#include "textfile.h"

#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QStringDecoder>

#include <algorithm>

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

int longestLine(QStringView text, int tabWidth)
{
    int longest = 0;
    int column = 0;
    for (const QChar character : text) {
        if (character == u'\n') {
            longest = std::max(longest, column);
            column = 0;
        } else if (character == u'\t') {
            column += tabWidth - column % tabWidth;
        } else if (!character.isLowSurrogate()) {
            ++column;
        }
    }
    return std::max(longest, column);
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

int TextFile::tabWidth() const
{
    return m_tabWidth;
}

void TextFile::setTabWidth(int tabWidth)
{
    tabWidth = std::max(1, tabWidth);
    if (m_tabWidth == tabWidth) {
        return;
    }
    m_tabWidth = tabWidth;
    Q_EMIT tabWidthChanged();
    measure();
}

int TextFile::columns() const
{
    return m_columns;
}

void TextFile::measure()
{
    const int columns = longestLine(m_text, m_tabWidth);
    if (m_columns == columns) {
        return;
    }
    m_columns = columns;
    Q_EMIT columnsChanged();
}

void TextFile::load()
{
    m_text.clear();
    m_error.clear();
    m_ready = false;
    m_truncated = false;

    if (m_source.isEmpty()) {
        measure();
        Q_EMIT contentChanged();
        return;
    }

    QFile file(m_source.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        m_error = tr("This file could not be read: %1.").arg(file.errorString());
        measure();
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
        m_error = tr("This file holds binary data, not text.");
        m_truncated = false;
        measure();
        Q_EMIT contentChanged();
        return;
    }

    m_text = decode(data);
    m_ready = true;
    measure();
    Q_EMIT contentChanged();
}
