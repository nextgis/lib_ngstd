/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Semantic style properties for Qt Widgets
 *****************************************************************************/
#include <ngstd/widgets/widget_style.h>

#include <ngstd/widgets/design_tokens.h>

#include <QAbstractButton>
#include <QAbstractScrollArea>
#include <QBoxLayout>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QStyle>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

namespace {

int nearestSpacingToken(int value)
{
    if (value <= 0) return 0;

    int nearest = ngstd::widgets::DesignTokens::spacing(1);
    int distance = qAbs(value - nearest);
    for (int level = 2; level <= 10; ++level) {
        const int candidate = ngstd::widgets::DesignTokens::spacing(level);
        const int candidateDistance = qAbs(value - candidate);
        if (candidateDistance < distance) {
            nearest = candidate;
            distance = candidateDistance;
        }
    }
    return nearest;
}

int tokenSpacingOrInherited(int value)
{
    return value < 0 ? value : nearestSpacingToken(value);
}

void normalizeLayoutSpacing(QLayout *layout)
{
    if (!layout) return;

    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
    layout->getContentsMargins(&left, &top, &right, &bottom);
    layout->setContentsMargins(nearestSpacingToken(left),
                               nearestSpacingToken(top),
                               nearestSpacingToken(right),
                               nearestSpacingToken(bottom));

    if (QGridLayout *grid = qobject_cast<QGridLayout *>(layout)) {
        const int horizontalSpacing = grid->horizontalSpacing();
        const int verticalSpacing = grid->verticalSpacing();
        grid->setHorizontalSpacing(
            tokenSpacingOrInherited(horizontalSpacing));
        grid->setVerticalSpacing(
            tokenSpacingOrInherited(verticalSpacing));
        return;
    }
    layout->setSpacing(tokenSpacingOrInherited(layout->spacing()));
}

} // namespace

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

void WidgetStyle::setNoticeTone(QWidget *widget, SemanticTone tone)
{
    if (!widget) return;
    if (QFrame *frame = qobject_cast<QFrame *>(widget))
        frame->setFrameShape(QFrame::NoFrame);
    widget->setProperty("_ngstdRole", QStringLiteral("notice"));
    widget->setAttribute(Qt::WA_StyledBackground, true);
    setTone(widget, tone);
}

void WidgetStyle::setCardVariant(QWidget *widget, CardVariant variant)
{
    if (!widget) return;
    if (QFrame *frame = qobject_cast<QFrame *>(widget))
        frame->setFrameShape(QFrame::NoFrame);
    widget->setProperty("_ngstdRole", QStringLiteral("card"));
    widget->setAttribute(Qt::WA_StyledBackground, true);
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
         value <= static_cast<int>(CardVariant::SurfaceMuted); ++value) {
        const CardVariant variant = static_cast<CardVariant>(value);
        if (cardVariantName(variant) == name) return variant;
    }
    return CardVariant::Default;
}

