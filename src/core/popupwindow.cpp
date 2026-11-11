#include "popupwindow.h"

#include "logging.h"

#include <KWindowSystem>

#include <QQmlError>
#include <QQuickView>

using namespace Qt::StringLiterals;

PopupWindow::PopupWindow(QObject *controller, QObject *parent)
    : QObject(parent)
    , m_view(std::make_unique<QQuickView>())
{
    m_view->setTitle(u"Keek"_s);
    m_view->setFlags(Qt::Window | Qt::FramelessWindowHint);
    m_view->setResizeMode(QQuickView::SizeRootObjectToView);
    m_view->resize(960, 640);
    m_view->setInitialProperties({{u"controller"_s, QVariant::fromValue(controller)}});
    m_view->loadFromModule("Keek", "Popup");

    const QList<QQmlError> errors = m_view->errors();
    for (const QQmlError &error : errors) {
        qCCritical(lcKeek).noquote() << error.toString();
    }
}

PopupWindow::~PopupWindow() = default;

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
    m_view->show();
    if (activationToken.isEmpty()) {
        m_view->requestActivate();
        return;
    }
    KWindowSystem::setCurrentXdgActivationToken(activationToken);
    KWindowSystem::activateWindow(m_view.get());
}

void PopupWindow::hide()
{
    m_view->hide();
}

bool PopupWindow::isVisible() const
{
    return m_view->isVisible();
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
