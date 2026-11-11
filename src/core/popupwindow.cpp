#include "popupwindow.h"

#include "logging.h"

#include <KWaylandExtras>
#include <KWindowSystem>

#include <QFuture>
#include <QIcon>
#include <QMetaMethod>

#include <QQmlError>
#include <QQuickItem>
#include <QQuickView>
#include <QScreen>
#include <QStandardPaths>

#include <chrono>
#include <utility>

using namespace Qt::StringLiterals;

namespace
{
constexpr QSize fallbackAvailableSize(1280, 800);
constexpr double screenShare = 0.85;
constexpr int maximumCorrections = 3;
constexpr std::chrono::milliseconds maximumSizeWait(150);

void ensureIconTheme()
{
    if (!QIcon::themeName().isEmpty()) {
        return;
    }
    QIcon::setThemeSearchPaths(QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, u"icons"_s, QStandardPaths::LocateDirectory));
    QIcon::setThemeName(u"breeze"_s);
}
}

PopupWindow::PopupWindow(QObject *controller, QObject *parent)
    : QObject(parent)
    , m_view(std::make_unique<QQuickView>())
{
    ensureIconTheme();
    m_view->setTitle(u"Keek"_s);
    m_view->setFlags(Qt::Window | Qt::FramelessWindowHint);
    m_view->setResizeMode(QQuickView::SizeRootObjectToView);
    m_view->setInitialProperties({{u"controller"_s, QVariant::fromValue(controller)}});
    m_sizeWait.setSingleShot(true);
    m_sizeWait.setInterval(maximumSizeWait);
    connect(&m_sizeWait, &QTimer::timeout, this, &PopupWindow::present);
    connect(m_view.get(), &QWindow::visibleChanged, this, &PopupWindow::visibleChanged);
    connect(m_view.get(), &QWindow::screenChanged, this, &PopupWindow::updateAvailableSize);
    connect(m_view.get(), &QQuickWindow::frameSwapped, this, [this] {
        if (!m_drawn) {
            m_drawn = true;
            QMetaObject::invokeMethod(this, &PopupWindow::contentSizeChanged, Qt::QueuedConnection);
        }
    });
    connect(m_view.get(), &QWindow::widthChanged, this, &PopupWindow::reassertSize, Qt::QueuedConnection);
    connect(m_view.get(), &QWindow::heightChanged, this, &PopupWindow::reassertSize, Qt::QueuedConnection);
}

PopupWindow::~PopupWindow() = default;

void PopupWindow::load()
{
    m_view->loadFromModule("Keek", "Popup");

    const QList<QQmlError> errors = m_view->errors();
    for (const QQmlError &error : errors) {
        qCCritical(lcKeek).noquote() << error.toString();
    }

    QQuickItem *root = m_view->rootObject();
    if (!root) {
        return;
    }
    const QMetaObject *meta = root->metaObject();
    const QMetaMethod changed = metaObject()->method(metaObject()->indexOfSlot("contentSizeChanged()"));
    for (const char *name : {"preferredSize", "sized"}) {
        connect(root, meta->property(meta->indexOfProperty(name)).notifySignal(), this, changed, Qt::QueuedConnection);
    }
    updateAvailableSize();
}

void PopupWindow::updateAvailableSize()
{
    QQuickItem *root = m_view->rootObject();
    if (!root) {
        return;
    }
    const QScreen *screen = m_view->screen();
    const QSize available = screen ? screen->availableSize() * screenShare : fallbackAvailableSize;
    root->setProperty("availableSize", QSizeF(available));
    contentSizeChanged();
}

QSize PopupWindow::preferredSize() const
{
    const QQuickItem *root = m_view->rootObject();
    return root ? root->property("preferredSize").toSizeF().toSize() : QSize();
}

bool PopupWindow::isContentSized() const
{
    const QQuickItem *root = m_view->rootObject();
    return root && root->property("sized").toBool();
}

void PopupWindow::contentSizeChanged()
{
    if (m_showPending) {
        if (isContentSized()) {
            present();
        }
        return;
    }
    m_corrections = 0;
    applyPreferredSize();
}

void PopupWindow::reassertSize()
{
    if (!m_view->isVisible() || m_corrections >= maximumCorrections || preferredSize() == m_view->size()) {
        return;
    }
    ++m_corrections;
    applyPreferredSize();
}

void PopupWindow::applyPreferredSize()
{
    QQuickItem *root = m_view->rootObject();
    if (!root || (m_view->isVisible() && !m_drawn)) {
        return;
    }
    const QSize preferred = preferredSize();
    if (preferred.isEmpty() || preferred == m_view->size()) {
        return;
    }
    m_view->resize(preferred);
    root->setSize(preferred);
}

bool PopupWindow::isReady() const
{
    return m_view->status() == QQuickView::Ready;
}

QQuickWindow *PopupWindow::quickWindow() const
{
    return m_view.get();
}

void PopupWindow::show(const QString &activationToken)
{
    m_pendingToken = activationToken;
    if (m_view->isVisible() || isContentSized()) {
        present();
        return;
    }
    m_showPending = true;
    m_sizeWait.start();
}

void PopupWindow::present()
{
    const bool wasVisible = m_view->isVisible();
    m_showPending = false;
    m_sizeWait.stop();
    m_corrections = 0;
    applyPreferredSize();
    if (!wasVisible) {
        m_drawn = false;
    }
    m_view->show();

    const QString token = std::exchange(m_pendingToken, QString());
    if (token.isEmpty()) {
        m_view->requestActivate();
        return;
    }
    KWindowSystem::setCurrentXdgActivationToken(token);
    KWindowSystem::activateWindow(m_view.get());
}

void PopupWindow::hide()
{
    m_showPending = false;
    m_sizeWait.stop();
    m_pendingToken.clear();
    m_view->hide();
}

void PopupWindow::requestActivationToken(std::function<void(const QString &)> callback)
{
    if (!KWindowSystem::isPlatformWayland()) {
        callback({});
        return;
    }
    KWaylandExtras::xdgActivationToken(m_view.get(), QString()).then(this, std::move(callback));
}

bool PopupWindow::isVisible() const
{
    return m_showPending || m_view->isVisible();
}

bool PopupWindow::isActive() const
{
    return m_view->isActive();
}

QSize PopupWindow::size() const
{
    return m_view->size();
}

QImage PopupWindow::grab()
{
    return m_view->grabWindow();
}

bool PopupWindow::isContentReady() const
{
    const QQuickItem *root = m_view->rootObject();
    return root && root->property("contentReady").toBool();
}

QString PopupWindow::contentError() const
{
    const QQuickItem *root = m_view->rootObject();
    return root ? root->property("contentError").toString() : QString();
}
