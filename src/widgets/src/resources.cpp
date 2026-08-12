/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Internal resource initialization
 *****************************************************************************/
#include "resources_p.h"

#include <QResource>

void initializeResourceCollection()
{
    Q_INIT_RESOURCE(widgets);
}

namespace ngstd {
namespace widgets {

void ensureResources()
{
    static const bool initialized = []() {
        initializeResourceCollection();
        return true;
    }();
    Q_UNUSED(initialized)
}

} // namespace widgets
} // namespace ngstd
