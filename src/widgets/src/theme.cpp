/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Scoped and application-wide NextGIS theme controller
 *****************************************************************************/
#include <ngstd/widgets/theme.h>

#include "design_tokens_p.h"
#include "resources_p.h"
#include "state_journal_p.h"
#include "theme_manager_p.h"
#include "widget_decorator_p.h"

#include <QApplication>
#include <QFile>
#include <QFontDatabase>
#include <QFocusFrame>
#include <QPalette>
#include <QPointer>
#include <QScopedValueRollback>
#include <QToolTip>
#include <QVariant>
#include <QVector>
#include <QWidget>

#include <memory>
#include <utility>
#include <vector>

namespace {

QString resourcePath(const QString &referencePath)
{
    return QStringLiteral(":/ngstd/widgets/") + referencePath;
}

QString combinedStyleSheet(const QString &original, const QString &theme)
{
    if (original.isEmpty()) return theme;
    return original + QStringLiteral("\n/* ngstd::widgets scoped theme */\n") +
           theme;
}

QString schemeName(ngstd::widgets::ColorScheme scheme)
{
    return scheme == ngstd::widgets::ColorScheme::Dark
               ? QStringLiteral("dark")
               : QStringLiteral("light");
}

QString animationPolicyName(ngstd::widgets::AnimationPolicy policy)
{
    switch (policy) {
    case ngstd::widgets::AnimationPolicy::Enabled:
        return QStringLiteral("enabled");
    case ngstd::widgets::AnimationPolicy::Disabled:
        return QStringLiteral("disabled");
    case ngstd::widgets::AnimationPolicy::System:
        return QStringLiteral("system");
    }
    return QStringLiteral("system");
}

QVector<QPointer<QWidget>> guardedWidgets(const QWidgetList &rawWidgets)
{
    QVector<QPointer<QWidget>> widgets;
    widgets.reserve(rawWidgets.size());
    for (QWidget *widget : rawWidgets)
        widgets.append(QPointer<QWidget>(widget));
    return widgets;
}

} // namespace

