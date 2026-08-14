/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Resource-backed NextGIS icons and logos
 *****************************************************************************/
#include <ngstd/widgets/icons.h>

#include "resources_p.h"

#include <QFile>
#include <QPainter>
#include <QPixmapCache>
#include <QRectF>
#include <QSvgRenderer>

namespace {

QPixmap renderSvg(const QString &path, const QSize &logicalSize,
                  qreal devicePixelRatio, const QColor &color)
{
    ngstd::widgets::ensureResources();
    if (logicalSize.isEmpty() || devicePixelRatio <= 0.0) return QPixmap();
    if (!QFile::exists(path)) return QPixmap();

    const QString cacheKey = QStringLiteral("ngstd:%1:%2x%3:%4:%5")
                                 .arg(path)
                                 .arg(logicalSize.width())
                                 .arg(logicalSize.height())
                                 .arg(devicePixelRatio, 0, 'g', 8)
                                 .arg(color.isValid() ? color.rgba() : 0U);
    QPixmap cachedPixmap;
    if (QPixmapCache::find(cacheKey, &cachedPixmap)) return cachedPixmap;

    QSvgRenderer renderer(path);
    if (!renderer.isValid()) return QPixmap();
    renderer.setAspectRatioMode(Qt::KeepAspectRatio);

    const QSize pixelSize(qRound(logicalSize.width() * devicePixelRatio),
                          qRound(logicalSize.height() * devicePixelRatio));
    QPixmap pixmap(pixelSize);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    renderer.render(&painter, QRectF(QPointF(0.0, 0.0), QSizeF(pixelSize)));
    if (color.isValid()) {
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(QRect(QPoint(0, 0), pixelSize), color);
    }
    painter.end();

    pixmap.setDevicePixelRatio(devicePixelRatio);
    QPixmapCache::insert(cacheKey, pixmap);
    return pixmap;
}

} // namespace

namespace ngstd {
namespace widgets {

QString iconResourcePath(IconRole role)
{
    switch (role) {
    case IconRole::AlertTriangle:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/lucide/alert-triangle.svg");
    case IconRole::Blocks:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/lucide/blocks.svg");
    case IconRole::Check:
        return QStringLiteral(":/ngstd/widgets/assets/icons/lucide/check.svg");
    case IconRole::CheckCircle:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/lucide/check-circle.svg");
    case IconRole::ChevronDown:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/lucide/chevron-down.svg");
    case IconRole::ChevronUp:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/lucide/chevron-up.svg");
    case IconRole::ChevronLeft:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/lucide/chevron-left.svg");
    case IconRole::ChevronRight:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/lucide/chevron-right.svg");
    case IconRole::CloseCircle:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/lucide/x-circle.svg");
    case IconRole::Copy:
        return QStringLiteral(":/ngstd/widgets/assets/icons/lucide/copy.svg");
    case IconRole::Download:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/lucide/download.svg");
    case IconRole::Ellipsis:
        return QStringLiteral(":/ngstd/widgets/assets/icons/ant/ellipsis.svg");
    case IconRole::Flame:
        return QStringLiteral(":/ngstd/widgets/assets/icons/lucide/flame.svg");
    case IconRole::Folder:
        return QStringLiteral(":/ngstd/widgets/assets/icons/lucide/folder.svg");
    case IconRole::FolderClock:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/lucide/folder-clock.svg");
    case IconRole::FolderInput:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/lucide/folder-input.svg");
    case IconRole::Gradient:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/lucide/gradient.svg");
    case IconRole::Grid:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/mdi/dots-grid.svg");
    case IconRole::HardDrive:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/lucide/hard-drive.svg");
    case IconRole::Help:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/lucide/circle-help.svg");
    case IconRole::Information:
        return QStringLiteral(":/ngstd/widgets/assets/icons/lucide/info.svg");
    case IconRole::Layers:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/lucide/layers.svg");
    case IconRole::Menu:
        return QStringLiteral(":/ngstd/widgets/assets/icons/mdi/menu.svg");
    case IconRole::Monitor:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/lucide/monitor.svg");
    case IconRole::MonitorCog:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/lucide/monitor-cog.svg");
    case IconRole::Moon:
        return QStringLiteral(":/ngstd/widgets/assets/icons/lucide/moon.svg");
    case IconRole::Package:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/lucide/package.svg");
    case IconRole::Search:
        return QStringLiteral(":/ngstd/widgets/assets/icons/ant/search.svg");
    case IconRole::Settings:
        return QStringLiteral(
            ":/ngstd/widgets/assets/icons/lucide/settings.svg");
    case IconRole::Sun:
        return QStringLiteral(":/ngstd/widgets/assets/icons/lucide/sun.svg");
    case IconRole::Undo:
        return QStringLiteral(":/ngstd/widgets/assets/icons/lucide/undo.svg");
    case IconRole::User:
        return QStringLiteral(":/ngstd/widgets/assets/icons/lucide/user.svg");
    case IconRole::Users:
        return QStringLiteral(":/ngstd/widgets/assets/icons/lucide/users.svg");
    }
    return QString();
}

QIcon icon(IconRole role)
{
    return QIcon(iconResourcePath(role));
}

QPixmap iconPixmap(IconRole role, const QSize &logicalSize,
                   qreal devicePixelRatio, const QColor &color)
{
    return renderSvg(iconResourcePath(role), logicalSize, devicePixelRatio,
                     color);
}

QString logoResourcePath(LogoRole role)
{
    switch (role) {
    case LogoRole::Data:
        return QStringLiteral(
            ":/ngstd/widgets/assets/logos/nextgis-data.svg");
    case LogoRole::Horizontal:
        return QStringLiteral(
            ":/ngstd/widgets/assets/logos/nextgis-horizontal.svg");
    case LogoRole::HorizontalOnDark:
        return QStringLiteral(
            ":/ngstd/widgets/assets/logos/nextgis-horizontal-on-dark.svg");
    case LogoRole::Symbol:
        return QStringLiteral(
            ":/ngstd/widgets/assets/logos/nextgis-symbol.svg");
    case LogoRole::SymbolOnDark:
        return QStringLiteral(
            ":/ngstd/widgets/assets/logos/nextgis-symbol-on-dark.svg");
    case LogoRole::MonoBrand:
        return QStringLiteral(
            ":/ngstd/widgets/assets/logos/nextgis-mono-brand.svg");
    case LogoRole::MonoDark:
        return QStringLiteral(
            ":/ngstd/widgets/assets/logos/nextgis-mono-dark.svg");
    case LogoRole::MonoLight:
        return QStringLiteral(
            ":/ngstd/widgets/assets/logos/nextgis-mono-light.svg");
    }
    return QString();
}

QPixmap logoPixmap(LogoRole role, const QSize &logicalSize,
                   qreal devicePixelRatio)
{
    return renderSvg(logoResourcePath(role), logicalSize, devicePixelRatio,
                     QColor());
}

} // namespace widgets
} // namespace ngstd
