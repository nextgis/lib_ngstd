/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Implicitly shared theme configuration
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_THEME_OPTIONS_H
#define NGSTD_WIDGETS_THEME_OPTIONS_H

#include <ngstd/widgets/types.h>

#include <QSharedDataPointer>

namespace ngstd {
namespace widgets {

class ThemeOptionsData;

class NGSTD_WIDGETS_EXPORT ThemeOptions final
{
public:
    ThemeOptions();
    ThemeOptions(const ThemeOptions &other);
    ThemeOptions(ThemeOptions &&other) noexcept;
    ~ThemeOptions();

    ThemeOptions &operator=(const ThemeOptions &other);
    ThemeOptions &operator=(ThemeOptions &&other) noexcept;

    ThemeMode themeMode() const;
    void setThemeMode(ThemeMode mode);

    AnimationPolicy animationPolicy() const;
    void setAnimationPolicy(AnimationPolicy policy);

    ThemeFeatures features() const;
    void setFeatures(ThemeFeatures features);
    void setFeatureEnabled(ThemeFeature feature, bool enabled = true);
    bool isFeatureEnabled(ThemeFeature feature) const;

    friend bool operator==(const ThemeOptions &left,
                           const ThemeOptions &right) noexcept;
    friend bool operator!=(const ThemeOptions &left,
                           const ThemeOptions &right) noexcept;

private:
    QSharedDataPointer<ThemeOptionsData> d;
};

NGSTD_WIDGETS_EXPORT bool operator==(const ThemeOptions &left,
                                     const ThemeOptions &right) noexcept;
NGSTD_WIDGETS_EXPORT bool operator!=(const ThemeOptions &left,
                                     const ThemeOptions &right) noexcept;

} // namespace widgets
} // namespace ngstd

Q_DECLARE_METATYPE(ngstd::widgets::ThemeOptions)

#endif // NGSTD_WIDGETS_THEME_OPTIONS_H
