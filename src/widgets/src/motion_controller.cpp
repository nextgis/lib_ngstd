/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Shared reduced-motion policy for component animations
 *****************************************************************************/
#include "motion_controller_p.h"

#include <ngstd/widgets/motion_adapter.h>

namespace ngstd {
namespace widgets {
namespace internal {

bool MotionController::configure(QVariantAnimation *animation,
                                 const QWidget *context, MotionSpec motion)
{
    return MotionAdapter::configure(animation, context, motion);
}

} // namespace internal
} // namespace widgets
} // namespace ngstd
