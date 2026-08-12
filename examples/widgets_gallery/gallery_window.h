/******************************************************************************
 * Project: NextGIS widgets gallery
 * Purpose: Corporate widgets style gallery
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_GALLERY_WINDOW_H
#define NGSTD_WIDGETS_GALLERY_WINDOW_H

#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/widget_style.h>

#include <QList>
#include <QMainWindow>
#include <QPointer>

class ColorTokenCard;
class HeroPanel;
class SemanticStateCard;
class QFrame;
class QGridLayout;
class QScrollArea;
class QPropertyAnimation;
class QToolButton;
class QVBoxLayout;
class QWidget;
class QResizeEvent;
class QShowEvent;

namespace ngstd {
namespace widgets {
class ThemeController;
class ThemeSwitch;
class Toast;
class ExpandableSection;
} // namespace widgets
} // namespace ngstd

class GalleryWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit GalleryWindow(
        ngstd::widgets::ThemeMode initialTheme,
        const QString &initialSection = QString(),
        ngstd::widgets::AnimationPolicy animationPolicy =
            ngstd::widgets::AnimationPolicy::System,
        ngstd::widgets::ThemeController *themeController = nullptr,
        QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    QWidget *createHeader();
    HeroPanel *createHero();
    QWidget *createQuickNavigation();
    ngstd::widgets::ExpandableSection *createColorsSection();
    ngstd::widgets::ExpandableSection *createTypographySection();
    ngstd::widgets::ExpandableSection *createComponentsSection();
    ngstd::widgets::ExpandableSection *createBackgroundsSection();
    ngstd::widgets::ExpandableSection *createBrandSection();
    ngstd::widgets::ExpandableSection *createIconsSection();
    ngstd::widgets::ExpandableSection *createMotionSection();
    ngstd::widgets::ExpandableSection *createTokensSection();
    void createStandardControlsDemo(QVBoxLayout *layout);
    void createCardsDemo(QVBoxLayout *layout, QObject *owner);
    void createItemViewsDemo(QVBoxLayout *layout);
    void createWizardDemo(QVBoxLayout *layout);
    QFrame *createComponentPanel(const QString &title,
                                 QVBoxLayout **contentLayout);
    QWidget *createSemanticCard(const QString &title, const QString &textToken,
                                ngstd::widgets::ColorRole textRole,
                                const QString &backgroundToken,
                                ngstd::widgets::ColorRole backgroundRole,
                                ngstd::widgets::SemanticTone tone);
    QWidget *createBackgroundCard(
        const QString &title, const QString &heading,
        const QString &description,
        ngstd::widgets::PageBackgroundVariant variant);

    void addNavigationLink(QGridLayout *layout, int index,
                           const QString &title,
                           ngstd::widgets::ExpandableSection *target);
    void scrollToPosition(int value);
    void scrollToSection(ngstd::widgets::ExpandableSection *target);
    void positionAtSection(ngstd::widgets::ExpandableSection *target);
    void applyTheme();
    void updateThemeVisuals();
    void updateThemeButtons();
    void openWizard();
    void setAllSectionsExpanded(bool expanded);
    void synchronizePageHeight();

    QWidget *m_themeRoot;
    QWidget *m_headerInner;
    QWidget *m_page;
    QScrollArea *m_scrollArea;
    QPointer<QPropertyAnimation> m_scrollAnimation;
    QPointer<ngstd::widgets::ThemeController> m_themeController;
    ngstd::widgets::Toast *m_toast;
    ngstd::widgets::ThemeMode m_themeMode;
    ngstd::widgets::AnimationPolicy m_animationPolicy;
    ngstd::widgets::ColorScheme m_colorScheme;
    ngstd::widgets::ThemeSwitch *m_themeSwitch;
    ngstd::widgets::ExpandableSection *m_initialSectionTarget;
    bool m_initialLayoutPending;
    QList<ngstd::widgets::ExpandableSection *> m_sections;
    QList<HeroPanel *> m_heroPanels;
    QList<ColorTokenCard *> m_colorCards;
    QList<SemanticStateCard *> m_semanticCards;
};

#endif // NGSTD_WIDGETS_GALLERY_WINDOW_H
