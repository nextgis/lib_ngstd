/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Runtime access to canonical NextGIS design tokens
 *****************************************************************************/
#include <ngstd/widgets/design_tokens.h>

#include "design_tokens_p.h"
#include "generated/token_data_p.h"

#include <QFontDatabase>
#include <QHash>
#include <QMap>
#include <QPointF>
#include <QRegularExpression>

namespace {

class CompiledTokenTable final
{
public:
    CompiledTokenTable()
    {
        using namespace ngstd::widgets::internal;
        for (std::size_t index = 0; index < generatedTokenEntryCount;
             ++index) {
            const GeneratedTokenEntry &entry = generatedTokenEntries[index];
            m_values.insert(QString::fromUtf8(entry.path), &entry);
        }
    }

    const ngstd::widgets::internal::GeneratedTokenEntry *entry(
        const QString &path) const
    {
        return m_values.value(path, nullptr);
    }

    QStringList strings(const QString &path, bool directChildrenOnly) const
    {
        const QString prefix =
            path.isEmpty() ? QString() : path + QLatin1Char('.');
        QMap<QString, QString> values;
        for (auto iterator = m_values.cbegin(); iterator != m_values.cend();
             ++iterator) {
            if (!iterator.key().startsWith(prefix)) continue;
            const QString remainder = iterator.key().mid(prefix.size());
            if (directChildrenOnly && remainder.contains(QLatin1Char('.')))
                continue;
            const ngstd::widgets::internal::GeneratedTokenEntry *tokenEntry =
                iterator.value();
            if (tokenEntry->kind !=
                ngstd::widgets::internal::GeneratedTokenKind::String) {
                continue;
            }
            values.insert(directChildrenOnly
                              ? QStringLiteral("%1").arg(remainder.toInt(), 10,
                                                         10, QLatin1Char('0'))
                              : remainder,
                          QString::fromUtf8(tokenEntry->text));
        }
        return values.values();
    }

