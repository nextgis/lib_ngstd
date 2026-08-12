/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Implicitly shared theme configuration
 *****************************************************************************/
#include <ngstd/widgets/theme_options.h>

#include <QSharedData>

namespace ngstd {
namespace widgets {

class ThemeOptionsData final : public QSharedData
{
public:
    ThemeMode themeMode = ThemeMode::System;
    AnimationPolicy animationPolicy = AnimationPolicy::System;
    ThemeFeatures features = ThemeFeature::StandardControls |
                             ThemeFeature::SemanticComponents |
                             ThemeFeature::DialogButtonBoxes;
};

ThemeOptions::ThemeOptions() : d(new ThemeOptionsData) {}

ThemeOptions::ThemeOptions(const ThemeOptions &other) = default;
ThemeOptions::ThemeOptions(ThemeOptions &&other) noexcept = default;
ThemeOptions::~ThemeOptions() = default;
ThemeOptions &ThemeOptions::operator=(const ThemeOptions &other) = default;
ThemeOptions &ThemeOptions::operator=(ThemeOptions &&other) noexcept = default;

ThemeMode ThemeOptions::themeMode() const
{
    return d->themeMode;
}

void ThemeOptions::setThemeMode(ThemeMode mode)
{
    d.detach();
    d->themeMode = mode;
}

AnimationPolicy ThemeOptions::animationPolicy() const
{
    return d->animationPolicy;
}

void ThemeOptions::setAnimationPolicy(AnimationPolicy policy)
{
    d.detach();
    d->animationPolicy = policy;
}

ThemeFeatures ThemeOptions::features() const
{
    return d->features;
}

void ThemeOptions::setFeatures(ThemeFeatures features)
{
    d.detach();
    d->features = features;
}

void ThemeOptions::setFeatureEnabled(ThemeFeature feature, bool enabled)
{
    d.detach();
    d->features.setFlag(feature, enabled);
}

bool ThemeOptions::isFeatureEnabled(ThemeFeature feature) const
{
    return d->features.testFlag(feature);
}

bool operator==(const ThemeOptions &left, const ThemeOptions &right) noexcept
{
    return left.d->themeMode == right.d->themeMode &&
           left.d->animationPolicy == right.d->animationPolicy &&
           left.d->features == right.d->features;
}

bool operator!=(const ThemeOptions &left, const ThemeOptions &right) noexcept
{
    return !(left == right);
}

} // namespace widgets
} // namespace ngstd
