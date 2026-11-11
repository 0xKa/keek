#include "controller.h"

#include "folderlisting.h"
#include "logging.h"
#include "popupwindow.h"

#include <KIO/OpenUrlJob>

#include <QFileInfo>
#include <QImage>
#include <QJsonArray>

#include <algorithm>

using namespace Qt::StringLiterals;

namespace
{
void openWithDefaultApplication(const QString &file, const QString &activationToken)
{
    auto *job = new KIO::OpenUrlJob(QUrl::fromLocalFile(file));
    job->setStartupId(activationToken.toUtf8());
    job->setRunExecutables(false);
    QObject::connect(job, &KJob::result, job, [job, file] {
        if (job->error()) {
            qCWarning(lcKeek).noquote() << "could not open" << file << ":" << job->errorString();
        }
    });
    job->start();
}
}

Controller::Controller(QObject *parent)
    : QObject(parent)
    , m_renderer(&m_registry.fallback())
    , m_opener(openWithDefaultApplication)
    , m_window(std::make_unique<PopupWindow>(this))
{
    connect(m_window.get(), &PopupWindow::visibleChanged, this, [this] {
        if (!m_window->isVisible()) {
            setShowing(false);
        }
    });
    m_window->load();
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
    return m_files.value(m_index);
}

QString Controller::currentFileName() const
{
    return QFileInfo(currentFile()).fileName();
}

QUrl Controller::currentFileUrl() const
{
    return m_files.isEmpty() ? QUrl() : QUrl::fromLocalFile(currentFile());
}

QString Controller::fileError() const
{
    return m_fileError;
}

QString Controller::rendererComponent() const
{
    return m_renderer->component;
}

bool Controller::isShowing() const
{
    return m_showing;
}

void Controller::setShowing(bool showing)
{
    if (m_showing == showing) {
        return;
    }
    m_showing = showing;
    Q_EMIT showingChanged();
}

void Controller::show(const QStringList &paths, const QString &activationToken)
{
    if (paths.isEmpty()) {
        return;
    }

    m_requested = paths;
    m_files = paths.size() == 1 ? browsableSiblings(paths.constFirst()) : paths;
    m_index = std::max<qsizetype>(0, m_files.indexOf(paths.constFirst()));
    m_hadActivationToken = !activationToken.isEmpty();
    m_lastError.clear();
    selectCurrent();
    setShowing(true);

    m_window->show(activationToken);
    qCInfo(lcKeek).noquote() << "show" << currentFile() << "renderer" << m_renderer->id;
}

void Controller::selectCurrent()
{
    m_fileError.clear();
    if (QFileInfo::exists(currentFile())) {
        const RendererRegistry::Match match = m_registry.select(currentFile());
        m_renderer = match.renderer;
        m_mimeType = match.mimeType;
    } else {
        m_renderer = &m_registry.fallback();
        m_mimeType = QMimeType();
        m_fileError = tr("This file does not exist");
    }
    Q_EMIT currentFileChanged();
}

bool Controller::next()
{
    return moveBy(1);
}

bool Controller::previous()
{
    return moveBy(-1);
}

bool Controller::moveBy(int step)
{
    const qsizetype target = m_index + step;
    if (!m_window->isVisible() || target < 0 || target >= m_files.size()) {
        return false;
    }
    m_index = target;
    m_lastError.clear();
    selectCurrent();
    qCInfo(lcKeek).noquote() << "move to" << currentFile() << "renderer" << m_renderer->id;
    return true;
}

void Controller::openCurrentFile()
{
    if (!m_window->isVisible() || m_files.isEmpty()) {
        return;
    }
    const QString file = currentFile();
    m_window->requestActivationToken([this, file](const QString &token) {
        qCInfo(lcKeek).noquote() << "open" << file;
        m_opener(file, token);
        hide();
    });
}

void Controller::useFallback()
{
    if (m_renderer == &m_registry.fallback()) {
        return;
    }
    qCInfo(lcKeek).noquote() << "renderer" << m_renderer->id << "could not show" << currentFile() << "- using" << m_registry.fallback().id;
    m_renderer = &m_registry.fallback();
    Q_EMIT currentFileChanged();
}

void Controller::setFileOpener(FileOpener opener)
{
    m_opener = std::move(opener);
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
    setShowing(false);
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

QString Controller::status() const
{
    if (!m_window->isVisible()) {
        return u"hidden"_s;
    }
    if (!m_fileError.isEmpty() || !m_window->contentError().isEmpty()) {
        return u"error"_s;
    }
    return m_window->isContentReady() ? u"ready"_s : u"loading"_s;
}

QJsonObject Controller::state() const
{
    const QSize size = m_window->size();
    const bool hasFile = !m_files.isEmpty();

    QString error = m_lastError;
    if (error.isEmpty() && m_window->isVisible()) {
        error = m_fileError.isEmpty() ? m_window->contentError() : m_fileError;
    }

    return {
        {u"visible"_s, m_window->isVisible()},
        {u"active"_s, m_window->isActive()},
        {u"status"_s, status()},
        {u"file"_s, hasFile ? QJsonValue(currentFile()) : QJsonValue()},
        {u"files"_s, QJsonArray::fromStringList(m_requested)},
        {u"index"_s, hasFile ? QJsonValue(m_index) : QJsonValue()},
        {u"count"_s, m_files.size()},
        {u"renderer"_s, hasFile ? QJsonValue(m_renderer->id) : QJsonValue()},
        {u"mimeType"_s, m_mimeType.isValid() ? QJsonValue(m_mimeType.name()) : QJsonValue()},
        {u"width"_s, size.width()},
        {u"height"_s, size.height()},
        {u"activationToken"_s, m_hadActivationToken},
        {u"lastError"_s, error.isEmpty() ? QJsonValue() : QJsonValue(error)},
    };
}
