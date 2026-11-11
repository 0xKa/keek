#pragma once

#include <QQuickPaintedItem>
#include <QtQmlIntegration/qqmlintegration.h>

class FileIcon : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)

public:
    explicit FileIcon(QQuickItem *parent = nullptr);

    QString name() const;
    void setName(const QString &name);

    void paint(QPainter *painter) override;

Q_SIGNALS:
    void nameChanged();

private:
    QString m_name;
};
