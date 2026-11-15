#include "controller.h"

#include "folderlisting.h"
#include "generalsettings.h"
#include "logging.h"
#include "mainwindow.h"
#include "memory.h"
#include "popupwindow.h"

#include <KIO/OpenUrlJob>

#include <QFileInfo>
#include <QImage>
#include <QJsonArray>

#include <algorithm>

using namespace Qt::StringLiterals;

namespace
{
constexpr std::chrono::seconds freeMemoryDelay(10);
constexpr std::chrono::seconds startupGrace(5);

void openWithDefaultApplication(const QString &file, const QString &activationToken, QObject *context, std::function<void()> finished)
{
    auto *job = new KIO::OpenUrlJob(QUrl::fromLocalFile(file));
    job->setStartupId(activationToken.toUtf8());
    job->setRunExecutables(false);
    QObject::connect(job, &KJob::result, job, [job, file] {
        if (job->error()) {
            qCWarning(lcKeek).noquote() << "could not open" << file << ":" << job->errorString();
        }
    });
    QObject::connect(job, &KJob::finished, context, std::move(finished));
    job->start();
}
}

Controller::Controller(QObject *parent)
    : QObject(parent)
    , m_renderer(&m_registry.fallback())
    , m_opener([this](const QString &file, const QString &activationToken) {
        ++m_openingFiles;
        openWithDefaultApplication(file, activationToken, this, [this] {
            --m_openingFiles;
            updateIdleTimer();
        });
    })
    , m_window(std::make_unique<PopupWindow>(this))
{
    m_idleTimer.setSingleShot(true);
    connect(&m_idleTimer, &QTimer::timeout, this, &Controller::idleTimeout);
    connect(GeneralSettings::instance(), &GeneralSettings::changed, this, &Controller::updateIdleTimer);
    connect(m_window.get(), &PopupWindow::visibleChanged, this, [this] {
        if (!m_window->isVisible()) {
            setShowing(false);
        }
        updateIdleTimer();
    });
    m_window->load();
    updateIdleTimer();
    if (m_idleTimer.isActive() && m_idleTimer.intervalAsDuration() < startupGrace) {
        m_idleTimer.start(startupGrace);
    }
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

MainWindow *Controller::mainWindow() const
{
    return m_mainWindow.get();
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

QString Controller::notice() const
{
    return m_notice;
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
    m_notice.clear();
    const QFileInfo info(currentFile());
    if (!info.exists()) {
        m_renderer = &m_registry.fallback();
        m_mimeType = QMimeType();
        m_fileError = tr("This file does not exist. It may have been moved or deleted.");
    } else {
        const RendererRegistry::Match match = m_registry.select(currentFile());
        m_mimeType = match.mimeType;
        if (info.isReadable()) {
            m_renderer = match.renderer;
        } else {
            m_renderer = &m_registry.fallback();
            m_fileError = info.isDir() ? tr("You do not have permission to open this folder.") : tr("You do not have permission to read this file.");
        }
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

void Controller::useFallback(const QString &reason)
{
    if (m_renderer == &m_registry.fallback()) {
        return;
    }
    qCInfo(lcKeek).noquote() << "renderer" << m_renderer->id << "could not show" << currentFile() << "- using" << m_registry.fallback().id;
    m_renderer = &m_registry.fallback();
    m_notice = reason;
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
    if (!m_window->isVisible()) {
        return fail(u"The popup is not visible"_s, error);
    }
    return saveScreenshot(m_window->grab(), u"popup"_s, file, error);
}

bool Controller::screenshotWindow(const QString &file, QString *error)
{
    if (!m_mainWindow || !m_mainWindow->isVisible()) {
        return fail(u"The Keek window is not open"_s, error);
    }
    return saveScreenshot(m_mainWindow->grab(), u"window"_s, file, error);
}

bool Controller::saveScreenshot(const QImage &image, const QString &what, const QString &file, QString *error)
{
    if (image.isNull()) {
        return fail(u"Could not grab the %1"_s.arg(what), error);
    }
    if (!image.save(file)) {
        return fail(u"Could not write %1"_s.arg(file), error);
    }
    return true;
}

bool Controller::fail(const QString &problem, QString *error)
{
    m_lastError = problem;
    if (error) {
        *error = problem;
    }
    qCWarning(lcKeek).noquote() << problem;
    return false;
}

bool Controller::openWindow(const QString &activationToken, QString *error)
{
    if (!m_mainWindow) {
        auto window = std::make_unique<MainWindow>();
        if (!window->load()) {
            return fail(u"The Keek window could not be loaded"_s, error);
        }
        connect(window.get(), &MainWindow::closed, this, [this] {
            QMetaObject::invokeMethod(this, &Controller::mainWindowClosed, Qt::QueuedConnection);
        });
        m_mainWindow = std::move(window);
    }
    m_lastError.clear();
    m_mainWindow->show(activationToken);
    updateIdleTimer();
    qCInfo(lcKeek) << "open window";
    return true;
}

void Controller::mainWindowClosed()
{
    if (!m_mainWindow || m_mainWindow->isVisible()) {
        return;
    }
    m_mainWindow.reset();
    releaseFreedMemory();
    qCInfo(lcKeek) << "close window";
    updateIdleTimer();
}

void Controller::setIdleInterval(std::chrono::milliseconds interval)
{
    m_idleInterval = interval;
    updateIdleTimer();
}

void Controller::updateIdleTimer()
{
    m_idleTimer.stop();
    const GeneralSettings::WhenClosed whenClosed = GeneralSettings::instance()->whenClosed();
    if (whenClosed == GeneralSettings::WhenClosed::KeepReady || m_window->isVisible() || m_mainWindow || m_openingFiles > 0) {
        return;
    }
    if (whenClosed == GeneralSettings::WhenClosed::FreeMemory && !m_window->isLoaded()) {
        return;
    }
    if (m_idleInterval) {
        m_idleTimer.setInterval(*m_idleInterval);
    } else if (whenClosed == GeneralSettings::WhenClosed::FreeMemory) {
        m_idleTimer.setInterval(freeMemoryDelay);
    } else {
        m_idleTimer.setInterval(GeneralSettings::instance()->stopDelay());
    }
    m_idleTimer.start();
}

void Controller::idleTimeout()
{
    if (m_window->isVisible() || m_mainWindow || m_openingFiles > 0) {
        return;
    }
    switch (GeneralSettings::instance()->whenClosed()) {
    case GeneralSettings::WhenClosed::KeepReady:
        break;
    case GeneralSettings::WhenClosed::FreeMemory:
        m_window->unload();
        break;
    case GeneralSettings::WhenClosed::Stop:
        qCInfo(lcKeek) << "stopping after being idle";
        Q_EMIT stopRequested();
        break;
    }
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
        {u"notice"_s, m_notice.isEmpty() ? QJsonValue() : QJsonValue(m_notice)},
        {u"width"_s, size.width()},
        {u"height"_s, size.height()},
        {u"activationToken"_s, m_hadActivationToken},
        {u"loaded"_s, m_window->isLoaded()},
        {u"window"_s, m_mainWindow != nullptr && m_mainWindow->isVisible()},
        {u"lastError"_s, error.isEmpty() ? QJsonValue() : QJsonValue(error)},
    };
}
