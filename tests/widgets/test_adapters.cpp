#include <ngstd/widgets/adapters.h>
#include <ngstd/widgets/combo_box.h>
#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/theme.h>

#include <QAccessible>
#include <QAccessibleInterface>
#include <QComboBox>
#include <QGraphicsOpacityEffect>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QListView>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTest>
#include <QVariantAnimation>
#include <QWizard>
#include <QWizardPage>

using namespace ngstd::widgets;

class AdaptersTest final : public QObject
{
    Q_OBJECT

private slots:
    void comboAdapterPreservesUserDelegate();
    void comboAdapterUsesScopedPopupSurface();
    void ownedComboPopupUsesPublicPlacement();
    void ownedComboSupportsKeyboardAndAccessibility();
    void ownedComboPopupUsesReadablePalette();
    void ownedComboPopupAnimatesLikeReference();
    void wizardAdapterIsConservativeByDefault();
    void wizardAdapterHandlesLatePages();
    void wizardHeaderTextUsesThemeColor();
};

void AdaptersTest::comboAdapterPreservesUserDelegate()
{
    QComboBox comboBox;
    QStyledItemDelegate *delegate = new QStyledItemDelegate(&comboBox);
    comboBox.setItemDelegate(delegate);
    const QPalette originalPalette = comboBox.view()->palette();
    const QPalette originalViewportPalette =
        comboBox.view()->viewport()->palette();
    const Qt::ScrollBarPolicy originalHorizontalPolicy =
        comboBox.view()->horizontalScrollBarPolicy();
    const bool originalViewMouseTracking =
        comboBox.view()->hasMouseTracking();
    const bool originalViewportMouseTracking =
        comboBox.view()->viewport()->hasMouseTracking();
    const bool originalViewportHover =
        comboBox.view()->viewport()->testAttribute(Qt::WA_Hover);
    ComboBoxAdapter *adapter = ComboBoxAdapter::attach(&comboBox);
    QVERIFY(adapter);
    QCOMPARE(comboBox.itemDelegate(), delegate);
    adapter->detach();
    QCOMPARE(comboBox.itemDelegate(), delegate);
    QCOMPARE(comboBox.view()->palette(), originalPalette);
    QCOMPARE(comboBox.view()->viewport()->palette(), originalViewportPalette);
    QCOMPARE(comboBox.view()->horizontalScrollBarPolicy(),
             originalHorizontalPolicy);
    QCOMPARE(comboBox.view()->hasMouseTracking(), originalViewMouseTracking);
    QCOMPARE(comboBox.view()->viewport()->hasMouseTracking(),
             originalViewportMouseTracking);
    QCOMPARE(comboBox.view()->viewport()->testAttribute(Qt::WA_Hover),
             originalViewportHover);
    QVERIFY(!comboBox.property("_ngstdComboBoxAdapter").isValid());
}

void AdaptersTest::comboAdapterUsesScopedPopupSurface()
{
    QWidget root;
    root.resize(360, 240);
    QComboBox comboBox(&root);
    comboBox.setGeometry(40, 24, 220, 42);
    comboBox.addItems({QStringLiteral("NextGIS Data"),
                       QStringLiteral("Local file"),
                       QStringLiteral("Connected service")});
    ThemeOptions options;
    options.setThemeMode(ThemeMode::Light);
    ThemeController *controller = ThemeController::attach(&root, options);
    QVERIFY(controller);
    ComboBoxAdapter *adapter = ComboBoxAdapter::attach(&comboBox);
    QVERIFY(adapter);

    root.show();
    comboBox.showPopup();
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();
    QWidget *popup = comboBox.view()->window();
    QVERIFY(popup);
    QVERIFY(popup->isVisible());
    QCOMPARE(popup->property("_ngstdColorScheme").toString(),
             QStringLiteral("light"));
    QCOMPARE(comboBox.view()->palette().color(QPalette::Text),
             DesignTokens::color(ColorRole::TextSecondary,
                                 ColorScheme::Light));
    const int popupGap = popup->geometry().top() -
                         comboBox.mapToGlobal(QPoint()).y() -
                         comboBox.height();
    QCOMPARE(popupGap,
             DesignTokens::componentMetric(
                 ComponentMetric::ComboBoxPopupOffset));
    const QImage popupImage = popup->grab().toImage();
    QVERIFY(popupImage.pixelColor(0, 0).alpha() < 16);
    comboBox.hidePopup();

    controller->setThemeMode(ThemeMode::Dark);
    comboBox.showPopup();
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();
    QCOMPARE(popup->property("_ngstdColorScheme").toString(),
             QStringLiteral("dark"));
    QCOMPARE(comboBox.view()->palette().color(QPalette::Text),
             DesignTokens::color(ColorRole::TextSecondary,
                                 ColorScheme::Dark));
    comboBox.hidePopup();
    controller->detach();
}

