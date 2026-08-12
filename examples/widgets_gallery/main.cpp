/******************************************************************************
 * Project: NextGIS widgets gallery
 * Purpose: Application entry point
 *****************************************************************************/
#include "demo_wizard.h"
#include "gallery_window.h"
#include "reference_widgets.h"

#include <ngstd/widgets/adapters.h>
#include <ngstd/widgets/button.h>
#include <ngstd/widgets/combo_box.h>
#include <ngstd/widgets/disclosure.h>
#include <ngstd/widgets/proxy_style.h>
#include <ngstd/widgets/theme.h>
#include <ngstd/widgets/types.h>
#include <ngstd/widgets/view_components.h>

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDir>
#include <QEvent>
#include <QFrame>
#include <QListView>
#include <QMouseEvent>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QRegularExpression>
#include <QTimer>

namespace {

ngstd::widgets::ThemeMode themeMode(const QString &value)
{
    if (value.compare(QStringLiteral("dark"), Qt::CaseInsensitive) == 0)
        return ngstd::widgets::ThemeMode::Dark;
    if (value.compare(QStringLiteral("system"), Qt::CaseInsensitive) == 0)
        return ngstd::widgets::ThemeMode::System;
    return ngstd::widgets::ThemeMode::Light;
}

QSize viewportSize(const QString &value)
{
    const QRegularExpression expression(QStringLiteral("^(\\d+)[xX](\\d+)$"));
    const QRegularExpressionMatch match = expression.match(value);
    if (!match.hasMatch()) return QSize();
    const int width = match.captured(1).toInt();
    const int height = match.captured(2).toInt();
    if (width <= 0 || height <= 0) return QSize();
    return QSize(width, height);
}

void startAnimationProbe(QApplication *application, GalleryWindow *window,
                         const QString &outputDirectory)
{
    if (!QDir().mkpath(outputDirectory)) {
        QTimer::singleShot(0, application,
                           [application]() { application->exit(1); });
        return;
    }
    using ngstd::widgets::Button;
    using ngstd::widgets::Disclosure;
    using ngstd::widgets::ExpandableSection;
    using ngstd::widgets::Spinner;
    Disclosure *disclosure = window->findChild<Disclosure *>(
        QStringLiteral("componentsDisclosure"));
    Spinner *spinner =
        window->findChild<Spinner *>(QStringLiteral("motionSpinner"));
    QProgressBar *progress =
        window->findChild<QProgressBar *>(QStringLiteral("motionProgress"));
    QPropertyAnimation *progressAnimation =
        progress ? progress->findChild<QPropertyAnimation *>(
                       QStringLiteral("motionProgressAnimation"))
                 : nullptr;
    Button *heroButton =
        window->findChild<Button *>(QStringLiteral("motionHeroButton"));
    Button *trialButton =
        window->findChild<Button *>(QStringLiteral("motionTrialButton"));
    Button *stateButton =
        window->findChild<Button *>(QStringLiteral("motionStateButton"));
    if (!disclosure || !spinner || !progress || !progressAnimation ||
        !heroButton || !trialButton || !stateButton) {
        QTimer::singleShot(0, application, &QApplication::quit);
        return;
    }
    for (QWidget *target :
         {static_cast<QWidget *>(disclosure), static_cast<QWidget *>(spinner),
          static_cast<QWidget *>(progress), static_cast<QWidget *>(heroButton),
          static_cast<QWidget *>(trialButton),
          static_cast<QWidget *>(stateButton)}) {
        QWidget *ancestor = target->parentWidget();
        while (ancestor) {
            ExpandableSection *section =
                qobject_cast<ExpandableSection *>(ancestor);
            if (section) {
                section->setExpanded(true, false);
                break;
            }
            ancestor = ancestor->parentWidget();
        }
    }
    disclosure->setProperty("_ngstdAnimationPolicy",
                            QStringLiteral("disabled"));
    disclosure->setExpanded(false);
    disclosure->setProperty("_ngstdAnimationPolicy",
                            QStringLiteral("enabled"));
    const QList<int> frameTimes = {0, 60, 120, 180, 300, 500, 760, 900, 960};
    QTimer::singleShot(
        100, window,
        [application, disclosure, spinner, progress, progressAnimation,
         heroButton, trialButton, stateButton, outputDirectory, frameTimes]() {
            for (int index = 0; index < frameTimes.size(); ++index) {
                const int elapsed = frameTimes.at(index);
                QTimer::singleShot(
                    elapsed, disclosure,
                    [application, disclosure, spinner, progress, heroButton,
                     trialButton, stateButton, outputDirectory, index, elapsed,
                     lastFrame = index == frameTimes.size() - 1]() {
                        const QString disclosurePath =
                            QStringLiteral("%1/disclosure_%2_%3ms.png")
                                .arg(outputDirectory)
                                .arg(index, 3, 10, QLatin1Char('0'))
                                .arg(elapsed, 4, 10, QLatin1Char('0'));
                        const QString spinnerPath =
                            QStringLiteral("%1/spinner_%2_%3ms.png")
                                .arg(outputDirectory)
                                .arg(index, 3, 10, QLatin1Char('0'))
                                .arg(elapsed, 4, 10, QLatin1Char('0'));
                        const QString heroPath =
                            QStringLiteral("%1/hero_%2_%3ms.png")
                                .arg(outputDirectory)
                                .arg(index, 3, 10, QLatin1Char('0'))
                                .arg(elapsed, 4, 10, QLatin1Char('0'));
                        const QString progressPath =
                            QStringLiteral("%1/progress_%2_%3ms.png")
                                .arg(outputDirectory)
                                .arg(index, 3, 10, QLatin1Char('0'))
                                .arg(elapsed, 4, 10, QLatin1Char('0'));
                        const QString trialPath =
                            QStringLiteral("%1/trial_%2_%3ms.png")
                                .arg(outputDirectory)
                                .arg(index, 3, 10, QLatin1Char('0'))
                                .arg(elapsed, 4, 10, QLatin1Char('0'));
                        const QString statePath =
                            QStringLiteral("%1/state_%2_%3ms.png")
                                .arg(outputDirectory)
                                .arg(index, 3, 10, QLatin1Char('0'))
                                .arg(elapsed, 4, 10, QLatin1Char('0'));
                        if (!disclosure->grab().save(disclosurePath) ||
                            !spinner->grab().save(spinnerPath) ||
                            !progress->grab().save(progressPath) ||
                            !heroButton->grab().save(heroPath) ||
                            !trialButton->grab().save(trialPath) ||
                            !stateButton->grab().save(statePath)) {
                            application->exit(1);
                            return;
                        }
                        if (lastFrame) application->quit();
                    });
            }
            disclosure->setExpanded(true);
            progressAnimation->stop();
            progressAnimation->start();
            QEvent heroEnter(QEvent::Enter);
            QApplication::sendEvent(heroButton, &heroEnter);
            QEvent trialEnter(QEvent::Enter);
            QApplication::sendEvent(trialButton, &trialEnter);
            QEvent stateEnter(QEvent::Enter);
            QApplication::sendEvent(stateButton, &stateEnter);
        });
}

void startThemeCycleProbe(QApplication *application, GalleryWindow *window,
                          const QString &screenshotPath)
{
    ngstd::widgets::ThemeSwitch *themeSwitch =
        window->findChild<ngstd::widgets::ThemeSwitch *>();
    if (!themeSwitch) {
        QTimer::singleShot(0, application,
                           [application]() { application->exit(1); });
        return;
    }
    const ngstd::widgets::ThemeMode initialMode = themeSwitch->themeMode();
    const ngstd::widgets::ThemeMode alternateMode =
        initialMode == ngstd::widgets::ThemeMode::Dark
            ? ngstd::widgets::ThemeMode::Light
            : ngstd::widgets::ThemeMode::Dark;
    const QList<ngstd::widgets::ThemeMode> modes = {
        alternateMode,
        ngstd::widgets::ThemeMode::System,
        alternateMode,
        initialMode,
    };
    QTimer *timer = new QTimer(application);
    timer->setInterval(80);
    QObject::connect(
        timer, &QTimer::timeout, application,
        [application, window, themeSwitch, modes, screenshotPath, timer,
         index = 0]() mutable {
            if (index >= modes.size()) {
                timer->stop();
                if (screenshotPath.isEmpty()) {
                    application->quit();
                    return;
                }
                QTimer::singleShot(
                    100, window, [application, window, screenshotPath]() {
                        application->exit(
                            window->grab().save(screenshotPath) ? 0 : 1);
                    });
                return;
            }
            themeSwitch->setThemeMode(modes.at(index));
            ++index;
        });
    timer->start();
}

void startAdapterProbe(QApplication *application, GalleryWindow *window,
                       const QString &outputDirectory)
{
    if (!QDir().mkpath(outputDirectory)) {
        QTimer::singleShot(0, application,
                           [application]() { application->exit(1); });
        return;
    }

    DemoWizard *wizard = new DemoWizard(window);
    ngstd::widgets::WizardAdapter::attach(wizard);
    wizard->show();
    QTimer::singleShot(150, wizard, [application, wizard, outputDirectory]() {
        const QString welcomePath =
            outputDirectory + QStringLiteral("/wizard-welcome.png");
        if (!wizard->grab().save(welcomePath)) {
            application->exit(1);
            return;
        }
        wizard->next();
        QTimer::singleShot(
            150, wizard, [application, wizard, outputDirectory]() {
                const QString settingsPath =
                    outputDirectory + QStringLiteral("/wizard-settings.png");
                if (!wizard->grab().save(settingsPath)) {
                    application->exit(1);
                    return;
                }
                ngstd::widgets::ComboBox *comboBox =
                    wizard->findChild<ngstd::widgets::ComboBox *>();
                if (!comboBox) {
                    application->exit(1);
                    return;
                }
                comboBox->showPopup();
                QTimer::singleShot(
                    150, wizard, [application, comboBox, outputDirectory]() {
                        QFrame *popup = comboBox->findChild<QFrame *>(
                            QStringLiteral("_ngstdComboBoxPopup"));
                        const QString popupPath =
                            outputDirectory +
                            QStringLiteral("/combo-popup.png");
                        if (!popup || !popup->isVisible() ||
                            !popup->grab().save(popupPath)) {
                            application->exit(1);
                            return;
                        }
                        comboBox->hidePopup();
                        application->quit();
                    });
            });
    });
}

void startInteractionProbe(QApplication *application, GalleryWindow *window,
                           const QString &outputDirectory)
{
    if (!QDir().mkpath(outputDirectory)) {
        QTimer::singleShot(0, application,
                           [application]() { application->exit(1); });
        return;
    }

    ngstd::widgets::ComboBox *source =
        window->findChild<ngstd::widgets::ComboBox *>(
            QStringLiteral("componentsSourceComboBox"));
    if (!source) {
        QTimer::singleShot(0, application,
                           [application]() { application->exit(1); });
        return;
    }

    source->showPopup();
    QTimer::singleShot(
        120, source, [application, source, outputDirectory]() {
            QFrame *popup = source->findChild<QFrame *>(
                QStringLiteral("_ngstdComboBoxPopup"));
            QListView *view = source->findChild<QListView *>(
                QStringLiteral("_ngstdComboBoxPopupView"));
            if (!popup || !view || !popup->isVisible()) {
                application->exit(1);
                return;
            }
            const QModelIndex index = view->model()->index(1, 0);
            const QPoint itemCenter = view->visualRect(index).center();
            QMouseEvent moveEvent(
                QEvent::MouseMove, QPointF(itemCenter),
                view->viewport()->mapToGlobal(itemCenter), Qt::NoButton,
                Qt::NoButton, Qt::NoModifier);
            QApplication::sendEvent(view->viewport(), &moveEvent);
            QApplication::processEvents();

            const QString sourcePath =
                outputDirectory +
                QStringLiteral("/source-combo-popup-hover.png");
            const bool saved = popup->grab().save(sourcePath);
            source->hidePopup();
            application->exit(saved ? 0 : 1);
        });
}

} // namespace

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("ngstd_widgets_gallery"));
    application.setOrganizationName(QStringLiteral("NextGIS"));
    application.setStyle(ngstd::widgets::NextgisProxyStyle::create());

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("NextGIS Qt Widgets component gallery"));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption themeOption(
        QStringLiteral("theme"),
        QStringLiteral("Initial theme: light, dark, or system."),
        QStringLiteral("theme"), QStringLiteral("light"));
    const QCommandLineOption sectionOption(
        QStringLiteral("section"), QStringLiteral("Open a gallery section."),
        QStringLiteral("section"));
    const QCommandLineOption screenshotOption(
        QStringLiteral("screenshot"),
        QStringLiteral("Save a deterministic screenshot and exit."),
        QStringLiteral("path"));
    const QCommandLineOption screenshotDelayOption(
        QStringLiteral("screenshot-delay"),
        QStringLiteral("Delay screenshot capture by MSEC milliseconds."),
        QStringLiteral("msec"), QStringLiteral("500"));
    const QCommandLineOption directionOption(
        QStringLiteral("direction"),
        QStringLiteral("Layout direction: ltr or rtl."),
        QStringLiteral("direction"), QStringLiteral("ltr"));
    const QCommandLineOption animationProbeOption(
        QStringLiteral("animation-probe"),
        QStringLiteral("Save disclosure, spinner, and promotional button "
                       "animation frames and exit."),
        QStringLiteral("directory"));
    const QCommandLineOption themeCycleProbeOption(
        QStringLiteral("theme-cycle-probe"),
        QStringLiteral("Cycle themes through ThemeSwitch and exit."));
    const QCommandLineOption adapterProbeOption(
        QStringLiteral("adapter-probe"),
        QStringLiteral("Save Wizard and ComboBox probe frames and exit."),
        QStringLiteral("directory"));
    const QCommandLineOption interactionProbeOption(
        QStringLiteral("interaction-probe"),
        QStringLiteral("Save canonical hover and popup interaction frames "
                       "and exit."),
        QStringLiteral("directory"));
    const QCommandLineOption viewportOption(
        QStringLiteral("viewport"),
        QStringLiteral("Set the capture viewport as WIDTHxHEIGHT."),
        QStringLiteral("size"));
    parser.addOption(themeOption);
    parser.addOption(sectionOption);
    parser.addOption(screenshotOption);
    parser.addOption(screenshotDelayOption);
    parser.addOption(directionOption);
    parser.addOption(animationProbeOption);
    parser.addOption(themeCycleProbeOption);
    parser.addOption(adapterProbeOption);
    parser.addOption(interactionProbeOption);
    parser.addOption(viewportOption);
    parser.process(application);

    const QString direction = parser.value(directionOption);
    application.setLayoutDirection(
        direction.compare(QStringLiteral("rtl"), Qt::CaseInsensitive) == 0
            ? Qt::RightToLeft
            : Qt::LeftToRight);
    ngstd::widgets::AnimationPolicy animationPolicy =
        ngstd::widgets::AnimationPolicy::Enabled;
    if (parser.isSet(screenshotOption))
        animationPolicy = ngstd::widgets::AnimationPolicy::Disabled;
    if (parser.isSet(animationProbeOption))
        animationPolicy = ngstd::widgets::AnimationPolicy::Enabled;
    const ngstd::widgets::ThemeMode initialTheme =
        themeMode(parser.value(themeOption));
    ngstd::widgets::ThemeOptions themeOptions;
    themeOptions.setThemeMode(initialTheme);
    themeOptions.setAnimationPolicy(animationPolicy);
    ngstd::widgets::ThemeController *themeController =
        ngstd::widgets::ThemeController::applyToApplication(&application,
                                                            themeOptions);
    if (!themeController) return 1;
    GalleryWindow window(initialTheme, parser.value(sectionOption),
                         animationPolicy, themeController);
    if (parser.isSet(viewportOption)) {
        const QSize requestedSize = viewportSize(parser.value(viewportOption));
        if (!requestedSize.isValid()) parser.showHelp(2);
        window.resize(requestedSize);
    }
    window.show();

    if (parser.isSet(themeCycleProbeOption)) {
        startThemeCycleProbe(&application, &window,
                             parser.value(screenshotOption));
        return application.exec();
    }

    const QString adapterDirectory = parser.value(adapterProbeOption);
    if (!adapterDirectory.isEmpty()) {
        startAdapterProbe(&application, &window, adapterDirectory);
        return application.exec();
    }

    const QString interactionDirectory =
        parser.value(interactionProbeOption);
    if (!interactionDirectory.isEmpty()) {
        startInteractionProbe(&application, &window, interactionDirectory);
        return application.exec();
    }

    const QString animationDirectory = parser.value(animationProbeOption);
    if (!animationDirectory.isEmpty()) {
        startAnimationProbe(&application, &window, animationDirectory);
        return application.exec();
    }
    const QString screenshotPath = parser.value(screenshotOption);
    if (!screenshotPath.isEmpty()) {
        bool validScreenshotDelay = false;
        const int screenshotDelay =
            parser.value(screenshotDelayOption).toInt(&validScreenshotDelay);
        if (!validScreenshotDelay || screenshotDelay < 0) parser.showHelp(2);
        QTimer::singleShot(screenshotDelay, &application,
                           [&application, &window, screenshotPath]() {
                               application.exit(
                                   window.grab().save(screenshotPath) ? 0 : 1);
                           });
    }
    return application.exec();
}
