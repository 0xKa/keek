#include "fileicon.h"

#include <QIcon>
#include <QPainter>

using namespace Qt::StringLiterals;

FileIcon::FileIcon(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
}

QString FileIcon::name() const
{
    return m_name;
}

void FileIcon::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }
    m_name = name;
    Q_EMIT nameChanged();
    update();
}

void FileIcon::paint(QPainter *painter)
{
    const QIcon icon = QIcon::fromTheme(m_name, QIcon::fromTheme(u"unknown"_s));
    icon.paint(painter, boundingRect().toRect());
}
