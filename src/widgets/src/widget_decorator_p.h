/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Theme widget decorator strategies
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_WIDGET_DECORATOR_P_H
#define NGSTD_WIDGETS_WIDGET_DECORATOR_P_H

#include <ngstd/widgets/theme_options.h>

#include <memory>
#include <vector>

class QWidget;

namespace ngstd {
namespace widgets {
namespace internal {

class StateJournal;

class WidgetDecorator
{
public:
    virtual ~WidgetDecorator() = default;

    virtual bool supports(QWidget *widget) const = 0;
    virtual void decorate(QWidget *widget, ColorScheme scheme,
                          const ThemeOptions &options,
                          StateJournal *journal) const = 0;
};

std::vector<std::unique_ptr<WidgetDecorator>> createWidgetDecorators();

} // namespace internal
} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_WIDGET_DECORATOR_P_H
