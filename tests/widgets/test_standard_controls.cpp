#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/proxy_style.h>
#include <ngstd/widgets/theme.h>
#include <ngstd/widgets/widget_style.h>

#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QProgressBar>
#include <QRadioButton>
#include <QSpinBox>
#include <QStyleOptionButton>
#include <QStyleOptionComboBox>
#include <QStyleOptionSpinBox>
#include <QStyleOptionViewItem>
#include <QTabBar>
#include <QTabWidget>
#include <QTest>
#include <QTreeWidget>
#include <QVariantAnimation>
#include <QVBoxLayout>
#include <QWidget>

using namespace ngstd::widgets;

namespace {

bool containsApproxColor(const QImage &image, const QColor &expected,
                         int tolerance = 2)
{
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const QColor pixel = image.pixelColor(x, y);
            if (qAbs(pixel.red() - expected.red()) <= tolerance &&
                qAbs(pixel.green() - expected.green()) <= tolerance &&
                qAbs(pixel.blue() - expected.blue()) <= tolerance &&
                qAbs(pixel.alpha() - expected.alpha()) <= tolerance) {
                return true;
            }
        }
    }
    return false;
}

} // namespace

class StandardControlsTest final : public QObject
{
    Q_OBJECT

private slots:
    void standardQtClassesReceiveTheme();
    void selectionIndicatorsUseTokenGeometry();
    void itemViewCheckIndicatorsReserveTextSpace();
    void selectionIndicatorsUseMotionPolicy();
    void selectedTabUsesTokenUnderline();
    void tabSizingReservesPaintedTextInk();
    void selectedTabUsesMotionPolicy();
    void fieldBordersUseMotionPolicy();
    void fieldPolishEnforcesTokenHeight();
    void disabledSpinBoxStyleUsesDisabledTokens();
    void complexControlsUseTokenGeometry();
    void treeViewUsesTokenIndentation();
    void progressBarUsesTokenHeight();
    void wizardPageLayoutNormalizesDescendantSpacing();
    void wizardPageHeaderUsesDedicatedSpacing();
    void semanticStatesAreIndependent();
    void semanticContainerStylesApplyPublicRoles();
};

void StandardControlsTest::standardQtClassesReceiveTheme()
{
    QWidget root;
    QCheckBox checkBox(&root);
    QRadioButton radioButton(&root);
    QTabWidget tabWidget(&root);
    QLineEdit lineEdit(&root);
    ThemeController *controller = ThemeController::attach(&root);
    QVERIFY(controller);
    QVERIFY(!root.styleSheet().isEmpty());
    QCOMPARE(checkBox.palette(), root.palette());
    QCOMPARE(radioButton.palette(), root.palette());
    QCOMPARE(tabWidget.palette(), root.palette());
    QCOMPARE(lineEdit.palette(), root.palette());
    controller->detach();
}

void StandardControlsTest::selectionIndicatorsUseTokenGeometry()
{
    NextgisProxyStyle style(QStringLiteral("Fusion"));
    const int expectedSize =
        DesignTokens::componentMetric(ComponentMetric::SelectionIndicatorSize);

    QCheckBox checkBox(QStringLiteral("Checkbox"));
    checkBox.setStyle(&style);
    checkBox.resize(180, 42);
    QStyleOptionButton checkOption;
    checkOption.initFrom(&checkBox);
    checkOption.text = checkBox.text();
    const QRect checkIndicator = style.subElementRect(
        QStyle::SE_CheckBoxIndicator, &checkOption, &checkBox);
    QCOMPARE(checkIndicator.size(), QSize(expectedSize, expectedSize));
    QCOMPARE(checkIndicator.center().y(), checkBox.rect().center().y());

    QRadioButton radioButton(QStringLiteral("Radio"));
    radioButton.setStyle(&style);
    radioButton.resize(180, 42);
    QStyleOptionButton radioOption;
    radioOption.initFrom(&radioButton);
    radioOption.text = radioButton.text();
    const QRect radioIndicator = style.subElementRect(
        QStyle::SE_RadioButtonIndicator, &radioOption, &radioButton);
    QCOMPARE(radioIndicator.size(), QSize(expectedSize, expectedSize));
    QCOMPARE(radioIndicator.center().y(), radioButton.rect().center().y());
}

