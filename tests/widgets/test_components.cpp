#include <ngstd/widgets/basic_components.h>
#include <ngstd/widgets/button.h>
#include <ngstd/widgets/card.h>
#include <ngstd/widgets/combo_box.h>
#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/disclosure.h>
#include <ngstd/widgets/expandable_section.h>
#include <ngstd/widgets/icons.h>
#include <ngstd/widgets/motion_adapter.h>
#include <ngstd/widgets/navigation_components.h>
#include <ngstd/widgets/page_background.h>
#include <ngstd/widgets/theme.h>
#include <ngstd/widgets/theme_switch.h>
#include <ngstd/widgets/view_components.h>
#include <ngstd/widgets/widget_style.h>

#include <QAbstractAnimation>
#include <QAccessible>
#include <QAccessibleInterface>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QListView>
#include <QPainter>
#include <QPixmap>
#include <QPointer>
#include <QProxyStyle>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalSpy>
#include <QTest>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariant>
#include <QVariantAnimation>

using namespace ngstd::widgets;

namespace {

class ButtonPaintRecorder final : public QProxyStyle
{
public:
    ButtonPaintRecorder() : QProxyStyle(QStringLiteral("Fusion")) {}

    void drawControl(ControlElement element, const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const override
    {
        if (element == QStyle::CE_PushButtonBevel && option)
            bevelRectangles.append(option->rect);
        QProxyStyle::drawControl(element, option, painter, widget);
    }

    void drawItemText(QPainter *painter, const QRect &rectangle, int flags,
                      const QPalette &palette, bool enabled,
                      const QString &text,
                      QPalette::ColorRole textRole) const override
    {
        labelText = text;
        labelTextRectangle = rectangle;
        labelColor = palette.color(textRole);
        QProxyStyle::drawItemText(painter, rectangle, flags, palette, enabled,
                                  text, textRole);
    }

    void drawItemPixmap(QPainter *painter, const QRect &rectangle,
                        int alignment, const QPixmap &pixmap) const override
    {
        labelPixmapPainted = !pixmap.isNull();
        labelPixmapRectangle = rectangle;
        QProxyStyle::drawItemPixmap(painter, rectangle, alignment, pixmap);
    }

    mutable QString labelText;
    mutable QRect labelTextRectangle;
    mutable QRect labelPixmapRectangle;
    mutable QColor labelColor;
    mutable QList<QRect> bevelRectangles;
    mutable bool labelPixmapPainted = false;
};

class InspectableSearchField final : public SearchField
{
public:
    using QLineEdit::cursorRect;
};

} // namespace

class ComponentsTest final : public QObject
{
    Q_OBJECT

private slots:
    void iconLabelUsesThemeTokens();
    void buttonPaintsPublishedLogoRole();
    void buttonSupportsLeadingAndTrailingIcons();
    void cardOwnershipIsSymmetric();
    void cardButtonUsesNativeButtonSemantics();
    void cardButtonOnlySelectableVariantToggles();
    void cardButtonReservesLeadingIndicatorSpace();
    void cardButtonAlignsSelectionIndicatorWithTitle();
    void cardButtonOwnsAnimatedExpandedBody();
    void cardButtonUsesTokenStateColors();
    void cardButtonUsesDarkToolboxDecoration();
    void cardButtonAnimatesStateColors();
    void cardButtonAnimatesSelectionFocusAndRipple();
    void revealOwnershipAndReducedMotion();
    void disclosureExposesExpandedState();
    void disclosureRestoresGeometryAfterAnimation();
    void disclosureSeparatorMatchesHeaderBoundary();
    void expandableSectionOwnsInteractionAndLayout();
    void pageBackgroundAppliesVariantDefaults();
    void pageBackgroundExposesCornerMode();
    void motionAdapterUsesTypedTokensAndPolicy();
    void navigationComponentsOwnOneLogicalBoundary();
    void embeddedSurfaceOwnsNoNestedFrame();
    void wizardPageLayoutUsesDesignTokens();
    void searchFieldUsesLeadingAction();
    void buttonAnimatesVisualStates();
    void primaryButtonTransitionNeverExposesWhiteBackground();
    void disabledButtonIgnoresHoverMotion();
    void iconButtonRemainsSquareInStretchingLayouts();
    void secondaryIconOnlyButtonUsesSquareToken();
    void iconButtonKeepsPressedIconVisible();
    void promotionalButtonsAnimateLikeReference();
    void dataButtonUsesRippleMotion();
    void checkableButtonUpdatesRoleIconColor();
    void loadingButtonPreservesIcon();
    void loadingButtonPaintsLabelAndIcon();
    void loadingButtonSeparatesSpinnerFromText();
    void loadingButtonExposesBusyState();
    void comboBoxPopupFitsAllVisibleRows();
    void comboBoxSecondClickClosesPopup();
    void comboBoxSizeHintFitsLongestItem();
    void spinnerUsesElapsedTime();
    void spinnerRunsWithSystemPolicy();
    void spinnerHonorsReducedMotion();
    void tableOwnershipIsSymmetric();
    void tableCellContentFitsHost();
    void themeSwitchHasAccessibleButtons();
    void themeSwitchSupportsKeyboardSelection();
    void themeSwitchUsesPulseMotion();
};

void ComponentsTest::iconLabelUsesThemeTokens()
{
    IconLabel emptyLabel;
    QVERIFY(!emptyLabel.hasIconRole());
    QVERIFY(emptyLabel.pixmap(Qt::ReturnByValue).isNull());
    QCOMPARE(emptyLabel.sizeHint(), QSize(DesignTokens::controlIconSize(),
                                          DesignTokens::controlIconSize()));

    IconLabel label(IconRole::HardDrive);
    QCOMPARE(label.iconRole(), IconRole::HardDrive);
    QVERIFY(label.hasIconRole());
    QCOMPARE(label.colorRole(), ColorRole::TextSecondary);
    QCOMPARE(label.iconSize(), DesignTokens::controlIconSize());
    QCOMPARE(label.sizeHint(), QSize(DesignTokens::controlIconSize(),
                                     DesignTokens::controlIconSize()));
    QVERIFY(!label.pixmap(Qt::ReturnByValue).isNull());

    label.setIconSize(DesignTokens::spacing(6));
    QCOMPARE(label.iconSize(), DesignTokens::spacing(6));
    QCOMPARE(label.sizeHint(),
             QSize(DesignTokens::spacing(6), DesignTokens::spacing(6)));
    label.resetIconSize();
    QCOMPARE(label.iconSize(), DesignTokens::controlIconSize());
    label.clearIconRole();
    QVERIFY(!label.hasIconRole());
    QVERIFY(label.pixmap(Qt::ReturnByValue).isNull());
    QCOMPARE(label.sizeHint(), QSize(DesignTokens::controlIconSize(),
                                     DesignTokens::controlIconSize()));
}

void ComponentsTest::buttonPaintsPublishedLogoRole()
{
    Button button;
    button.setVariant(ButtonVariant::Ghost);
    button.setLogoRole(LogoRole::Horizontal, QSize(142, 20));
    button.resize(button.sizeHint());

    QVERIFY(button.hasLogoRole());
    QCOMPARE(button.logoRole(), LogoRole::Horizontal);
    QCOMPARE(button.logoSize(), QSize(142, 20));
    QVERIFY(button.minimumSizeHint().width() >= 158);

    const QImage image = button.grab().toImage();
    int logoColorPixels = 0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const QColor pixel = image.pixelColor(x, y);
            if (pixel.alpha() > 0 &&
                (pixel.lightness() < 128 || pixel.blue() > pixel.red() + 32)) {
                ++logoColorPixels;
            }
        }
    }
    QVERIFY(logoColorPixels > 100);

    button.clearLogoRole();
    QVERIFY(!button.hasLogoRole());
}

void ComponentsTest::buttonSupportsLeadingAndTrailingIcons()
{
    ButtonPaintRecorder style;
    Button button(QStringLiteral("Next"));
    button.setStyle(&style);
    button.setIconRole(IconRole::ChevronRight);
    button.setIconPlacement(ButtonIconPlacement::Trailing);
    button.resize(button.sizeHint());
    button.show();
    QPixmap image(button.size());
    image.fill(Qt::transparent);
    button.render(&image);

    QCOMPARE(button.iconPlacement(), ButtonIconPlacement::Trailing);
    QVERIFY(style.labelPixmapPainted);
    QVERIFY(style.labelPixmapRectangle.left() >
            style.labelTextRectangle.right());

    style.labelPixmapPainted = false;
    button.setIconRole(IconRole::ChevronLeft);
    button.resetIconPlacement();
    image.fill(Qt::transparent);
    button.render(&image);

    QCOMPARE(button.iconPlacement(), ButtonIconPlacement::Leading);
    QVERIFY(style.labelPixmapPainted);
    QVERIFY(style.labelPixmapRectangle.right() <
            style.labelTextRectangle.left());
}