void AdaptersTest::ownedComboPopupUsesPublicPlacement()
{
    ComboBox comboBox;
    comboBox.addItems({
        QStringLiteral("One"),
        QStringLiteral("Two"),
    });
    comboBox.setPopupPlacement(ComboBoxPopupPlacement::Above);
    comboBox.setPopupAlignment(ComboBoxPopupAlignment::Right);
    comboBox.setProperty("_ngstdAnimationPolicy",
                         QStringLiteral("disabled"));
    comboBox.resize(180, 42);
    comboBox.move(300, 300);
    comboBox.show();
    comboBox.showPopup();
    QFrame *popup =
        comboBox.findChild<QFrame *>(QStringLiteral("_ngstdComboBoxPopup"));
    QVERIFY(popup);
    QVERIFY(popup->isVisible());
    const int gap = comboBox.mapToGlobal(QPoint()).y() -
                    popup->geometry().bottom() - 1;
    QCOMPARE(gap, DesignTokens::componentMetric(
                      ComponentMetric::ComboBoxPopupOffset));
    const QMargins popupMargins = popup->layout()->contentsMargins();
    const int expectedPadding = DesignTokens::componentMetric(
        ComponentMetric::ComboBoxPopupPadding);
    QCOMPARE(popupMargins,
             QMargins(expectedPadding, expectedPadding, expectedPadding,
                      expectedPadding));
    QListView *popupView = comboBox.findChild<QListView *>(
        QStringLiteral("_ngstdComboBoxPopupView"));
    QVERIFY(popupView);
    QCOMPARE(popupView->spacing(),
             DesignTokens::componentMetric(
                 ComponentMetric::ComboBoxPopupItemSpacing));
    const QImage popupImage = popup->grab().toImage();
    QVERIFY(popupImage.pixelColor(0, 0).alpha() < 16);
    comboBox.hidePopup();
}

void AdaptersTest::ownedComboSupportsKeyboardAndAccessibility()
{
    ComboBox comboBox;
    comboBox.setAccessibleName(QStringLiteral("Layer"));
    comboBox.addItems({
        QStringLiteral("Roads"),
        QStringLiteral("Buildings"),
    });
    comboBox.show();
    comboBox.setFocus();
    QTest::keyClick(&comboBox, Qt::Key_Down);
    QCOMPARE(comboBox.currentIndex(), 1);
    QTest::keyClick(&comboBox, Qt::Key_Down, Qt::AltModifier);
    QFrame *popup =
        comboBox.findChild<QFrame *>(QStringLiteral("_ngstdComboBoxPopup"));
    QVERIFY(popup);
    QTRY_VERIFY(popup->isVisible());
    comboBox.hidePopup();

    QAccessibleInterface *interface =
        QAccessible::queryAccessibleInterface(&comboBox);
    QVERIFY(interface);
    QCOMPARE(interface->text(QAccessible::Name), QStringLiteral("Layer"));
}

void AdaptersTest::ownedComboPopupUsesReadablePalette()
{
    QWidget root;
    ComboBox comboBox(&root);
    comboBox.addItems({
        QStringLiteral("NextGIS Web"),
        QStringLiteral("Local file"),
        QStringLiteral("WFS service"),
    });
    ThemeOptions options;
    options.setThemeMode(ThemeMode::Light);
    ThemeController *controller = ThemeController::attach(&root, options);
    QVERIFY(controller);
    comboBox.setProperty("_ngstdAnimationPolicy", QStringLiteral("disabled"));
    root.show();
    comboBox.showPopup();
    QFrame *popup =
        comboBox.findChild<QFrame *>(QStringLiteral("_ngstdComboBoxPopup"));
    QListView *popupView = comboBox.findChild<QListView *>(
        QStringLiteral("_ngstdComboBoxPopupView"));
    QVERIFY(popup);
    QVERIFY(popupView);
    QCOMPARE(
        popupView->palette().color(QPalette::Text),
        DesignTokens::color(ColorRole::TextSecondary, ColorScheme::Light));
    QCOMPARE(popup->grab().toImage().pixelColor(20, 50),
             DesignTokens::color(ColorRole::Surface, ColorScheme::Light));
    comboBox.hidePopup();

    controller->setThemeMode(ThemeMode::Dark);
    comboBox.showPopup();
    QCOMPARE(popupView->palette().color(QPalette::Text),
             DesignTokens::color(ColorRole::TextSecondary, ColorScheme::Dark));
    QCOMPARE(popup->grab().toImage().pixelColor(20, 50),
             DesignTokens::color(ColorRole::Surface, ColorScheme::Dark));
    comboBox.hidePopup();
    controller->detach();
}

