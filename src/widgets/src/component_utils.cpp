/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Shared private helpers for reusable widget implementations
 *****************************************************************************/
#include "component_utils_p.h"

#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpression>
#include <QSvgRenderer>
#include <QtMath>

namespace ngstd {
namespace widgets {
namespace internal {

QString gradientName(PageBackgroundGradient gradient)
{
    switch (gradient) {
    case PageBackgroundGradient::None:
        return QStringLiteral("none");
    case PageBackgroundGradient::Down:
        return QStringLiteral("down");
    case PageBackgroundGradient::Up:
        return QStringLiteral("up");
    case PageBackgroundGradient::Center:
        return QStringLiteral("center");
    case PageBackgroundGradient::ToCenter:
        return QStringLiteral("toCenter");
    }
    return QStringLiteral("none");
}

ColorScheme colorSchemeFor(const QWidget *widget)
{
    const QWidget *candidate = widget;
    while (candidate) {
        const QString scheme =
            candidate->property("_ngstdColorScheme").toString();
        if (scheme == QStringLiteral("dark")) return ColorScheme::Dark;
        if (scheme == QStringLiteral("light")) return ColorScheme::Light;
        candidate = candidate->parentWidget();
    }
    if (qApp) {
        const QString applicationScheme =
            qApp->property("_ngstdColorScheme").toString();
        if (applicationScheme == QStringLiteral("dark"))
            return ColorScheme::Dark;
        if (applicationScheme == QStringLiteral("light"))
            return ColorScheme::Light;
    }
    const QPalette palette =
        widget ? widget->palette() : (qApp ? qApp->palette() : QPalette());
    const QColor &color = palette.color(QPalette::Window);
    const qreal luminance = 0.2126 * color.redF() + 0.7152 * color.greenF() +
                            0.0722 * color.blueF();
    return luminance < 0.42 ? ColorScheme::Dark : ColorScheme::Light;
}

IconRole iconForTone(SemanticTone tone)
{
    switch (tone) {
    case SemanticTone::Success:
        return IconRole::CheckCircle;
    case SemanticTone::Danger:
        return IconRole::CloseCircle;
    case SemanticTone::Warning:
        return IconRole::AlertTriangle;
    case SemanticTone::Neutral:
    case SemanticTone::Information:
        return IconRole::Information;
    }
    return IconRole::Information;
}

ColorRole colorRoleForTone(SemanticTone tone)
{
    switch (tone) {
    case SemanticTone::Success:
        return ColorRole::Success;
    case SemanticTone::Warning:
        return ColorRole::Warning;
    case SemanticTone::Danger:
        return ColorRole::Danger;
    case SemanticTone::Neutral:
    case SemanticTone::Information:
        return ColorRole::Link;
    }
    return ColorRole::Link;
}

bool animationsEnabled(const QWidget *widget)
{
    const QWidget *candidate = widget;
    while (candidate) {
        const QString policy =
            candidate->property("_ngstdAnimationPolicy").toString();
        if (policy == QStringLiteral("disabled")) return false;
        if (policy == QStringLiteral("enabled")) return true;
        candidate = candidate->parentWidget();
    }
    if (qApp) {
        const QString policy =
            qApp->property("_ngstdAnimationPolicy").toString();
        if (policy == QStringLiteral("disabled")) return false;
        if (policy == QStringLiteral("enabled")) return true;
        const QVariant reducedMotion = qApp->property("_ngstdReducedMotion");
        if (reducedMotion.isValid()) return !reducedMotion.toBool();
    }
    // Qt 5 and the minimum supported Qt 6 versions do not expose a
    // cross-platform reduced-motion hint. System policy therefore keeps
    // motion enabled unless the host publishes an explicit override.
    return true;
}

QPalette comboBoxPopupPalette(const QWidget *widget)
{
    QPalette palette =
        widget ? widget->palette() : (qApp ? qApp->palette() : QPalette());
    const ColorScheme scheme = colorSchemeFor(widget);
    const QColor surface = DesignTokens::color(ColorRole::Surface, scheme);
    const QColor text = DesignTokens::color(ColorRole::TextSecondary, scheme);
    const QColor disabledText =
        DesignTokens::color(ColorRole::TextDisabled, scheme);
    const QColor highlight =
        DesignTokens::color(ColorRole::SurfaceBrand, scheme);
    const QColor highlightedText =
        DesignTokens::color(ColorRole::Link, scheme);
    for (QPalette::ColorGroup group : {QPalette::Active, QPalette::Inactive}) {
        palette.setColor(group, QPalette::Base, surface);
        palette.setColor(group, QPalette::Window, surface);
        palette.setColor(group, QPalette::Text, text);
        palette.setColor(group, QPalette::Highlight, highlight);
        palette.setColor(group, QPalette::HighlightedText, highlightedText);
    }
    palette.setColor(QPalette::Disabled, QPalette::Base, surface);
    palette.setColor(QPalette::Disabled, QPalette::Window, surface);
    palette.setColor(QPalette::Disabled, QPalette::Text, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::Highlight, highlight);
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText,
                     disabledText);
    return palette;
}

void renderSvg(QPainter *painter, const QString &path, const QRectF &target,
               qreal opacity, const QColor &tint,
               Qt::AspectRatioMode aspectRatioMode)
{
    if (!painter || target.isEmpty()) return;
    QSvgRenderer renderer(path);
    if (!renderer.isValid()) return;

    painter->save();
    painter->setOpacity(opacity);
    QRectF renderedTarget = target;
    const QSize sourceSize = renderer.defaultSize();
    if (!sourceSize.isEmpty() && aspectRatioMode != Qt::IgnoreAspectRatio) {
        const qreal scale = aspectRatioMode == Qt::KeepAspectRatio
                                ? qMin(target.width() / sourceSize.width(),
                                       target.height() / sourceSize.height())
                                : qMax(target.width() / sourceSize.width(),
                                       target.height() / sourceSize.height());
        const QSizeF size(sourceSize.width() * scale,
                          sourceSize.height() * scale);
        renderedTarget = QRectF(target.center().x() - size.width() * 0.5,
                                target.center().y() - size.height() * 0.5,
                                size.width(), size.height());
        if (aspectRatioMode == Qt::KeepAspectRatioByExpanding)
            painter->setClipRect(target);
    }

    if (!tint.isValid()) {
        renderer.render(painter, renderedTarget);
        painter->restore();
        return;
    }

    const qreal devicePixelRatio =
        painter->device() ? painter->device()->devicePixelRatioF() : 1.0;
    const int supersampling =
        qMax(1, internal::tokenInteger(
                    QStringLiteral("desktop.motion.vectorSupersampling")));
    const qreal imageScale = devicePixelRatio * supersampling;
    const QSize imageSize(qMax(1, qCeil(target.width() * imageScale)),
                          qMax(1, qCeil(target.height() * imageScale)));
    QImage image(imageSize, QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(imageScale);
    image.fill(Qt::transparent);
    QPainter imagePainter(&image);
    renderer.render(&imagePainter,
                    renderedTarget.translated(-target.topLeft()));
    imagePainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    imagePainter.fillRect(image.rect(), tint);
    imagePainter.end();
    painter->drawImage(target.topLeft(), image);
    painter->restore();
}

QColor cssColor(const QString &value)
{
    QColor color(value);
    if (color.isValid()) return color;
    static const QRegularExpression expression(
        QStringLiteral("^rgba\\(\\s*(\\d+)\\s*,\\s*(\\d+)\\s*,\\s*(\\d+)"
                       "\\s*,\\s*([0-9.]+)\\s*\\)$"));
    const QRegularExpressionMatch match = expression.match(value);
    if (!match.hasMatch()) return QColor();
    color.setRgb(match.captured(1).toInt(), match.captured(2).toInt(),
                 match.captured(3).toInt());
    color.setAlphaF(static_cast<float>(match.captured(4).toDouble()));
    return color;
}

QColor interpolateColor(const QColor &start, const QColor &end, qreal progress)
{
    const qreal bounded = qBound(0.0, progress, 1.0);
    return QColor::fromRgbF(
        static_cast<float>(start.redF() +
                           (end.redF() - start.redF()) * bounded),
        static_cast<float>(start.greenF() +
                           (end.greenF() - start.greenF()) * bounded),
        static_cast<float>(start.blueF() +
                           (end.blueF() - start.blueF()) * bounded),
        static_cast<float>(start.alphaF() +
                           (end.alphaF() - start.alphaF()) * bounded));
}

qreal selectionFeedbackOpacity(qreal progress)
{
    return qSin(qBound(0.0, progress, 1.0) * M_PI);
}

qreal selectionFeedbackInset(qreal progress)
{
    const qreal boundedProgress = qBound(0.0, progress, 1.0);
    const qreal expansion = DesignTokens::componentMetric(
        ComponentMetric::SelectionFeedbackExpansion);
    const qreal strokeWidth = DesignTokens::componentMetric(
        ComponentMetric::SelectionFeedbackStrokeWidth);
    return expansion * (1.0 - boundedProgress) + strokeWidth * 0.5;
}

QRectF selectionFeedbackBounds(const QRectF &bounds, qreal progress)
{
    const qreal inset = selectionFeedbackInset(progress);
    return bounds.adjusted(inset, inset, -inset, -inset);
}

qreal selectionFeedbackRadius(qreal radius, qreal progress)
{
    return qMax(0.0, radius - selectionFeedbackInset(progress));
}

void paintSelectionFeedback(QPainter *painter, const QRectF &bounds,
                            qreal radius, qreal progress, QColor color,
                            bool circular)
{
    const qreal boundedProgress = qBound(0.0, progress, 1.0);
    if (!painter || bounds.isEmpty() || boundedProgress >= 1.0) return;

    color.setAlphaF(static_cast<float>(
        color.alphaF() * selectionFeedbackOpacity(boundedProgress)));
    if (color.alpha() == 0) return;

    const qreal strokeWidth = DesignTokens::componentMetric(
        ComponentMetric::SelectionFeedbackStrokeWidth);
    const QRectF outerBounds =
        bounds.adjusted(strokeWidth * 0.25, strokeWidth * 0.25,
                        -strokeWidth * 0.25, -strokeWidth * 0.25);
    const QRectF innerBounds =
        selectionFeedbackBounds(bounds, boundedProgress);
    const qreal outerRadius = qMax(0.0, radius - strokeWidth * 0.25);
    const qreal innerRadius =
        selectionFeedbackRadius(radius, boundedProgress);

    QPainterPath band;
    band.setFillRule(Qt::OddEvenFill);
    if (circular) {
        band.addEllipse(outerBounds);
        band.addEllipse(innerBounds);
    }
    else {
        band.addRoundedRect(outerBounds, outerRadius, outerRadius);
        band.addRoundedRect(innerBounds, innerRadius, innerRadius);
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawPath(band);
    painter->setPen(QPen(color, strokeWidth, Qt::SolidLine, Qt::RoundCap,
                         Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);
    if (circular)
        painter->drawEllipse(innerBounds);
    else
        painter->drawRoundedRect(innerBounds, innerRadius, innerRadius);
    painter->restore();
}

QBrush backgroundGradient(PageBackgroundGradient direction,
                          const QRectF &target, const QColor &strong,
                          const QColor &clear)
{
    if (direction == PageBackgroundGradient::Center ||
        direction == PageBackgroundGradient::ToCenter) {
        QRadialGradient gradient(target.center(),
                                 qMax(target.width(), target.height()) * 0.72);
        gradient.setColorAt(
            0.0, direction == PageBackgroundGradient::Center ? clear : strong);
        gradient.setColorAt(
            1.0, direction == PageBackgroundGradient::Center ? strong : clear);
        return QBrush(gradient);
    }
    QLinearGradient gradient(
        direction == PageBackgroundGradient::Up ? target.bottomLeft()
                                                : target.topLeft(),
        direction == PageBackgroundGradient::Up ? target.topLeft()
                                                : target.bottomLeft());
    gradient.setColorAt(0.0, strong);
    gradient.setColorAt(1.0, clear);
    return QBrush(gradient);
}

QRectF coverRect(const QSize &sourceSize, const QRectF &target)
{
    if (sourceSize.isEmpty() || target.isEmpty()) return QRectF();
    const qreal scale = qMax(target.width() / sourceSize.width(),
                             target.height() / sourceSize.height());
    const QSizeF scaledSize(sourceSize.width() * scale,
                            sourceSize.height() * scale);
    return QRectF(target.center().x() - scaledSize.width() / 2.0,
                  target.center().y() - scaledSize.height() / 2.0,
                  scaledSize.width(), scaledSize.height());
}

} // namespace internal
} // namespace widgets
} // namespace ngstd