void StandardControlsTest::itemViewCheckIndicatorsReserveTextSpace()
{
    NextgisProxyStyle style(QStringLiteral("Fusion"));
    QListView listView;
    listView.setStyle(&style);

    for (Qt::LayoutDirection direction :
         {Qt::LeftToRight, Qt::RightToLeft}) {
        QStyleOptionViewItem itemOption;
        itemOption.initFrom(&listView);
        itemOption.direction = direction;
        itemOption.rect = QRect(0, 0, 240, 40);
        itemOption.features = QStyleOptionViewItem::HasCheckIndicator |
                              QStyleOptionViewItem::HasDisplay;
        itemOption.checkState = Qt::Checked;
        itemOption.text = QStringLiteral("Item");

        const QRect indicatorRectangle = style.subElementRect(
            QStyle::SE_ItemViewItemCheckIndicator, &itemOption, &listView);
        const QRect textRectangle = style.subElementRect(
            QStyle::SE_ItemViewItemText, &itemOption, &listView);
        QCOMPARE(indicatorRectangle.size(),
                 QSize(DesignTokens::componentMetric(
                           ComponentMetric::SelectionIndicatorSize),
                       DesignTokens::componentMetric(
                           ComponentMetric::SelectionIndicatorSize)));
        QCOMPARE(indicatorRectangle.center().y(),
                 itemOption.rect.center().y());
        QVERIFY(!indicatorRectangle.intersects(textRectangle));
        if (direction == Qt::LeftToRight) {
            QVERIFY(textRectangle.left() > indicatorRectangle.right());
        }
        else {
            QVERIFY(textRectangle.right() < indicatorRectangle.left());
        }
    }
}

void StandardControlsTest::selectionIndicatorsUseMotionPolicy()
{
    NextgisProxyStyle style(QStringLiteral("Fusion"));
    QCheckBox checkBox(QStringLiteral("Checkbox"));
    checkBox.setStyle(&style);
    checkBox.setProperty("_ngstdAnimationPolicy", QStringLiteral("enabled"));
    checkBox.resize(180, 42);
    checkBox.show();
    QCoreApplication::processEvents();

    QVERIFY(!checkBox.findChild<QWidget *>(
        QStringLiteral("_ngstdSelectionMotionOverlay"),
        Qt::FindDirectChildrenOnly));
    QVariantAnimation *selectionAnimation =
        style.findChild<QVariantAnimation *>(
            QStringLiteral("_ngstdSelectionStateAnimation"));
    QVariantAnimation *feedbackAnimation =
        style.findChild<QVariantAnimation *>(
            QStringLiteral("_ngstdSelectionFeedbackAnimation"));
    QVERIFY(selectionAnimation);
    QVERIFY(feedbackAnimation);
    QVERIFY(!style.findChild<QVariantAnimation *>(
        QStringLiteral("_ngstdSelectionFocusAnimation")));
    checkBox.setChecked(true);
    QCOMPARE(selectionAnimation->duration(),
             DesignTokens::duration(MotionDuration::Fast));
    QCOMPARE(feedbackAnimation->duration(),
             DesignTokens::duration(MotionDuration::Slow));
    QCOMPARE(selectionAnimation->state(), QAbstractAnimation::Running);
    QCOMPARE(feedbackAnimation->state(), QAbstractAnimation::Running);

    checkBox.setProperty("_ngstdAnimationPolicy", QStringLiteral("disabled"));
    checkBox.setChecked(false);
    QCOMPARE(selectionAnimation->duration(), 0);
    QCOMPARE(feedbackAnimation->duration(), 0);
    QCOMPARE(selectionAnimation->state(), QAbstractAnimation::Stopped);
    QCOMPARE(feedbackAnimation->state(), QAbstractAnimation::Stopped);
}

