/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Per-application theme scope manager
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_THEME_MANAGER_P_H
#define NGSTD_WIDGETS_THEME_MANAGER_P_H

#include <ngstd/widgets/types.h>

#include <QObject>
#include <QPointer>
#include <QVector>

class QApplication;
class QEvent;
class QWidget;

namespace ngstd {
namespace widgets {

class ThemeController;
class ThemeOptions;

namespace internal {

class ThemeManager final : public QObject
{
    Q_OBJECT

public:
    static ThemeManager *instance(QApplication *application);

    ThemeController *attach(QWidget *rootWidget, const ThemeOptions &options);
    ThemeController *applyToApplication(const ThemeOptions &options);

    void registerController(ThemeController *controller);
    void unregisterController(ThemeController *controller);
    void apply(ThemeController *controller);
    void reapplyAll();

    ThemeController *controllerFor(const QWidget *widget) const;
    ColorScheme systemColorScheme() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    explicit ThemeManager(QApplication *application);

    void decorate(QWidget *widget);
    void updateSystemTheme();
    void reapplySystemControllers();
    bool applicationPaletteIsManaged() const;

    QPointer<QApplication> m_application;
    QVector<QPointer<ThemeController>> m_controllers;
    ColorScheme m_systemColorScheme = ColorScheme::Light;
    bool m_applying = false;

    friend class ngstd::widgets::ThemeController;
};

} // namespace internal
} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_THEME_MANAGER_P_H
