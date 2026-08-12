/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Shared token-backed push button label layout
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_BUTTON_LABEL_P_H
#define NGSTD_WIDGETS_BUTTON_LABEL_P_H

class QPainter;
class QStyleOptionButton;
class QWidget;

namespace ngstd {
namespace widgets {
namespace internal {

int buttonIconTextGap();
int nativeButtonIconTextGap();
void drawButtonLabel(const QStyleOptionButton &option, QPainter *painter,
                     const QWidget *widget);

} // namespace internal
} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_BUTTON_LABEL_P_H
