/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Public enum and flag declarations
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_TYPES_H
#define NGSTD_WIDGETS_TYPES_H

#include <ngstd/widgets/widgets.h>

#include <QMetaType>
#include <QObject>

namespace ngstd {
namespace widgets {

Q_NAMESPACE_EXPORT(NGSTD_WIDGETS_EXPORT)

enum class ColorScheme : quint8
{
    Light,
    Dark
};
Q_ENUM_NS(ColorScheme)

enum class ThemeMode : quint8
{
    System,
    Light,
    Dark
};
Q_ENUM_NS(ThemeMode)

enum class AnimationPolicy : quint8
{
    System,
    Enabled,
    Disabled
};
Q_ENUM_NS(AnimationPolicy)

enum class ThemeFeature : quint32
{
    None = 0x0000,
    StandardControls = 0x0001,
    SemanticComponents = 0x0002,
    DialogButtonBoxes = 0x0004,
    ComboBoxAdapters = 0x0008,
    WizardAdapters = 0x0010
};
Q_ENUM_NS(ThemeFeature)
Q_DECLARE_FLAGS(ThemeFeatures, ThemeFeature)
Q_FLAG_NS(ThemeFeatures)

enum class ColorRole : quint8
{
    Brand,
    White,
    Black,
    BrandHover,
    BrandActive,
    BrandSoft,
    Background,
    PageSoft,
    Surface,
    SurfaceMuted,
    SurfaceBrand,
    SurfaceNested,
    Border,
    BorderStrong,
    Text,
    TextSecondary,
    TextMuted,
    TextDisabled,
    Link,
    LinkHover,
    Focus,
    FeedbackRing,
    HeroShine,
    Success,
    SuccessSoft,
    Warning,
    WarningSoft,
    Danger,
    DangerSoft,
    DangerAction,
    DangerActionHover,
    DataSurface,
    DataSurfaceMuted,
    DataText,
    DataTextSecondary,
    DataActionBackground,
    DataActionText,
    DataRipple,
    DataBorder,
    ToolboxSurface,
    ToolboxCardSurface,
    ToolboxText,
    ToolboxTextSecondary,
    ToolboxAccent,
    CorporateSurface,
    CorporateGrid,
    CorporateText,
    CorporateActionText,
    CorporateTextSecondary,
    CorporateBorderSecondary,
    CorporateSurfaceHover,
    CorporateSurfaceActive,
    FieldworkSurface,
    FieldworkText,
    FieldworkTextSecondary,
    FieldworkSurfaceHover,
    FieldworkSurfaceActive
};
Q_ENUM_NS(ColorRole)

enum class RadiusRole : quint8
{
    Field,
    Button,
    Card,
    Panel,
    Section,
    Hero,
    Pill
};
Q_ENUM_NS(RadiusRole)

enum class ControlSize : quint8
{
    Small,
    Medium,
    Large
};
Q_ENUM_NS(ControlSize)

enum class TypographyRole : quint8
{
    Display,
    Heading1,
    Heading2,
    Heading3,
    BodyLarge,
    Body,
    Control,
    Caption,
    Mono,
    QtBody,
    QtHeading1,
    QtHeading2
};
Q_ENUM_NS(TypographyRole)

enum class MotionDuration : quint8
{
    Instant,
    Fast,
    Normal,
    Slow
};
Q_ENUM_NS(MotionDuration)

enum class MotionEasing : quint8
{
    Standard,
    Enter,
    Exit
};
Q_ENUM_NS(MotionEasing)

struct MotionSpec
{
    MotionDuration duration = MotionDuration::Normal;
    MotionEasing easing = MotionEasing::Standard;
};

enum class ComponentMetric : quint8
{
    ButtonIconTextGap,
    TagHeight,
    TagPaddingHorizontal,
    TagContentSpacing,
    NoticePaddingHorizontal,
    NoticePaddingVertical,
    NoticeContentSpacing,
    NoticeAccentWidth,
    SearchFieldTextOffset,
    SearchFieldIconOffset,
    SearchFieldIconSize,
    DisclosureHeaderHeight,
    DisclosureContentPadding,
    ExpandableSectionHeaderHeight,
    ExpandableSectionHeaderPaddingHorizontal,
    ExpandableSectionHeaderContentSpacing,
    ExpandableSectionIconSize,
    ExpandableSectionIconGlyphSize,
    ExpandableSectionChevronSize,
    ExpandableSectionTitleDescriptionSpacing,
    ExpandableSectionTitleLineHeight,
    ExpandableSectionDescriptionLineHeight,
    ExpandableSectionContentPaddingHorizontal,
    ExpandableSectionContentPaddingBottom,
    CardContentPadding,
    CardBorderWidth,
    FocusFrameStrokeWidth,
    FocusFrameMargin,
    SelectionIndicatorSize,
    SelectionIndicatorRadius,
    SelectionIndicatorGap,
    SelectionRadioDotSize,
    SelectionFeedbackExpansion,
    SelectionFeedbackStrokeWidth,
    TabUnderlineHeight,
    ItemViewRowHeight,
    ItemViewIndentation,
    TableHeaderHeight,
    TableRowHeight,
    TableCellPaddingHorizontal,
    TableCellPaddingVertical,
    ScrollBarExtent,
    ScrollBarMinimumThumb,
    ProgressHeight,
    SpinnerSize,
    SpinnerStrokeWidth,
    SpinnerPeriod,
    SpinnerLabelSpacing,
    WizardPageMargin,
    WizardPageSpacing,
    WizardNavigationMargin,
    WizardNavigationSpacing,
    WizardButtonMinimumWidth,
    WizardSidebarWidth,
    WizardTitleBottomSpacing,
    ToggleStateMinimumWidth,
    ToggleStateHeight,
    ComboBoxDropDownWidth,
    ComboBoxArrowSize,
    SpinBoxButtonWidth,
    SpinBoxButtonInset,
    SpinBoxArrowSize,
    TabPaddingHorizontal,
    TabPaddingVertical,
    ComboBoxPopupOffset,
    ComboBoxPopupPadding,
    ComboBoxPopupItemSpacing
};
Q_ENUM_NS(ComponentMetric)

enum class ButtonVariant : quint8
{
    Default,
    Primary,
    Secondary,
    Text,
    Ghost,
    Danger,
    Icon,
    Hero,
    Trial,
    DataFilled,
    DataOutline,
    OnBrand,
    OnBrandSecondary,
    Photo,
    PhotoText
};
Q_ENUM_NS(ButtonVariant)

enum class SemanticTone : quint8
{
    Neutral,
    Information,
    Success,
    Warning,
    Danger
};
Q_ENUM_NS(SemanticTone)

enum class CardVariant : quint8
{
    Default,
    Data,
    Toolbox,
    ToolboxNew,
    Media,
    Background,
    Panel,
    Selectable
};
Q_ENUM_NS(CardVariant)

enum class PageBackgroundVariant : quint8
{
    Main,
    Toolbox,
    Data,
    Workspace,
    Corporate,
    Fieldwork
};
Q_ENUM_NS(PageBackgroundVariant)

enum class PageBackgroundGradient : quint8
{
    None,
    Down,
    Up,
    Center,
    ToCenter
};
Q_ENUM_NS(PageBackgroundGradient)

enum class PageBackgroundCornerMode : quint8
{
    Square,
    Rounded,
    RoundedTop
};
Q_ENUM_NS(PageBackgroundCornerMode)

enum class ComboBoxPopupPlacement : quint8
{
    Below,
    Above
};
Q_ENUM_NS(ComboBoxPopupPlacement)

enum class ComboBoxPopupAlignment : quint8
{
    Left,
    Center,
    Right
};
Q_ENUM_NS(ComboBoxPopupAlignment)

enum class IconRole : quint8
{
    AlertTriangle,
    Check,
    CheckCircle,
    ChevronDown,
    CloseCircle,
    Copy,
    Download,
    Ellipsis,
    Flame,
    Gradient,
    Grid,
    Information,
    Layers,
    Menu,
    Monitor,
    Moon,
    Search,
    Sun
};
Q_ENUM_NS(IconRole)

enum class LogoRole : quint8
{
    Horizontal,
    HorizontalOnDark,
    Symbol,
    SymbolOnDark,
    MonoBrand,
    MonoDark,
    MonoLight
};
Q_ENUM_NS(LogoRole)

} // namespace widgets
} // namespace ngstd

