#include "controller.h"

#include "logging.h"
#include "popupwindow.h"

#include <QFileInfo>
#include <QImage>
#include <QJsonArray>

using namespace Qt::StringLiterals;

Controller::Controller(QObject *parent)
    : QObject(parent)
    , m_window(std::make_unique<PopupWindow>(this))
{
}

Controller::~Controller() = default;

bool Controller::isReady() const
{
    return m_window->isReady();
}

PopupWindow *Controller::window() const
{
    return m_window.get();
}

QString Controller::currentFile() const
{
    return m_files.value(0);
}

QString Controller::currentFileName() const
{
    return QFileInfo(currentFile()).fileName();
}

void Controller::show(const QStringList &paths, const QString &activationToken)
{
    if (paths.isEmpty()) {
        return;
    }

    const bool fileChanged = currentFile() != paths.first();
    m_files = paths;
    m_hadActivationToken = !activationToken.isEmpty();
    m_lastError.clear();
    if (fileChanged) {
        Q_EMIT currentFileChanged();
    }

    m_window->show(activationToken);
    qCInfo(lcKeek).noquote() << "show" << currentFile();
}

void Controller::toggle(const QStringList &paths, const QString &activationToken)
{
    if (m_window->isVisible() && !paths.isEmpty() && paths.first() == currentFile()) {
        hide();
        return;
    }
    show(paths, activationToken);
}

void Controller::hide()
{
    if (!m_window->isVisible()) {
        return;
    }
    m_window->hide();
    qCInfo(lcKeek) << "hide";
}

bool Controller::screenshot(const QString &file, QString *error)
{
    QString problem;
    if (!m_window->isVisible()) {
        problem = u"The popup is not visible"_s;
    } else {
        const QImage image = m_window->grab();
        if (image.isNull()) {
            problem = u"Could not grab the popup"_s;
        } else if (!image.save(file)) {
            problem = u"Could not write %1"_s.arg(file);
        }
    }

    if (problem.isEmpty()) {
        return true;
    }

    m_lastError = problem;
    if (error) {
        *error = problem;
    }
    qCWarning(lcKeek).noquote() << "screenshot failed:" << problem;
    return false;
}

QJsonObject Controller::state() const
{
    const QSize size = m_window->size();
    const bool hasFile = !m_files.isEmpty();
    return {
        {u"visible"_s, m_window->isVisible()},
        {u"active"_s, m_window->isActive()},
        {u"file"_s, hasFile ? QJsonValue(currentFile()) : QJsonValue()},
        {u"files"_s, QJsonArray::fromStringList(m_files)},
        {u"renderer"_s, hasFile ? QJsonValue(u"placeholder"_s) : QJsonValue()},
        {u"width"_s, size.width()},
        {u"height"_s, size.height()},
        {u"activationToken"_s, m_hadActivationToken},
        {u"lastError"_s, m_lastError.isEmpty() ? QJsonValue() : QJsonValue(m_lastError)},
    };
}