void AdaptersTest::ownedComboPopupAnimatesLikeReference()
{
    ComboBox comboBox;
    comboBox.addItems({QStringLiteral("One"), QStringLiteral("Two")});
    comboBox.setProperty("_ngstdAnimationPolicy", QStringLiteral("enabled"));
    comboBox.resize(180, 40);
    comboBox.show();
    comboBox.showPopup();

    QFrame *popup =
        comboBox.findChild<QFrame *>(QStringLiteral("_ngstdComboBoxPopup"));
    QVariantAnimation *arrowAnimation =
        comboBox.findChild<QVariantAnimation *>(
            QStringLiteral("_ngstdComboBoxArrowAnimation"));
    QVariantAnimation *popupAnimation =
        comboBox.findChild<QVariantAnimation *>(
            QStringLiteral("_ngstdComboBoxPopupAnimation"));
    QVERIFY(popup);
    QVERIFY(arrowAnimation);
    QVERIFY(popupAnimation);
    QCOMPARE(arrowAnimation->duration(),
             DesignTokens::duration(MotionDuration::Normal));
    QCOMPARE(popupAnimation->duration(),
             DesignTokens::duration(MotionDuration::Fast));
    QCOMPARE(arrowAnimation->state(), QAbstractAnimation::Running);
    QCOMPARE(popupAnimation->state(), QAbstractAnimation::Running);

    QGraphicsOpacityEffect *effect =
        qobject_cast<QGraphicsOpacityEffect *>(popup->graphicsEffect());
    QVERIFY(effect);
    QTRY_COMPARE_WITH_TIMEOUT(effect->opacity(), 1.0, 500);
    comboBox.hidePopup();
    QVERIFY(!comboBox.property("_ngstdPopupOpen").toBool());
    QCOMPARE(popupAnimation->state(), QAbstractAnimation::Running);
    QTRY_VERIFY_WITH_TIMEOUT(!popup->isVisible(), 500);

    comboBox.setProperty("_ngstdAnimationPolicy", QStringLiteral("disabled"));
    comboBox.showPopup();
    QCOMPARE(popupAnimation->duration(), 0);
    QCOMPARE(effect->opacity(), 1.0);
    comboBox.hidePopup();
    QVERIFY(!popup->isVisible());
}

void AdaptersTest::wizardAdapterIsConservativeByDefault()
{
    QWizard wizard;
    wizard.addPage(new QWizardPage);
    wizard.setWizardStyle(QWizard::ModernStyle);
    const QWizard::WizardStyle originalStyle = wizard.wizardStyle();
    WizardAdapter *adapter = WizardAdapter::attach(&wizard);
    QVERIFY(adapter);
    QCOMPARE(wizard.wizardStyle(), originalStyle);
    adapter->setChangeWizardStyle(true);
    QCOMPARE(wizard.wizardStyle(), QWizard::ClassicStyle);
    adapter->detach();
    QCOMPARE(wizard.wizardStyle(), originalStyle);
}

void AdaptersTest::wizardAdapterHandlesLatePages()
{
    QWizard wizard;
    WizardAdapter *adapter = WizardAdapter::attach(&wizard);
    QWizardPage *page = new QWizardPage;
    page->setTitle(QStringLiteral("Late page"));
    wizard.addPage(page);
    QCoreApplication::processEvents();
    QTRY_COMPARE(page->accessibleName(), QStringLiteral("Late page"));
    adapter->detach();
    QCOMPARE(page->accessibleName(), QString());
}

void AdaptersTest::wizardHeaderTextUsesThemeColor()
{
    QWizard wizard;
    wizard.resize(720, 480);
    QWizardPage *page = new QWizardPage;
    page->setTitle(QStringLiteral("Connection settings"));
    page->setSubTitle(QStringLiteral("Fields use shared design tokens."));
    wizard.addPage(page);
    ThemeOptions options;
    options.setThemeMode(ThemeMode::Light);
    ThemeController *controller = ThemeController::attach(&wizard, options);
    QVERIFY(controller);
    WizardAdapter *adapter = WizardAdapter::attach(&wizard);
    QVERIFY(adapter);
    wizard.show();
    QCoreApplication::processEvents();

    const auto matchingPixels = [&wizard, page](ColorScheme scheme) {
        const QColor expected = DesignTokens::color(ColorRole::Text, scheme);
        int count = 0;
        const QList<QLabel *> labels = wizard.findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (label->text() != page->title() &&
                label->text() != page->subTitle()) {
                continue;
            }
            const QImage image = label->grab().toImage();
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    const QColor pixel = image.pixelColor(x, y);
                    const int distance =
                        qAbs(pixel.red() - expected.red()) +
                        qAbs(pixel.green() - expected.green()) +
                        qAbs(pixel.blue() - expected.blue());
                    if (pixel.alpha() > 0 && distance <= 12) ++count;
                }
            }
        }
        return count;
    };
    QVERIFY(matchingPixels(ColorScheme::Light) > 10);
    controller->setThemeMode(ThemeMode::Dark);
    QCoreApplication::processEvents();
    QVERIFY(matchingPixels(ColorScheme::Dark) > 10);
    adapter->detach();
    controller->detach();
}

QTEST_MAIN(AdaptersTest)

#include "test_adapters.moc"