void ComponentsTest::cardOwnershipIsSymmetric()
{
    Card card;
    QWidget *top = new QWidget;
    QWidget *body = new QWidget;
    card.setTopWidget(top);
    card.setBodyWidget(body);
    QCOMPARE(top->parentWidget(), &card);
    QCOMPARE(body->parentWidget(), &card);
    QCOMPARE(card.takeTopWidget(), top);
    QCOMPARE(card.takeBodyWidget(), body);
    QVERIFY(!top->parent());
    QVERIFY(!body->parent());
    delete top;
    delete body;
}

void ComponentsTest::cardButtonUsesNativeButtonSemantics()
{
    CardButton first;
    CardButton second;
    first.setAutoExclusive(true);
    second.setAutoExclusive(true);
    QButtonGroup group;
    group.setExclusive(true);
    group.addButton(&first);
    group.addButton(&second);
    first.resize(180, 80);
    first.show();
    QTest::mouseClick(&first, Qt::LeftButton);
    QVERIFY(first.isChecked());
    QVERIFY(!second.isChecked());

    second.show();
    second.setFocus();
    QTest::keyClick(&second, Qt::Key_Space);
    QVERIFY(second.isChecked());
    QVERIFY(!first.isChecked());

    QAccessibleInterface *interface =
        QAccessible::queryAccessibleInterface(&second);
    QVERIFY(interface);
    QCOMPARE(interface->role(), QAccessible::RadioButton);
    QCOMPARE(interface->text(QAccessible::Value), QStringLiteral("checked"));
    QVERIFY(interface->state().checkable);
    QVERIFY(interface->state().checked);
    auto *actions = static_cast<QAccessibleActionInterface *>(
        interface->interface_cast(QAccessible::ActionInterface));
    QVERIFY(actions);
    actions->doAction(QAccessibleActionInterface::toggleAction());
    QVERIFY(second.isChecked());

    group.setExclusive(false);
    first.setAutoExclusive(false);
    second.setAutoExclusive(false);
    QCOMPARE(interface->role(), QAccessible::CheckBox);
    actions->doAction(QAccessibleActionInterface::toggleAction());
    QVERIFY(!second.isChecked());
    QCOMPARE(interface->text(QAccessible::Value), QStringLiteral("unchecked"));
}

void ComponentsTest::cardButtonOnlySelectableVariantToggles()
{
    CardButton card;
    QVERIFY(card.isCheckable());
    card.setVariant(CardVariant::Data);
    QVERIFY(!card.isCheckable());
    card.show();
    QTest::mouseClick(&card, Qt::LeftButton);
    QVERIFY(!card.isChecked());

    card.setVariant(CardVariant::Selectable);
    QVERIFY(card.isCheckable());
    QTest::mouseClick(&card, Qt::LeftButton);
    QVERIFY(card.isChecked());
}

void ComponentsTest::cardButtonReservesLeadingIndicatorSpace()
{
    CardButton card;
    const int padding =
        DesignTokens::componentMetric(ComponentMetric::CardContentPadding);
    const int reserved =
        DesignTokens::componentMetric(
            ComponentMetric::SelectionIndicatorSize) +
        DesignTokens::componentMetric(ComponentMetric::SelectionIndicatorGap);
    QMargins margins = card.contentLayout()->contentsMargins();
    QCOMPARE(margins.left(), padding + reserved);
    QCOMPARE(margins.right(), padding);

    card.setLayoutDirection(Qt::RightToLeft);
    margins = card.contentLayout()->contentsMargins();
    QCOMPARE(margins.left(), padding);
    QCOMPARE(margins.right(), padding + reserved);

    card.setVariant(CardVariant::Media);
    margins = card.contentLayout()->contentsMargins();
    QCOMPARE(margins.left(), padding);
    QCOMPARE(margins.right(), padding);
}

void ComponentsTest::cardButtonAlignsSelectionIndicatorWithTitle()
{
    const int size =
        DesignTokens::componentMetric(ComponentMetric::SelectionIndicatorSize);
    const int padding =
        DesignTokens::componentMetric(ComponentMetric::CardContentPadding);
    const int indicatorCenterX = padding + size / 2;
    const QColor indicatorColor =
        DesignTokens::color(ColorRole::White, ColorScheme::Light);
    const auto containsColor = [](const QImage &image, const QPoint &center,
                                  const QColor &color) {
        for (int y = center.y() - 5; y <= center.y() + 5; ++y) {
            for (int x = center.x() - 5; x <= center.x() + 5; ++x) {
                if (image.valid(x, y) && image.pixelColor(x, y) == color)
                    return true;
            }
        }
        return false;
    };
    const auto verifyAlignment = [&](bool radio) {
        CardButton card;
        card.setProperty("_ngstdAnimationPolicy", QStringLiteral("disabled"));
        card.setAutoExclusive(radio);
        card.setChecked(true);
        card.setProperty("_ngstdColorScheme", QStringLiteral("light"));
        QWidget *titleHost = new QWidget;
        QVBoxLayout *titleLayout = new QVBoxLayout(titleHost);
        titleLayout->setContentsMargins(0, 0, 0, 0);
        QLabel *title = new QLabel(QStringLiteral("Title"), titleHost);
        title->setFixedHeight(24);
        titleLayout->addWidget(title);
        card.setTopWidget(titleHost);
        card.setSelectionIndicatorAnchor(title);
        QCOMPARE(card.selectionIndicatorAnchor(), title);
        QWidget *body = new QWidget;
        body->setMinimumHeight(72);
        card.setBodyWidget(body);
        card.contentLayout()->addStretch();
        card.resize(240, 144);
        card.show();
        QCoreApplication::processEvents();
        QCOMPARE(body->geometry().top() - titleHost->geometry().bottom() - 1,
                 card.contentLayout()->spacing());

        QPixmap pixmap(card.size());
        pixmap.fill(Qt::transparent);
        card.render(&pixmap);
        const QImage image = pixmap.toImage();
        const QPoint titleOrigin = title->mapTo(&card, QPoint(0, 0));
        const QPoint titleCenter(indicatorCenterX,
                                 titleOrigin.y() + title->height() / 2);
        const QPoint cardCenter(indicatorCenterX, card.height() / 2);
        QVERIFY(containsColor(image, titleCenter, indicatorColor));
        QVERIFY(!containsColor(image, cardCenter, indicatorColor));
    };
    verifyAlignment(false);
    verifyAlignment(true);
}

