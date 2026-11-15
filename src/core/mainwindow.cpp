#include "mainwindow.h"

#include "logging.h"

#include <KWindowSystem>

#include <QQmlError>
#include <QQuickItem>
#include <QQuickView>

#include <cmath>

using namespace Qt::StringLiterals;

namespace
{
constexpr QSize defaultSize(640, 520);

QSize implicitSize(const QQuickItem *item)
{
    return QSize(static_cast<int>(std::ceil(item->implicitWidth())), static_cast<int>(std::ceil(item->implicitHeight())));
}
}

MainWindow::MainWindow(QObject *parent)
    : QObject(parent)
    , m_view(std::make_unique<QQuickView>())
{
    m_view->setTitle(u"Keek"_s);
    m_view->setResizeMode(QQuickView::SizeRootObjectToView);
    connect(m_view.get(), &QWindow::visibleChanged, this, [this](bool visible) {
        if (!visible) {
            Q_EMIT closed();
        }
    });
}

MainWindow::~MainWindow() = default;

bool MainWindow::load()
{
    m_view->loadFromModule("Keek", "MainWindow");

    const QList<QQmlError> errors = m_view->errors();
    for (const QQmlError &error : errors) {
        qCCritical(lcKeek).noquote() << error.toString();
    }

    const QQuickItem *root = m_view->rootObject();
    if (!root) {
        return false;
    }
    const QSize minimum = implicitSize(root);
    m_view->setMinimumSize(minimum);
    m_view->resize(defaultSize.expandedTo(minimum));
    return true;
}

QQuickWindow *MainWindow::quickWindow() const
{
    return m_view.get();
}

void MainWindow::show(const QString &activationToken)
{
    m_view->show();
    if (activationToken.isEmpty()) {
        m_view->requestActivate();
        return;
    }
    KWindowSystem::setCurrentXdgActivationToken(activationToken);
    KWindowSystem::activateWindow(m_view.get());
}

void MainWindow::close()
{
    m_view->close();
}

bool MainWindow::isVisible() const
{
    return m_view->isVisible();
}

bool MainWindow::isActive() const
{
    return m_view->isActive();
}

QSize MainWindow::size() const
{
    return m_view->size();
}

QImage MainWindow::grab()
{
    return m_view->grabWindow();
}
