/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Resource-backed NextGIS icons and logos
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_ICONS_H
#define NGSTD_WIDGETS_ICONS_H

#include <ngstd/widgets/types.h>

#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QSize>
#include <QString>

namespace ngstd {
namespace widgets {

NGSTD_WIDGETS_EXPORT QString iconResourcePath(IconRole role);
NGSTD_WIDGETS_EXPORT QIcon icon(IconRole role);
NGSTD_WIDGETS_EXPORT QPixmap
iconPixmap(IconRole role, const QSize &logicalSize = QSize(18, 18),
           qreal devicePixelRatio = 1.0, const QColor &color = QColor());

NGSTD_WIDGETS_EXPORT QString logoResourcePath(LogoRole role);
NGSTD_WIDGETS_EXPORT QPixmap logoPixmap(LogoRole role,
                                        const QSize &logicalSize,
                                        qreal devicePixelRatio = 1.0);

} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_ICONS_H