void ComponentsTest::cardButtonOwnsAnimatedExpandedBody()
{
    QWidget host;
    QVBoxLayout hostLayout(&host);
    CardButton card;
    hostLayout.addWidget(&card, 0, Qt::AlignTop);
    card.setProperty("_ngstdAnimationPolicy", QStringLiteral("disabled"));
    QWidget *body = new QWidget;
    QWidget *expandedBody = new QWidget;
    body->setFixedHeight(40);
    expandedBody->setFixedHeight(80);
    QSignalSpy expandedSpy(&card, &CardButton::expandedChanged);

    card.setBodyWidget(body);
    card.setExpandedBodyWidget(expandedBody);
    QCOMPARE(card.expandedBodyWidget(), expandedBody);
    QVERIFY(card.isAncestorOf(expandedBody));
    QFrame *expandedDivider = card.findChild<QFrame *>(
        QStringLiteral("_ngstdCardExpandedBodyDivider"));
    QVERIFY(expandedDivider);
    QVERIFY(expandedDivider->isHidden());
    QVERIFY(!card.isExpanded());
    host.resize(320, 300);
    host.show();
    QApplication::processEvents();
    const int collapsedHeight = card.height();
    const int collapsedSizeHintHeight = card.sizeHint().height();
    const int collapsedMaximumHeight = card.maximumHeight();
    card.setExpanded(true);
    QApplication::processEvents();
    QVERIFY(card.isExpanded());
    QVERIFY(card.height() > collapsedHeight);
    QVERIFY(card.sizeHint().height() > collapsedSizeHintHeight);
    QCOMPARE(card.maximumHeight(), collapsedMaximumHeight);
    QVERIFY(!expandedDivider->isHidden());
    QCOMPARE(expandedDivider->property("_ngstdRole").toString(),
             QStringLiteral("divider"));
    const QRect expandedRectangle(expandedBody->mapTo(&card, QPoint(0, 0)),
                                  expandedBody->size());
    QVERIFY(card.rect().contains(expandedRectangle));
    QCOMPARE(expandedSpy.count(), 1);
    card.collapse();
    QVERIFY(!card.isExpanded());
    QVERIFY(expandedDivider->isHidden());
    QCOMPARE(card.sizeHint().height(), collapsedSizeHintHeight);
    QCOMPARE(card.maximumHeight(), collapsedMaximumHeight);
    QCOMPARE(expandedSpy.count(), 2);

    QWidget *replacementBody = new QWidget;
    card.setBodyWidget(replacementBody);
    QCOMPARE(card.bodyWidget(), replacementBody);
    QCOMPARE(card.contentLayout()->indexOf(replacementBody), 0);
    QWidget *expandedBodyReveal =
        card.findChild<QWidget *>(QStringLiteral("_ngstdCardExpandedBody"));
    QVERIFY(expandedBodyReveal);
    QVERIFY(card.contentLayout()->indexOf(expandedBodyReveal) > 0);
    QVERIFY(card.contentLayout()->indexOf(expandedDivider)
            < card.contentLayout()->indexOf(expandedBodyReveal));

    QCOMPARE(card.takeExpandedBodyWidget(), expandedBody);
    QVERIFY(!expandedBody->parent());
    delete expandedBody;

    QScrollArea scrollArea;
    scrollArea.setWidgetResizable(true);
    QWidget *scrollContent = new QWidget;
    scrollContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    QVBoxLayout *scrollContentLayout = new QVBoxLayout(scrollContent);
    scrollContentLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    CardButton *scrollCard = new CardButton;
    scrollCard->setProperty("_ngstdAnimationPolicy",
                            QStringLiteral("enabled"));
    QWidget *scrollCardHeader = new QWidget;
    QWidget *scrollCardBody = new QWidget;
    scrollCardHeader->setFixedHeight(40);
    scrollCardBody->setFixedHeight(220);
    scrollCard->setTopWidget(scrollCardHeader);
    scrollCard->setExpandedBodyWidget(scrollCardBody);
    QCOMPARE(scrollCard->sizePolicy().verticalPolicy(), QSizePolicy::Minimum);
    scrollContentLayout->addWidget(scrollCard);
    QWidget *followingContent = new QWidget;
    followingContent->setFixedHeight(150);
    scrollContentLayout->addWidget(followingContent);
    scrollContentLayout->addStretch();
    scrollArea.setWidget(scrollContent);
    scrollArea.resize(320, 300);
    scrollArea.show();
    QApplication::processEvents();

    scrollCard->setExpanded(true);
    RevealWidget *scrollReveal = scrollCard->findChild<RevealWidget *>(
        QStringLiteral("_ngstdCardExpandedBody"));
    QVERIFY(scrollReveal);
    QTRY_VERIFY_WITH_TIMEOUT(
        qFuzzyCompare(scrollReveal->revealProgress(), 1.0), 1000);
    const QRect scrollBodyRectangle(
        scrollCardBody->mapTo(scrollCard, QPoint(0, 0)),
        scrollCardBody->size());
    QVERIFY(scrollCard->rect().contains(scrollBodyRectangle));
    QCOMPARE(scrollContentLayout->indexOf(scrollCard), 0);
    QVERIFY(followingContent->geometry().top() >
            scrollCard->geometry().bottom());
    QVERIFY(scrollArea.verticalScrollBar()->maximum() > 0);
}

void ComponentsTest::cardButtonUsesTokenStateColors()
{
    CardButton card;
    card.setProperty("_ngstdColorScheme", QStringLiteral("light"));
    card.setProperty("_ngstdAnimationPolicy", QStringLiteral("disabled"));
    card.resize(160, 80);
    card.show();
    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(&card, &enterEvent);

    QPixmap pixmap(card.size());
    pixmap.fill(Qt::transparent);
    card.render(&pixmap);
    QCOMPARE(pixmap.toImage().pixelColor(card.rect().center()),
             DesignTokens::color(ColorRole::SurfaceBrand, ColorScheme::Light));
}

void ComponentsTest::cardButtonUsesDarkToolboxDecoration()
{
    CardButton card;
    card.setVariant(CardVariant::ToolboxNew);
    card.setProperty("_ngstdColorScheme", QStringLiteral("dark"));
    card.resize(320, 160);
    card.show();

    QPixmap pixmap(card.size());
    pixmap.fill(Qt::transparent);
    card.render(&pixmap);
    const QColor centerColor =
        pixmap.toImage().pixelColor(card.rect().center());
    QVERIFY(centerColor.lightness() < 100);
}

void ComponentsTest::cardButtonAnimatesStateColors()
{
    CardButton card;
    card.setProperty("_ngstdColorScheme", QStringLiteral("light"));
    card.setProperty("_ngstdAnimationPolicy", QStringLiteral("enabled"));
    card.resize(160, 80);
    card.show();

    QPixmap pixmap(card.size());
    pixmap.fill(Qt::transparent);
    card.render(&pixmap);
    const QColor idleColor = pixmap.toImage().pixelColor(card.rect().center());
    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(&card, &enterEvent);
    QTest::qWait(40);
    pixmap.fill(Qt::transparent);
    card.render(&pixmap);
    const QColor intermediateColor =
        pixmap.toImage().pixelColor(card.rect().center());
    QVERIFY(intermediateColor != idleColor);
    QVERIFY(intermediateColor !=
            DesignTokens::color(ColorRole::SurfaceBrand, ColorScheme::Light));
    QTest::qWait(160);
    pixmap.fill(Qt::transparent);
    card.render(&pixmap);
    QCOMPARE(pixmap.toImage().pixelColor(card.rect().center()),
             DesignTokens::color(ColorRole::SurfaceBrand, ColorScheme::Light));
}

void ComponentsTest::cardButtonAnimatesSelectionFocusAndRipple()
{
    CardButton card;
    card.setProperty("_ngstdColorScheme", QStringLiteral("light"));
    card.setProperty("_ngstdAnimationPolicy", QStringLiteral("enabled"));
    card.resize(180, 96);
    card.show();
    QCoreApplication::processEvents();

    card.setChecked(true);
    QVariantAnimation *selectionAnimation =
        card.findChild<QVariantAnimation *>(
            QStringLiteral("_ngstdCardSelectionAnimation"));
    QVariantAnimation *feedbackAnimation = card.findChild<QVariantAnimation *>(
        QStringLiteral("_ngstdCardFeedbackAnimation"));
    QVERIFY(selectionAnimation);
    QVERIFY(feedbackAnimation);
    QCOMPARE(selectionAnimation->duration(),
             DesignTokens::duration(MotionDuration::Normal));
    QCOMPARE(feedbackAnimation->duration(),
             DesignTokens::duration(MotionDuration::Slow));
    QCOMPARE(selectionAnimation->state(), QAbstractAnimation::Running);
    QCOMPARE(feedbackAnimation->state(), QAbstractAnimation::Running);

    QVERIFY(!card.findChild<QVariantAnimation *>(
        QStringLiteral("_ngstdCardFocusAnimation")));

    card.setVariant(CardVariant::Data);
    QTest::mousePress(&card, Qt::LeftButton, Qt::NoModifier,
                      card.rect().center());
    QVariantAnimation *rippleAnimation = card.findChild<QVariantAnimation *>(
        QStringLiteral("_ngstdCardRippleAnimation"));
    QVariantAnimation *rippleOpacityAnimation =
        card.findChild<QVariantAnimation *>(
            QStringLiteral("_ngstdCardRippleOpacityAnimation"));
    QVERIFY(rippleAnimation);
    QVERIFY(rippleOpacityAnimation);
    QCOMPARE(rippleAnimation->duration(),
             DesignTokens::duration(MotionDuration::Normal));
    QCOMPARE(rippleOpacityAnimation->duration(),
             DesignTokens::duration(MotionDuration::Fast));
    QCOMPARE(rippleAnimation->state(), QAbstractAnimation::Running);
    QCOMPARE(rippleOpacityAnimation->state(), QAbstractAnimation::Running);
    QTest::mouseRelease(&card, Qt::LeftButton, Qt::NoModifier,
                        card.rect().center());
    QTRY_COMPARE_WITH_TIMEOUT(rippleOpacityAnimation->duration(),
                              DesignTokens::duration(MotionDuration::Slow),
                              500);

    card.setProperty("_ngstdAnimationPolicy", QStringLiteral("disabled"));
    QTest::mousePress(&card, Qt::LeftButton, Qt::NoModifier,
                      card.rect().center());
    QCOMPARE(rippleAnimation->duration(), 0);
    QCOMPARE(rippleAnimation->state(), QAbstractAnimation::Stopped);
    QTest::mouseRelease(&card, Qt::LeftButton, Qt::NoModifier,
                        card.rect().center());
}

void ComponentsTest::revealOwnershipAndReducedMotion()
{
    RevealWidget reveal;
    reveal.setProperty("_ngstdAnimationPolicy", QStringLiteral("disabled"));
    QLabel *content = new QLabel(QStringLiteral("Content"));
    content->setFixedHeight(60);
    reveal.resize(200, 0);
    reveal.setContentWidget(content);
    reveal.setExpanded(true);
    QCOMPARE(reveal.revealProgress(), 1.0);
    QCOMPARE(content->geometry().top(), 0);
    QCOMPARE(content->geometry().left(), 0);
    QCOMPARE(reveal.height(), content->height());
    QCOMPARE(reveal.takeContentWidget(), content);
    QCOMPARE(reveal.height(), 0);
    QVERIFY(!content->parent());
    delete content;
}

