#include <ngstd/widgets/proxy_style.h>
#include <ngstd/widgets/theme.h>

#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFocusEvent>
#include <QFocusFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QSignalSpy>
#include <QStyle>
#include <QTabWidget>
#include <QTest>
#include <QVariantAnimation>
#include <QVBoxLayout>
#include <QWidget>

using namespace ngstd::widgets;

class ThemeLifecycleTest final : public QObject
{
    Q_OBJECT

private slots:
    void detachRestoresCompleteRootState();
    void repeatedAttachUpdatesAllOptions();
    void nestedScopeUsesNearestAncestor();
    void nestedScopeSurvivesParentDetach();
    void lateChildrenAreDecorated();
    void scopedStyleSpecializationSurvivesPolish();
    void keyboardFocusStateTracksFocusReason();
    void equivalentSystemModeDoesNotRepolishScope();
    void systemModeTracksApplicationPalette();
    void applicationThemeDoesNotReplaceStyle();
    void scopedThemeSurvivesApplicationDetach();
    void destroyingRootDestroysController();
    void applicationThemeChangesWithProxyStyle();
};

void ThemeLifecycleTest::detachRestoresCompleteRootState()
{
    QWidget root;
    root.setStyleSheet(QStringLiteral("QWidget { padding: 1px; }"));
    root.setFont(QFont(QStringLiteral("Sans Serif"), 11));
    root.setProperty("userProperty", 42);
    const QString originalStyleSheet = root.styleSheet();
    const QFont originalFont = root.font();
    const bool originalPaletteAttribute =
        root.testAttribute(Qt::WA_SetPalette);

    ThemeOptions options;
    options.setThemeMode(ThemeMode::Dark);
    ThemeController *controller = ThemeController::attach(&root, options);
    QVERIFY(controller);
    QVERIFY(controller->isAttached());
    QVERIFY(root.styleSheet().contains(QStringLiteral("ngstd::widgets")));
    QVERIFY(root.property("_ngstdThemeScope").toBool());

    controller->detach();
    controller->detach();
    QCOMPARE(root.styleSheet(), originalStyleSheet);
    QCOMPARE(root.font(), originalFont);
    QCOMPARE(root.property("userProperty").toInt(), 42);
    QVERIFY(!root.property("_ngstdThemeScope").isValid());
    QCOMPARE(root.testAttribute(Qt::WA_SetPalette), originalPaletteAttribute);
}

void ThemeLifecycleTest::repeatedAttachUpdatesAllOptions()
{
    QWidget root;
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               &root);
    ThemeOptions firstOptions;
    firstOptions.setThemeMode(ThemeMode::Light);
    ThemeController *firstController =
        ThemeController::attach(&root, firstOptions);

    ThemeOptions secondOptions;
    secondOptions.setThemeMode(ThemeMode::Dark);
    secondOptions.setAnimationPolicy(AnimationPolicy::Disabled);
    secondOptions.setFeatures(ThemeFeature::StandardControls);
    ThemeController *secondController =
        ThemeController::attach(&root, secondOptions);

    QCOMPARE(firstController, secondController);
    QCOMPARE(secondController->options(), secondOptions);
    QCOMPARE(secondController->colorScheme(), ColorScheme::Dark);
    QCOMPARE(root.property("_ngstdAnimationPolicy").toString(),
             QStringLiteral("disabled"));
    QVERIFY(!buttonBox.button(QDialogButtonBox::Ok)
                 ->property("ngstdButtonVariant")
                 .isValid());
    secondController->detach();
}

void ThemeLifecycleTest::nestedScopeUsesNearestAncestor()
{
    QWidget outer;
    QWidget inner(&outer);
    QWidget leaf(&inner);
    ThemeOptions lightOptions;
    lightOptions.setThemeMode(ThemeMode::Light);
    ThemeOptions darkOptions;
    darkOptions.setThemeMode(ThemeMode::Dark);
    ThemeController *outerController =
        ThemeController::attach(&outer, lightOptions);
    ThemeController *innerController =
        ThemeController::attach(&inner, darkOptions);

    QCOMPARE(leaf.palette().color(QPalette::Window),
             DesignTokens::color(ColorRole::Background, ColorScheme::Dark));
    outerController->setThemeMode(ThemeMode::Dark);
    innerController->setThemeMode(ThemeMode::Light);
    QCOMPARE(leaf.palette().color(QPalette::Window),
             DesignTokens::color(ColorRole::Background, ColorScheme::Light));

    innerController->detach();
    QCOMPARE(leaf.palette().color(QPalette::Window),
             DesignTokens::color(ColorRole::Background, ColorScheme::Dark));
    outerController->detach();
}