void StandardControlsTest::selectedTabUsesTokenUnderline()
{
    NextgisProxyStyle style(QStringLiteral("Fusion"));
    QWidget root;
    QTabWidget tabWidget(&root);
    tabWidget.setStyle(&style);
    tabWidget.addTab(new QWidget, QStringLiteral("Map"));
    tabWidget.addTab(new QWidget, QStringLiteral("Data"));
    tabWidget.tabBar()->setStyle(&style);
    tabWidget.resize(320, 160);
    ThemeController *controller = ThemeController::attach(&root);
    QVERIFY(controller);
    root.show();
    QCoreApplication::processEvents();

    QTabBar *tabBar = tabWidget.tabBar();
    QPixmap pixmap(tabBar->size());
    pixmap.fill(Qt::transparent);
    tabBar->render(&pixmap);
    const QImage image = pixmap.toImage();
    const QRect selectedRect = tabBar->tabRect(0);
    const QColor brand =
        DesignTokens::color(ColorRole::Brand, ColorScheme::Light);
    bool foundUnderline = false;
    const int underlineHeight =
        DesignTokens::componentMetric(ComponentMetric::TabUnderlineHeight);
    for (int y = selectedRect.bottom() - underlineHeight;
         y <= selectedRect.bottom(); ++y) {
        for (int x = selectedRect.left(); x <= selectedRect.right(); ++x) {
            if (image.pixelColor(x, y) == brand) {
                foundUnderline = true;
                break;
            }
        }
    }
    QVERIFY(foundUnderline);
    controller->detach();
}

void StandardControlsTest::tabSizingReservesPaintedTextInk()
{
    NextgisProxyStyle style(QStringLiteral("Fusion"));
    QTabBar tabBar;
    tabBar.setStyle(&style);
    tabBar.addTab(QStringLiteral("General"));
    tabBar.resize(320, 48);
    QStyleOptionTab option;
    option.initFrom(&tabBar);
    option.text = QStringLiteral("General");
    option.state |= QStyle::State_Selected;

    const QSize tabSize = style.sizeFromContents(
        QStyle::CT_TabBarTab, &option,
        option.fontMetrics.size(Qt::TextSingleLine, option.text), &tabBar);
    const int paintedTextWidth = QFontMetrics(
        DesignTokens::font(TypographyRole::Control))
                                     .boundingRect(option.text)
                                     .width();
    const int padding = DesignTokens::componentMetric(
        ComponentMetric::TabPaddingHorizontal);
    QVERIFY(tabSize.width() >= paintedTextWidth + padding * 2 + 4);
}

void StandardControlsTest::selectedTabUsesMotionPolicy()
{
    NextgisProxyStyle style(QStringLiteral("Fusion"));
    QTabWidget tabWidget;
    tabWidget.setStyle(&style);
    tabWidget.addTab(new QWidget, QStringLiteral("Map"));
    tabWidget.addTab(new QWidget, QStringLiteral("Data"));
    tabWidget.addTab(new QWidget, QStringLiteral("Style"));
    tabWidget.tabBar()->setStyle(&style);
    tabWidget.setProperty("_ngstdAnimationPolicy", QStringLiteral("enabled"));
    tabWidget.resize(320, 160);
    tabWidget.show();
    QCoreApplication::processEvents();

    QTabBar *tabBar = tabWidget.tabBar();
    QVERIFY(!tabBar->findChild<QWidget *>(
        QStringLiteral("_ngstdTabMotionOverlay"),
        Qt::FindDirectChildrenOnly));
    QVariantAnimation *animation = style.findChild<QVariantAnimation *>(
        QStringLiteral("_ngstdTabStateAnimation"));
    QVERIFY(animation);
    tabWidget.setCurrentIndex(1);
    QCOMPARE(animation->duration(),
             DesignTokens::duration(MotionDuration::Normal));
    QCOMPARE(animation->state(), QAbstractAnimation::Running);

    tabWidget.setProperty("_ngstdAnimationPolicy", QStringLiteral("disabled"));
    tabWidget.setCurrentIndex(2);
    QCOMPARE(animation->duration(), 0);
    QCOMPARE(animation->state(), QAbstractAnimation::Stopped);
}

