#pragma once

#include "textfile.h"

#include <KSyntaxHighlighting/SyntaxHighlighter>

#include <QObject>
#include <QPointer>
#include <QQuickTextDocument>
#include <QtQmlIntegration/qqmlintegration.h>

class TextHighlighter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged FINAL)
    Q_PROPERTY(TextFile *textFile READ textFile WRITE setTextFile NOTIFY textFileChanged FINAL)
    Q_PROPERTY(bool dark READ isDark WRITE setDark NOTIFY darkChanged FINAL)

public:
    explicit TextHighlighter(QObject *parent = nullptr);

    QQuickTextDocument *document() const;
    void setDocument(QQuickTextDocument *document);

    TextFile *textFile() const;
    void setTextFile(TextFile *textFile);

    bool isDark() const;
    void setDark(bool dark);

Q_SIGNALS:
    void documentChanged();
    void textFileChanged();
    void darkChanged();

private:
    void apply();

    QPointer<QQuickTextDocument> m_document;
    QPointer<TextFile> m_file;
    QMetaObject::Connection m_fileConnection;
    bool m_dark = false;
    KSyntaxHighlighting::SyntaxHighlighter m_highlighter;
};
