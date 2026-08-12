/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Reusable product page backgrounds
 *****************************************************************************/
#include <ngstd/widgets/page_background.h>

#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/widget_style.h>

#include "component_utils_p.h"
#include "design_tokens_p.h"

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QVariant>
#include <QtMath>

#include <memory>

namespace {

using ngstd::widgets::ColorRole;
using ngstd::widgets::ColorScheme;
using ngstd::widgets::DesignTokens;
using ngstd::widgets::PageBackgroundGradient;
using ngstd::widgets::PageBackgroundCornerMode;
using ngstd::widgets::PageBackgroundVariant;

struct BackgroundRenderOptions
{
    QRectF target;
    ColorScheme scheme = ColorScheme::Light;
    PageBackgroundGradient gradient = PageBackgroundGradient::Down;
    bool decorationVisible = true;
    bool gridVisible = true;
};

QString backgroundTokenPath(const QString &name)
{
    return QStringLiteral("desktop.component.pageBackground.%1").arg(name);
}

int backgroundInteger(const QString &name)
{
    return ngstd::widgets::internal::tokenInteger(backgroundTokenPath(name));
}

qreal backgroundNumber(const QString &name)
{
    return ngstd::widgets::internal::tokenNumber(backgroundTokenPath(name));
}

QString backgroundResource(const QString &name)
{
    return QStringLiteral(":/ngstd/widgets/") +
           ngstd::widgets::internal::tokenString(backgroundTokenPath(name));
}

qreal darkOpacity(ColorScheme scheme, const QString &tokenName,
                  qreal lightOpacity = 1.0)
{
    return scheme == ColorScheme::Dark ? backgroundNumber(tokenName)
                                       : lightOpacity;
}

void drawGrid(QPainter *painter, const QRectF &target, int step,
              const QPen &pen)
{
    if (step <= 0) return;
    painter->save();
    painter->setPen(pen);
    const int columnCount = qCeil(target.width() / step);
    const int rowCount = qCeil(target.height() / step);
    for (int i = 0; i <= columnCount; ++i) {
        const qreal x = target.left() + i * step;
        painter->drawLine(QPointF(x, target.top()),
                          QPointF(x, target.bottom()));
    }
    for (int j = 0; j <= rowCount; ++j) {
        const qreal y = target.top() + j * step;
        painter->drawLine(QPointF(target.left(), y),
                          QPointF(target.right(), y));
    }
    painter->restore();
}

class PageBackgroundRenderer
{
public:
    virtual ~PageBackgroundRenderer() = default;
    virtual void paint(QPainter *painter,
                       const BackgroundRenderOptions &options) const = 0;
};

class MainBackgroundRenderer final : public PageBackgroundRenderer
{
public:
    void paint(QPainter *painter,
               const BackgroundRenderOptions &options) const override
    {
        if (options.gridVisible) {
            drawGrid(
                painter, options.target,
                backgroundInteger(QStringLiteral("mainGridStepPx")),
                QPen(DesignTokens::color(ColorRole::PageSoft, options.scheme),
                     backgroundNumber(QStringLiteral("mainGridWidthPx"))));
        }
        if (!options.decorationVisible) return;

        const qreal opacity = darkOpacity(
            options.scheme, QStringLiteral("mainDecorationOpacityDark"));
        const QPixmap roads(
            backgroundResource(QStringLiteral("qtRoadsAsset")));
        if (!roads.isNull()) {
            const qreal width =
                backgroundNumber(QStringLiteral("mainRoadsWidthPx"));
            const qreal right =
                backgroundNumber(QStringLiteral("mainRoadsRightPx"));
            painter->save();
            painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
            painter->setOpacity(opacity);
            painter->drawPixmap(
                QRectF(options.target.right() - width - right,
                       backgroundNumber(QStringLiteral("mainRoadsTopPx")),
                       width,
                       backgroundNumber(QStringLiteral("mainRoadsHeightPx"))),
                roads, QRectF(roads.rect()));
            painter->restore();
        }
        ngstd::widgets::internal::renderSvg(
            painter,
            QStringLiteral(":/ngstd/widgets/assets/backgrounds/"
                           "for-mainpage-isolines.svg"),
            QRectF(backgroundNumber(QStringLiteral("mainIsolinesLeftPx")),
                   backgroundNumber(QStringLiteral("mainIsolinesTopPx")),
                   backgroundNumber(QStringLiteral("mainIsolinesWidthPx")),
                   backgroundNumber(QStringLiteral("mainIsolinesHeightPx"))),
            opacity, QColor(), Qt::KeepAspectRatio);
    }
};

class ToolboxBackgroundRenderer final : public PageBackgroundRenderer
{
public:
    void paint(QPainter *painter,
               const BackgroundRenderOptions &options) const override
    {
        if (options.decorationVisible) {
            ngstd::widgets::internal::renderSvg(
                painter, backgroundResource(QStringLiteral("toolboxAsset")),
                options.target,
                darkOpacity(options.scheme,
                            QStringLiteral("toolboxDecorationOpacityDark")),
                QColor(), Qt::KeepAspectRatioByExpanding);
        }
        if (options.gradient == PageBackgroundGradient::None) return;
        const QString schemeName = options.scheme == ColorScheme::Dark
                                       ? QStringLiteral("dark")
                                       : QStringLiteral("light");
        const QString colorPath =
            QStringLiteral("product.toolbox.%1.").arg(schemeName);
        const QColor strong = ngstd::widgets::internal::cssColor(
            ngstd::widgets::internal::tokenString(
                colorPath + QStringLiteral("gradientStrong")));
        const QColor clear = ngstd::widgets::internal::cssColor(
            ngstd::widgets::internal::tokenString(
                colorPath + QStringLiteral("gradientClear")));
        painter->fillRect(
            options.target,
            ngstd::widgets::internal::backgroundGradient(
                options.gradient, options.target, strong, clear));
    }
};

class DataBackgroundRenderer final : public PageBackgroundRenderer
{
public:
    void paint(QPainter *painter,
               const BackgroundRenderOptions &options) const override
    {
        if (!options.decorationVisible) return;
        const qreal routeSize =
            backgroundNumber(QStringLiteral("dataRouteSizePx"));
        ngstd::widgets::internal::renderSvg(
            painter, backgroundResource(QStringLiteral("dataRouteAsset")),
            QRectF(options.target.right() - routeSize -
                       backgroundNumber(QStringLiteral("dataRouteRightPx")),
                   backgroundNumber(QStringLiteral("dataRouteTopPx")),
                   routeSize, routeSize),
            1.0, QColor(), Qt::KeepAspectRatio);

        const QPixmap collage(
            backgroundResource(QStringLiteral("dataCollageAsset")));
        if (collage.isNull()) return;
        const qreal collageWidth = qMin(
            options.target.width() *
                backgroundNumber(QStringLiteral("dataCollageWidthRatio")),
            backgroundNumber(QStringLiteral("dataCollageMaximumWidthPx")));
        const qreal collageHeight = collageWidth *
                                    static_cast<qreal>(collage.height()) /
                                    static_cast<qreal>(collage.width());
        painter->save();
        if (options.scheme == ColorScheme::Dark) {
            painter->setOpacity(
                backgroundNumber(QStringLiteral("dataCollageOpacityDark")));
        }
        painter->drawPixmap(
            QRectF(options.target.right() - collageWidth +
                       options.target.width() *
                           backgroundNumber(QStringLiteral(
                               "dataCollageHorizontalOffsetRatio")),
                   options.target.center().y() - collageHeight * 0.5,
                   collageWidth, collageHeight),
            collage, QRectF(collage.rect()));
        painter->restore();
    }
};

class WorkspaceBackgroundRenderer final : public PageBackgroundRenderer
{
public:
    void paint(QPainter *painter,
               const BackgroundRenderOptions &options) const override
    {
        if (!options.gridVisible) return;
        QColor gridColor =
            DesignTokens::color(ColorRole::Border, options.scheme);
        gridColor.setAlphaF(static_cast<float>(backgroundNumber(
            options.scheme == ColorScheme::Dark
                ? QStringLiteral("workspaceGridOpacityDark")
                : QStringLiteral("workspaceGridOpacityLight"))));
        drawGrid(painter, options.target,
                 backgroundInteger(QStringLiteral("workspaceGridStepPx")),
                 QPen(gridColor, backgroundNumber(
                                     QStringLiteral("workspaceGridWidthPx"))));
    }
};

class CorporateBackgroundRenderer final : public PageBackgroundRenderer
{
public:
    void paint(QPainter *painter,
               const BackgroundRenderOptions &options) const override
    {
        if (options.gradient != PageBackgroundGradient::None) {
            const QColor strong(ngstd::widgets::internal::tokenString(
                QStringLiteral("product.corporate.gradientStrong")));
            const QColor clear(ngstd::widgets::internal::tokenString(
                QStringLiteral("product.corporate.gradientClear")));
            painter->fillRect(
                options.target,
                ngstd::widgets::internal::backgroundGradient(
                    options.gradient, options.target, strong, clear));
        }
        if (!options.gridVisible) return;
        QColor gridColor =
            DesignTokens::color(ColorRole::CorporateGrid, options.scheme);
        QBrush gridBrush(gridColor);
        if (options.gradient != PageBackgroundGradient::None) {
            QColor transparent = gridColor;
            transparent.setAlpha(0);
            gridBrush = ngstd::widgets::internal::backgroundGradient(
                options.gradient, options.target, gridColor, transparent);
        }
        drawGrid(painter, options.target,
                 backgroundInteger(QStringLiteral("corporateGridStepPx")),
                 QPen(gridBrush, backgroundNumber(
                                     QStringLiteral("corporateGridWidthPx"))));
    }
};

class FieldworkBackgroundRenderer final : public PageBackgroundRenderer
{
public:
    void paint(QPainter *painter,
               const BackgroundRenderOptions &options) const override
    {
        if (options.decorationVisible) {
            const QPixmap image(
                backgroundResource(QStringLiteral("fieldworkAsset")));
            if (!image.isNull()) {
                painter->drawPixmap(ngstd::widgets::internal::coverRect(
                                        image.size(), options.target),
                                    image, QRectF(image.rect()));
            }
        }
        painter->fillRect(options.target,
                          DesignTokens::color(ColorRole::FieldworkSurfaceHover,
                                              options.scheme));
    }
};

std::unique_ptr<PageBackgroundRenderer> createBackgroundRenderer(
    PageBackgroundVariant variant)
{
    switch (variant) {
    case PageBackgroundVariant::Main:
        return std::make_unique<MainBackgroundRenderer>();
    case PageBackgroundVariant::Toolbox:
        return std::make_unique<ToolboxBackgroundRenderer>();
    case PageBackgroundVariant::Data:
        return std::make_unique<DataBackgroundRenderer>();
    case PageBackgroundVariant::Workspace:
        return std::make_unique<WorkspaceBackgroundRenderer>();
    case PageBackgroundVariant::Corporate:
        return std::make_unique<CorporateBackgroundRenderer>();
    case PageBackgroundVariant::Fieldwork:
        return std::make_unique<FieldworkBackgroundRenderer>();
    }
    return std::make_unique<MainBackgroundRenderer>();
}

struct PageBackgroundDefaults
{
    bool decorationVisible;
    bool gridVisible;
    PageBackgroundGradient gradient;
};

PageBackgroundDefaults defaultsForVariant(PageBackgroundVariant variant)
{
    switch (variant) {
    case PageBackgroundVariant::Main:
        return {true, true, PageBackgroundGradient::Down};
    case PageBackgroundVariant::Toolbox:
        return {true, false, PageBackgroundGradient::Down};
    case PageBackgroundVariant::Data:
        return {true, false, PageBackgroundGradient::None};
    case PageBackgroundVariant::Workspace:
        return {false, false, PageBackgroundGradient::None};
    case PageBackgroundVariant::Corporate:
        return {false, true, PageBackgroundGradient::Center};
    case PageBackgroundVariant::Fieldwork:
        return {true, false, PageBackgroundGradient::None};
    }
    return {true, true, PageBackgroundGradient::Down};
}

QString cornerModeName(PageBackgroundCornerMode cornerMode)
{
    switch (cornerMode) {
    case PageBackgroundCornerMode::Square:
        return QStringLiteral("square");
    case PageBackgroundCornerMode::Rounded:
        return QStringLiteral("rounded");
    case PageBackgroundCornerMode::RoundedTop:
        return QStringLiteral("roundedTop");
    }
    return QStringLiteral("square");
}

} // namespace