void StandardControlsTest::fieldBordersUseMotionPolicy()
{
    NextgisProxyStyle style(QStringLiteral("Fusion"));
    QLineEdit lineEdit;
    lineEdit.setStyle(&style);
    lineEdit.setProperty("_ngstdAnimationPolicy", QStringLiteral("enabled"));
    lineEdit.resize(240, 40);
    lineEdit.show();
    QCoreApplication::processEvents();

    QVERIFY(!lineEdit.findChild<QWidget *>(
        QStringLiteral("_ngstdFieldMotionOverlay"),
        Qt::FindDirectChildrenOnly));
    QVariantAnimation *borderAnimation =
        style.findChild<QVariantAnimation *>(
            QStringLiteral("_ngstdFieldBorderAnimation"));
    QVERIFY(borderAnimation);
    QVERIFY(!style.findChild<QVariantAnimation *>(
        QStringLiteral("_ngstdFieldFocusAnimation")));

    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(&lineEdit, &enterEvent);
    QCOMPARE(borderAnimation->duration(),
             DesignTokens::duration(MotionDuration::Fast));
    QCOMPARE(borderAnimation->state(), QAbstractAnimation::Running);

    lineEdit.setProperty("_ngstdAnimationPolicy", QStringLiteral("disabled"));
    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(&lineEdit, &leaveEvent);
    QCOMPARE(borderAnimation->duration(), 0);
    QCOMPARE(borderAnimation->state(), QAbstractAnimation::Stopped);
}

void StandardControlsTest::fieldPolishEnforcesTokenHeight()
{
    NextgisProxyStyle style(QStringLiteral("Fusion"));
    QWidget root;
    root.setStyle(&style);
    root.setStyleSheet(QStringLiteral("QWidget { color: black; }"));
    QGridLayout layout(&root);
    QLineEdit lineEdit;
    QSpinBox spinBox;
    spinBox.setEnabled(false);
    layout.addWidget(&lineEdit, 0, 0);
    layout.addWidget(&spinBox, 0, 1);

    root.resize(320, 96);
    root.show();
    QCoreApplication::processEvents();

    const int expectedHeight =
        DesignTokens::controlHeight(ControlSize::Medium);
    QCOMPARE(lineEdit.height(), expectedHeight);
    QCOMPARE(spinBox.height(), expectedHeight);
    QCOMPARE(spinBox.minimumHeight(), expectedHeight);

    style.unpolish(&spinBox);
    QCOMPARE(spinBox.minimumHeight(), 0);
}