void ComponentsTest::disclosureExposesExpandedState()
{
    Disclosure disclosure;
    disclosure.setTitle(QStringLiteral("Details"));
    QLabel *content = new QLabel(QStringLiteral("Body"));
    disclosure.setContentWidget(content);
    QCOMPARE(disclosure.contentWidget(), content);
    QSignalSpy spy(&disclosure, &Disclosure::expandedChanged);
    disclosure.setExpanded(true);
    QVariantAnimation *chevronAnimation =
        disclosure.findChild<QVariantAnimation *>(
            QStringLiteral("_ngstdDisclosureChevronAnimation"));
    QVariantAnimation *revealAnimation =
        disclosure.findChild<QVariantAnimation *>(
            QStringLiteral("_ngstdRevealAnimation"));
    QVERIFY(chevronAnimation);
    QVERIFY(revealAnimation);
    QCOMPARE(chevronAnimation->duration(),
             DesignTokens::duration(MotionDuration::Slow));
    QCOMPARE(revealAnimation->duration(),
             DesignTokens::duration(MotionDuration::Slow));
    QVERIFY(disclosure.isExpanded());
    QCOMPARE(spy.count(), 1);
    QToolButton *header = disclosure.findChild<QToolButton *>();
    QVERIFY(header);
    QCOMPARE(header->accessibleName(), QStringLiteral("Details"));
    QCOMPARE(header->cursor().shape(), Qt::PointingHandCursor);

    QAccessibleInterface *interface =
        QAccessible::queryAccessibleInterface(&disclosure);
    QVERIFY(interface);
    QVERIFY(interface->state().expandable);
    QVERIFY(interface->state().expanded);
    auto *actions = static_cast<QAccessibleActionInterface *>(
        interface->interface_cast(QAccessible::ActionInterface));
    QVERIFY(actions);
    actions->doAction(QAccessibleActionInterface::toggleAction());
    QVERIFY(!disclosure.isExpanded());
    QVERIFY(interface->state().collapsed);
    QCOMPARE(disclosure.takeContentWidget(), content);
    QVERIFY(!content->parent());
    delete content;
}

void ComponentsTest::disclosureRestoresGeometryAfterAnimation()
{
    QWidget host;
    QVBoxLayout *layout = new QVBoxLayout(&host);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    Disclosure *disclosure = new Disclosure;
    disclosure->setMinimumWidth(360);
    disclosure->setProperty("_ngstdAnimationPolicy",
                            QStringLiteral("enabled"));
    QLabel *content = new QLabel(
        QStringLiteral("A longer disclosure body that wraps at a stable "
                       "width and keeps its full geometry."));
    content->setWordWrap(true);
    disclosure->setContentWidget(content);
    layout->addWidget(disclosure);
    host.show();
    QCoreApplication::processEvents();

    const int collapsedHeight = disclosure->height();
    QCOMPARE(disclosure->sizeHint().height(), collapsedHeight);
    disclosure->setExpanded(true);
    QTRY_VERIFY_WITH_TIMEOUT(
        qFuzzyCompare(disclosure->revealWidget()->revealProgress(), 1.0),
        1000);
    const int expandedHeight = disclosure->height();
    QVERIFY(expandedHeight > collapsedHeight);
    QCOMPARE(disclosure->sizeHint().height(), expandedHeight);
    const int padding = DesignTokens::componentMetric(
        ComponentMetric::DisclosureContentPadding);
    QCOMPARE(content->geometry().left(), padding);
    QCOMPARE(content->geometry().top(), padding);
    QCOMPARE(content->geometry().right(),
             disclosure->revealWidget()->width() - padding - 1);

    disclosure->setExpanded(false);
    QTRY_VERIFY_WITH_TIMEOUT(
        qFuzzyIsNull(disclosure->revealWidget()->revealProgress()), 1000);
    QCOMPARE(disclosure->height(), collapsedHeight);
    QCOMPARE(disclosure->sizeHint().height(), collapsedHeight);
    disclosure->setExpanded(true);
    QTRY_VERIFY_WITH_TIMEOUT(
        qFuzzyCompare(disclosure->revealWidget()->revealProgress(), 1.0),
        1000);
    QCOMPARE(disclosure->height(), expandedHeight);
    QCOMPARE(disclosure->sizeHint().height(), expandedHeight);
}

void ComponentsTest::disclosureSeparatorMatchesHeaderBoundary()
{
    QWidget root;
    Disclosure disclosure(&root);
    disclosure.setProperty("_ngstdAnimationPolicy",
                           QStringLiteral("disabled"));
    disclosure.setTitle(QStringLiteral("Details"));
    QWidget *content = new QWidget;
    content->setFixedHeight(24);
    disclosure.setContentWidget(content);
    disclosure.setFixedWidth(320);
    ThemeController *controller = ThemeController::attach(&root);
    QVERIFY(controller);
    disclosure.setExpanded(true);
    root.show();
    QCoreApplication::processEvents();

    QToolButton *header = disclosure.findChild<QToolButton *>();
    QVERIFY(header);
    const QImage image = disclosure.grab().toImage();
    const QColor separator =
        DesignTokens::color(ColorRole::Border, ColorScheme::Light);
    int boundaryPixels = 0;
    int displacedPixels = 0;
    const int boundaryY = header->geometry().bottom() + 1;
    const int displacedY =
        boundaryY + DesignTokens::componentMetric(
                        ComponentMetric::DisclosureContentPadding);
    for (int x = 4; x < image.width() - 4; ++x) {
        if (image.pixelColor(x, boundaryY) == separator) ++boundaryPixels;
        if (image.pixelColor(x, displacedY) == separator) ++displacedPixels;
    }
    QVERIFY(boundaryPixels > image.width() * 3 / 4);
    QVERIFY(displacedPixels < image.width() / 4);
    controller->detach();
}

void ComponentsTest::expandableSectionOwnsInteractionAndLayout()
{
    ExpandableSection section(
        QStringLiteral("Components"),
        QStringLiteral("Reusable controls and patterns."), IconRole::Check);
    section.setProperty("_ngstdAnimationPolicy", QStringLiteral("enabled"));
    section.setIconText(QStringLiteral("Aa"));
    QCOMPARE(section.title(), QStringLiteral("Components"));
    QCOMPARE(section.description(),
             QStringLiteral("Reusable controls and patterns."));
    QCOMPARE(section.iconRole(), IconRole::Check);
    QCOMPARE(section.iconText(), QStringLiteral("Aa"));

    QLabel *content = new QLabel(QStringLiteral("Section content"));
    content->setFixedHeight(32);
    section.contentLayout()->addWidget(content);
    const QMargins margins = section.contentLayout()->contentsMargins();
    QCOMPARE(margins.left(),
             DesignTokens::componentMetric(
                 ComponentMetric::ExpandableSectionContentPaddingHorizontal));
    QCOMPARE(margins.left(), margins.right());
    QCOMPARE(margins.bottom(),
             DesignTokens::componentMetric(
                 ComponentMetric::ExpandableSectionContentPaddingBottom));

    QSignalSpy expandedSpy(&section, &ExpandableSection::expandedChanged);
    section.setExpanded(true);
    QVERIFY(section.isExpanded());
    QCOMPARE(expandedSpy.count(), 1);
    QVariantAnimation *chevronAnimation =
        section.findChild<QVariantAnimation *>(
            QStringLiteral("_ngstdExpandableSectionChevronAnimation"));
    QVariantAnimation *revealAnimation =
        section.findChild<QVariantAnimation *>(
            QStringLiteral("_ngstdRevealAnimation"));
    QVERIFY(chevronAnimation);
    QVERIFY(revealAnimation);
    QCOMPARE(chevronAnimation->duration(),
             DesignTokens::duration(MotionDuration::Slow));
    QCOMPARE(revealAnimation->duration(),
             DesignTokens::duration(MotionDuration::Slow));

    QAbstractButton *header = section.findChild<QAbstractButton *>(
        QStringLiteral("_ngstdExpandableSectionHeader"));
    QVERIFY(header);
    QCOMPARE(header->cursor().shape(), Qt::PointingHandCursor);
    QCOMPARE(header->accessibleName(),
             QStringLiteral("Components. Reusable controls and patterns."));
    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(header, &enterEvent);
    QVariantAnimation *hoverAnimation = section.findChild<QVariantAnimation *>(
        QStringLiteral("_ngstdExpandableSectionHoverAnimation"));
    QVERIFY(hoverAnimation);
    QCOMPARE(hoverAnimation->duration(),
             DesignTokens::duration(MotionDuration::Fast));
}