void WidgetStyle::setPageBackgroundVariant(QWidget *widget,
                                           PageBackgroundVariant variant)
{
    if (!widget) return;
    widget->setProperty("_ngstdRole", QStringLiteral("pageBackground"));
    widget->setAttribute(Qt::WA_StyledBackground, true);
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

void WidgetStyle::setTextColorRole(QWidget *widget, ColorRole role)
{
    if (!widget) return;
    widget->setProperty("ngstdTextColorRole", textColorRoleName(role));
    refresh(widget);
}

void WidgetStyle::setSoftDivider(QWidget *widget)
{
    if (!widget) return;
    widget->setProperty("_ngstdRole", QStringLiteral("softDivider"));
    if (QFrame *frame = qobject_cast<QFrame *>(widget))
        frame->setFrameShape(QFrame::NoFrame);
    widget->setFixedHeight(1);
    refresh(widget);
}

void WidgetStyle::setDivider(QWidget *widget)
{
    if (!widget) return;
    if (QFrame *frame = qobject_cast<QFrame *>(widget))
        frame->setFrameShape(QFrame::NoFrame);
    widget->setProperty("_ngstdRole", QStringLiteral("divider"));
    widget->setAttribute(Qt::WA_StyledBackground, true);
    refresh(widget);
}

void WidgetStyle::setEmbeddedSurface(QWidget *surface)
{
    if (!surface) return;
    surface->setProperty("ngstdSurfaceMode", QStringLiteral("embedded"));
    if (QFrame *frame = qobject_cast<QFrame *>(surface))
        frame->setFrameShape(QFrame::NoFrame);
    surface->setAutoFillBackground(false);
    QAbstractScrollArea *scrollArea =
        qobject_cast<QAbstractScrollArea *>(surface);
    if (scrollArea && scrollArea->viewport()) {
        QWidget *viewport = scrollArea->viewport();
        viewport->setProperty("ngstdSurfaceMode",
                              QStringLiteral("embeddedViewport"));
        viewport->setAutoFillBackground(false);
        refresh(viewport);
    }
    refresh(surface);
}

void WidgetStyle::applyWizardPageLayout(QWidget *page)
{
    if (!page || !page->layout()) return;

    const QList<QLayout *> layouts = page->findChildren<QLayout *>();
    for (QLayout *layout : layouts) normalizeLayoutSpacing(layout);
    normalizeLayoutSpacing(page->layout());

    const int margin = DesignTokens::componentMetric(
        ComponentMetric::WizardPageMargin);
    page->layout()->setContentsMargins(margin, margin, margin, margin);
    page->layout()->setSpacing(DesignTokens::componentMetric(
        ComponentMetric::WizardPageSpacing));
}

void WidgetStyle::applyWizardPageHeader(QLabel *title, QLabel *subtitle)
{
    if (!title || !subtitle) return;
    setTypographyRole(title, TypographyRole::Heading2);
    setTypographyRole(subtitle, TypographyRole::Heading1Subtitle);
    if (title->property("_ngstdWizardPageHeaderGrouped").toBool()) return;
    if (title->parentWidget() != subtitle->parentWidget()) return;

    QBoxLayout *parentLayout = qobject_cast<QBoxLayout *>(
        title->parentWidget() ? title->parentWidget()->layout() : nullptr);
    if (!parentLayout) return;
    const int titleIndex = parentLayout->indexOf(title);
    const int subtitleIndex = parentLayout->indexOf(subtitle);
    if (titleIndex < 0 || subtitleIndex != titleIndex + 1) return;

    parentLayout->removeWidget(title);
    parentLayout->removeWidget(subtitle);
    QVBoxLayout *headerLayout = new QVBoxLayout;
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(DesignTokens::componentMetric(
        ComponentMetric::WizardTitleBottomSpacing));
    headerLayout->addWidget(title);
    headerLayout->addWidget(subtitle);
    parentLayout->insertLayout(titleIndex, headerLayout);
    title->setProperty("_ngstdWizardPageHeaderGrouped", true);
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
    case CardVariant::SurfaceBrand:
        return QStringLiteral("surfaceBrand");
    case CardVariant::SurfaceMuted:
        return QStringLiteral("surfaceMuted");
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
    case TypographyRole::Title:
        return QStringLiteral("title");
    case TypographyRole::Heading1:
        return QStringLiteral("heading1");
    case TypographyRole::Heading1Subtitle:
        return QStringLiteral("heading1Subtitle");
    case TypographyRole::Heading2:
        return QStringLiteral("heading2");
    case TypographyRole::Heading3:
        return QStringLiteral("heading3");
    case TypographyRole::Heading4:
        return QStringLiteral("heading4");
    case TypographyRole::BodyLarge:
        return QStringLiteral("bodyLarge");
    case TypographyRole::Body:
    case TypographyRole::QtBody:
        return QStringLiteral("body");
    case TypographyRole::BodySmall:
        return QStringLiteral("bodySmall");
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

QString WidgetStyle::textColorRoleName(ColorRole role)
{
    switch (role) {
    case ColorRole::Text:
        return QStringLiteral("text");
    case ColorRole::TextSecondary:
        return QStringLiteral("secondary");
    case ColorRole::TextMuted:
        return QStringLiteral("muted");
    case ColorRole::TextDisabled:
        return QStringLiteral("disabled");
    default:
        return QString();
    }
}

} // namespace widgets
} // namespace ngstd