namespace ngstd {
namespace widgets {

class ThemeControllerPrivate final
{
public:
    QPointer<QWidget> rootWidget;
    QPointer<QApplication> application;
    QPointer<internal::ThemeManager> manager;
    ThemeOptions options;
    ColorScheme colorScheme = ColorScheme::Light;
    bool attached = true;
    bool applicationWide = false;
    QPointer<QFocusFrame> focusFrame;
    QMetaObject::Connection rootDestroyedConnection;
    QPalette originalToolTipPalette;
    bool toolTipPaletteCaptured = false;
    std::unique_ptr<internal::StateJournal> journal;
    std::vector<std::unique_ptr<internal::WidgetDecorator>> decorators =
        internal::createWidgetDecorators();
};

ThemeController *ThemeController::attach(QWidget *rootWidget,
                                         const ThemeOptions &options)
{
    if (!rootWidget || !qApp) return nullptr;
    internal::ThemeManager *manager = internal::ThemeManager::instance(qApp);
    return manager ? manager->attach(rootWidget, options) : nullptr;
}

ThemeController *ThemeController::applyToApplication(
    QApplication *application, const ThemeOptions &options)
{
    internal::ThemeManager *manager =
        internal::ThemeManager::instance(application);
    return manager ? manager->applyToApplication(options) : nullptr;
}

ThemeController::ThemeController(QWidget *rootWidget,
                                 const ThemeOptions &options,
                                 internal::ThemeManager *manager)
    : QObject(manager), d(new ThemeControllerPrivate)
{
    d->rootWidget = rootWidget;
    d->application = qobject_cast<QApplication *>(manager->parent());
    d->manager = manager;
    d->options = options;
    d->rootDestroyedConnection =
        connect(rootWidget, &QObject::destroyed, this, [this]() {
            detach();
            deleteLater();
        });
    manager->registerController(this);
    manager->apply(this);
}

ThemeController::ThemeController(QApplication *application,
                                 const ThemeOptions &options,
                                 internal::ThemeManager *manager)
    : QObject(manager), d(new ThemeControllerPrivate)
{
    d->application = application;
    d->manager = manager;
    d->options = options;
    d->applicationWide = true;
    d->originalToolTipPalette = QToolTip::palette();
    d->toolTipPaletteCaptured = true;
    manager->registerController(this);
    manager->apply(this);
}

ThemeController::~ThemeController()
{
    detach();
}

ThemeOptions ThemeController::options() const
{
    return d->options;
}

void ThemeController::setOptions(const ThemeOptions &options)
{
    if (!d->attached || d->options == options) return;
    const ThemeOptions previousOptions = d->options;
    const ThemeMode previousMode = d->options.themeMode();
    d->options = options;
    ColorScheme targetScheme = d->colorScheme;
    switch (d->options.themeMode()) {
    case ThemeMode::Dark:
        targetScheme = ColorScheme::Dark;
        break;
    case ThemeMode::Light:
        targetScheme = ColorScheme::Light;
        break;
    case ThemeMode::System:
        if (d->manager) targetScheme = d->manager->systemColorScheme();
        break;
    }
    const bool visualOptionsChanged =
        previousOptions.animationPolicy() != d->options.animationPolicy() ||
        previousOptions.features() != d->options.features();
    if (d->manager &&
        (visualOptionsChanged || targetScheme != d->colorScheme)) {
        d->manager->apply(this);
    }
    emit optionsChanged(d->options);
    if (previousMode != d->options.themeMode())
        emit themeModeChanged(d->options.themeMode());
}

void ThemeController::resetOptions()
{
    setOptions(ThemeOptions());
}

ThemeMode ThemeController::themeMode() const
{
    return d->options.themeMode();
}

void ThemeController::setThemeMode(ThemeMode mode)
{
    ThemeOptions updatedOptions = d->options;
    updatedOptions.setThemeMode(mode);
    setOptions(updatedOptions);
}

void ThemeController::resetThemeMode()
{
    setThemeMode(ThemeMode::System);
}

ColorScheme ThemeController::colorScheme() const
{
    return d->colorScheme;
}

QWidget *ThemeController::rootWidget() const
{
    return d->rootWidget.data();
}

bool ThemeController::isApplicationWide() const
{
    return d->applicationWide;
}

bool ThemeController::isAttached() const
{
    return d->attached;
}

void ThemeController::detach()
{
    if (!d->attached) return;
    QWidget *detachedRoot = d->rootWidget.data();
    const bool wasApplicationWide = d->applicationWide;
    d->attached = false;
    delete d->focusFrame.data();
    d->focusFrame.clear();
    QObject::disconnect(d->rootDestroyedConnection);
    if (d->manager) {
        QScopedValueRollback<bool> guard(d->manager->m_applying, true);
        d->manager->unregisterController(this);
        d->journal.reset();
        if (wasApplicationWide && d->toolTipPaletteCaptured)
            QToolTip::setPalette(d->originalToolTipPalette);
        if (detachedRoot || wasApplicationWide) d->manager->reapplyAll();
    }
    else {
        d->journal.reset();
    }
    emit attachedChanged(false);
}

void ThemeController::registerFonts()
{
    ensureResources();
    static bool registered = false;
    if (registered) return;
    registered = true;
    const QStringList resources =
        internal::tokenStrings(QStringLiteral("desktop.fontResources"));
    for (const QString &resource : resources)
        QFontDatabase::addApplicationFont(resourcePath(resource));
}

QString ThemeController::styleSheet(ColorScheme scheme)
{
    ensureResources();
    QFile file(QStringLiteral(":/ngstd/widgets/qt/nextgis-%1.qss")
                   .arg(schemeName(scheme)));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
    return QString::fromUtf8(file.readAll());
}

QPalette ThemeController::palette(ColorScheme scheme)
{
    QPalette result;
    const QColor background =
        DesignTokens::color(ColorRole::Background, scheme);
    const QColor surface = DesignTokens::color(ColorRole::Surface, scheme);
    const QColor surfaceMuted =
        DesignTokens::color(ColorRole::SurfaceMuted, scheme);
    const QColor border = DesignTokens::color(ColorRole::Border, scheme);
    const QColor borderStrong =
        DesignTokens::color(ColorRole::BorderStrong, scheme);
    const QColor text = DesignTokens::color(ColorRole::Text, scheme);
    const QColor textSecondary =
        DesignTokens::color(ColorRole::TextSecondary, scheme);
    const QColor disabled =
        DesignTokens::color(ColorRole::TextDisabled, scheme);
    const QColor brand = DesignTokens::color(ColorRole::Brand, scheme);
    const QColor white = DesignTokens::color(ColorRole::White, scheme);

    result.setColor(QPalette::Window, background);
    result.setColor(QPalette::WindowText, text);
    result.setColor(QPalette::Base, surface);
    result.setColor(QPalette::AlternateBase, surfaceMuted);
    result.setColor(QPalette::ToolTipBase, text);
    result.setColor(QPalette::ToolTipText, background);
    result.setColor(QPalette::Text, text);
    result.setColor(QPalette::Button, surface);
    result.setColor(QPalette::ButtonText, textSecondary);
    result.setColor(QPalette::Light, surface);
    result.setColor(QPalette::Midlight, border);
    result.setColor(QPalette::Mid, border);
    result.setColor(QPalette::Dark, borderStrong);
    result.setColor(QPalette::Shadow, text);
    result.setColor(QPalette::BrightText,
                    DesignTokens::color(ColorRole::Danger, scheme));
    result.setColor(QPalette::Link,
                    DesignTokens::color(ColorRole::Link, scheme));
    result.setColor(QPalette::LinkVisited,
                    DesignTokens::color(ColorRole::LinkHover, scheme));
    result.setColor(QPalette::Highlight, brand);
    result.setColor(QPalette::HighlightedText, white);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    result.setColor(QPalette::PlaceholderText,
                    DesignTokens::color(ColorRole::TextMuted, scheme));
#endif
    result.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
    result.setColor(QPalette::Disabled, QPalette::Text, disabled);
    result.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
    result.setColor(QPalette::Disabled, QPalette::Base, surfaceMuted);
    result.setColor(QPalette::Disabled, QPalette::Button, surfaceMuted);
    return result;
}

void ThemeController::applyState()
{
    if (!d->attached || !d->manager) return;
    const ColorScheme previousScheme = d->colorScheme;
    switch (d->options.themeMode()) {
    case ThemeMode::Dark:
        d->colorScheme = ColorScheme::Dark;
        break;
    case ThemeMode::Light:
        d->colorScheme = ColorScheme::Light;
        break;
    case ThemeMode::System:
        d->colorScheme = d->manager->systemColorScheme();
        break;
    }

    d->journal.reset(new internal::StateJournal);
    registerFonts();
    const QPalette themePalette = palette(d->colorScheme);
    const QFont themeFont = DesignTokens::font(TypographyRole::QtBody);
    const QString themeStyleSheet = styleSheet(d->colorScheme);

    if (d->applicationWide && d->application) {
        d->journal->captureApplication(d->application.data());
        d->journal->setProperty(d->application.data(),
                                QByteArrayLiteral("_ngstdColorScheme"),
                                schemeName(d->colorScheme));
        d->journal->setProperty(
            d->application.data(), QByteArrayLiteral("_ngstdAnimationPolicy"),
            animationPolicyName(d->options.animationPolicy()));
        d->application->setPalette(themePalette);
        d->application->setFont(themeFont);
        QToolTip::setPalette(themePalette);
        const QVector<QPointer<QWidget>> widgets =
            guardedWidgets(d->application->allWidgets());
        for (const QPointer<QWidget> &widget : widgets) {
            if (widget) decorateWidget(widget.data());
        }
    }
    else if (d->rootWidget) {
        QWidget *rootWidget = d->rootWidget.data();
        d->journal->captureWidget(rootWidget);
        d->journal->setProperty(rootWidget,
                                QByteArrayLiteral("_ngstdThemeScope"), true);
        d->journal->setProperty(rootWidget,
                                QByteArrayLiteral("_ngstdColorScheme"),
                                schemeName(d->colorScheme));
        d->journal->setProperty(
            rootWidget, QByteArrayLiteral("_ngstdAnimationPolicy"),
            animationPolicyName(d->options.animationPolicy()));
        rootWidget->setStyleSheet(
            combinedStyleSheet(rootWidget->styleSheet(), themeStyleSheet));
        rootWidget->setPalette(themePalette);
        rootWidget->setFont(themeFont);
        decorateWidget(rootWidget);
        const QVector<QPointer<QWidget>> widgets =
            guardedWidgets(rootWidget->findChildren<QWidget *>());
        for (const QPointer<QWidget> &widget : widgets) {
            if (widget) decorateWidget(widget.data());
        }
        rootWidget->update();
    }
    if (previousScheme != d->colorScheme)
        emit colorSchemeChanged(d->colorScheme);
}

void ThemeController::decorateWidget(QWidget *widget)
{
    if (!widget || !d->attached || !d->journal || !d->manager ||
        d->manager->controllerFor(widget) != this) {
        return;
    }
    const bool firstDecoration = !d->journal->containsWidget(widget);
    d->journal->captureWidget(widget);
    if (firstDecoration) widget->setPalette(palette(d->colorScheme));
    if (d->applicationWide && widget->isWindow() &&
        !widget->property("_ngstdThemeStyleApplied").toBool()) {
        d->journal->setProperty(
            widget, QByteArrayLiteral("_ngstdThemeStyleApplied"), true);
        widget->setStyleSheet(combinedStyleSheet(
            widget->styleSheet(), styleSheet(d->colorScheme)));
    }
    for (const std::unique_ptr<internal::WidgetDecorator> &decorator :
         d->decorators) {
        if (decorator->supports(widget)) {
            decorator->decorate(widget, d->colorScheme, d->options,
                                d->journal.get());
        }
    }
}

void ThemeController::setKeyboardFocus(QWidget *widget, bool focused)
{
    if (!widget || !d->attached || !d->journal || !d->manager ||
        d->manager->controllerFor(widget) != this) {
        return;
    }
    d->journal->setProperty(widget, QByteArrayLiteral("_ngstdKeyboardFocus"),
                            focused);
    if (focused) {
        QWidget *frameParent = widget->parentWidget();
        if (!frameParent) {
            if (d->focusFrame) {
                d->focusFrame->setWidget(nullptr);
                d->focusFrame->hide();
            }
            return;
        }
        if (!d->focusFrame || d->focusFrame->parentWidget() != frameParent) {
            delete d->focusFrame.data();
            d->focusFrame = new QFocusFrame(frameParent);
            d->focusFrame->setObjectName(
                QStringLiteral("_ngstdKeyboardFocusFrame"));
            d->focusFrame->setProperty("_ngstdRole",
                                       QStringLiteral("focusFrame"));
        }
        d->focusFrame->setWidget(widget);
        d->focusFrame->show();
        d->focusFrame->raise();
    }
    else if (d->focusFrame && d->focusFrame->widget() == widget) {
        d->focusFrame->setWidget(nullptr);
        d->focusFrame->hide();
    }
}

} // namespace widgets
} // namespace ngstd
