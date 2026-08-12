/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Reusable animated theme switch
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_THEME_SWITCH_H
#define NGSTD_WIDGETS_THEME_SWITCH_H

#include <ngstd/widgets/types.h>

#include <QFrame>
#include <QScopedPointer>

class QPaintEvent;
class QToolButton;

namespace ngstd {
namespace widgets {

class ThemeSwitchPrivate;

class NGSTD_WIDGETS_EXPORT ThemeSwitch : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(
        ngstd::widgets::ThemeMode themeMode
        READ themeMode
        WRITE setThemeMode
        RESET resetThemeMode
        NOTIFY themeModeChanged)
    Q_PROPERTY(
        ngstd::widgets::ColorScheme colorScheme
        READ colorScheme
        WRITE setColorScheme
        RESET resetColorScheme
        NOTIFY colorSchemeChanged)

public:
    explicit ThemeSwitch(QWidget *parent = nullptr);
    ~ThemeSwitch() override;

    ThemeMode themeMode() const;
    void setThemeMode(ThemeMode mode);
    void resetThemeMode();

    ColorScheme colorScheme() const;
    void setColorScheme(ColorScheme scheme);
    void resetColorScheme();

    QToolButton *button(ThemeMode mode) const;

signals:
    void themeModeChanged(ngstd::widgets::ThemeMode mode);
    void colorSchemeChanged(ngstd::widgets::ColorScheme scheme);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    NGSTD_WIDGETS_LOCAL static int indexForMode(ThemeMode mode);
    NGSTD_WIDGETS_LOCAL static ThemeMode modeForIndex(int index);
    NGSTD_WIDGETS_LOCAL qreal selectionOpacity(int index) const;
    NGSTD_WIDGETS_LOCAL void updateIcons();

    Q_DISABLE_COPY(ThemeSwitch)
    QScopedPointer<ThemeSwitchPrivate> d;
};

} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_THEME_SWITCH_H