    static const CompiledTokenTable &instance()
    {
        static const CompiledTokenTable table;
        return table;
    }

private:
    QHash<QString, const ngstd::widgets::internal::GeneratedTokenEntry *>
        m_values;
};

QString schemeName(ngstd::widgets::ColorScheme scheme)
{
    return scheme == ngstd::widgets::ColorScheme::Dark
               ? QStringLiteral("dark")
               : QStringLiteral("light");
}

QString colorPath(ngstd::widgets::ColorRole role,
                  ngstd::widgets::ColorScheme scheme)
{
    using ngstd::widgets::ColorRole;

    const QString theme = schemeName(scheme);
    switch (role) {
    case ColorRole::Brand:
        return QStringLiteral("color.shared.brand");
    case ColorRole::White:
        return QStringLiteral("color.shared.white");
    case ColorRole::Black:
        return QStringLiteral("color.shared.black");
    case ColorRole::BrandHover:
        return QStringLiteral("color.%1.brandHover").arg(theme);
    case ColorRole::BrandActive:
        return QStringLiteral("color.%1.brandActive").arg(theme);
    case ColorRole::BrandSoft:
        return QStringLiteral("color.%1.brandSoft").arg(theme);
    case ColorRole::Background:
        return QStringLiteral("color.%1.background").arg(theme);
    case ColorRole::PageSoft:
        return QStringLiteral("color.%1.pageSoft").arg(theme);
    case ColorRole::Surface:
        return QStringLiteral("color.%1.surface").arg(theme);
    case ColorRole::SurfaceMuted:
        return QStringLiteral("color.%1.surfaceMuted").arg(theme);
    case ColorRole::SurfaceBrand:
        return QStringLiteral("color.%1.surfaceBrand").arg(theme);
    case ColorRole::SurfaceNested:
        return QStringLiteral("color.%1.surfaceNested").arg(theme);
    case ColorRole::Border:
        return QStringLiteral("color.%1.border").arg(theme);
    case ColorRole::BorderStrong:
        return QStringLiteral("color.%1.borderStrong").arg(theme);
    case ColorRole::Text:
        return QStringLiteral("color.%1.text").arg(theme);
    case ColorRole::TextSecondary:
        return QStringLiteral("color.%1.textSecondary").arg(theme);
    case ColorRole::TextMuted:
        return QStringLiteral("color.%1.textMuted").arg(theme);
    case ColorRole::TextDisabled:
        return QStringLiteral("color.%1.textDisabled").arg(theme);
    case ColorRole::Link:
        return QStringLiteral("color.%1.link").arg(theme);
    case ColorRole::LinkHover:
        return QStringLiteral("color.%1.linkHover").arg(theme);
    case ColorRole::Focus:
        return QStringLiteral("color.%1.focus").arg(theme);
    case ColorRole::FeedbackRing:
        return QStringLiteral("color.%1.feedbackRing").arg(theme);
    case ColorRole::HeroShine:
        return QStringLiteral("product.hero.shine");
    case ColorRole::Success:
        return QStringLiteral("color.%1.success").arg(theme);
    case ColorRole::SuccessSoft:
        return QStringLiteral("color.%1.successSoft").arg(theme);
    case ColorRole::Warning:
        return QStringLiteral("color.%1.warning").arg(theme);
    case ColorRole::WarningSoft:
        return QStringLiteral("color.%1.warningSoft").arg(theme);
    case ColorRole::Danger:
        return QStringLiteral("color.%1.danger").arg(theme);
    case ColorRole::DangerSoft:
        return QStringLiteral("color.%1.dangerSoft").arg(theme);
    case ColorRole::DangerAction:
        return QStringLiteral("color.%1.dangerAction").arg(theme);
    case ColorRole::DangerActionHover:
        return QStringLiteral("color.%1.dangerActionHover").arg(theme);
    case ColorRole::DataSurface:
        return QStringLiteral("product.data.%1.surface").arg(theme);
    case ColorRole::DataSurfaceMuted:
        return QStringLiteral("product.data.%1.surfaceMuted").arg(theme);
    case ColorRole::DataText:
        return QStringLiteral("product.data.%1.text").arg(theme);
    case ColorRole::DataTextSecondary:
        return QStringLiteral("product.data.%1.textSecondary").arg(theme);
    case ColorRole::DataActionBackground:
        return QStringLiteral("product.data.%1.actionBackground").arg(theme);
    case ColorRole::DataActionText:
        return QStringLiteral("product.data.%1.actionText").arg(theme);
    case ColorRole::DataRipple:
        return QStringLiteral("product.data.%1.ripple").arg(theme);
    case ColorRole::DataBorder:
        return QStringLiteral("product.data.%1.border").arg(theme);
    case ColorRole::ToolboxSurface:
        return QStringLiteral("product.toolbox.%1.surface").arg(theme);
    case ColorRole::ToolboxCardSurface:
        return QStringLiteral("product.toolbox.%1.cardSurface").arg(theme);
    case ColorRole::ToolboxText:
        return QStringLiteral("product.toolbox.%1.text").arg(theme);
    case ColorRole::ToolboxTextSecondary:
        return QStringLiteral("product.toolbox.%1.textSecondary").arg(theme);
    case ColorRole::ToolboxAccent:
        return QStringLiteral("product.toolbox.%1.accent").arg(theme);
    case ColorRole::CorporateSurface:
        return QStringLiteral("product.corporate.surface");
    case ColorRole::CorporateGrid:
        return QStringLiteral("product.corporate.grid");
    case ColorRole::CorporateText:
        return QStringLiteral("product.corporate.text");
    case ColorRole::CorporateActionText:
        return QStringLiteral("product.corporate.actionText");
    case ColorRole::CorporateTextSecondary:
        return QStringLiteral("product.corporate.textSecondary");
    case ColorRole::CorporateBorderSecondary:
        return QStringLiteral("product.corporate.borderSecondary");
    case ColorRole::CorporateSurfaceHover:
        return QStringLiteral("product.corporate.surfaceHover");
    case ColorRole::CorporateSurfaceActive:
        return QStringLiteral("product.corporate.surfaceActive");
    case ColorRole::FieldworkSurface:
        return QStringLiteral("product.fieldwork.surface");
    case ColorRole::FieldworkText:
        return QStringLiteral("product.fieldwork.text");
    case ColorRole::FieldworkTextSecondary:
        return QStringLiteral("product.fieldwork.textSecondary");
    case ColorRole::FieldworkSurfaceHover:
        return QStringLiteral("product.fieldwork.surfaceHover");
    case ColorRole::FieldworkSurfaceActive:
        return QStringLiteral("product.fieldwork.surfaceActive");
    }
    return QString();
}

QColor parseColor(const QString &value)
{
    QColor color(value);
    if (color.isValid()) return color;

    static const QRegularExpression expression(
        QStringLiteral("^rgba?\\(\\s*(\\d+)\\s*,\\s*(\\d+)\\s*,\\s*(\\d+)"
                       "(?:\\s*,\\s*([0-9.]+))?\\s*\\)$"));
    const QRegularExpressionMatch match = expression.match(value);
    if (!match.hasMatch()) return QColor();

    const int red = match.captured(1).toInt();
    const int green = match.captured(2).toInt();
    const int blue = match.captured(3).toInt();
    const qreal alpha =
        match.captured(4).isEmpty() ? 1.0 : match.captured(4).toDouble();
    return QColor(red, green, blue, qRound(alpha * 255.0));
}

QString typographyName(ngstd::widgets::TypographyRole role)
{
    using ngstd::widgets::TypographyRole;

    switch (role) {
    case TypographyRole::Display:
        return QStringLiteral("display");
    case TypographyRole::Heading1:
        return QStringLiteral("h1");
    case TypographyRole::Heading2:
        return QStringLiteral("h2");
    case TypographyRole::Heading3:
        return QStringLiteral("h3");
    case TypographyRole::BodyLarge:
        return QStringLiteral("bodyLarge");
    case TypographyRole::Body:
        return QStringLiteral("body");
    case TypographyRole::Control:
        return QStringLiteral("control");
    case TypographyRole::Caption:
        return QStringLiteral("caption");
    case TypographyRole::Mono:
        return QStringLiteral("mono");
    case TypographyRole::QtBody:
        return QStringLiteral("body");
    case TypographyRole::QtHeading1:
        return QStringLiteral("heading1");
    case TypographyRole::QtHeading2:
        return QStringLiteral("heading2");
    }
    return QString();
}

QFont::Weight fontWeight(int cssWeight)
{
    switch (cssWeight) {
    case 100:
        return QFont::Thin;
    case 200:
        return QFont::ExtraLight;
    case 300:
        return QFont::Light;
    case 500:
        return QFont::Medium;
    case 600:
        return QFont::DemiBold;
    case 700:
        return QFont::Bold;
    case 800:
        return QFont::ExtraBold;
    case 900:
        return QFont::Black;
    case 400:
    default:
        return QFont::Normal;
    }
}

QString componentMetricPath(ngstd::widgets::ComponentMetric metric)
{
    using ngstd::widgets::ComponentMetric;

    switch (metric) {
    case ComponentMetric::ButtonIconTextGap:
        return QStringLiteral("desktop.component.button.iconTextGapPx");
    case ComponentMetric::TagHeight:
        return QStringLiteral("desktop.component.tag.heightPx");
    case ComponentMetric::TagPaddingHorizontal:
        return QStringLiteral("desktop.component.tag.paddingHorizontalPx");
    case ComponentMetric::TagContentSpacing:
        return QStringLiteral("desktop.component.tag.contentSpacingPx");
    case ComponentMetric::NoticePaddingHorizontal:
        return QStringLiteral("desktop.component.notice.paddingHorizontalPx");
    case ComponentMetric::NoticePaddingVertical:
        return QStringLiteral("desktop.component.notice.paddingVerticalPx");
    case ComponentMetric::NoticeContentSpacing:
        return QStringLiteral("desktop.component.notice.contentSpacingPx");
    case ComponentMetric::NoticeAccentWidth:
        return QStringLiteral("desktop.component.notice.accentWidthPx");
    case ComponentMetric::SearchFieldTextOffset:
        return QStringLiteral("desktop.component.field.searchPaddingLeftPx");
    case ComponentMetric::SearchFieldIconOffset:
        return QStringLiteral("desktop.component.field.searchIconOffsetPx");
    case ComponentMetric::SearchFieldIconSize:
        return QStringLiteral("desktop.component.field.searchIconSizePx");
    case ComponentMetric::DisclosureHeaderHeight:
        return QStringLiteral("desktop.component.disclosure.headerHeightPx");
    case ComponentMetric::DisclosureContentPadding:
        return QStringLiteral("desktop.component.disclosure.contentPaddingPx");
    case ComponentMetric::ExpandableSectionHeaderHeight:
        return QStringLiteral(
            "desktop.component.expandableSection.headerHeightPx");
    case ComponentMetric::ExpandableSectionHeaderPaddingHorizontal:
        return QStringLiteral(
            "desktop.component.expandableSection.headerPaddingHorizontalPx");
    case ComponentMetric::ExpandableSectionHeaderContentSpacing:
        return QStringLiteral(
            "desktop.component.expandableSection.headerContentSpacingPx");
    case ComponentMetric::ExpandableSectionIconSize:
        return QStringLiteral(
            "desktop.component.expandableSection.iconSizePx");
    case ComponentMetric::ExpandableSectionIconGlyphSize:
        return QStringLiteral(
            "desktop.component.expandableSection.iconGlyphSizePx");
    case ComponentMetric::ExpandableSectionChevronSize:
        return QStringLiteral(
            "desktop.component.expandableSection.chevronSizePx");
    case ComponentMetric::ExpandableSectionTitleDescriptionSpacing:
        return QStringLiteral(
            "desktop.component.expandableSection."
            "titleDescriptionSpacingPx");
    case ComponentMetric::ExpandableSectionTitleLineHeight:
        return QStringLiteral(
            "desktop.component.expandableSection.titleLineHeightPx");
    case ComponentMetric::ExpandableSectionDescriptionLineHeight:
        return QStringLiteral(
            "desktop.component.expandableSection.descriptionLineHeightPx");
    case ComponentMetric::ExpandableSectionContentPaddingHorizontal:
        return QStringLiteral(
            "desktop.component.expandableSection."
            "contentPaddingHorizontalPx");
    case ComponentMetric::ExpandableSectionContentPaddingBottom:
        return QStringLiteral(
            "desktop.component.expandableSection.contentPaddingBottomPx");
    case ComponentMetric::CardContentPadding:
        return QStringLiteral("desktop.component.card.contentPaddingPx");
    case ComponentMetric::CardBorderWidth:
        return QStringLiteral("desktop.component.card.borderWidthPx");
    case ComponentMetric::FocusFrameStrokeWidth:
        return QStringLiteral("desktop.component.focusFrame.strokeWidthPx");
    case ComponentMetric::FocusFrameMargin:
        return QStringLiteral("desktop.component.focusFrame.marginPx");
    case ComponentMetric::SelectionIndicatorSize:
        return QStringLiteral(
            "desktop.component.selectionControl.indicatorSizePx");
    case ComponentMetric::SelectionIndicatorRadius:
        return QStringLiteral(
            "desktop.component.selectionControl.indicatorRadiusPx");
    case ComponentMetric::SelectionIndicatorGap:
        return QStringLiteral(
            "desktop.component.card.selectionIndicatorGapPx");
    case ComponentMetric::SelectionRadioDotSize:
        return QStringLiteral(
            "desktop.component.selectionControl.radioDotSizePx");
    case ComponentMetric::SelectionFeedbackExpansion:
        return QStringLiteral(
            "desktop.component.selectionControl.feedbackExpansionPx");
    case ComponentMetric::SelectionFeedbackStrokeWidth:
        return QStringLiteral(
            "desktop.component.selectionControl.feedbackStrokeWidthPx");
    case ComponentMetric::TabUnderlineHeight:
        return QStringLiteral("desktop.component.tab.underlineHeightPx");
    case ComponentMetric::ItemViewRowHeight:
        return QStringLiteral("desktop.component.itemView.rowHeightPx");
    case ComponentMetric::ItemViewIndentation:
        return QStringLiteral("desktop.component.itemView.indentationPx");
    case ComponentMetric::TableHeaderHeight:
        return QStringLiteral("desktop.component.table.headerHeightPx");
    case ComponentMetric::TableRowHeight:
        return QStringLiteral("desktop.component.table.rowHeightPx");
    case ComponentMetric::TableCellPaddingHorizontal:
        return QStringLiteral(
            "desktop.component.table.cellPaddingHorizontalPx");
    case ComponentMetric::TableCellPaddingVertical:
        return QStringLiteral("desktop.component.table.cellPaddingVerticalPx");
    case ComponentMetric::ScrollBarExtent:
        return QStringLiteral("desktop.component.scrollBar.extentPx");
    case ComponentMetric::ScrollBarMinimumThumb:
        return QStringLiteral("desktop.component.scrollBar.minimumThumbPx");
    case ComponentMetric::ProgressHeight:
        return QStringLiteral("desktop.component.progress.heightPx");
    case ComponentMetric::SpinnerSize:
        return QStringLiteral("desktop.component.spinner.sizePx");
    case ComponentMetric::SpinnerStrokeWidth:
        return QStringLiteral("desktop.component.spinner.strokeWidthPx");
    case ComponentMetric::SpinnerPeriod:
        return QStringLiteral("desktop.component.spinner.periodMs");
    case ComponentMetric::SpinnerLabelSpacing:
        return QStringLiteral("desktop.component.spinner.labelSpacingPx");
    case ComponentMetric::WizardPageMargin:
        return QStringLiteral("desktop.component.wizard.pageMarginPx");
    case ComponentMetric::WizardPageSpacing:
        return QStringLiteral("desktop.component.wizard.pageSpacingPx");
    case ComponentMetric::WizardNavigationMargin:
        return QStringLiteral("desktop.component.wizard.navigationMarginPx");
    case ComponentMetric::WizardNavigationSpacing:
        return QStringLiteral("desktop.component.wizard.navigationSpacingPx");
    case ComponentMetric::WizardButtonMinimumWidth:
        return QStringLiteral("desktop.component.wizard.buttonMinimumWidthPx");
    case ComponentMetric::WizardSidebarWidth:
        return QStringLiteral("desktop.component.wizard.sidebarWidthPx");
    case ComponentMetric::WizardTitleBottomSpacing:
        return QStringLiteral("desktop.component.wizard.titleBottomSpacingPx");
    case ComponentMetric::ToggleStateMinimumWidth:
        return QStringLiteral("desktop.component.toggle.stateMinimumWidthPx");
    case ComponentMetric::ToggleStateHeight:
        return QStringLiteral("desktop.component.toggle.stateHeightPx");
    case ComponentMetric::ComboBoxDropDownWidth:
        return QStringLiteral("desktop.component.comboBox.dropDownWidthPx");
    case ComponentMetric::ComboBoxArrowSize:
        return QStringLiteral("desktop.component.comboBox.arrowSizePx");
    case ComponentMetric::SpinBoxButtonWidth:
        return QStringLiteral("desktop.component.spinBox.buttonWidthPx");
    case ComponentMetric::SpinBoxButtonInset:
        return QStringLiteral("desktop.component.spinBox.buttonInsetPx");
    case ComponentMetric::SpinBoxArrowSize:
        return QStringLiteral("desktop.component.spinBox.arrowSizePx");
    case ComponentMetric::TabPaddingHorizontal:
        return QStringLiteral("desktop.component.tab.paddingHorizontalPx");
    case ComponentMetric::TabPaddingVertical:
        return QStringLiteral("desktop.component.tab.paddingVerticalPx");
    case ComponentMetric::ComboBoxPopupOffset:
        return QStringLiteral("desktop.component.comboBox.popupOffsetPx");
    case ComponentMetric::ComboBoxPopupPadding:
        return QStringLiteral("desktop.component.comboBox.popupPaddingPx");
    case ComponentMetric::ComboBoxPopupItemSpacing:
        return QStringLiteral(
            "desktop.component.comboBox.popupItemSpacingPx");
    }
    return QString();
}

} // namespace

