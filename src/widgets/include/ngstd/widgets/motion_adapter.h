/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Qt animation adapter for corporate motion specifications
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_MOTION_ADAPTER_H
#define NGSTD_WIDGETS_MOTION_ADAPTER_H

#include <ngstd/widgets/types.h>

class QVariantAnimation;
class QWidget;

namespace ngstd {
namespace widgets {

class NGSTD_WIDGETS_EXPORT MotionAdapter final
{
public:
    static bool configure(QVariantAnimation *animation,
                          const QWidget *context, MotionSpec motion);
};

} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_MOTION_ADAPTER_H
