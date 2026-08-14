/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Semantic style properties for Qt Widgets
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_WIDGET_STYLE_H
#define NGSTD_WIDGETS_WIDGET_STYLE_H

#include <ngstd/widgets/types.h>

#include <QString>

class QAbstractButton;
class QAbstractScrollArea;
class QLabel;
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
    static void setNoticeTone(QWidget *widget, SemanticTone tone);

    static void setCardVariant(QWidget *widget, CardVariant variant);
    static CardVariant cardVariant(const QWidget *widget);

    static void setPageBackgroundVariant(QWidget *widget,
                                         PageBackgroundVariant variant);
    static PageBackgroundVariant pageBackgroundVariant(const QWidget *widget);

    static void setError(QWidget *widget, bool error);
    static void setSelected(QWidget *widget, bool selected);
    static void setTypographyRole(QWidget *widget, TypographyRole role);
    static void setTextColorRole(QWidget *widget, ColorRole role);
    static void setDivider(QWidget *widget);
    static void setSoftDivider(QWidget *widget);
    static void setEmbeddedSurface(QWidget *surface);
    static void applyWizardPageLayout(QWidget *page);
    static void applyWizardPageHeader(QLabel *title, QLabel *subtitle);
    static void refresh(QWidget *widget);
    static void refreshVisual(QWidget *widget);

    static QString buttonVariantName(ButtonVariant variant);
    static QString toneName(SemanticTone tone);
    static QString cardVariantName(CardVariant variant);
    static QString pageBackgroundVariantName(PageBackgroundVariant variant);
    static QString typographyRoleName(TypographyRole role);
    static QString textColorRoleName(ColorRole role);
};

} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_WIDGET_STYLE_H