Q_DECLARE_OPERATORS_FOR_FLAGS(ngstd::widgets::ThemeFeatures)

Q_DECLARE_METATYPE(ngstd::widgets::AnimationPolicy)
Q_DECLARE_METATYPE(ngstd::widgets::ButtonVariant)
Q_DECLARE_METATYPE(ngstd::widgets::CardVariant)
Q_DECLARE_METATYPE(ngstd::widgets::ColorScheme)
Q_DECLARE_METATYPE(ngstd::widgets::ColorRole)
Q_DECLARE_METATYPE(ngstd::widgets::ComboBoxPopupAlignment)
Q_DECLARE_METATYPE(ngstd::widgets::ComboBoxPopupPlacement)
Q_DECLARE_METATYPE(ngstd::widgets::ComponentMetric)
Q_DECLARE_METATYPE(ngstd::widgets::ControlSize)
Q_DECLARE_METATYPE(ngstd::widgets::IconRole)
Q_DECLARE_METATYPE(ngstd::widgets::LogoRole)
Q_DECLARE_METATYPE(ngstd::widgets::MotionDuration)
Q_DECLARE_METATYPE(ngstd::widgets::MotionEasing)
Q_DECLARE_METATYPE(ngstd::widgets::MotionSpec)
Q_DECLARE_METATYPE(ngstd::widgets::PageBackgroundGradient)
Q_DECLARE_METATYPE(ngstd::widgets::PageBackgroundCornerMode)
Q_DECLARE_METATYPE(ngstd::widgets::PageBackgroundVariant)
Q_DECLARE_METATYPE(ngstd::widgets::RadiusRole)
Q_DECLARE_METATYPE(ngstd::widgets::SemanticTone)
Q_DECLARE_METATYPE(ngstd::widgets::ThemeFeature)
Q_DECLARE_METATYPE(ngstd::widgets::ThemeFeatures)
Q_DECLARE_METATYPE(ngstd::widgets::ThemeMode)
Q_DECLARE_METATYPE(ngstd::widgets::TypographyRole)

#endif // NGSTD_WIDGETS_TYPES_H