void StandardControlsTest::disabledSpinBoxStyleUsesDisabledTokens()
{
    NextgisProxyStyle style(QStringLiteral("Fusion"));
    QWidget root;
    root.setStyle(&style);
    QVBoxLayout layout(&root);
    QSpinBox spinBox;
    spinBox.setValue(42);
    spinBox.setEnabled(false);
    spinBox.resize(96, DesignTokens::controlHeight(ControlSize::Medium));
    layout.addWidget(&spinBox);

    ThemeOptions options;
    options.setThemeMode(ThemeMode::Light);
    ThemeController *controller = ThemeController::attach(&root, options);
    QVERIFY(controller);
    root.show();
    QCoreApplication::processEvents();

    const QColor lightDisabledText =
        DesignTokens::color(ColorRole::TextDisabled, ColorScheme::Light);
    const QColor lightSurfaceMuted =
        DesignTokens::color(ColorRole::SurfaceMuted, ColorScheme::Light);
    QCOMPARE(spinBox.palette().color(QPalette::Disabled, QPalette::Text),
             lightDisabledText);
    if (QLineEdit *lineEdit = spinBox.findChild<QLineEdit *>()) {
        QCOMPARE(lineEdit->palette().color(QPalette::Disabled,
                                           QPalette::Text),
                 lightDisabledText);
    }
    QImage lightImage(spinBox.size(), QImage::Format_ARGB32_Premultiplied);
    lightImage.fill(Qt::transparent);
    spinBox.render(&lightImage);
    QVERIFY(containsApproxColor(lightImage, lightSurfaceMuted));
    QVERIFY(containsApproxColor(lightImage, lightDisabledText, 8));

    controller->setThemeMode(ThemeMode::Dark);
    QCoreApplication::processEvents();
    const QColor darkDisabledText =
        DesignTokens::color(ColorRole::TextDisabled, ColorScheme::Dark);
    const QColor darkSurfaceMuted =
        DesignTokens::color(ColorRole::SurfaceMuted, ColorScheme::Dark);
    QCOMPARE(spinBox.palette().color(QPalette::Disabled, QPalette::Text),
             darkDisabledText);
    QImage darkImage(spinBox.size(), QImage::Format_ARGB32_Premultiplied);
    darkImage.fill(Qt::transparent);
    spinBox.render(&darkImage);
    QVERIFY(containsApproxColor(darkImage, darkSurfaceMuted));
    QVERIFY(containsApproxColor(darkImage, darkDisabledText, 8));
    controller->detach();
}

void StandardControlsTest::complexControlsUseTokenGeometry()
{
    NextgisProxyStyle style(QStringLiteral("Fusion"));
    QComboBox comboBox;
    comboBox.setStyle(&style);
    comboBox.resize(220, DesignTokens::controlHeight(ControlSize::Medium));
    QStyleOptionComboBox comboOption;
    comboOption.initFrom(&comboBox);
    comboOption.subControls = QStyle::SC_All;
    const QRect comboArrow = style.subControlRect(
        QStyle::CC_ComboBox, &comboOption, QStyle::SC_ComboBoxArrow,
        &comboBox);
    const QRect comboEdit = style.subControlRect(
        QStyle::CC_ComboBox, &comboOption, QStyle::SC_ComboBoxEditField,
        &comboBox);
    QCOMPARE(comboArrow.width(), DesignTokens::componentMetric(
                                     ComponentMetric::ComboBoxDropDownWidth));
    QVERIFY(!comboArrow.intersects(comboEdit));

    const QSize comboContents = comboBox.fontMetrics().size(
        Qt::TextSingleLine, QStringLiteral("System default"));
    const QSize comboSize = style.sizeFromContents(
        QStyle::CT_ComboBox, &comboOption, comboContents, &comboBox);
    QCOMPARE(comboSize.height(),
             DesignTokens::controlHeight(ControlSize::Medium));
    comboBox.resize(comboSize);
    comboOption.initFrom(&comboBox);
    const QRect contentSizedEdit = style.subControlRect(
        QStyle::CC_ComboBox, &comboOption, QStyle::SC_ComboBoxEditField,
        &comboBox);
    QVERIFY(contentSizedEdit.width() >= comboContents.width() + 4);

    QSpinBox spinBox;
    spinBox.setStyle(&style);
    spinBox.resize(90, DesignTokens::controlHeight(ControlSize::Medium));
    QStyleOptionSpinBox spinOption;
    spinOption.initFrom(&spinBox);
    spinOption.subControls = QStyle::SC_All;
    spinOption.stepEnabled = QAbstractSpinBox::StepUpEnabled |
                             QAbstractSpinBox::StepDownEnabled;
    const QRect upButton = style.subControlRect(
        QStyle::CC_SpinBox, &spinOption, QStyle::SC_SpinBoxUp, &spinBox);
    const QRect downButton = style.subControlRect(
        QStyle::CC_SpinBox, &spinOption, QStyle::SC_SpinBoxDown, &spinBox);
    const QRect spinEdit = style.subControlRect(
        QStyle::CC_SpinBox, &spinOption, QStyle::SC_SpinBoxEditField,
        &spinBox);
    const int expectedButtonWidth = DesignTokens::componentMetric(
        ComponentMetric::SpinBoxButtonWidth);
    QCOMPARE(upButton.width(), expectedButtonWidth);
    QCOMPARE(downButton.width(), expectedButtonWidth);
    QVERIFY(upButton.width() <= spinBox.width() / 3);
    QVERIFY(spinEdit.width() > upButton.width());
    QCOMPARE(upButton.height(), downButton.height());
    QCOMPARE(upButton.united(downButton).center().y(),
             spinBox.rect().center().y());
    QVERIFY(!upButton.intersects(spinEdit));
    QVERIFY(!downButton.intersects(spinEdit));
    QCOMPARE(upButton.bottom() + 1, downButton.top());

    QLineEdit lineEdit;
    lineEdit.setStyle(&style);
    QStyleOptionFrame lineEditOption;
    lineEditOption.initFrom(&lineEdit);
    const QSize lineEditSize = style.sizeFromContents(
        QStyle::CT_LineEdit, &lineEditOption,
        lineEdit.fontMetrics().size(Qt::TextSingleLine,
                                    QStringLiteral("Value")),
        &lineEdit);
    const QSize spinSize = style.sizeFromContents(
        QStyle::CT_SpinBox, &spinOption,
        spinBox.fontMetrics().size(Qt::TextSingleLine,
                                   QStringLiteral("65535")),
        &spinBox);
    QCOMPARE(lineEditSize.height(), comboSize.height());
    QCOMPARE(spinSize.height(), comboSize.height());
}

