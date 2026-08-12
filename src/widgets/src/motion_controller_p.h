/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Shared reduced-motion policy for component animations
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_MOTION_CONTROLLER_P_H
#define NGSTD_WIDGETS_MOTION_CONTROLLER_P_H

#include <ngstd/widgets/types.h>

class QVariantAnimation;
class QWidget;

namespace ngstd {
namespace widgets {
namespace internal {

class MotionController final
{
public:
    static bool configure(QVariantAnimation *animation, const QWidget *context,
                          MotionSpec motion);
};

} // namespace internal
} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_MOTION_CONTROLLER_P_H
