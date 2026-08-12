/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Library version declarations
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_VERSION_H
#define NGSTD_WIDGETS_VERSION_H

#include <ngstd/widgets/widgets.h>

#define NGSTD_WIDGETS_VERSION_MAJOR 1
#define NGSTD_WIDGETS_VERSION_MINOR 0
#define NGSTD_WIDGETS_VERSION_PATCH 0
#define NGSTD_WIDGETS_VERSION_STR "1.0.0"

namespace ngstd {
namespace widgets {

NGSTD_WIDGETS_EXPORT const char *versionString() noexcept;

} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_VERSION_H