void ThemeLifecycleTest::nestedScopeSurvivesParentDetach()
{
    QWidget outer;
    QWidget inner(&outer);
    QWidget leaf(&inner);
    ThemeOptions lightOptions;
    lightOptions.setThemeMode(ThemeMode::Light);
    ThemeOptions darkOptions;
    darkOptions.setThemeMode(ThemeMode::Dark);
    ThemeController *outerController =
        ThemeController::attach(&outer, lightOptions);
    ThemeController *innerController =
        ThemeController::attach(&inner, darkOptions);

    outerController->detach();
    QVERIFY(innerController->isAttached());
    QCOMPARE(leaf.palette().color(QPalette::Window),
             DesignTokens::color(ColorRole::Background, ColorScheme::Dark));
    innerController->detach();
}

void ThemeLifecycleTest::lateChildrenAreDecorated()
{
    QWidget root;
    ThemeController *controller = ThemeController::attach(&root);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &root);
    const QIcon originalOkIcon =
        buttonBox->button(QDialogButtonBox::Ok)->icon();
    const QIcon originalCancelIcon =
        buttonBox->button(QDialogButtonBox::Cancel)->icon();
    root.show();
    buttonBox->show();
    QCoreApplication::processEvents();
    QTRY_COMPARE(buttonBox->button(QDialogButtonBox::Ok)
                     ->property("ngstdButtonVariant")
                     .toString(),
                 QStringLiteral("primary"));
    QVERIFY(!buttonBox->button(QDialogButtonBox::Ok)->icon().isNull());
    QVERIFY(!buttonBox->button(QDialogButtonBox::Cancel)->icon().isNull());
    controller->detach();
    QVERIFY(!buttonBox->button(QDialogButtonBox::Ok)
                 ->property("ngstdButtonVariant")
                 .isValid());
    QCOMPARE(buttonBox->button(QDialogButtonBox::Ok)->icon().cacheKey(),
             originalOkIcon.cacheKey());
    QCOMPARE(buttonBox->button(QDialogButtonBox::Cancel)->icon().cacheKey(),
             originalCancelIcon.cacheKey());
}

void ThemeLifecycleTest::scopedStyleSpecializationSurvivesPolish()
{
    QWidget root;
    QLabel heading(QStringLiteral("Heading"), &root);
    heading.setProperty("_testRole", QStringLiteral("heading"));

    ThemeOptions options;
    options.setThemeMode(ThemeMode::Dark);
    ThemeController *controller = ThemeController::attach(&root, options);
    root.setStyleSheet(root.styleSheet() +
                       QStringLiteral("QLabel[_testRole=heading] {"
                                      " font-size: 20px; }"));
    root.show();
    heading.show();
    QCoreApplication::processEvents();

    QCOMPARE(heading.font().pixelSize(), 20);
    controller->detach();
}

void ThemeLifecycleTest::keyboardFocusStateTracksFocusReason()
{
    QWidget root;
    QPushButton button(QStringLiteral("Action"), &root);
    ThemeController *controller = ThemeController::attach(&root);
    root.show();
    button.show();

    QFocusEvent keyboardFocusIn(QEvent::FocusIn, Qt::TabFocusReason);
    QApplication::sendEvent(&button, &keyboardFocusIn);
    QVERIFY(button.property("_ngstdKeyboardFocus").toBool());
    QFocusFrame *focusFrame = root.findChild<QFocusFrame *>(
        QStringLiteral("_ngstdKeyboardFocusFrame"));
    QVERIFY(focusFrame);
    QCOMPARE(focusFrame->widget(), &button);
    QVERIFY(focusFrame->isVisible());

    QFocusEvent focusOut(QEvent::FocusOut, Qt::OtherFocusReason);
    QApplication::sendEvent(&button, &focusOut);
    QVERIFY(!button.property("_ngstdKeyboardFocus").toBool());
    QVERIFY(!focusFrame->isVisible());
    QVERIFY(!focusFrame->widget());
    QFocusEvent mouseFocusIn(QEvent::FocusIn, Qt::MouseFocusReason);
    QApplication::sendEvent(&button, &mouseFocusIn);
    QVERIFY(!button.property("_ngstdKeyboardFocus").toBool());

    controller->detach();
    QVERIFY(!button.property("_ngstdKeyboardFocus").isValid());
}

void ThemeLifecycleTest::equivalentSystemModeDoesNotRepolishScope()
{
    QWidget root;
    ThemeController *controller = ThemeController::attach(&root);
    QVERIFY(controller);
    const ThemeMode explicitMode =
        controller->colorScheme() == ColorScheme::Dark ? ThemeMode::Dark
                                                       : ThemeMode::Light;
    controller->setThemeMode(explicitMode);
    const QString styleSheet = root.styleSheet();
    const QPalette palette = root.palette();
    QSignalSpy schemeSpy(controller, &ThemeController::colorSchemeChanged);

    controller->setThemeMode(ThemeMode::System);

    QCOMPARE(controller->themeMode(), ThemeMode::System);
    QCOMPARE(controller->colorScheme(),
             explicitMode == ThemeMode::Dark ? ColorScheme::Dark
                                             : ColorScheme::Light);
    QCOMPARE(root.styleSheet(), styleSheet);
    QCOMPARE(root.palette(), palette);
    QCOMPARE(schemeSpy.count(), 0);
    controller->detach();
}

