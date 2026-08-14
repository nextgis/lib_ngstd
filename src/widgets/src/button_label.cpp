/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Shared token-backed push button label layout
 *****************************************************************************/
#include "button_label_p.h"

#include "design_tokens_p.h"

#include <ngstd/widgets/design_tokens.h>

#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionButton>
#include <QWidget>

namespace ngstd {
namespace widgets {
namespace internal {

int buttonIconTextGap()
{
    return DesignTokens::componentMetric(ComponentMetric::ButtonIconTextGap);
}

int nativeButtonIconTextGap()
{
    return tokenInteger(
        QStringLiteral("desktop.component.button.qtNativeIconTextGapPx"));
}

void drawButtonLabel(const QStyleOptionButton &option, QPainter *painter,
                     const QWidget *widget)
{
    if (!painter) return;
    const QStyle *style = widget ? widget->style() : QApplication::style();
    QRect contents = widget
                         ? style->subElementRect(QStyle::SE_PushButtonContents,
                                                 &option, widget)
                         : option.rect;
    if (option.state & QStyle::State_Sunken) {
        contents.translate(style->pixelMetric(QStyle::PM_ButtonShiftHorizontal,
                                              &option, widget),
                           style->pixelMetric(QStyle::PM_ButtonShiftVertical,
                                              &option, widget));
    }

    const bool hasIcon = !option.icon.isNull();
    const bool hasText = !option.text.isEmpty();
    const int gap = hasIcon && hasText ? buttonIconTextGap() : 0;
    const int iconWidth = hasIcon ? option.iconSize.width() : 0;
    int textFlags =
        Qt::AlignVCenter | Qt::TextSingleLine | Qt::TextShowMnemonic;
    if (!style->styleHint(QStyle::SH_UnderlineShortcut, &option, widget))
        textFlags |= Qt::TextHideMnemonic;
    const int textWidth =
        hasText ? option.fontMetrics.horizontalAdvance(option.text) + 2 : 0;
    const int groupWidth = iconWidth + gap + textWidth;
    const int groupLeft =
        contents.left() + qMax(0, (contents.width() - groupWidth) / 2);
    const bool trailingIcon =
        widget && widget->property("_ngstdButtonIconPlacement").toString() ==
                      QStringLiteral("trailing");

    if (hasIcon) {
        const QRect logicalIconRect(
            trailingIcon ? groupLeft + textWidth + gap : groupLeft,
            contents.top() +
                (contents.height() - option.iconSize.height()) / 2,
            option.iconSize.width(), option.iconSize.height());
        const QRect iconRect =
            style->visualRect(option.direction, contents, logicalIconRect);
        const QIcon::Mode mode = option.state & QStyle::State_Enabled
                                     ? QIcon::Normal
                                     : QIcon::Disabled;
        const QIcon::State state =
            option.state & QStyle::State_On ? QIcon::On : QIcon::Off;
        const QPixmap pixmap =
            option.icon.pixmap(option.iconSize, mode, state);
        style->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
    }

    if (hasText) {
        const QRect logicalTextRect(
            groupLeft + (trailingIcon ? 0 : iconWidth + gap), contents.top(),
            textWidth, contents.height());
        const QRect textRect =
            style->visualRect(option.direction, contents, logicalTextRect);
        const Qt::Alignment alignment = QStyle::visualAlignment(
            option.direction, Qt::AlignLeft | Qt::AlignVCenter);
        style->drawItemText(painter, textRect, alignment | textFlags,
                            option.palette,
                            option.state & QStyle::State_Enabled, option.text,
                            QPalette::ButtonText);
    }
}

} // namespace internal
} // namespace widgets
} // namespace ngstd
