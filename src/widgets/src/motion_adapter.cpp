/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Qt animation adapter for corporate motion specifications
 *****************************************************************************/
#include <ngstd/widgets/motion_adapter.h>

#include "component_utils_p.h"

#include <ngstd/widgets/design_tokens.h>

#include <QVariantAnimation>

namespace ngstd {
namespace widgets {

bool MotionAdapter::configure(QVariantAnimation *animation,
                              const QWidget *context, MotionSpec motion)
{
    if (!animation) return false;
    const int duration = internal::animationsEnabled(context)
                             ? DesignTokens::duration(motion.duration)
                             : 0;
    animation->setDuration(duration);
    animation->setEasingCurve(DesignTokens::easingCurve(motion.easing));
    return duration > 0;
}

} // namespace widgets
} // namespace ngstd
