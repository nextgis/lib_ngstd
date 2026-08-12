/******************************************************************************
 * Project: NextGIS widgets gallery
 * Purpose: Gallery-only token access
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_GALLERY_TOKENS_H
#define NGSTD_WIDGETS_GALLERY_TOKENS_H

#include <QJsonValue>
#include <QStringList>

enum class GalleryMetric
{
    ContentMaxWidth,
    ViewportMargin,
    HeaderHeight,
    ThemeSwitchWidth,
    ThemeSwitchHeight,
    PagePaddingTop,
    PagePaddingBottom,
    HeroPadding,
    HeroContentMaxWidth,
    HeroTitleSize,
    HeroLeadSize,
    HeroActionSpacing,
    HeroDesktopHeight,
    QuickNavigationMarginTop,
    QuickNavigationPadding,
    SectionMarginTop,
    SectionIntroMarginVertical,
    SectionIntroLineHeight,
    ColorGridSpacing,
    ColorSampleHeight,
    ColorSamplePadding,
    ColorBodyTitleSpacing,
    ComponentGridSpacing,
    ComponentPanelPadding,
    ProductActionPanelPaddingHorizontal,
    ProductActionPanelPaddingVertical,
    ProductActionPanelContentGap,
    ProductActionPanelActionGap,
    ProductActionPanelCopySpacing,
    TypeStagePadding,
    TypeStageDisplaySize,
    TypeSpecimenTokenWidth,
    TypeSpecimenPaddingHorizontal,
    TypeSpecimenPaddingVertical,
    BackgroundDemoHeight,
    BackgroundCaptionHeight,
    MotionProgressTarget
};

class GalleryTokens final
{
public:
    static int metric(GalleryMetric metric);
    static QJsonValue value(const QStringList &path);
};

#endif // NGSTD_WIDGETS_GALLERY_TOKENS_H
