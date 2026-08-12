/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Runtime access to canonical NextGIS design tokens
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_DESIGN_TOKENS_H
#define NGSTD_WIDGETS_DESIGN_TOKENS_H

#include <ngstd/widgets/types.h>

#include <QColor>
#include <QEasingCurve>
#include <QFont>
#include <QString>
#include <QStringList>

namespace ngstd {
namespace widgets {

struct NGSTD_WIDGETS_EXPORT TypographyToken
{
    QStringList families;
    int pixelSize = 0;
    qreal lineHeight = 1.0;
    QFont::Weight weight = QFont::Normal;
};

class NGSTD_WIDGETS_EXPORT DesignTokens final
{
public:
    static QString name();
    static QString version();

    static QColor color(ColorRole role, ColorScheme scheme);
    static int spacing(int level);
    static int radius(RadiusRole role);
    static int controlHeight(ControlSize size);
    static int controlPadding();
    static int controlIconSize();
    static int duration(MotionDuration duration);
    static qreal decorationOpacity(ColorScheme scheme);
    static QEasingCurve easingCurve(
        MotionEasing easing = MotionEasing::Standard);
    static TypographyToken typography(TypographyRole role);
    static QFont font(TypographyRole role);
    static int componentMetric(ComponentMetric metric);
};

} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_DESIGN_TOKENS_H