void ComponentsTest::pageBackgroundAppliesVariantDefaults()
{
    PageBackground background;
    background.setVariant(PageBackgroundVariant::Workspace);
    QVERIFY(!background.isDecorationVisible());
    QVERIFY(!background.isGridVisible());
    QCOMPARE(background.gradient(), PageBackgroundGradient::None);

    background.setGridVisible(true);
    background.resetGridVisible();
    QVERIFY(!background.isGridVisible());

    background.setVariant(PageBackgroundVariant::Corporate);
    QVERIFY(!background.isDecorationVisible());
    QVERIFY(background.isGridVisible());
    QCOMPARE(background.gradient(), PageBackgroundGradient::Center);

    background.setVariant(PageBackgroundVariant::Main);
    QVERIFY(background.isDecorationVisible());
    QVERIFY(background.isGridVisible());
    QCOMPARE(background.gradient(), PageBackgroundGradient::Down);

    background.setGridVisible(false);
    background.setGradient(PageBackgroundGradient::None);
    background.resetVariant();
    QVERIFY(background.isGridVisible());
    QCOMPARE(background.gradient(), PageBackgroundGradient::Down);
}

void ComponentsTest::pageBackgroundExposesCornerMode()
{
    PageBackground background;
    QSignalSpy spy(&background, &PageBackground::cornerModeChanged);
    QCOMPARE(background.cornerMode(), PageBackgroundCornerMode::Square);
    background.setCornerMode(PageBackgroundCornerMode::RoundedTop);
    QCOMPARE(background.cornerMode(), PageBackgroundCornerMode::RoundedTop);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(background.property("_ngstdCornerMode").toString(),
             QStringLiteral("roundedTop"));
    background.resetCornerMode();
    QCOMPARE(background.cornerMode(), PageBackgroundCornerMode::Square);
}

void ComponentsTest::motionAdapterUsesTypedTokensAndPolicy()
{
    QWidget context;
    QVariantAnimation animation;
    context.setProperty("_ngstdAnimationPolicy", QStringLiteral("enabled"));
    QVERIFY(MotionAdapter::configure(
        &animation, &context, {MotionDuration::Normal, MotionEasing::Enter}));
    QCOMPARE(animation.duration(),
             DesignTokens::duration(MotionDuration::Normal));
    QCOMPARE(animation.easingCurve(),
             DesignTokens::easingCurve(MotionEasing::Enter));

    context.setProperty("_ngstdAnimationPolicy", QStringLiteral("disabled"));
    QVERIFY(!MotionAdapter::configure(
        &animation, &context, {MotionDuration::Slow, MotionEasing::Exit}));
    QCOMPARE(animation.duration(), 0);
    QCOMPARE(animation.easingCurve(),
             DesignTokens::easingCurve(MotionEasing::Exit));
}

void ComponentsTest::navigationComponentsOwnOneLogicalBoundary()
{
    NavigationPanel panel;
    panel.resize(48, 32);
    QCOMPARE(panel.frameShape(), QFrame::NoFrame);
    QCOMPARE(panel.property("_ngstdRole").toString(),
             QStringLiteral("navigationPanel"));

    NavigationListWidget *list = new NavigationListWidget(&panel);
    list->setGeometry(panel.rect().adjusted(0, 0, -1, 0));
    QCOMPARE(list->frameShape(), QFrame::NoFrame);
    QCOMPARE(list->property("_ngstdRole").toString(),
             QStringLiteral("navigationList"));
    QVERIFY(!list->autoFillBackground());
    QVERIFY(!list->viewport()->autoFillBackground());
    QCOMPARE(list->iconSize(),
             QSize(DesignTokens::componentMetric(
                       ComponentMetric::WizardSidebarIconExtent),
                   DesignTokens::componentMetric(
                       ComponentMetric::WizardSidebarIconExtent)));

    QListWidgetItem *currentItem =
        new QListWidgetItem(QStringLiteral("Current"), list);
    list->setStepState(currentItem, NavigationListWidget::StepState::Current);
    QCOMPARE(list->stepState(currentItem),
             NavigationListWidget::StepState::Current);
    QVERIFY(!currentItem->icon().isNull());
    QCOMPARE(currentItem->foreground().color(),
             DesignTokens::color(ColorRole::Text, ColorScheme::Light));
    QCOMPARE(currentItem->font().weight(), QFont::Medium);
    QCOMPARE(currentItem->sizeHint().height(),
             DesignTokens::componentMetric(
                 ComponentMetric::WizardSidebarRowHeight));

    list->setProperty("_ngstdColorScheme", QStringLiteral("dark"));
    QEvent styleChange(QEvent::StyleChange);
    QApplication::sendEvent(list, &styleChange);
    QCOMPARE(currentItem->foreground().color(),
             DesignTokens::color(ColorRole::Text, ColorScheme::Dark));

    const QColor surface =
        DesignTokens::color(ColorRole::Surface, ColorScheme::Light);
    const QColor border =
        DesignTokens::color(ColorRole::Border, ColorScheme::Light);
    const QImage leftToRight = panel.grab().toImage();
    QCOMPARE(leftToRight.pixelColor(0, panel.height() / 2), surface);
    QCOMPARE(leftToRight.pixelColor(panel.width() - 1, panel.height() / 2),
             border);

    panel.setLayoutDirection(Qt::RightToLeft);
    const QImage rightToLeft = panel.grab().toImage();
    QCOMPARE(rightToLeft.pixelColor(0, panel.height() / 2), border);
    QCOMPARE(rightToLeft.pixelColor(panel.width() - 1, panel.height() / 2),
             surface);
}

void ComponentsTest::embeddedSurfaceOwnsNoNestedFrame()
{
    QListView view;
    view.setFrameShape(QFrame::StyledPanel);
    view.setAutoFillBackground(true);
    view.viewport()->setAutoFillBackground(true);

    WidgetStyle::setEmbeddedSurface(&view);

    QCOMPARE(view.frameShape(), QFrame::NoFrame);
    QCOMPARE(view.property("ngstdSurfaceMode").toString(),
             QStringLiteral("embedded"));
    QCOMPARE(view.viewport()->property("ngstdSurfaceMode").toString(),
             QStringLiteral("embeddedViewport"));
    QVERIFY(!view.autoFillBackground());
    QVERIFY(!view.viewport()->autoFillBackground());
}

void ComponentsTest::wizardPageLayoutUsesDesignTokens()
{
    QWidget page;
    QVBoxLayout *layout = new QVBoxLayout(&page);
    layout->setContentsMargins(1, 2, 3, 4);
    layout->setSpacing(5);

    WidgetStyle::applyWizardPageLayout(&page);

    const int margin =
        DesignTokens::componentMetric(ComponentMetric::WizardPageMargin);
    QCOMPARE(layout->contentsMargins(),
             QMargins(margin, margin, margin, margin));
    QCOMPARE(layout->spacing(), DesignTokens::componentMetric(
                                    ComponentMetric::WizardPageSpacing));
}

void ComponentsTest::searchFieldUsesLeadingAction()
{
    QWidget root;
    InspectableSearchField field;
    field.setParent(&root);
    field.setText(QStringLiteral("Query"));
    field.setCursorPosition(0);
    field.resize(300, DesignTokens::controlHeight(ControlSize::Medium));
    QCOMPARE(field.minimumHeight(),
             DesignTokens::controlHeight(ControlSize::Medium));
    ThemeController *controller = ThemeController::attach(&root);
    QVERIFY(controller);
    root.resize(340, 80);
    root.show();
    QCoreApplication::processEvents();
    QCOMPARE(field.actions().size(), 1);
    QVERIFY(field.actions().constFirst()->isEnabled());
    QVERIFY(!field.actions().constFirst()->icon().isNull());
    const int cursorOffset = field.cursorRect().left();
    QVERIFY(cursorOffset > 0);
    field.setLayoutDirection(Qt::RightToLeft);
    QCoreApplication::processEvents();
    QCOMPARE(field.actions().size(), 1);
    QVERIFY(!field.actions().constFirst()->icon().isNull());
    controller->detach();
}

void ComponentsTest::promotionalButtonsAnimateLikeReference()
{
    Button button(QStringLiteral("Promotion"));
    button.setVariant(ButtonVariant::Hero);
    button.setProperty("_ngstdAnimationPolicy", QStringLiteral("enabled"));
    button.resize(220, 40);
    button.show();
    QCoreApplication::processEvents();

    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(&button, &enterEvent);
    QVariantAnimation *shineAnimation = nullptr;
    const QList<QVariantAnimation *> animations =
        button.findChildren<QVariantAnimation *>();
    for (QVariantAnimation *animation : animations) {
        if (animation->duration() ==
            DesignTokens::duration(MotionDuration::Slow)) {
            shineAnimation = animation;
            break;
        }
    }
    QVERIFY(shineAnimation);
    QCOMPARE(shineAnimation->state(), QAbstractAnimation::Running);

    QTest::qWait(90);
    QPixmap firstFrame(button.size());
    firstFrame.fill(Qt::transparent);
    button.render(&firstFrame);
    QTest::qWait(140);
    QPixmap secondFrame(button.size());
    secondFrame.fill(Qt::transparent);
    button.render(&secondFrame);
    QVERIFY(firstFrame.toImage() != secondFrame.toImage());

    Button trialButton(QStringLiteral("Trial"));
    trialButton.setVariant(ButtonVariant::Trial);
    trialButton.setProperty("_ngstdAnimationPolicy",
                            QStringLiteral("enabled"));
    trialButton.resize(220, 44);
    trialButton.show();
    QCoreApplication::processEvents();
    QGraphicsDropShadowEffect *trialEffect =
        qobject_cast<QGraphicsDropShadowEffect *>(
            trialButton.graphicsEffect());
    QVERIFY(trialEffect);
    const qreal idleBlur = trialEffect->blurRadius();
    const qreal idleOffset = trialEffect->yOffset();
    QEvent trialEnterEvent(QEvent::Enter);
    QApplication::sendEvent(&trialButton, &trialEnterEvent);
    QTest::qWait(120);
    QVERIFY(trialEffect->blurRadius() > idleBlur);
    QVERIFY(trialEffect->yOffset() > idleOffset);
}

