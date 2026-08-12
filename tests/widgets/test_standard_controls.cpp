#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/proxy_style.h>
#include <ngstd/widgets/theme.h>
#include <ngstd/widgets/widget_style.h>

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QProgressBar>
#include <QRadioButton>
#include <QSpinBox>
#include <QStyleOptionButton>
#include <QStyleOptionComboBox>
#include <QStyleOptionSpinBox>
#include <QTabBar>
#include <QTabWidget>
#include <QTest>
#include <QTreeWidget>
#include <QVariantAnimation>
#include <QWidget>

using namespace ngstd::widgets;

class StandardControlsTest final : public QObject
{
    Q_OBJECT

private slots:
    void standardQtClassesReceiveTheme();
    void selectionIndicatorsUseTokenGeometry();
    void selectionIndicatorsUseMotionPolicy();
    void selectedTabUsesTokenUnderline();
    void selectedTabUsesMotionPolicy();
    void fieldBordersUseMotionPolicy();
    void complexControlsUseTokenGeometry();
    void treeViewUsesTokenIndentation();
    void progressBarUsesTokenHeight();
    void semanticStatesAreIndependent();
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

QTEST_MAIN(StandardControlsTest)

#include "test_standard_controls.moc"
