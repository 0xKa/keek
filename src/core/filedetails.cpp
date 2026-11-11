#include "filedetails.h"

#include <KFileItem>
#include <KFormat>

#include <QDateTime>
#include <QFileInfo>
#include <QLocale>

FileDetails::FileDetails(QObject *parent)
    : QObject(parent)
{
}

QUrl FileDetails::source() const
{
    return m_source;
}

void FileDetails::setSource(const QUrl &source)
{
    if (m_source == source) {
        return;
    }
    m_source = source;

    m_exists = source.isLocalFile() && QFileInfo::exists(source.toLocalFile());
    if (m_exists) {
        KFileItem item(source);
        m_iconName = item.iconName();
        m_typeName = item.mimeComment();
        m_size = item.isDir() ? QString() : KFormat().formatByteSize(static_cast<double>(item.size()));
        m_modified = QLocale().toString(item.time(KFileItem::ModificationTime), QLocale::ShortFormat);
    } else {
        m_iconName.clear();
        m_typeName.clear();
        m_size.clear();
        m_modified.clear();
    }
    Q_EMIT sourceChanged();
}

bool FileDetails::exists() const
{
    return m_exists;
}

QString FileDetails::iconName() const
{
    return m_iconName;
}

QString FileDetails::typeName() const
{
    return m_typeName;
}

QString FileDetails::size() const
{
    return m_size;
}

QString FileDetails::modified() const
{
    return m_modified;
}
