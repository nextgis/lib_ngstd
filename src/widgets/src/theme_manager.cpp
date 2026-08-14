/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Per-application theme scope manager
 *****************************************************************************/
#include "theme_manager_p.h"

#include <ngstd/widgets/theme.h>

#include <QApplication>
#include <QChildEvent>
#include <QEvent>
#include <QFocusEvent>
#include <QScopedValueRollback>
#include <QStyleHints>
#include <QTimer>
#include <QWidget>

#include <algorithm>

namespace ngstd {
namespace widgets {
namespace internal {

namespace {

const char managerObjectName[] = "_ngstd_theme_manager";

bool isDark(const QColor &color)
{
    const qreal luminance = 0.2126 * color.redF() + 0.7152 * color.greenF() +
                            0.0722 * color.blueF();
    return luminance < 0.42;
}

int ancestorDistance(const QWidget *widget, const QWidget *ancestor)
{
    int distance = 0;
    const QWidget *candidate = widget;
    while (candidate) {
        if (candidate == ancestor) return distance;
        candidate = candidate->parentWidget();
        ++distance;
    }
    return -1;
}

int widgetDepth(const QWidget *widget)
{
    int depth = 0;
    for (const QWidget *candidate = widget; candidate;
         candidate = candidate->parentWidget()) {
        ++depth;
    }
    return depth;
}

} // namespace

ThemeManager *ThemeManager::instance(QApplication *application)
{
    if (!application) return nullptr;
    ThemeManager *manager = application->findChild<ThemeManager *>(
        QString::fromLatin1(managerObjectName), Qt::FindDirectChildrenOnly);
    if (manager) return manager;
    return new ThemeManager(application);
}

ThemeManager::ThemeManager(QApplication *application)
    : QObject(application), m_application(application),
      m_systemColorScheme(
          isDark(application->palette().color(QPalette::Window))
              ? ColorScheme::Dark
              : ColorScheme::Light)
{
    setObjectName(QString::fromLatin1(managerObjectName));
    application->installEventFilter(this);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(application->styleHints(), &QStyleHints::colorSchemeChanged, this,
            [this](Qt::ColorScheme scheme) {
                if (scheme == Qt::ColorScheme::Unknown) return;
                const ColorScheme colorScheme =
                    scheme == Qt::ColorScheme::Dark ? ColorScheme::Dark
                                                    : ColorScheme::Light;
                if (m_systemColorScheme == colorScheme) return;
                m_systemColorScheme = colorScheme;
                reapplySystemControllers();
            });
#endif
}

ThemeController *ThemeManager::attach(QWidget *rootWidget,
                                      const ThemeOptions &options)
{
    if (!rootWidget) return nullptr;
    for (const QPointer<ThemeController> &controller : m_controllers) {
        if (controller && controller->rootWidget() == rootWidget) {
            controller->setOptions(options);
            return controller;
        }
    }
    return new ThemeController(rootWidget, options, this);
}

ThemeController *ThemeManager::applyToApplication(const ThemeOptions &options)
{
    for (const QPointer<ThemeController> &controller : m_controllers) {
        if (controller && controller->isApplicationWide()) {
            controller->setOptions(options);
            return controller;
        }
    }
    return new ThemeController(m_application.data(), options, this);
}

void ThemeManager::registerController(ThemeController *controller)
{
    if (!controller || m_controllers.contains(controller)) return;
    m_controllers.append(controller);
}

void ThemeManager::unregisterController(ThemeController *controller)
{
    m_controllers.erase(
        std::remove_if(
            m_controllers.begin(), m_controllers.end(),
            [controller](const QPointer<ThemeController> &candidate) {
                return !candidate || candidate == controller;
            }),
        m_controllers.end());
}

void ThemeManager::apply(ThemeController *controller)
{
    if (!controller || !controller->isAttached()) return;
    QScopedValueRollback<bool> guard(m_applying, true);
    controller->applyState();
    QVector<QPointer<ThemeController>> nestedControllers;
    for (const QPointer<ThemeController> &candidate : m_controllers) {
        if (!candidate || candidate == controller ||
            !candidate->isAttached() || candidate->isApplicationWide()) {
            continue;
        }
        const bool isNested = controller->isApplicationWide() ||
                              ancestorDistance(candidate->rootWidget(),
                                               controller->rootWidget()) > 0;
        if (isNested) nestedControllers.append(candidate);
    }
    std::sort(nestedControllers.begin(), nestedControllers.end(),
              [](const QPointer<ThemeController> &left,
                 const QPointer<ThemeController> &right) {
                  return ancestorDistance(right->rootWidget(),
                                          left->rootWidget()) > 0;
              });
    for (const QPointer<ThemeController> &nestedController :
         nestedControllers) {
        if (nestedController) nestedController->applyState();
    }
}

void ThemeManager::reapplyAll()
{
    QScopedValueRollback<bool> guard(m_applying, true);
    QVector<QPointer<ThemeController>> controllers;
    for (const QPointer<ThemeController> &controller : m_controllers) {
        if (controller && controller->isAttached() &&
            (controller->isApplicationWide() || controller->rootWidget())) {
            controllers.append(controller);
        }
    }
    std::stable_sort(controllers.begin(), controllers.end(),
                     [](const QPointer<ThemeController> &left,
                        const QPointer<ThemeController> &right) {
                         const int leftDepth =
                             left->isApplicationWide()
                                 ? -1
                                 : widgetDepth(left->rootWidget());
                         const int rightDepth =
                             right->isApplicationWide()
                                 ? -1
                                 : widgetDepth(right->rootWidget());
                         return leftDepth < rightDepth;
                     });
    for (const QPointer<ThemeController> &controller : controllers) {
        if (controller) controller->applyState();
    }
}

ThemeController *ThemeManager::controllerFor(const QWidget *widget) const
{
    if (!widget) return nullptr;
    ThemeController *applicationController = nullptr;
    ThemeController *nearestController = nullptr;
    int nearestDistance = -1;
    for (const QPointer<ThemeController> &candidate : m_controllers) {
        if (!candidate || !candidate->isAttached()) continue;
        if (candidate->isApplicationWide()) {
            applicationController = candidate.data();
            continue;
        }
        const int distance = ancestorDistance(widget, candidate->rootWidget());
        if (distance >= 0 &&
            (nearestDistance < 0 || distance < nearestDistance)) {
            nearestDistance = distance;
            nearestController = candidate.data();
        }
    }
    return nearestController ? nearestController : applicationController;
}

ColorScheme ThemeManager::systemColorScheme() const
{
    return m_systemColorScheme;
}

bool ThemeManager::eventFilter(QObject *watched, QEvent *event)
{
    if (!event) return QObject::eventFilter(watched, event);
    if (event->type() == QEvent::ApplicationPaletteChange && !m_applying &&
        !applicationPaletteIsManaged()) {
        updateSystemTheme();
    }
    else if (event->type() == QEvent::Polish) {
        decorate(qobject_cast<QWidget *>(watched));
    }
    else if (event->type() == QEvent::ChildAdded) {
        QChildEvent *childEvent = static_cast<QChildEvent *>(event);
        QPointer<QWidget> child = qobject_cast<QWidget *>(childEvent->child());
        if (child) {
            QTimer::singleShot(0, this,
                               [this, child]() { decorate(child.data()); });
        }
    }
    else if (event->type() == QEvent::FocusIn ||
             event->type() == QEvent::FocusOut) {
        QWidget *widget = qobject_cast<QWidget *>(watched);
        ThemeController *controller = controllerFor(widget);
        if (controller) {
            bool keyboardFocus = false;
            if (event->type() == QEvent::FocusIn) {
                const Qt::FocusReason reason =
                    static_cast<QFocusEvent *>(event)->reason();
                keyboardFocus = reason == Qt::TabFocusReason ||
                                reason == Qt::BacktabFocusReason ||
                                reason == Qt::ShortcutFocusReason;
            }
            controller->setKeyboardFocus(widget, keyboardFocus);
        }
    }
    else if (event->type() == QEvent::WindowDeactivate ||
             event->type() == QEvent::Hide ||
             event->type() == QEvent::Close) {
        QWidget *widget = qobject_cast<QWidget *>(watched);
        if (widget && widget->isWindow()) {
            ThemeController *controller = controllerFor(widget);
            if (controller) controller->clearKeyboardFocus();
        }
    }
    return QObject::eventFilter(watched, event);
}

void ThemeManager::decorate(QWidget *widget)
{
    ThemeController *controller = controllerFor(widget);
    if (controller) controller->decorateWidget(widget);
}

void ThemeManager::updateSystemTheme()
{
    if (!m_application) return;
    const ColorScheme colorScheme =
        isDark(m_application->palette().color(QPalette::Window))
            ? ColorScheme::Dark
            : ColorScheme::Light;
    if (m_systemColorScheme == colorScheme) return;
    m_systemColorScheme = colorScheme;
    reapplySystemControllers();
}

void ThemeManager::reapplySystemControllers()
{
    const QVector<QPointer<ThemeController>> controllers = m_controllers;
    for (const QPointer<ThemeController> &controller : controllers) {
        if (controller && controller->isAttached() &&
            controller->themeMode() == ThemeMode::System) {
            apply(controller.data());
        }
    }
}

bool ThemeManager::applicationPaletteIsManaged() const
{
    for (const QPointer<ThemeController> &controller : m_controllers) {
        if (controller && controller->isAttached() &&
            controller->isApplicationWide()) {
            return true;
        }
    }
    return false;
}

} // namespace internal
} // namespace widgets
} // namespace ngstd