void StandardControlsTest::treeViewUsesTokenIndentation()
{
    NextgisProxyStyle style(QStringLiteral("Fusion"));
    QTreeWidget treeWidget;
    treeWidget.setStyle(&style);
    QCOMPARE(style.pixelMetric(QStyle::PM_TreeViewIndentation, nullptr,
                               &treeWidget),
             DesignTokens::componentMetric(
                 ComponentMetric::ItemViewIndentation));
}

void StandardControlsTest::progressBarUsesTokenHeight()
{
    QWidget root;
    QProgressBar progressBar(&root);
    ThemeController *controller = ThemeController::attach(&root);
    QVERIFY(controller);
    root.show();
    QCoreApplication::processEvents();
    const int expectedHeight =
        DesignTokens::componentMetric(ComponentMetric::ProgressHeight);
    QCOMPARE(progressBar.minimumHeight(), expectedHeight);
    QCOMPARE(progressBar.maximumHeight(), expectedHeight);
    controller->detach();
}

void StandardControlsTest::wizardPageLayoutNormalizesDescendantSpacing()
{
    QWidget page;
    QVBoxLayout rootLayout(&page);
    rootLayout.setContentsMargins(31, 29, 27, 23);
    rootLayout.setSpacing(14);

    QWidget content(&page);
    QGridLayout contentLayout(&content);
    contentLayout.setContentsMargins(18, 14, 10, 6);
    contentLayout.setHorizontalSpacing(28);
    contentLayout.setVerticalSpacing(7);
    rootLayout.addWidget(&content);

    WidgetStyle::applyWizardPageLayout(&page);

    const int pageMargin = DesignTokens::componentMetric(
        ComponentMetric::WizardPageMargin);
    QCOMPARE(rootLayout.contentsMargins(),
             QMargins(pageMargin, pageMargin, pageMargin, pageMargin));
    QCOMPARE(rootLayout.spacing(), DesignTokens::componentMetric(
                                      ComponentMetric::WizardPageSpacing));
    QCOMPARE(contentLayout.contentsMargins(),
             QMargins(DesignTokens::spacing(4), DesignTokens::spacing(3),
                      DesignTokens::spacing(2), DesignTokens::spacing(1)));
    QCOMPARE(contentLayout.horizontalSpacing(), DesignTokens::spacing(6));
    QCOMPARE(contentLayout.verticalSpacing(), DesignTokens::spacing(2));
}