namespace ngstd {
namespace widgets {

namespace internal {

QString tokenString(const QString &path)
{
    const GeneratedTokenEntry *entry =
        CompiledTokenTable::instance().entry(path);
    if (!entry || entry->kind != GeneratedTokenKind::String) return QString();
    return QString::fromUtf8(entry->text);
}

double tokenNumber(const QString &path)
{
    const GeneratedTokenEntry *entry =
        CompiledTokenTable::instance().entry(path);
    if (!entry || entry->kind != GeneratedTokenKind::Number) return 0.0;
    return entry->number;
}

int tokenInteger(const QString &path)
{
    return qRound(tokenNumber(path));
}

bool tokenBoolean(const QString &path)
{
    const GeneratedTokenEntry *entry =
        CompiledTokenTable::instance().entry(path);
    return entry && entry->kind == GeneratedTokenKind::Boolean ? entry->boolean
                                                               : false;
}

QStringList tokenStringList(const QString &path)
{
    return CompiledTokenTable::instance().strings(path, true);
}

QStringList tokenStrings(const QString &path)
{
    return CompiledTokenTable::instance().strings(path, false);
}

} // namespace internal

QString DesignTokens::name()
{
    return internal::tokenString(QStringLiteral("name"));
}

QString DesignTokens::version()
{
    return internal::tokenString(QStringLiteral("version"));
}

QColor DesignTokens::color(ColorRole role, ColorScheme scheme)
{
    return parseColor(internal::tokenString(colorPath(role, scheme)));
}

int DesignTokens::spacing(int level)
{
    if (level < 0 || level > 10) return 0;
    return internal::tokenInteger(QStringLiteral("spacingPx.%1").arg(level));
}

int DesignTokens::radius(RadiusRole role)
{
    QString name;
    switch (role) {
    case RadiusRole::Field:
        name = QStringLiteral("field");
        break;
    case RadiusRole::Button:
        name = QStringLiteral("button");
        break;
    case RadiusRole::Card:
        name = QStringLiteral("card");
        break;
    case RadiusRole::Panel:
        name = QStringLiteral("panel");
        break;
    case RadiusRole::Section:
        name = QStringLiteral("section");
        break;
    case RadiusRole::Hero:
        name = QStringLiteral("hero");
        break;
    case RadiusRole::Pill:
        name = QStringLiteral("pill");
        break;
    }
    return internal::tokenInteger(QStringLiteral("radiusPx.%1").arg(name));
}

int DesignTokens::controlHeight(ControlSize size)
{
    QString name;
    switch (size) {
    case ControlSize::Small:
        name = QStringLiteral("small");
        break;
    case ControlSize::Medium:
        name = QStringLiteral("medium");
        break;
    case ControlSize::Large:
        name = QStringLiteral("large");
        break;
    }
    return internal::tokenInteger(
        QStringLiteral("control.heightPx.%1").arg(name));
}

int DesignTokens::controlPadding()
{
    return internal::tokenInteger(QStringLiteral("control.paddingX"));
}

int DesignTokens::controlIconSize()
{
    return internal::tokenInteger(QStringLiteral("control.iconPx"));
}

int DesignTokens::duration(MotionDuration duration)
{
    QString path;
    switch (duration) {
    case MotionDuration::Instant:
        path = QStringLiteral("motion.instantMs");
        break;
    case MotionDuration::Fast:
        path = QStringLiteral("motion.fastMs");
        break;
    case MotionDuration::Normal:
        path = QStringLiteral("motion.normalMs");
        break;
    case MotionDuration::Slow:
        path = QStringLiteral("motion.slowMs");
        break;
    }
    return internal::tokenInteger(path);
}

qreal DesignTokens::decorationOpacity(ColorScheme scheme)
{
    return internal::tokenNumber(
        QStringLiteral("effect.%1.decorationOpacity").arg(schemeName(scheme)));
}

QEasingCurve DesignTokens::easingCurve(MotionEasing easing)
{
    QString path;
    QEasingCurve::Type fallback = QEasingCurve::OutCubic;
    switch (easing) {
    case MotionEasing::Standard:
        path = QStringLiteral("motion.easingStandard");
        break;
    case MotionEasing::Enter:
        path = QStringLiteral("motion.easingEnter");
        break;
    case MotionEasing::Exit:
        path = QStringLiteral("motion.easingExit");
        fallback = QEasingCurve::InCubic;
        break;
    }
    const QString valueString = internal::tokenString(path);
    static const QRegularExpression expression(
        QStringLiteral("^cubic-bezier\\(([-0-9.]+),\\s*([-0-9.]+),\\s*"
                       "([-0-9.]+),\\s*([-0-9.]+)\\)$"));
    const QRegularExpressionMatch match = expression.match(valueString);
    if (!match.hasMatch()) return QEasingCurve(fallback);

    QEasingCurve curve(QEasingCurve::BezierSpline);
    curve.addCubicBezierSegment(
        QPointF(match.captured(1).toDouble(), match.captured(2).toDouble()),
        QPointF(match.captured(3).toDouble(), match.captured(4).toDouble()),
        QPointF(1.0, 1.0));
    return curve;
}

TypographyToken DesignTokens::typography(TypographyRole role)
{
    TypographyToken token;
    const bool qtScale = role == TypographyRole::Mono ||
                         role == TypographyRole::QtBody ||
                         role == TypographyRole::QtHeading1 ||
                         role == TypographyRole::QtHeading2;
    const bool heading =
        role == TypographyRole::Display || role == TypographyRole::Heading1 ||
        role == TypographyRole::Heading2 || role == TypographyRole::Heading3 ||
        role == TypographyRole::QtHeading1 ||
        role == TypographyRole::QtHeading2;
    const QString familyName =
        role == TypographyRole::Mono
            ? QStringLiteral("mono")
            : (heading ? QStringLiteral("heading") : QStringLiteral("body"));
    const QString desktopFamily = internal::tokenString(
        QStringLiteral("desktop.fontFamilies.%1").arg(familyName));
    if (!desktopFamily.isEmpty()) token.families.append(desktopFamily);
    const QStringList fallbackFamilies = internal::tokenStringList(
        QStringLiteral("typography.families.%1").arg(familyName));
    for (const QString &family : fallbackFamilies) {
        if (!token.families.contains(family)) token.families.append(family);
    }

    const QString scaleName =
        qtScale ? QStringLiteral("qtScale") : QStringLiteral("scale");
    const QString scalePath = QStringLiteral("typography.%1.%2")
                                  .arg(scaleName, typographyName(role));
    token.pixelSize =
        internal::tokenInteger(scalePath + QStringLiteral(".sizePx"));
    token.weight = fontWeight(
        internal::tokenInteger(scalePath + QStringLiteral(".weight")));
    const double relativeLineHeight =
        internal::tokenNumber(scalePath + QStringLiteral(".lineHeight"));
    const double pixelLineHeight =
        internal::tokenNumber(scalePath + QStringLiteral(".lineHeightPx"));
    if (relativeLineHeight > 0.0)
        token.lineHeight = relativeLineHeight;
    else if (pixelLineHeight > 0.0)
        token.lineHeight = pixelLineHeight;
    return token;
}

QFont DesignTokens::font(TypographyRole role)
{
    const TypographyToken token = typography(role);
    QFont result;
    if (!token.families.isEmpty()) result.setFamily(token.families.first());
    result.setPixelSize(token.pixelSize);
    result.setWeight(token.weight);
    if (role == TypographyRole::Mono) {
        const QFontDatabase fontDatabase;
        const QStringList availableFamilies = fontDatabase.families();
        QString resolvedFamily;
        for (const QString &family : token.families) {
            if (family.compare(QStringLiteral("monospace"),
                               Qt::CaseInsensitive) == 0) {
                continue;
            }
            if (availableFamilies.contains(family, Qt::CaseInsensitive)) {
                resolvedFamily = family;
                break;
            }
        }
        if (resolvedFamily.isEmpty()) {
            resolvedFamily =
                QFontDatabase::systemFont(QFontDatabase::FixedFont).family();
        }
        result.setFamily(resolvedFamily);
        result.setStyleHint(QFont::TypeWriter);
        result.setFixedPitch(true);
    }
    return result;
}

int DesignTokens::componentMetric(ComponentMetric metric)
{
    return internal::tokenInteger(componentMetricPath(metric));
}

} // namespace widgets
} // namespace ngstd