void ThemeLifecycleTest::systemModeTracksApplicationPalette()
{
    const QPalette originalPalette = qApp->palette();
    QPalette darkPalette = originalPalette;
    darkPalette.setColor(QPalette::Window, QColor(Qt::black));
    qApp->setPalette(darkPalette);
    QCoreApplication::processEvents();

    QWidget root;
    ThemeController *controller = ThemeController::attach(&root);
    QCOMPARE(controller->colorScheme(), ColorScheme::Dark);

    QPalette lightPalette = originalPalette;
    lightPalette.setColor(QPalette::Window, QColor(Qt::white));
    qApp->setPalette(lightPalette);
    QCoreApplication::processEvents();
    QCOMPARE(controller->colorScheme(), ColorScheme::Light);
    controller->detach();
    qApp->setPalette(originalPalette);
}

void ThemeLifecycleTest::applicationThemeDoesNotReplaceStyle()
{
    QStyle *originalStyle = qApp->style();
    ThemeOptions options;
    options.setThemeMode(ThemeMode::Dark);
    ThemeController *controller =
        ThemeController::applyToApplication(qApp, options);
    QVERIFY(controller);
    QCOMPARE(qApp->style(), originalStyle);
    controller->detach();
    QCOMPARE(qApp->style(), originalStyle);
}

void ThemeLifecycleTest::scopedThemeSurvivesApplicationDetach()
{
    QWidget root;
    ThemeOptions applicationOptions;
    applicationOptions.setThemeMode(ThemeMode::Light);
    ThemeOptions scopedOptions;
    scopedOptions.setThemeMode(ThemeMode::Dark);
    ThemeController *applicationController =
        ThemeController::applyToApplication(qApp, applicationOptions);
    ThemeController *scopedController =
        ThemeController::attach(&root, scopedOptions);

    applicationController->detach();
    QVERIFY(scopedController->isAttached());
    QCOMPARE(root.palette().color(QPalette::Window),
             DesignTokens::color(ColorRole::Background, ColorScheme::Dark));
    scopedController->detach();
}

void ThemeLifecycleTest::destroyingRootDestroysController()
{
    QWidget *root = new QWidget;
    QPointer<ThemeController> controller = ThemeController::attach(root);
    QVERIFY(controller);
    delete root;
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QVERIFY(controller.isNull());
}

void ThemeLifecycleTest::applicationThemeChangesWithProxyStyle()
{
    NextgisProxyStyle *style = NextgisProxyStyle::create();
    qApp->setStyle(style);

    ThemeOptions options;
    options.setThemeMode(ThemeMode::Light);
    ThemeController *controller =
        ThemeController::applyToApplication(qApp, options);
    QVERIFY(controller);

    QWidget root;
    QVBoxLayout *layout = new QVBoxLayout(&root);
    QCheckBox *checkBox = new QCheckBox(QStringLiteral("Check"), &root);
    QLineEdit *lineEdit = new QLineEdit(&root);
    QTabWidget *tabWidget = new QTabWidget(&root);
    tabWidget->addTab(new QWidget(tabWidget), QStringLiteral("First"));
    tabWidget->addTab(new QWidget(tabWidget), QStringLiteral("Second"));
    layout->addWidget(checkBox);
    layout->addWidget(lineEdit);
    layout->addWidget(tabWidget);
    root.show();
    QCoreApplication::processEvents();

    const auto hasLegacyOverlay = [&root]() {
        return root.findChild<QWidget *>(
                   QStringLiteral("_ngstdSelectionMotionOverlay")) ||
               root.findChild<QWidget *>(
                   QStringLiteral("_ngstdFieldMotionOverlay")) ||
               root.findChild<QWidget *>(
                   QStringLiteral("_ngstdTabMotionOverlay"));
    };
    QVERIFY(!hasLegacyOverlay());
    QCOMPARE(style->findChildren<QVariantAnimation *>().size(), 5);

    const QList<ThemeMode> modes = {ThemeMode::Dark, ThemeMode::Light,
                                    ThemeMode::System, ThemeMode::Dark};
    for (ThemeMode mode : modes) {
        controller->setThemeMode(mode);
        QCoreApplication::processEvents();
        QVERIFY(controller->isAttached());
        QVERIFY(!hasLegacyOverlay());
        QCOMPARE(style->findChildren<QVariantAnimation *>().size(), 5);
        QPixmap pixmap(root.size());
        pixmap.fill(Qt::transparent);
        root.render(&pixmap);
        QVERIFY(!pixmap.isNull());
    }

    controller->detach();
}

QTEST_MAIN(ThemeLifecycleTest)

#include "test_theme_lifecycle.moc"