void ComponentsTest::buttonAnimatesVisualStates()
{
    Button button(QStringLiteral("State"));
    button.setVariant(ButtonVariant::Secondary);
    button.setCheckable(true);
    button.setProperty("_ngstdAnimationPolicy", QStringLiteral("enabled"));
    button.resize(160, 40);
    button.show();
    QCoreApplication::processEvents();

    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(&button, &enterEvent);
    QVariantAnimation *stateAnimation = button.findChild<QVariantAnimation *>(
        QStringLiteral("_ngstdButtonStateAnimation"));
    QVERIFY(stateAnimation);
    QCOMPARE(stateAnimation->duration(),
             DesignTokens::duration(MotionDuration::Fast));
    QCOMPARE(stateAnimation->state(), QAbstractAnimation::Running);
    QTest::qWait(40);
    QVERIFY(stateAnimation->currentValue().toReal() > 0.0);
    QVERIFY(stateAnimation->currentValue().toReal() < 1.0);

    QTest::qWait(DesignTokens::duration(MotionDuration::Fast));
    button.setChecked(true);
    QCOMPARE(stateAnimation->duration(),
             DesignTokens::duration(MotionDuration::Normal));
    QCOMPARE(stateAnimation->state(), QAbstractAnimation::Running);

    button.setProperty("_ngstdAnimationPolicy", QStringLiteral("disabled"));
    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(&button, &leaveEvent);
    QCOMPARE(stateAnimation->duration(), 0);
    QCOMPARE(stateAnimation->state(), QAbstractAnimation::Stopped);
}

void ComponentsTest::primaryButtonTransitionNeverExposesWhiteBackground()
{
    QWidget root;
    Button button(QStringLiteral("Primary"), &root);
    button.setVariant(ButtonVariant::Primary);
    button.setProperty("_ngstdAnimationPolicy", QStringLiteral("enabled"));
    button.setGeometry(12, 12, 140, 42);
    root.resize(164, 66);
    ThemeOptions options;
    options.setThemeMode(ThemeMode::Light);
    ThemeController *controller = ThemeController::attach(&root, options);
    QVERIFY(controller);
    root.show();
    QCoreApplication::processEvents();

    QTest::mousePress(&button, Qt::LeftButton, Qt::NoModifier,
                      button.rect().center());
    QVariantAnimation *animation = button.findChild<QVariantAnimation *>(
        QStringLiteral("_ngstdButtonStateAnimation"));
    QVERIFY(animation);
    QVERIFY(animation->duration() > 0);
    animation->setCurrentTime(animation->duration() / 2);
    animation->pause();
    const QColor background =
        button.grab().toImage().pixelColor(8, button.height() / 2);
    QVERIFY(background.red() < 100);
    QVERIFY(background.green() < 190);
    QVERIFY(background.blue() < 240);

    QTest::mouseRelease(&button, Qt::LeftButton, Qt::NoModifier,
                        button.rect().center());
    controller->detach();
}

void ComponentsTest::disabledButtonIgnoresHoverMotion()
{
    QWidget root;
    Button button(QStringLiteral("Disabled"), &root);
    button.setVariant(ButtonVariant::Secondary);
    button.setProperty("_ngstdAnimationPolicy", QStringLiteral("enabled"));
    button.setGeometry(12, 12, 140, 42);
    button.setEnabled(false);
    root.resize(164, 66);
    ThemeController *controller = ThemeController::attach(&root);
    QVERIFY(controller);
    root.show();
    QCoreApplication::processEvents();

    const QImage idleImage = button.grab().toImage();
    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(&button, &enterEvent);
    QCoreApplication::processEvents();
    QVariantAnimation *animation = button.findChild<QVariantAnimation *>(
        QStringLiteral("_ngstdButtonStateAnimation"));
    QVERIFY(animation);
    QCOMPARE(animation->state(), QAbstractAnimation::Stopped);
    QCOMPARE(button.grab().toImage(), idleImage);
    controller->detach();
}

void ComponentsTest::iconButtonKeepsPressedIconVisible()
{
    QWidget root;
    Button button(&root);
    button.setVariant(ButtonVariant::Icon);
    button.setIconRole(IconRole::Download);
    button.setGeometry(12, 12, 42, 42);
    root.resize(66, 66);
    ThemeOptions options;
    options.setThemeMode(ThemeMode::Light);
    ThemeController *controller = ThemeController::attach(&root, options);
    QVERIFY(controller);
    root.show();
    QCoreApplication::processEvents();

    QTest::mousePress(&button, Qt::LeftButton, Qt::NoModifier,
                      button.rect().center());
    const QImage iconImage = button.icon().pixmap(button.iconSize()).toImage();
    const QColor expected =
        DesignTokens::color(ColorRole::BrandActive, ColorScheme::Light);
    bool containsExpectedColor = false;
    bool containsWhite = false;
    for (int y = 0; y < iconImage.height(); ++y) {
        for (int x = 0; x < iconImage.width(); ++x) {
            const QColor pixel = iconImage.pixelColor(x, y);
            if (pixel.alpha() < 96) continue;
            const int expectedDistance =
                qAbs(pixel.red() - expected.red()) +
                qAbs(pixel.green() - expected.green()) +
                qAbs(pixel.blue() - expected.blue());
            containsExpectedColor =
                containsExpectedColor || expectedDistance < 24;
            containsWhite =
                containsWhite || (pixel.red() > 245 && pixel.green() > 245 &&
                                  pixel.blue() > 245);
        }
    }
    QVERIFY(containsExpectedColor);
    QVERIFY(!containsWhite);
    QTest::mouseRelease(&button, Qt::LeftButton, Qt::NoModifier,
                        button.rect().center());
    controller->detach();
}

void ComponentsTest::iconButtonRemainsSquareInStretchingLayouts()
{
    QWidget root;
    QHBoxLayout layout(&root);
    Button button;
    const QSizePolicy regularPolicy = button.sizePolicy();
    button.setVariant(ButtonVariant::Icon);
    button.setIconRole(IconRole::Folder);
    layout.addWidget(&button, 1);
    root.resize(320, 80);
    root.show();
    QCoreApplication::processEvents();

    const int extent = DesignTokens::controlHeight(ControlSize::Medium);
    QCOMPARE(button.sizeHint(), QSize(extent, extent));
    QCOMPARE(button.minimumSizeHint(), QSize(extent, extent));
    QCOMPARE(button.size(), QSize(extent, extent));
    QCOMPARE(button.sizePolicy().horizontalPolicy(), QSizePolicy::Fixed);
    QCOMPARE(button.sizePolicy().verticalPolicy(), QSizePolicy::Fixed);

    button.setVariant(ButtonVariant::Secondary);
    QCOMPARE(button.sizePolicy(), regularPolicy);
}

void ComponentsTest::secondaryIconOnlyButtonUsesSquareToken()
{
    Button button(QStringLiteral("Show formats"));
    QSignalSpy iconOnlySpy(&button, &Button::iconOnlyChanged);

    button.setVariant(ButtonVariant::Secondary);
    button.setIconRole(IconRole::ChevronDown);
    button.setIconOnly(true);

    const int extent = DesignTokens::controlHeight(ControlSize::Medium);
    QCOMPARE(button.iconOnly(), true);
    QCOMPARE(button.property("ngstdIconOnly").toBool(), true);
    QCOMPARE(button.sizeHint(), QSize(extent, extent));
    QCOMPARE(button.minimumSizeHint(), QSize(extent, extent));
    QCOMPARE(button.sizePolicy().horizontalPolicy(), QSizePolicy::Fixed);
    QCOMPARE(button.sizePolicy().verticalPolicy(), QSizePolicy::Fixed);
    QCOMPARE(button.iconRole(), IconRole::ChevronDown);
    QVERIFY(!iconPixmap(IconRole::ChevronDown).isNull());
    QVERIFY(!iconPixmap(IconRole::ChevronUp).isNull());

    button.setIconRole(IconRole::ChevronUp);
    QCOMPARE(button.iconRole(), IconRole::ChevronUp);
    button.setIconOnly(false);
    QCOMPARE(button.iconOnly(), false);
    QCOMPARE(button.property("ngstdIconOnly").toBool(), false);
    QVERIFY(button.sizePolicy().horizontalPolicy() != QSizePolicy::Fixed);
    QCOMPARE(button.sizePolicy().verticalPolicy(), QSizePolicy::Fixed);
    QCOMPARE(iconOnlySpy.count(), 2);
}

