/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Scoped and application-wide NextGIS theme controller
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_THEME_H
#define NGSTD_WIDGETS_THEME_H

#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/theme_options.h>

#include <QObject>
#include <QScopedPointer>

class QApplication;
class QWidget;

namespace ngstd {
namespace widgets {

namespace internal {
class ThemeManager;
}

class ThemeControllerPrivate;

class NGSTD_WIDGETS_EXPORT ThemeController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(
        ngstd::widgets::ThemeOptions options
        READ options
        WRITE setOptions
        RESET resetOptions
        NOTIFY optionsChanged)
    Q_PROPERTY(
        ngstd::widgets::ThemeMode themeMode
        READ themeMode
        WRITE setThemeMode
        RESET resetThemeMode
        NOTIFY themeModeChanged)
    Q_PROPERTY(
        ngstd::widgets::ColorScheme colorScheme
        READ colorScheme
        NOTIFY colorSchemeChanged)
    Q_PROPERTY(bool attached READ isAttached NOTIFY attachedChanged)

public:
    static ThemeController *attach(
        QWidget *rootWidget, const ThemeOptions &options = ThemeOptions());
    static ThemeController *applyToApplication(
        QApplication *application,
        const ThemeOptions &options = ThemeOptions());

    ~ThemeController() override;

    ThemeOptions options() const;
    void setOptions(const ThemeOptions &options);
    void resetOptions();

    ThemeMode themeMode() const;
    void setThemeMode(ThemeMode mode);
    void resetThemeMode();

    ColorScheme colorScheme() const;
    QWidget *rootWidget() const;
    bool isApplicationWide() const;
    bool isAttached() const;

    void detach();

    static void registerFonts();
    static QString styleSheet(ColorScheme scheme);
    static QPalette palette(ColorScheme scheme);

signals:
    void optionsChanged(const ngstd::widgets::ThemeOptions &options);
    void themeModeChanged(ngstd::widgets::ThemeMode mode);
    void colorSchemeChanged(ngstd::widgets::ColorScheme scheme);
    void attachedChanged(bool attached);

private:
    NGSTD_WIDGETS_LOCAL explicit ThemeController(
        QWidget *rootWidget, const ThemeOptions &options,
        internal::ThemeManager *manager);
    NGSTD_WIDGETS_LOCAL explicit ThemeController(
        QApplication *application, const ThemeOptions &options,
        internal::ThemeManager *manager);

    NGSTD_WIDGETS_LOCAL void applyState();
    NGSTD_WIDGETS_LOCAL void decorateWidget(QWidget *widget);
    NGSTD_WIDGETS_LOCAL void setKeyboardFocus(QWidget *widget, bool focused);
    NGSTD_WIDGETS_LOCAL void clearKeyboardFocus();

    Q_DISABLE_COPY(ThemeController)
    QScopedPointer<ThemeControllerPrivate> d;

    friend class internal::ThemeManager;
};

} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_THEME_H
