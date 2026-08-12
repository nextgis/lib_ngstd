#include <ngstd/widgets/components.h>
#include <ngstd/widgets/theme_options.h>
#include <ngstd/widgets/version.h>

#include <QApplication>

#include <cstring>

int main(int argumentCount, char *argumentValues[])
{
    QApplication application(argumentCount, argumentValues);
    ngstd::widgets::ThemeOptions options;
    ngstd::widgets::ExpandableSection section;
    ngstd::widgets::PageBackground background;
    background.setCornerMode(
        ngstd::widgets::PageBackgroundCornerMode::RoundedTop);
    const bool defaultsAreValid =
        options.themeMode() == ngstd::widgets::ThemeMode::System &&
        options.isFeatureEnabled(
            ngstd::widgets::ThemeFeature::StandardControls) &&
        !section.isExpanded() &&
        background.cornerMode() ==
            ngstd::widgets::PageBackgroundCornerMode::RoundedTop;
    return defaultsAreValid && std::strcmp(ngstd::widgets::versionString(),
                                           NGSTD_WIDGETS_VERSION_STR) == 0
               ? 0
               : 1;
}