void ComponentsTest::loadingButtonPreservesIcon()
{
    Button button(QStringLiteral("Save"));
    button.setIconRole(IconRole::Check);
    const qint64 iconKey = button.icon().cacheKey();
    button.setLoading(true);
    QCOMPARE(button.icon().cacheKey(), iconKey);
    QVERIFY(button.property("_ngstdBusy").toBool());
    button.setLoading(false);
    QCOMPARE(button.icon().cacheKey(), iconKey);
}

void ComponentsTest::checkableButtonUpdatesRoleIconColor()
{
    Button button;
    button.setProperty("_ngstdColorScheme", QStringLiteral("light"));
    button.setVariant(ButtonVariant::Icon);
    button.setIconRole(IconRole::Download);
    button.setCheckable(true);
    const qint64 idleIconKey = button.icon().cacheKey();

    button.setChecked(true);
    QVERIFY(button.icon().cacheKey() != idleIconKey);
    const QImage checkedIcon =
        button.icon().pixmap(button.iconSize()).toImage();
    const QColor expected =
        DesignTokens::color(ColorRole::White, ColorScheme::Light);
    bool foundCheckedColor = false;
    for (int y = 0; y < checkedIcon.height(); ++y) {
        for (int x = 0; x < checkedIcon.width(); ++x) {
            if (checkedIcon.pixelColor(x, y) == expected) {
                foundCheckedColor = true;
                break;
            }
        }
        if (foundCheckedColor) break;
    }
    QVERIFY(foundCheckedColor);

    const qint64 checkedIconKey = button.icon().cacheKey();
    button.setChecked(false);
    QVERIFY(button.icon().cacheKey() != checkedIconKey);

    ButtonPaintRecorder style;
    Button textButton(QStringLiteral("Active option"));
    textButton.setStyle(&style);
    textButton.setProperty("_ngstdColorScheme", QStringLiteral("light"));
    textButton.setProperty("_ngstdAnimationPolicy",
                           QStringLiteral("disabled"));
    textButton.setCheckable(true);
    textButton.setChecked(true);
    textButton.resize(180, 40);
    textButton.show();
    QPixmap image(textButton.size());
    image.fill(Qt::transparent);
    textButton.render(&image);
    QCOMPARE(style.labelColor,
             DesignTokens::color(ColorRole::White, ColorScheme::Light));
}

void ComponentsTest::dataButtonUsesRippleMotion()
{
    Button button(QStringLiteral("Data primary"));
    button.setVariant(ButtonVariant::DataFilled);
    button.setProperty("_ngstdAnimationPolicy", QStringLiteral("enabled"));
    button.resize(180, 48);
    button.show();
    QTest::mousePress(&button, Qt::LeftButton, Qt::NoModifier, QPoint(40, 24));
    QVariantAnimation *rippleAnimation = button.findChild<QVariantAnimation *>(
        QStringLiteral("_ngstdButtonRippleAnimation"));
    QVariantAnimation *opacityAnimation =
        button.findChild<QVariantAnimation *>(
            QStringLiteral("_ngstdButtonRippleOpacityAnimation"));
    QVERIFY(rippleAnimation);
    QVERIFY(opacityAnimation);
    QCOMPARE(rippleAnimation->duration(),
             DesignTokens::duration(MotionDuration::Normal));
    QCOMPARE(rippleAnimation->state(), QAbstractAnimation::Running);
    QTest::mouseRelease(&button, Qt::LeftButton, Qt::NoModifier,
                        QPoint(40, 24));
    QCOMPARE(opacityAnimation->duration(),
             DesignTokens::duration(MotionDuration::Slow));
    QCOMPARE(opacityAnimation->state(), QAbstractAnimation::Running);
}

void ComponentsTest::loadingButtonPaintsLabelAndIcon()
{
    ButtonPaintRecorder style;
    Button button(QStringLiteral("Loading"));
    button.setStyle(&style);
    button.setIconRole(IconRole::Check);
    const qint64 iconKey = button.icon().cacheKey();
    const int idleWidth = button.sizeHint().width();
    button.setLoading(true);
    QVERIFY(button.sizeHint().width() > idleWidth);
    button.resize(180, 40);
    button.show();

    QPixmap image(button.size());
    image.fill(Qt::transparent);
    button.render(&image);

    QCOMPARE(style.labelText, QStringLiteral("Loading"));
    QVERIFY(style.labelPixmapPainted);
    QCOMPARE(button.icon().cacheKey(), iconKey);
    QCOMPARE(
        style.labelTextRectangle.left() - style.labelPixmapRectangle.right() -
            1,
        DesignTokens::componentMetric(ComponentMetric::ButtonIconTextGap));
    Spinner *spinner = button.findChild<Spinner *>();
    QVERIFY(spinner);
    QVERIFY(spinner->isVisible());
    QVERIFY(button.rect().contains(spinner->geometry()));
    QVERIFY(spinner->geometry().center().x() < button.rect().center().x());
    QVERIFY(spinner->geometry().right() < style.labelPixmapRectangle.left());
    QVERIFY(style.bevelRectangles.isEmpty());
    QVERIFY(image.toImage().pixelColor(4, 4).alpha() > 0);

    button.setLayoutDirection(Qt::RightToLeft);
    style.bevelRectangles.clear();
    QCoreApplication::processEvents();
    button.render(&image);
    QVERIFY(spinner->geometry().center().x() > button.rect().center().x());
    QVERIFY(style.labelTextRectangle.right() < spinner->geometry().left());
    QVERIFY(style.bevelRectangles.isEmpty());
}

void ComponentsTest::loadingButtonSeparatesSpinnerFromText()
{
    ButtonPaintRecorder style;
    Button button(QStringLiteral("Loading"));
    button.setStyle(&style);
    button.setLoading(true);
    button.resize(button.sizeHint());
    button.show();

    QPixmap image(button.size());
    image.fill(Qt::transparent);
    button.render(&image);

    Spinner *spinner = button.findChild<Spinner *>();
    QVERIFY(spinner);
    QVERIFY(button.rect().contains(spinner->geometry()));
    QVERIFY(spinner->geometry().right() < style.labelTextRectangle.left());
    QVERIFY(style.bevelRectangles.isEmpty());
    QVERIFY(image.toImage().pixelColor(4, 4).alpha() > 0);

    button.setLayoutDirection(Qt::RightToLeft);
    style.bevelRectangles.clear();
    QCoreApplication::processEvents();
    button.render(&image);
    QVERIFY(style.labelTextRectangle.right() < spinner->geometry().left());
    QVERIFY(style.bevelRectangles.isEmpty());
}

void ComponentsTest::loadingButtonExposesBusyState()
{
    Button button(QStringLiteral("Save"));
    button.setAccessibleName(QStringLiteral("Save project"));
    button.setLoading(true);
    QAccessibleInterface *interface =
        QAccessible::queryAccessibleInterface(&button);
    QVERIFY(interface);
    QCOMPARE(interface->role(), QAccessible::Button);
    QCOMPARE(interface->text(QAccessible::Name),
             QStringLiteral("Save project"));
    QVERIFY(interface->state().busy);
    button.setLoading(false);
    QVERIFY(!interface->state().busy);
}

