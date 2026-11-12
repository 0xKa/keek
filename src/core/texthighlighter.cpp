#include "texthighlighter.h"

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

#include <QCoreApplication>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

namespace
{
Q_GLOBAL_STATIC(KSyntaxHighlighting::Repository, repository)

void setLineDirections(QTextDocument *document)
{
    QTextCursor cursor(document);
    cursor.beginEditBlock();
    for (QTextBlock block = document->begin(); block.isValid(); block = block.next()) {
        QTextBlockFormat format;
        format.setLayoutDirection(block.text().isRightToLeft() ? Qt::RightToLeft : Qt::LeftToRight);
        cursor.setPosition(block.position());
        cursor.mergeBlockFormat(format);
    }
    cursor.endEditBlock();
}
}

TextHighlighter::TextHighlighter(QObject *parent)
    : QObject(parent)
{
}

QQuickTextDocument *TextHighlighter::document() const
{
    return m_document;
}

void TextHighlighter::setDocument(QQuickTextDocument *document)
{
    if (m_document == document) {
        return;
    }
    m_document = document;
    Q_EMIT documentChanged();
    apply();
}

TextFile *TextHighlighter::textFile() const
{
    return m_file;
}

void TextHighlighter::setTextFile(TextFile *textFile)
{
    if (m_file == textFile) {
        return;
    }
    disconnect(m_fileConnection);
    m_file = textFile;
    if (m_file) {
        m_fileConnection = connect(m_file, &TextFile::contentChanged, this, &TextHighlighter::apply);
    }
    Q_EMIT textFileChanged();
    apply();
}

bool TextHighlighter::isDark() const
{
    return m_dark;
}

void TextHighlighter::setDark(bool dark)
{
    if (m_dark == dark) {
        return;
    }
    m_dark = dark;
    Q_EMIT darkChanged();
    apply();
}

void TextHighlighter::apply()
{
    if (!m_document || !m_file) {
        return;
    }
    QTextDocument *document = m_document->textDocument();

    QCoreApplication::removePostedEvents(&m_highlighter, QEvent::MetaCall);
    m_highlighter.setDocument(nullptr);
    document->setPlainText(m_file->text());
    setLineDirections(document);
    m_highlighter.setDefinition(repository->definitionForFileName(m_file->fileName()));
    m_highlighter.setTheme(repository->defaultTheme(m_dark ? KSyntaxHighlighting::Repository::DarkTheme : KSyntaxHighlighting::Repository::LightTheme));
    m_highlighter.setDocument(document);
    m_highlighter.rehighlight();
}