void StandardControlsTest::wizardPageHeaderUsesDedicatedSpacing()
{
    QWidget page;
    QVBoxLayout layout(&page);
    QLabel title(QStringLiteral("Title"), &page);
    QLabel subtitle(QStringLiteral("Subtitle"), &page);
    QLabel body(QStringLiteral("Body"), &page);
    layout.addWidget(&title);
    layout.addWidget(&subtitle);
    layout.addWidget(&body);

    WidgetStyle::applyWizardPageLayout(&page);
    WidgetStyle::applyWizardPageHeader(&title, &subtitle);

    QCOMPARE(title.property("ngstdTypographyRole").toString(),
             QStringLiteral("heading2"));
    QCOMPARE(subtitle.property("ngstdTypographyRole").toString(),
             QStringLiteral("heading1Subtitle"));
    QLayout *headerLayout = layout.itemAt(0)->layout();
    QVERIFY(headerLayout);
    QCOMPARE(headerLayout->spacing(), DesignTokens::componentMetric(
                                          ComponentMetric::WizardTitleBottomSpacing));
    QCOMPARE(layout.spacing(), DesignTokens::componentMetric(
                                   ComponentMetric::WizardPageSpacing));
}

void StandardControlsTest::semanticStatesAreIndependent()
{
    QLineEdit lineEdit;
    WidgetStyle::setError(&lineEdit, true);
    WidgetStyle::setSelected(&lineEdit, true);
    QVERIFY(lineEdit.property("ngstdError").toBool());
    QVERIFY(lineEdit.property("ngstdSelected").toBool());
    WidgetStyle::setError(&lineEdit, false);
    QVERIFY(!lineEdit.property("ngstdError").toBool());
    QVERIFY(lineEdit.property("ngstdSelected").toBool());
}

void StandardControlsTest::semanticContainerStylesApplyPublicRoles()
{
    QFrame card;
    card.setFrameShape(QFrame::StyledPanel);
    WidgetStyle::setCardVariant(&card, CardVariant::Panel);
    QCOMPARE(card.property("_ngstdRole").toString(), QStringLiteral("card"));
    QCOMPARE(WidgetStyle::cardVariant(&card), CardVariant::Panel);
    QVERIFY(card.testAttribute(Qt::WA_StyledBackground));
    QCOMPARE(card.frameShape(), QFrame::NoFrame);

    QFrame notice;
    notice.setFrameShape(QFrame::StyledPanel);
    WidgetStyle::setNoticeTone(&notice, SemanticTone::Warning);
    QCOMPARE(notice.property("_ngstdRole").toString(),
             QStringLiteral("notice"));
    QCOMPARE(WidgetStyle::tone(&notice), SemanticTone::Warning);
    QVERIFY(notice.testAttribute(Qt::WA_StyledBackground));
    QCOMPARE(notice.frameShape(), QFrame::NoFrame);

    QWidget page;
    WidgetStyle::setPageBackgroundVariant(
        &page, PageBackgroundVariant::Corporate);
    QCOMPARE(page.property("_ngstdRole").toString(),
             QStringLiteral("pageBackground"));
    QCOMPARE(WidgetStyle::pageBackgroundVariant(&page),
             PageBackgroundVariant::Corporate);
    QVERIFY(page.testAttribute(Qt::WA_StyledBackground));
}

QTEST_MAIN(StandardControlsTest)

#include "test_standard_controls.moc"
