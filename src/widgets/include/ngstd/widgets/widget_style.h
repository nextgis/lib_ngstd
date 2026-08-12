/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Semantic style properties for Qt Widgets
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_WIDGET_STYLE_H
#define NGSTD_WIDGETS_WIDGET_STYLE_H

#include <ngstd/widgets/types.h>

#include <QString>

class QAbstractButton;
class QWidget;

namespace ngstd {
namespace widgets {

class NGSTD_WIDGETS_EXPORT WidgetStyle final
{
public:
    static void setButtonVariant(QAbstractButton *button,
                                 ButtonVariant variant);
    static ButtonVariant buttonVariant(const QAbstractButton *button);

    static void setTone(QWidget *widget, SemanticTone tone);
    static SemanticTone tone(const QWidget *widget);

    static void setCardVariant(QWidget *widget, CardVariant variant);
    static CardVariant cardVariant(const QWidget *widget);

    static void setPageBackgroundVariant(QWidget *widget,
                                         PageBackgroundVariant variant);
    static PageBackgroundVariant pageBackgroundVariant(const QWidget *widget);

    static void setError(QWidget *widget, bool error);
    static void setSelected(QWidget *widget, bool selected);
    static void setTypographyRole(QWidget *widget, TypographyRole role);
    static void refresh(QWidget *widget);
    static void refreshVisual(QWidget *widget);

    static QString buttonVariantName(ButtonVariant variant);
    static QString toneName(SemanticTone tone);
    static QString cardVariantName(CardVariant variant);
    static QString pageBackgroundVariantName(PageBackgroundVariant variant);
    static QString typographyRoleName(TypographyRole role);
};

} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_WIDGET_STYLE_H
