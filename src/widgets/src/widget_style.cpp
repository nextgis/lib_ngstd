/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Semantic style properties for Qt Widgets
 *****************************************************************************/
#include <ngstd/widgets/widget_style.h>

#include <ngstd/widgets/design_tokens.h>

#include <QAbstractButton>
#include <QStyle>
#include <QVariant>
#include <QWidget>

namespace ngstd {
namespace widgets {

void WidgetStyle::setButtonVariant(QAbstractButton *button,
                                   ButtonVariant variant)
{
    if (!button) return;
    const QString name = buttonVariantName(variant);
    button->setProperty("ngstdButtonVariant",
                        name.isEmpty() ? QVariant() : QVariant(name));
    refresh(button);
}

ButtonVariant WidgetStyle::buttonVariant(const QAbstractButton *button)
{
    if (!button) return ButtonVariant::Default;
    const QString name = button->property("ngstdButtonVariant").toString();
    for (int value = static_cast<int>(ButtonVariant::Default);
         value <= static_cast<int>(ButtonVariant::PhotoText); ++value) {
        const ButtonVariant variant = static_cast<ButtonVariant>(value);
        if (buttonVariantName(variant) == name) return variant;
    }
    return ButtonVariant::Default;
}

void WidgetStyle::setTone(QWidget *widget, SemanticTone tone)
{
    if (!widget) return;
    const QString name = toneName(tone);
    widget->setProperty("ngstdTone",
                        name.isEmpty() ? QVariant() : QVariant(name));
    refresh(widget);
}

SemanticTone WidgetStyle::tone(const QWidget *widget)
{
    if (!widget) return SemanticTone::Neutral;
    const QString name = widget->property("ngstdTone").toString();
    for (int value = static_cast<int>(SemanticTone::Neutral);
         value <= static_cast<int>(SemanticTone::Danger); ++value) {
        const SemanticTone toneValue = static_cast<SemanticTone>(value);
        if (toneName(toneValue) == name) return toneValue;
    }
    return SemanticTone::Neutral;
}

void WidgetStyle::setCardVariant(QWidget *widget, CardVariant variant)
{
    if (!widget) return;
    const QString name = cardVariantName(variant);
    widget->setProperty("ngstdCardVariant",
                        name.isEmpty() ? QVariant() : QVariant(name));
    refresh(widget);
}

CardVariant WidgetStyle::cardVariant(const QWidget *widget)
{
    if (!widget) return CardVariant::Default;
    const QString name = widget->property("ngstdCardVariant").toString();
    for (int value = static_cast<int>(CardVariant::Default);
         value <= static_cast<int>(CardVariant::Selectable); ++value) {
        const CardVariant variant = static_cast<CardVariant>(value);
        if (cardVariantName(variant) == name) return variant;
    }
    return CardVariant::Default;
}

void WidgetStyle::setPageBackgroundVariant(QWidget *widget,
                                           PageBackgroundVariant variant)
{
    if (!widget) return;
    widget->setProperty("ngstdPageBackgroundVariant",
                        pageBackgroundVariantName(variant));
    refresh(widget);
}

PageBackgroundVariant WidgetStyle::pageBackgroundVariant(const QWidget *widget)
{
    if (!widget) return PageBackgroundVariant::Main;
    const QString name =
        widget->property("ngstdPageBackgroundVariant").toString();
    for (int value = static_cast<int>(PageBackgroundVariant::Main);
         value <= static_cast<int>(PageBackgroundVariant::Fieldwork);
         ++value) {
        const PageBackgroundVariant variant =
            static_cast<PageBackgroundVariant>(value);
        if (pageBackgroundVariantName(variant) == name) return variant;
    }
    return PageBackgroundVariant::Main;
}

void WidgetStyle::setError(QWidget *widget, bool error)
{
    if (!widget) return;
    widget->setProperty("ngstdError", error);
    refresh(widget);
}

void WidgetStyle::setSelected(QWidget *widget, bool selected)
{
    if (!widget) return;
    widget->setProperty("ngstdSelected", selected);
    refresh(widget);
}

void WidgetStyle::setTypographyRole(QWidget *widget, TypographyRole role)
{
    if (!widget) return;
    widget->setProperty("ngstdTypographyRole", typographyRoleName(role));
    widget->setFont(DesignTokens::font(role));
    refresh(widget);
}

void WidgetStyle::refresh(QWidget *widget)
{
    if (!widget) return;
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->updateGeometry();
    widget->update();
}

void WidgetStyle::refreshVisual(QWidget *widget)
{
    if (!widget) return;
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

QString WidgetStyle::buttonVariantName(ButtonVariant variant)
{
    switch (variant) {
    case ButtonVariant::Default:
        return QString();
    case ButtonVariant::Primary:
        return QStringLiteral("primary");
    case ButtonVariant::Secondary:
        return QStringLiteral("secondary");
    case ButtonVariant::Text:
        return QStringLiteral("text");
    case ButtonVariant::Ghost:
        return QStringLiteral("ghost");
    case ButtonVariant::Danger:
        return QStringLiteral("danger");
    case ButtonVariant::Icon:
        return QStringLiteral("icon");
    case ButtonVariant::Hero:
        return QStringLiteral("hero");
    case ButtonVariant::Trial:
        return QStringLiteral("trial");
    case ButtonVariant::DataFilled:
        return QStringLiteral("dataFilled");
    case ButtonVariant::DataOutline:
        return QStringLiteral("dataOutline");
    case ButtonVariant::OnBrand:
        return QStringLiteral("onBrand");
    case ButtonVariant::OnBrandSecondary:
        return QStringLiteral("onBrandSecondary");
    case ButtonVariant::Photo:
        return QStringLiteral("photo");
    case ButtonVariant::PhotoText:
        return QStringLiteral("photoText");
    }
    return QString();
}

QString WidgetStyle::toneName(SemanticTone tone)
{
    switch (tone) {
    case SemanticTone::Neutral:
        return QString();
    case SemanticTone::Information:
        return QStringLiteral("info");
    case SemanticTone::Success:
        return QStringLiteral("success");
    case SemanticTone::Warning:
        return QStringLiteral("warning");
    case SemanticTone::Danger:
        return QStringLiteral("danger");
    }
    return QString();
}

QString WidgetStyle::cardVariantName(CardVariant variant)
{
    switch (variant) {
    case CardVariant::Default:
        return QString();
    case CardVariant::Data:
        return QStringLiteral("data");
    case CardVariant::Toolbox:
        return QStringLiteral("toolbox");
    case CardVariant::ToolboxNew:
        return QStringLiteral("toolboxNew");
    case CardVariant::Media:
        return QStringLiteral("media");
    case CardVariant::Background:
        return QStringLiteral("background");
    case CardVariant::Panel:
        return QStringLiteral("panel");
    case CardVariant::Selectable:
        return QStringLiteral("selectable");
    }
    return QString();
}

QString WidgetStyle::pageBackgroundVariantName(PageBackgroundVariant variant)
{
    switch (variant) {
    case PageBackgroundVariant::Main:
        return QStringLiteral("main");
    case PageBackgroundVariant::Toolbox:
        return QStringLiteral("toolbox");
    case PageBackgroundVariant::Data:
        return QStringLiteral("data");
    case PageBackgroundVariant::Workspace:
        return QStringLiteral("workspace");
    case PageBackgroundVariant::Corporate:
        return QStringLiteral("corporate");
    case PageBackgroundVariant::Fieldwork:
        return QStringLiteral("fieldwork");
    }
    return QStringLiteral("main");
}

QString WidgetStyle::typographyRoleName(TypographyRole role)
{
    switch (role) {
    case TypographyRole::Display:
        return QStringLiteral("display");
    case TypographyRole::Heading1:
        return QStringLiteral("heading1");
    case TypographyRole::Heading2:
        return QStringLiteral("heading2");
    case TypographyRole::Heading3:
        return QStringLiteral("heading3");
    case TypographyRole::BodyLarge:
        return QStringLiteral("bodyLarge");
    case TypographyRole::Body:
    case TypographyRole::QtBody:
        return QStringLiteral("body");
    case TypographyRole::Control:
        return QStringLiteral("control");
    case TypographyRole::Caption:
        return QStringLiteral("caption");
    case TypographyRole::Mono:
        return QStringLiteral("mono");
    case TypographyRole::QtHeading1:
        return QStringLiteral("heading1");
    case TypographyRole::QtHeading2:
        return QStringLiteral("heading2");
    }
    return QStringLiteral("body");
}

} // namespace widgets
} // namespace ngstd