void ComponentsTest::comboBoxPopupFitsAllVisibleRows()
{
    QWidget root;
    root.resize(420, 320);
    ComboBox comboBox(&root);
    comboBox.setGeometry(40, 32, 300, 42);
    comboBox.addItems({QStringLiteral("NextGIS Data"),
                       QStringLiteral("Local file"),
                       QStringLiteral("Connected service")});
    ThemeOptions options;
    options.setThemeMode(ThemeMode::Dark);
    options.setAnimationPolicy(AnimationPolicy::Disabled);
    ThemeController *controller = ThemeController::attach(&root, options);
    QVERIFY(controller);
    root.show();
    comboBox.showPopup();
    QCoreApplication::processEvents();

    QFrame *popup =
        comboBox.findChild<QFrame *>(QStringLiteral("_ngstdComboBoxPopup"));
    QListView *view = comboBox.findChild<QListView *>(
        QStringLiteral("_ngstdComboBoxPopupView"));
    QVERIFY(popup);
    QVERIFY(view);
    QVERIFY(popup->isVisible());
    QCOMPARE(view->verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
    QVERIFY(!view->verticalScrollBar()->isVisible());
    for (int row = 0; row < comboBox.count(); ++row) {
        const QRect itemRectangle = view->visualRect(
            view->model()->index(row, comboBox.modelColumn()));
        QVERIFY2(view->viewport()->rect().contains(itemRectangle),
                 qPrintable(QStringLiteral("row %1: item=%2,%3 %4x%5 "
                                           "viewport=%6,%7 %8x%9")
                                .arg(row)
                                .arg(itemRectangle.x())
                                .arg(itemRectangle.y())
                                .arg(itemRectangle.width())
                                .arg(itemRectangle.height())
                                .arg(view->viewport()->rect().x())
                                .arg(view->viewport()->rect().y())
                                .arg(view->viewport()->rect().width())
                                .arg(view->viewport()->rect().height())));
    }
    QCOMPARE(popup->geometry().left(), comboBox.mapToGlobal(QPoint()).x());
    QVERIFY(popup->geometry().top() >
            comboBox.mapToGlobal(QPoint(0, comboBox.height())).y());
    comboBox.hidePopup();
    controller->detach();
}

void ComponentsTest::comboBoxSecondClickClosesPopup()
{
    QWidget root;
    root.resize(360, 220);
    ComboBox comboBox(&root);
    comboBox.setGeometry(32, 24, 260, 42);
    comboBox.addItems({QStringLiteral("LTS"), QStringLiteral("Release")});
    ThemeOptions options;
    options.setAnimationPolicy(AnimationPolicy::Disabled);
    ThemeController *controller = ThemeController::attach(&root, options);
    QVERIFY(controller);
    root.show();
    QCoreApplication::processEvents();

    QTest::mouseClick(&comboBox, Qt::LeftButton);
    QFrame *popup =
        comboBox.findChild<QFrame *>(QStringLiteral("_ngstdComboBoxPopup"));
    QVERIFY(popup);
    QTRY_VERIFY(popup->isVisible());
    QTest::mouseClick(&comboBox, Qt::LeftButton);
    QTRY_VERIFY(!popup->isVisible());
    controller->detach();
}

void ComponentsTest::comboBoxSizeHintFitsLongestItem()
{
    ComboBox comboBox;
    const QString longest =
        QStringLiteral("Use the system language and regional settings");
    comboBox.addItems({QStringLiteral("English"), longest});
    const int chrome =
        DesignTokens::componentMetric(ComponentMetric::ComboBoxDropDownWidth) +
        DesignTokens::spacing(3) * 2;
    QVERIFY(comboBox.sizeHint().width() >=
            comboBox.fontMetrics().horizontalAdvance(longest) + chrome);
}

void ComponentsTest::spinnerUsesElapsedTime()
{
    Spinner spinner;
    spinner.setProperty("_ngstdAnimationPolicy", QStringLiteral("enabled"));
    spinner.setRunning(true);
    const qreal startPhase = spinner.phase();
    QTest::qWait(50);
    QVERIFY(spinner.phase() != startPhase);
    spinner.stop();
}

void ComponentsTest::spinnerRunsWithSystemPolicy()
{
    const QVariant previousPolicy = qApp->property("_ngstdAnimationPolicy");
    const QVariant previousReducedMotion =
        qApp->property("_ngstdReducedMotion");
    qApp->setProperty("_ngstdAnimationPolicy", QStringLiteral("system"));
    qApp->setProperty("_ngstdReducedMotion", QVariant());
    Spinner spinner;
    spinner.setRunning(true);
    const qreal startPhase = spinner.phase();
    QTest::qWait(50);
    QVERIFY(spinner.phase() != startPhase);
    spinner.stop();
    qApp->setProperty("_ngstdAnimationPolicy", previousPolicy);
    qApp->setProperty("_ngstdReducedMotion", previousReducedMotion);
}

void ComponentsTest::spinnerHonorsReducedMotion()
{
    Spinner spinner;
    spinner.setProperty("_ngstdAnimationPolicy", QStringLiteral("disabled"));
    spinner.setRunning(true);
    QTest::qWait(50);
    QCOMPARE(spinner.phase(), 0.0);
    spinner.stop();
}

void ComponentsTest::tableOwnershipIsSymmetric()
{
    TableWidget table(1, 1);
    QLabel *content = new QLabel(QStringLiteral("Cell"));
    table.setCellContent(0, 0, content);
    QCOMPARE(table.cellContent(0, 0), content);
    QCOMPARE(table.takeCellContent(0, 0), content);
    QVERIFY(!content->parent());
    delete content;
}

void ComponentsTest::tableCellContentFitsHost()
{
    TableWidget table(2, 5);
    table.setHorizontalHeaderLabels({
        QStringLiteral("Layer"),
        QStringLiteral("Enabled"),
        QStringLiteral("Preview"),
        QStringLiteral("State"),
        QStringLiteral("Action"),
    });
    table.horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table.horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::ResizeToContents);
    table.horizontalHeader()->setSectionResizeMode(
        2, QHeaderView::ResizeToContents);
    table.horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    table.horizontalHeader()->setSectionResizeMode(
        4, QHeaderView::ResizeToContents);
    Button *actionButton = nullptr;
    for (int row = 0; row < table.rowCount(); ++row) {
        table.setCellContent(row, 0,
                             new QLabel(row == 0 ? QStringLiteral("districts")
                                                 : QStringLiteral("roads")));
        QCheckBox *enabled = new QCheckBox;
        enabled->setChecked(row == 0);
        table.setCellContent(row, 1, enabled, Qt::AlignCenter);
        QLabel *preview = new QLabel(QStringLiteral("64 x 34"));
        preview->setFixedSize(64, 34);
        table.setCellContent(row, 2, preview, Qt::AlignCenter);
        Tag *state = new Tag(row == 0 ? QStringLiteral("Ready")
                                      : QStringLiteral("Processing"));
        table.setCellContent(row, 3, state, Qt::AlignCenter);
        Button *button = new Button(QStringLiteral("Open"));
        button->setVariant(ButtonVariant::Text);
        table.setCellContent(row, 4, button, Qt::AlignCenter);
        if (row == 0) actionButton = button;
    }
    ThemeController *controller = ThemeController::attach(&table);
    QVERIFY(controller);
    table.resize(900, 160);
    table.show();
    QCoreApplication::processEvents();
    QWidget *host = table.cellWidget(0, 4);
    QVERIFY(host);
    QVERIFY(actionButton);
    const QMargins margins = host->layout()->contentsMargins();
    QCOMPARE(margins.left(), margins.right());
    QCOMPARE(margins.top(), margins.bottom());
    const QRect available = host->rect().marginsRemoved(margins);
    const QByteArray geometryMessage =
        QStringLiteral("host=%1,%2 %3x%4 available=%5,%6 %7x%8 "
                       "button=%9,%10 %11x%12")
            .arg(host->x())
            .arg(host->y())
            .arg(host->width())
            .arg(host->height())
            .arg(available.x())
            .arg(available.y())
            .arg(available.width())
            .arg(available.height())
            .arg(actionButton->x())
            .arg(actionButton->y())
            .arg(actionButton->width())
            .arg(actionButton->height())
            .toUtf8();
    QVERIFY2(available.contains(actionButton->geometry()),
             geometryMessage.constData());
    QVERIFY(actionButton->width() >= actionButton->minimumSizeHint().width());
    QVERIFY(actionButton->height() >=
            actionButton->minimumSizeHint().height());
    QVERIFY(actionButton->geometry().bottom() < host->rect().bottom());
    QVERIFY(actionButton->geometry().right() < host->rect().right());
    controller->detach();
}

void ComponentsTest::themeSwitchHasAccessibleButtons()
{
    ThemeSwitch themeSwitch;
    for (ThemeMode mode : {
             ThemeMode::System,
             ThemeMode::Light,
             ThemeMode::Dark,
         }) {
        QToolButton *button = themeSwitch.button(mode);
        QVERIFY(button);
        QVERIFY(!button->accessibleName().isEmpty());
    }
}

void ComponentsTest::themeSwitchSupportsKeyboardSelection()
{
    ThemeSwitch themeSwitch;
    themeSwitch.show();
    QToolButton *darkButton = themeSwitch.button(ThemeMode::Dark);
    QVERIFY(darkButton);
    darkButton->setFocus();
    QTest::keyClick(darkButton, Qt::Key_Space);
    QCOMPARE(themeSwitch.themeMode(), ThemeMode::Dark);
    QVERIFY(darkButton->isChecked());
}

void ComponentsTest::themeSwitchUsesPulseMotion()
{
    ThemeSwitch themeSwitch;
    themeSwitch.setProperty("_ngstdAnimationPolicy",
                            QStringLiteral("enabled"));
    themeSwitch.setThemeMode(ThemeMode::System);
    themeSwitch.show();
    themeSwitch.setThemeMode(ThemeMode::Light);
    QVariantAnimation *animation = themeSwitch.findChild<QVariantAnimation *>(
        QStringLiteral("_ngstdThemeSwitchAnimation"));
    QVERIFY(animation);
    QCOMPARE(animation->duration(),
             DesignTokens::duration(MotionDuration::Normal));
    QCOMPARE(animation->state(), QAbstractAnimation::Running);
}

QTEST_MAIN(ComponentsTest)

#include "test_components.moc"
