/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Shared private helpers for reusable widget implementations
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_COMPONENT_UTILS_P_H
#define NGSTD_WIDGETS_COMPONENT_UTILS_P_H

#include "design_tokens_p.h"

#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/icons.h>
#include <ngstd/widgets/page_background.h>
#include <ngstd/widgets/widget_style.h>

#include <QBrush>
#include <QColor>
#include <QPalette>
#include <QRectF>
#include <QString>

class QPainter;
class QWidget;

namespace ngstd {
namespace widgets {
namespace internal {

QString gradientName(PageBackgroundGradient gradient);
ColorScheme colorSchemeFor(const QWidget *widget);
IconRole iconForTone(SemanticTone tone);
ColorRole colorRoleForTone(SemanticTone tone);
bool animationsEnabled(const QWidget *widget);
QPalette comboBoxPopupPalette(const QWidget *widget);

void renderSvg(QPainter *painter, const QString &path, const QRectF &target,
               qreal opacity, const QColor &tint = QColor(),
               Qt::AspectRatioMode aspectRatioMode = Qt::IgnoreAspectRatio);

QColor cssColor(const QString &value);
QColor interpolateColor(const QColor &start, const QColor &end,
                        qreal progress);
qreal selectionFeedbackOpacity(qreal progress);
qreal selectionFeedbackInset(qreal progress);
QRectF selectionFeedbackBounds(const QRectF &bounds, qreal progress);
qreal selectionFeedbackRadius(qreal radius, qreal progress);
void paintSelectionFeedback(QPainter *painter, const QRectF &bounds,
                            qreal radius, qreal progress, QColor color,
                            bool circular = false);
QBrush backgroundGradient(PageBackgroundGradient direction,
                          const QRectF &target, const QColor &strong,
                          const QColor &clear);
QRectF coverRect(const QSize &sourceSize, const QRectF &target);

} // namespace internal
} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_COMPONENT_UTILS_P_H
