/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Library version implementation
 *****************************************************************************/
#include <ngstd/widgets/version.h>

namespace ngstd {
namespace widgets {

const char *versionString() noexcept
{
    return NGSTD_WIDGETS_VERSION_STR;
}

} // namespace widgets
} // namespace ngstd
