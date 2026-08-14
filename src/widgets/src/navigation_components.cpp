/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Reusable navigation panel and list components
 *****************************************************************************/
#include <ngstd/widgets/navigation_components.h>

#include "component_utils_p.h"

#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/icons.h>

#include <QEvent>
#include <QFont>
#include <QIcon>
#include <QBrush>
#include <QListWidgetItem>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>

namespace ngstd {
namespace widgets {

NavigationPanel::NavigationPanel(QWidget *parent) : QFrame(parent)
{
    setProperty("_ngstdRole", QStringLiteral("navigationPanel"));
    setFrameShape(QFrame::NoFrame);
    setAttribute(Qt::WA_OpaquePaintEvent);
}

void NavigationPanel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    const ColorScheme scheme = internal::colorSchemeFor(this);
    const QRectF panelRect(rect());
    QPainter painter(this);
    painter.fillRect(panelRect,
                     DesignTokens::color(ColorRole::Background, scheme));

    constexpr qreal borderWidth = 1.0;
    const qreal borderX = layoutDirection() == Qt::RightToLeft
        ? panelRect.left() + borderWidth * 0.5
        : panelRect.right() - borderWidth * 0.5;
    painter.setPen(QPen(DesignTokens::color(ColorRole::Border, scheme),
                        borderWidth));
    painter.drawLine(QPointF(borderX, panelRect.top()),
                     QPointF(borderX, panelRect.bottom()));
}

NavigationListWidget::NavigationListWidget(QWidget *parent)
    : QListWidget(parent)
{
    setProperty("_ngstdRole", QStringLiteral("navigationList"));
    setFrameShape(QFrame::NoFrame);
    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    setViewportMargins(layoutDirection() == Qt::RightToLeft ? 1 : 0, 0,
                       layoutDirection() == Qt::LeftToRight ? 1 : 0, 0);
    const int iconExtent = DesignTokens::componentMetric(
        ComponentMetric::WizardSidebarIconExtent);
    setIconSize(QSize(iconExtent, iconExtent));
}

NavigationListWidget::StepState NavigationListWidget::stepState(
    const QListWidgetItem *item) const
{
    if (!item) return StepState::Pending;
    return static_cast<StepState>(
        item->data(Qt::UserRole + 1).toInt());
}

void NavigationListWidget::setStepState(
    QListWidgetItem *item, StepState state)
{
    if (!item) return;
    item->setData(Qt::UserRole + 1, static_cast<int>(state));
    updateStepAppearance(item);
}

void NavigationListWidget::changeEvent(QEvent *event)
{
    QListWidget::changeEvent(event);
    if (event->type() == QEvent::PaletteChange ||
        event->type() == QEvent::StyleChange ||
        event->type() == QEvent::FontChange ||
        event->type() == QEvent::DevicePixelRatioChange ||
        event->type() == QEvent::LayoutDirectionChange) {
        setViewportMargins(layoutDirection() == Qt::RightToLeft ? 1 : 0, 0,
                           layoutDirection() == Qt::LeftToRight ? 1 : 0, 0);
        updateStepAppearances();
    }
}

void NavigationListWidget::updateStepAppearance(QListWidgetItem *item)
{
    if (!item) return;

    const StepState state = stepState(item);
    IconRole iconRole = IconRole::Ellipsis;
    ColorRole iconColorRole = ColorRole::Border;
    ColorRole textColorRole = ColorRole::TextMuted;
    if (state == StepState::Complete) {
        iconRole = IconRole::CheckCircle;
        iconColorRole = ColorRole::Success;
        textColorRole = ColorRole::Text;
    } else if (state == StepState::Current) {
        iconRole = IconRole::ChevronRight;
        iconColorRole = ColorRole::Brand;
        textColorRole = ColorRole::Text;
    } else if (state == StepState::Available) {
        iconColorRole = ColorRole::TextSecondary;
        textColorRole = ColorRole::TextSecondary;
    }

    const ColorScheme scheme = internal::colorSchemeFor(this);
    const int iconPixelSize = DesignTokens::componentMetric(
        ComponentMetric::WizardSidebarIconSize);
    item->setIcon(QIcon(iconPixmap(
        iconRole, QSize(iconPixelSize, iconPixelSize), devicePixelRatioF(),
        DesignTokens::color(iconColorRole, scheme))));
    item->setForeground(QBrush(DesignTokens::color(textColorRole, scheme)));
    QFont font = DesignTokens::font(TypographyRole::QtBody);
    font.setStyleStrategy(QFont::PreferAntialias);
    font.setWeight(state == StepState::Current
                       ? QFont::Medium : QFont::Normal);
    item->setFont(font);
    const int rowHeight = DesignTokens::componentMetric(
        ComponentMetric::WizardSidebarRowHeight);
    item->setSizeHint(QSize(viewport()->width(), rowHeight));
}

void NavigationListWidget::updateStepAppearances()
{
    for (int row = 0; row < count(); ++row)
        updateStepAppearance(item(row));
}

} // namespace widgets
} // namespace ngstd