namespace ngstd {
namespace widgets {

class PageBackgroundPrivate final
{
public:
    PageBackgroundVariant variant = PageBackgroundVariant::Main;
    bool decorationVisible = true;
    bool gridVisible = true;
    PageBackgroundGradient gradient = PageBackgroundGradient::Down;
    PageBackgroundCornerMode cornerMode = PageBackgroundCornerMode::Square;
    std::unique_ptr<PageBackgroundRenderer> renderer =
        createBackgroundRenderer(variant);
};

PageBackground::PageBackground(QWidget *parent)
    : QFrame(parent), d(new PageBackgroundPrivate)
{
    setProperty("_ngstdRole", QStringLiteral("pageBackground"));
    setProperty("_ngstdPaintedBackground", true);
    setFrameShape(QFrame::NoFrame);
    WidgetStyle::setPageBackgroundVariant(this, d->variant);
    setProperty("_ngstdGradient", internal::gradientName(d->gradient));
    setProperty("_ngstdCornerMode", cornerModeName(d->cornerMode));
}

PageBackground::~PageBackground() = default;

PageBackgroundVariant PageBackground::variant() const
{
    return d->variant;
}

void PageBackground::setVariant(PageBackgroundVariant variant)
{
    if (d->variant == variant) return;
    d->variant = variant;
    d->renderer = createBackgroundRenderer(d->variant);
    WidgetStyle::setPageBackgroundVariant(this, d->variant);
    const PageBackgroundDefaults defaults = defaultsForVariant(d->variant);
    setDecorationVisible(defaults.decorationVisible);
    setGridVisible(defaults.gridVisible);
    setGradient(defaults.gradient);
    update();
    emit variantChanged(d->variant);
}

void PageBackground::resetVariant()
{
    if (d->variant == PageBackgroundVariant::Main) {
        resetDecorationVisible();
        resetGridVisible();
        resetGradient();
        return;
    }
    setVariant(PageBackgroundVariant::Main);
}

bool PageBackground::isDecorationVisible() const
{
    return d->decorationVisible;
}

void PageBackground::setDecorationVisible(bool visible)
{
    if (d->decorationVisible == visible) return;
    d->decorationVisible = visible;
    update();
    emit decorationVisibleChanged(visible);
}

void PageBackground::resetDecorationVisible()
{
    setDecorationVisible(defaultsForVariant(d->variant).decorationVisible);
}

bool PageBackground::isGridVisible() const
{
    return d->gridVisible;
}

void PageBackground::setGridVisible(bool visible)
{
    if (d->gridVisible == visible) return;
    d->gridVisible = visible;
    update();
    emit gridVisibleChanged(visible);
}

void PageBackground::resetGridVisible()
{
    setGridVisible(defaultsForVariant(d->variant).gridVisible);
}

PageBackgroundGradient PageBackground::gradient() const
{
    return d->gradient;
}

void PageBackground::setGradient(PageBackgroundGradient gradient)
{
    if (d->gradient == gradient && property("_ngstdGradient").toString() ==
                                       internal::gradientName(gradient)) {
        return;
    }
    d->gradient = gradient;
    setProperty("_ngstdGradient", internal::gradientName(d->gradient));
    WidgetStyle::refresh(this);
    update();
    emit gradientChanged(d->gradient);
}

void PageBackground::resetGradient()
{
    setGradient(defaultsForVariant(d->variant).gradient);
}

PageBackgroundCornerMode PageBackground::cornerMode() const
{
    return d->cornerMode;
}

void PageBackground::setCornerMode(PageBackgroundCornerMode cornerMode)
{
    if (d->cornerMode == cornerMode &&
        property("_ngstdCornerMode").toString() ==
            cornerModeName(cornerMode)) {
        return;
    }
    d->cornerMode = cornerMode;
    setProperty("_ngstdCornerMode", cornerModeName(d->cornerMode));
    WidgetStyle::refresh(this);
    update();
    emit cornerModeChanged(d->cornerMode);
}

void PageBackground::resetCornerMode()
{
    setCornerMode(PageBackgroundCornerMode::Square);
}

void PageBackground::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    if (d->cornerMode != PageBackgroundCornerMode::Square) {
        const qreal radius = DesignTokens::radius(RadiusRole::Panel);
        QPainterPath clipPath;
        clipPath.setFillRule(Qt::WindingFill);
        clipPath.addRoundedRect(QRectF(rect()), radius, radius);
        if (d->cornerMode == PageBackgroundCornerMode::RoundedTop) {
            clipPath.addRect(QRectF(0.0, radius, width(),
                                    qMax(0.0, height() - radius)));
        }
        painter.setClipPath(clipPath);
    }
    BackgroundRenderOptions options;
    options.target = rect();
    options.scheme = internal::colorSchemeFor(this);
    options.gradient = d->gradient;
    options.decorationVisible = d->decorationVisible;
    options.gridVisible = d->gridVisible;
    d->renderer->paint(&painter, options);
}

} // namespace widgets
} // namespace ngstd
