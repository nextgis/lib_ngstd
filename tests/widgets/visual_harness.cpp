/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Isolated canonical rendering of public Qt Widgets components
 *****************************************************************************/
#include <ngstd/widgets/adapters.h>
#include <ngstd/widgets/components.h>
#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/icons.h>
#include <ngstd/widgets/theme.h>
#include <ngstd/widgets/theme_options.h>
#include <ngstd/widgets/widget_style.h>

#include <QApplication>
#include <QAbstractAnimation>
#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDialogButtonBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QFocusFrame>
#include <QGraphicsOpacityEffect>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMouseEvent>
#include <QListWidget>
#include <QPainter>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QRadioButton>
#include <QScopedPointer>
#include <QSpinBox>
#include <QStyleOptionSpinBox>
#include <QTabWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QVariantAnimation>

#include <functional>

using namespace ngstd::widgets;

class WidgetVisualHarness final
{
public:
    WidgetVisualHarness(const QString &outputDirectory,
                        ThemeController *themeController,
                        NextgisProxyStyle *proxyStyle)
        : m_outputDirectory(outputDirectory),
          m_themeController(themeController), m_proxyStyle(proxyStyle)
    {}

    int run()
    {
        if (!QDir().mkpath(m_outputDirectory)) return 1;

        Button *button = new Button(QStringLiteral("Primary"));
        button->setVariant(ButtonVariant::Primary);
        button->setIconRole(IconRole::Check);
        if (!capture(QStringLiteral("button"), button)) return 1;
        if (!capture(QStringLiteral("buttons"), createButtonVariants()))
            return 1;

        Button *loadingButton = new Button(QStringLiteral("Loading"));
        loadingButton->setVariant(ButtonVariant::Secondary);
        loadingButton->setLoading(true);
        if (!capture(QStringLiteral("loading-button"), loadingButton))
            return 1;

        QLineEdit *lineEdit =
            new QLineEdit(QStringLiteral("District boundaries"));
        if (!capture(QStringLiteral("line-edit"), lineEdit, QSize(360, 40)))
            return 1;

        QComboBox *nativeComboBox = new QComboBox;
        nativeComboBox->addItems(
            {QStringLiteral("NextGIS Data"), QStringLiteral("Local file")});
        if (!capture(QStringLiteral("native-combo-box"), nativeComboBox,
                     QSize(360, 40))) {
            return 1;
        }
        if (!captureNativeComboPopup()) return 1;

        SearchField *searchField = new SearchField;
        searchField->setPlaceholderText(
            QStringLiteral("Address, layer, or category"));
        if (!capture(QStringLiteral("search-field"), searchField,
                     QSize(360, 40))) {
            return 1;
        }

        QSpinBox *spinBox = new QSpinBox;
        spinBox->setRange(0, 1000000);
        spinBox->setValue(12500);
        if (!capture(QStringLiteral("spin-box"), spinBox, QSize(360, 40)))
            return 1;

        QDoubleSpinBox *doubleSpinBox = new QDoubleSpinBox;
        doubleSpinBox->setRange(0.1, 1000.0);
        doubleSpinBox->setDecimals(1);
        doubleSpinBox->setSuffix(QStringLiteral(" m"));
        doubleSpinBox->setValue(5.0);
        if (!capture(QStringLiteral("double-spin-box"), doubleSpinBox,
                     QSize(360, 40))) {
            return 1;
        }

        QCheckBox *checkBox = new QCheckBox(QStringLiteral("Use local cache"));
        checkBox->setChecked(true);
        if (!capture(QStringLiteral("check-box"), checkBox)) return 1;

        QRadioButton *radioButton =
            new QRadioButton(QStringLiteral("Recommended"));
        radioButton->setChecked(true);
        if (!capture(QStringLiteral("radio-button"), radioButton)) return 1;

        Tag *tag = new Tag(QStringLiteral("Vector"));
        tag->setTone(SemanticTone::Information);
        if (!capture(QStringLiteral("tag"), tag)) return 1;

        if (!capture(QStringLiteral("tab-widget"), createTabWidget(),
                     QSize(420, 180))) {
            return 1;
        }

        Notice *notice = new Notice;
        notice->setTone(SemanticTone::Information);
        notice->setTitle(QStringLiteral("Information."));
        notice->setText(QStringLiteral("Changes take effect after saving."));
        if (!capture(QStringLiteral("notice"), notice, QSize(420, 48)))
            return 1;

        Disclosure *disclosure = new Disclosure;
        disclosure->setTitle(QStringLiteral("Layer options"));
        disclosure->setContentWidget(
            new QLabel(QStringLiteral("Coordinate reference system")));
        if (!capture(QStringLiteral("disclosure"), disclosure,
                     QSize(480, 50))) {
            return 1;
        }
        if (!capture(QStringLiteral("expandable-section"),
                     createExpandableSection(false), QSize(480, 88))) {
            return 1;
        }
        if (!capture(QStringLiteral("expandable-section-expanded"),
                     createExpandableSection(true), QSize(480, 148))) {
            return 1;
        }

        if (!capture(QStringLiteral("card"), createCard(), QSize(360, 120)))
            return 1;
        if (!capture(QStringLiteral("card-button"),
                     createCardButton(CardVariant::Default,
                                      QStringLiteral("Card with icon"),
                                      QStringLiteral("Reusable component")),
                     QSize(360, 130))) {
            return 1;
        }
        if (!capture(QStringLiteral("card-button-media"), createMediaCard(),
                     QSize(360, 280))) {
            return 1;
        }
        if (!capture(QStringLiteral("card-button-background"),
                     createBackgroundCard(), QSize(360, 260))) {
            return 1;
        }

        CardButton *selectableCard = createCardButton(
            CardVariant::Selectable, QStringLiteral("Cache layer"),
            QStringLiteral("An independent selectable option."));
        selectableCard->setChecked(true);
        if (!capture(QStringLiteral("card-button-selectable"), selectableCard,
                     QSize(360, 104))) {
            return 1;
        }
        if (!capture(
                QStringLiteral("card-button-data"),
                createCardButton(CardVariant::Data, QStringLiteral("Basemap"),
                                 QStringLiteral("Daily updated vector map")),
                QSize(360, 180))) {
            return 1;
        }
        if (!capture(QStringLiteral("card-button-toolbox"),
                     createCardButton(
                         CardVariant::Toolbox,
                         QStringLiteral("EXIF photos to Web GIS layer"),
                         QStringLiteral("Converts georeferenced photos")),
                     QSize(360, 160))) {
            return 1;
        }

        if (!capture(QStringLiteral("reveal-widget"), createRevealWidget(),
                     QSize(420, 64))) {
            return 1;
        }
        if (!capture(QStringLiteral("page-background"), createPageBackground(),
                     QSize(480, 240))) {
            return 1;
        }
        if (!capture(QStringLiteral("list-widget"), createListWidget(),
                     QSize(360, 150))) {
            return 1;
        }
        if (!capture(QStringLiteral("tree-widget"), createTreeWidget(),
                     QSize(360, 150))) {
            return 1;
        }
        if (!captureListWidgetHover()) return 1;
        if (!captureTreeWidgetHover()) return 1;
        if (!capture(QStringLiteral("table-widget"), createTableWidget(),
                     QSize(640, 160))) {
            return 1;
        }

        QDialogButtonBox *buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
            QDialogButtonBox::Apply);
        if (!capture(QStringLiteral("dialog-button-box"), buttonBox,
                     QSize(420, 40))) {
            return 1;
        }

        ThemeSwitch *themeSwitch = new ThemeSwitch;
        themeSwitch->setThemeMode(ThemeMode::Dark);
        if (!capture(QStringLiteral("theme-switch"), themeSwitch)) return 1;

        Spinner *spinner = new Spinner;
        spinner->setPhase(0.25);
        if (!capture(QStringLiteral("spinner"), spinner)) return 1;

        QProgressBar *progressBar = new QProgressBar;
        progressBar->setRange(0, 100);
        progressBar->setValue(60);
        progressBar->setTextVisible(false);
        if (!capture(QStringLiteral("progress-bar"), progressBar,
                     QSize(320, 10))) {
            return 1;
        }

        ComboBox *comboBox = new ComboBox;
        comboBox->addItems({QStringLiteral("Open menu"),
                            QStringLiteral("First option"),
                            QStringLiteral("Second option")});
        if (!capture(QStringLiteral("combo-box"), comboBox, QSize(320, 40)))
            return 1;

        if (!captureToast()) return 1;
        if (!m_themeController) return 1;
        m_themeController->setThemeMode(ThemeMode::Light);
        QApplication::processEvents();
        if (!captureThemeSwitchPulse()) return 1;
        if (!captureExpandedDisclosure()) return 1;
        if (!captureExpandableSectionHover()) return 1;
        if (!captureExpandableSectionTransition()) return 1;
        if (!capturePrimaryButtonHover()) return 1;
        if (!capturePrimaryButtonPressed()) return 1;
        if (!captureDisabledButtonHover()) return 1;
        if (!captureIconButtonPressed()) return 1;
        if (!captureDataButtonRipple()) return 1;
        if (!captureCheckedButton()) return 1;
        if (!captureButtonSelectionPulse()) return 1;
        if (!captureCardSelectionPulse()) return 1;
        if (!captureKeyboardFocusFrame()) return 1;
        if (!captureSpinBoxHover()) return 1;
        QSpinBox *compactSpinBox = new QSpinBox;
        compactSpinBox->setRange(0, 999);
        compactSpinBox->setValue(42);
        if (!capture(QStringLiteral("spin-box-compact"), compactSpinBox,
                     QSize(90, 42))) {
            return 1;
        }
        if (!captureSelectionPulse()) return 1;
        if (!captureComboPopup(QStringLiteral("combo-box-popup"), false))
            return 1;
        if (!captureComboPopup(QStringLiteral("combo-box-popup-hover"),
                               true)) {
            return 1;
        }
        return capture(QStringLiteral("table-widget-action"),
                       createTableActionWidget(), QSize(900, 160))
                   ? 0
                   : 1;
    }

private:
    bool captureListWidgetHover() const
    {
        QListWidget *list = createListWidget();
        list->clearSelection();
        list->setCurrentItem(nullptr);
        list->setMouseTracking(true);
        return capturePrepared(
            QStringLiteral("list-widget-hover"), list, QSize(360, 150),
            [](QWidget *widget) {
                QListWidget *listWidget = qobject_cast<QListWidget *>(widget);
                if (!listWidget) return false;
                const QModelIndex index = listWidget->model()->index(1, 0);
                const QPoint center = listWidget->visualRect(index).center();
                QMouseEvent moveEvent(
                    QEvent::MouseMove, QPointF(center),
                    listWidget->viewport()->mapToGlobal(center), Qt::NoButton,
                    Qt::NoButton, Qt::NoModifier);
                QApplication::sendEvent(listWidget->viewport(), &moveEvent);
                return true;
            });
    }

    bool captureTreeWidgetHover() const
    {
        QTreeWidget *tree = createTreeWidget();
        tree->clearSelection();
        tree->setCurrentItem(nullptr);
        tree->setMouseTracking(true);
        return capturePrepared(
            QStringLiteral("tree-widget-hover"), tree, QSize(360, 150),
            [](QWidget *widget) {
                QTreeWidget *treeWidget = qobject_cast<QTreeWidget *>(widget);
                if (!treeWidget || treeWidget->topLevelItemCount() == 0)
                    return false;
                const QModelIndex index = treeWidget->indexFromItem(
                    treeWidget->topLevelItem(0));
                const QPoint center = treeWidget->visualRect(index).center();
                QMouseEvent moveEvent(
                    QEvent::MouseMove, QPointF(center),
                    treeWidget->viewport()->mapToGlobal(center), Qt::NoButton,
                    Qt::NoButton, Qt::NoModifier);
                QApplication::sendEvent(treeWidget->viewport(), &moveEvent);
                return true;
            });
    }

    bool captureNativeComboPopup() const
    {
        QScopedPointer<QWidget> host(new QWidget);
        host->setProperty("_ngstdRole", QStringLiteral("page"));
        host->setAttribute(Qt::WA_StyledBackground, true);
        host->resize(320, 280);
        ThemeOptions options;
        options.setThemeMode(ThemeMode::Light);
        options.setAnimationPolicy(AnimationPolicy::Disabled);
        ThemeController *controller = ThemeController::attach(host.data(),
                                                               options);
        if (!controller) return false;

        QComboBox *comboBox = new QComboBox(host.data());
        comboBox->setGeometry(40, 24, 220, 42);
        comboBox->addItems({QStringLiteral("NextGIS Data"),
                            QStringLiteral("Local file"),
                            QStringLiteral("Connected service")});
        ComboBoxAdapter *adapter = ComboBoxAdapter::attach(comboBox);
        if (!adapter) return false;
        host->show();
        comboBox->showPopup();
        QApplication::sendPostedEvents();
        QApplication::processEvents();
        QApplication::processEvents();

        QAbstractItemView *view = comboBox->view();
        QWidget *popup = view ? view->window() : nullptr;
        if (!popup || !popup->isVisible() ||
            popup->property("_ngstdColorScheme").toString() !=
                QStringLiteral("light")) {
            return false;
        }
        const QRect comboGeometry(comboBox->mapToGlobal(QPoint()),
                                  comboBox->size());
        const QRect popupGeometry = popup->geometry();
        const QRect unionGeometry =
            comboGeometry.united(popupGeometry).adjusted(-4, -4, 4, 4);
        QImage image(unionGeometry.size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(
            DesignTokens::color(ColorRole::PageSoft, ColorScheme::Light));
        QPainter painter(&image);
        painter.drawPixmap(comboGeometry.topLeft() - unionGeometry.topLeft(),
                           comboBox->grab());
        painter.drawPixmap(popupGeometry.topLeft() - unionGeometry.topLeft(),
                           popup->grab());
        painter.end();
        const QString path = QStringLiteral("%1/native-combo-box-popup.png")
                                 .arg(m_outputDirectory);
        comboBox->hidePopup();
        controller->detach();
        return image.save(path);
    }

    bool capture(const QString &name, QWidget *widget,
                 const QSize &size = QSize()) const
    {
        QScopedPointer<QWidget> owner(widget);
        if (size.isValid())
            widget->resize(size);
        else
            widget->adjustSize();
        widget->show();
        QApplication::sendPostedEvents();
        QApplication::processEvents();
        QWidget *focusedWidget = QApplication::focusWidget();
        if (focusedWidget) focusedWidget->clearFocus();
        QApplication::processEvents();
        return save(name, widget);
    }

    bool save(const QString &name, QWidget *widget) const
    {
        if (!widget || !widget->isVisible() || !widget->size().isValid())
            return false;
        const QString path =
            QStringLiteral("%1/%2.png").arg(m_outputDirectory, name);
        return widget->grab().save(path);
    }

    bool capturePrepared(
        const QString &name, QWidget *widget, const QSize &size,
        const std::function<bool(QWidget *)> &prepare) const
    {
        QScopedPointer<QWidget> owner(widget);
        if (size.isValid())
            widget->resize(size);
        else
            widget->adjustSize();
        widget->show();
        QApplication::sendPostedEvents();
        QApplication::processEvents();
        QWidget *focusedWidget = QApplication::focusWidget();
        if (focusedWidget) focusedWidget->clearFocus();
        QApplication::processEvents();
        if (!prepare(widget)) return false;
        QApplication::processEvents();
        return save(name, widget);
    }

    bool captureThemeSwitchPulse() const
    {
        ThemeSwitch *themeSwitch = new ThemeSwitch;
        themeSwitch->setProperty("_ngstdAnimationPolicy",
                                 QStringLiteral("enabled"));
        themeSwitch->setThemeMode(ThemeMode::System);
        return capturePrepared(
            QStringLiteral("theme-switch-pulse"), themeSwitch, QSize(),
            [](QWidget *widget) {
                ThemeSwitch *themeSwitchWidget =
                    qobject_cast<ThemeSwitch *>(widget);
                if (!themeSwitchWidget) return false;
                themeSwitchWidget->setThemeMode(ThemeMode::Light);
                QVariantAnimation *animation =
                    themeSwitchWidget->findChild<QVariantAnimation *>(
                        QStringLiteral("_ngstdThemeSwitchAnimation"));
                if (!animation || animation->duration() <= 0) return false;
                animation->pause();
                animation->setCurrentTime(animation->duration() / 2);
                return true;
            });
    }

    bool captureExpandedDisclosure() const
    {
        Disclosure *disclosure = new Disclosure;
        disclosure->setProperty("_ngstdAnimationPolicy",
                                QStringLiteral("disabled"));
        disclosure->setTitle(QStringLiteral("Layer options"));
        QLabel *content =
            new QLabel(QStringLiteral("Coordinate reference system"));
        content->setFixedHeight(24);
        disclosure->setContentWidget(content);
        return capturePrepared(
            QStringLiteral("disclosure-expanded"), disclosure,
            QSize(480, 98), [](QWidget *widget) {
                Disclosure *disclosureWidget =
                    qobject_cast<Disclosure *>(widget);
                if (!disclosureWidget) return false;
                disclosureWidget->setExpanded(true);
                return true;
            });
    }

    bool captureExpandableSectionHover() const
    {
        ExpandableSection *section = createExpandableSection(false);
        section->setProperty("_ngstdAnimationPolicy",
                             QStringLiteral("enabled"));
        return capturePrepared(
            QStringLiteral("expandable-section-hover"), section,
            QSize(480, 88), [](QWidget *widget) {
                QAbstractButton *header =
                    widget->findChild<QAbstractButton *>(
                        QStringLiteral("_ngstdExpandableSectionHeader"));
                if (!header) return false;
                QEvent enterEvent(QEvent::Enter);
                QApplication::sendEvent(header, &enterEvent);
                QVariantAnimation *animation =
                    widget->findChild<QVariantAnimation *>(QStringLiteral(
                        "_ngstdExpandableSectionHoverAnimation"));
                if (!animation || animation->duration() <= 0) return false;
                animation->pause();
                animation->setCurrentTime(animation->duration());
                return true;
            });
    }

    bool captureExpandableSectionTransition() const
    {
        ExpandableSection *section = createExpandableSection(false);
        section->setProperty("_ngstdAnimationPolicy",
                             QStringLiteral("enabled"));
        return capturePrepared(
            QStringLiteral("expandable-section-transition"), section,
            QSize(), [](QWidget *widget) {
                ExpandableSection *expandableSection =
                    qobject_cast<ExpandableSection *>(widget);
                if (!expandableSection) return false;
                expandableSection->setExpanded(true);
                QVariantAnimation *chevronAnimation =
                    widget->findChild<QVariantAnimation *>(QStringLiteral(
                        "_ngstdExpandableSectionChevronAnimation"));
                QPropertyAnimation *revealAnimation =
                    widget->findChild<QPropertyAnimation *>(
                        QStringLiteral("_ngstdRevealAnimation"));
                if (!chevronAnimation || !revealAnimation ||
                    revealAnimation->duration() <= 0) {
                    return false;
                }
                chevronAnimation->pause();
                revealAnimation->pause();
                chevronAnimation->setCurrentTime(
                    chevronAnimation->duration() / 2);
                revealAnimation->setCurrentTime(
                    revealAnimation->duration() / 3);
                widget->adjustSize();
                return true;
            });
    }

    bool captureDataButtonRipple() const
    {
        Button *button = new Button(QStringLiteral("Data primary"));
        button->setVariant(ButtonVariant::DataFilled);
        button->setProperty("_ngstdAnimationPolicy",
                            QStringLiteral("enabled"));
        return capturePrepared(
            QStringLiteral("button-data-ripple"), button, QSize(180, 48),
            [](QWidget *widget) {
                Button *dataButton = qobject_cast<Button *>(widget);
                if (!dataButton) return false;
                QMouseEvent pressEvent(
                    QEvent::MouseButtonPress,
                    QPointF(dataButton->width() * 0.25,
                            dataButton->height() * 0.5),
                    dataButton->mapToGlobal(
                        QPoint(dataButton->width() / 4,
                               dataButton->height() / 2)),
                    Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(dataButton, &pressEvent);
                QVariantAnimation *animation =
                    dataButton->findChild<QVariantAnimation *>(
                        QStringLiteral("_ngstdButtonRippleAnimation"));
                if (!animation || animation->duration() <= 0) return false;
                animation->pause();
                animation->setCurrentTime(animation->duration() / 4);
                return true;
            });
    }

    bool capturePrimaryButtonHover() const
    {
        Button *button = new Button(QStringLiteral("Primary action"));
        button->setVariant(ButtonVariant::Primary);
        button->setIconRole(IconRole::Check);
        button->setProperty("_ngstdAnimationPolicy",
                            QStringLiteral("enabled"));
        return capturePrepared(
            QStringLiteral("button-primary-hover"), button, QSize(180, 40),
            [](QWidget *widget) {
                Button *primaryButton = qobject_cast<Button *>(widget);
                if (!primaryButton) return false;
                QEvent enterEvent(QEvent::Enter);
                QApplication::sendEvent(primaryButton, &enterEvent);
                QVariantAnimation *animation =
                    primaryButton->findChild<QVariantAnimation *>(
                        QStringLiteral("_ngstdButtonStateAnimation"));
                if (!animation || animation->duration() <= 0) return false;
                animation->pause();
                animation->setCurrentTime(animation->duration() / 2);
                return true;
            });
    }

    bool capturePrimaryButtonPressed() const
    {
        Button *button = new Button(QStringLiteral("Primary action"));
        button->setVariant(ButtonVariant::Primary);
        button->setIconRole(IconRole::Check);
        button->setProperty("_ngstdAnimationPolicy",
                            QStringLiteral("enabled"));
        return capturePrepared(
            QStringLiteral("button-primary-pressed"), button,
            QSize(180, 40), [](QWidget *widget) {
                Button *primaryButton = qobject_cast<Button *>(widget);
                if (!primaryButton) return false;
                const QPoint center = primaryButton->rect().center();
                QMouseEvent pressEvent(
                    QEvent::MouseButtonPress, QPointF(center),
                    primaryButton->mapToGlobal(center), Qt::LeftButton,
                    Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(primaryButton, &pressEvent);
                QVariantAnimation *animation =
                    primaryButton->findChild<QVariantAnimation *>(
                        QStringLiteral("_ngstdButtonStateAnimation"));
                if (!animation || animation->duration() <= 0) return false;
                animation->pause();
                animation->setCurrentTime(animation->duration() / 2);
                return true;
            });
    }

    bool captureDisabledButtonHover() const
    {
        Button *button = new Button(QStringLiteral("Disabled"));
        button->setVariant(ButtonVariant::Secondary);
        button->setEnabled(false);
        return capturePrepared(
            QStringLiteral("button-disabled-hover"), button,
            QSize(150, 40), [](QWidget *widget) {
                QEvent enterEvent(QEvent::Enter);
                QApplication::sendEvent(widget, &enterEvent);
                QVariantAnimation *animation =
                    widget->findChild<QVariantAnimation *>(
                        QStringLiteral("_ngstdButtonStateAnimation"));
                return animation &&
                       animation->state() == QAbstractAnimation::Stopped;
            });
    }

    bool captureIconButtonPressed() const
    {
        Button *button = new Button;
        button->setVariant(ButtonVariant::Icon);
        button->setIconRole(IconRole::Download);
        return capturePrepared(
            QStringLiteral("button-icon-pressed"), button, QSize(42, 42),
            [](QWidget *widget) {
                Button *iconButton = qobject_cast<Button *>(widget);
                if (!iconButton) return false;
                const QPoint center = iconButton->rect().center();
                QMouseEvent pressEvent(
                    QEvent::MouseButtonPress, QPointF(center),
                    iconButton->mapToGlobal(center), Qt::LeftButton,
                    Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(iconButton, &pressEvent);
                return iconButton->isDown();
            });
    }

    bool captureCheckedButton() const
    {
        Button *button = new Button(QStringLiteral("Active option"));
        button->setCheckable(true);
        button->setIconRole(IconRole::Check);
        button->setProperty("_ngstdAnimationPolicy",
                            QStringLiteral("disabled"));
        button->setChecked(true);
        return capture(QStringLiteral("button-checkable-checked"), button,
                       QSize(180, 40));
    }

    bool captureButtonSelectionPulse() const
    {
        Button *button = new Button(QStringLiteral("Snap to grid"));
        button->setVariant(ButtonVariant::Secondary);
        button->setCheckable(true);
        button->setProperty("_ngstdAnimationPolicy",
                            QStringLiteral("enabled"));
        return capturePrepared(
            QStringLiteral("button-checkable-pulse"), button,
            QSize(160, 40), [](QWidget *widget) {
                Button *checkableButton = qobject_cast<Button *>(widget);
                if (!checkableButton) return false;
                checkableButton->setChecked(true);
                QVariantAnimation *stateAnimation =
                    checkableButton->findChild<QVariantAnimation *>(
                        QStringLiteral("_ngstdButtonStateAnimation"));
                QVariantAnimation *feedbackAnimation =
                    checkableButton->findChild<QVariantAnimation *>(
                        QStringLiteral("_ngstdButtonFeedbackAnimation"));
                if (!stateAnimation || !feedbackAnimation ||
                    feedbackAnimation->duration() <= 0) {
                    return false;
                }
                stateAnimation->pause();
                stateAnimation->setCurrentTime(stateAnimation->duration());
                feedbackAnimation->pause();
                feedbackAnimation->setCurrentTime(
                    feedbackAnimation->duration() / 2);
                return true;
            });
    }

    bool captureCardSelectionPulse() const
    {
        CardButton *card = new CardButton;
        card->setAutoExclusive(true);
        card->setProperty("_ngstdAnimationPolicy",
                          QStringLiteral("enabled"));
        card->setTopWidget(new QLabel(QStringLiteral("Recommended option")));
        return capturePrepared(
            QStringLiteral("card-button-pulse"), card, QSize(260, 96),
            [](QWidget *widget) {
                CardButton *cardButton = qobject_cast<CardButton *>(widget);
                if (!cardButton) return false;
                cardButton->setChecked(true);
                QVariantAnimation *selectionAnimation =
                    cardButton->findChild<QVariantAnimation *>(
                        QStringLiteral("_ngstdCardSelectionAnimation"));
                QVariantAnimation *feedbackAnimation =
                    cardButton->findChild<QVariantAnimation *>(
                        QStringLiteral("_ngstdCardFeedbackAnimation"));
                if (!selectionAnimation || !feedbackAnimation ||
                    feedbackAnimation->duration() <= 0) {
                    return false;
                }
                selectionAnimation->pause();
                selectionAnimation->setCurrentTime(
                    selectionAnimation->duration());
                feedbackAnimation->pause();
                feedbackAnimation->setCurrentTime(
                    feedbackAnimation->duration() / 2);
                return true;
            });
    }

    bool captureKeyboardFocusFrame() const
    {
        QWidget *panel = new QWidget;
        QVBoxLayout *layout = new QVBoxLayout(panel);
        layout->setContentsMargins(24, 18, 24, 18);
        Button *button = new Button(QStringLiteral("Keyboard focus"));
        button->setObjectName(QStringLiteral("focusTarget"));
        layout->addWidget(button);
        return capturePrepared(
            QStringLiteral("button-keyboard-focus"), panel, QSize(220, 80),
            [](QWidget *widget) {
                Button *focusTarget =
                    widget->findChild<Button *>(QStringLiteral("focusTarget"));
                if (!focusTarget) return false;
                focusTarget->setFocus(Qt::TabFocusReason);
                QApplication::processEvents();
                QFocusFrame *focusFrame = widget->findChild<QFocusFrame *>(
                    QStringLiteral("_ngstdKeyboardFocusFrame"));
                return focusFrame && focusFrame->isVisible() &&
                       focusFrame->widget() == focusTarget;
            });
    }

    bool captureSpinBoxHover() const
    {
        QSpinBox *spinBox = new QSpinBox;
        spinBox->setRange(0, 1000);
        spinBox->setValue(125);
        spinBox->setProperty("_ngstdAnimationPolicy",
                             QStringLiteral("disabled"));
        return capturePrepared(
            QStringLiteral("spin-box-hover"), spinBox, QSize(240, 42),
            [](QWidget *widget) {
                QSpinBox *spinBoxWidget = qobject_cast<QSpinBox *>(widget);
                if (!spinBoxWidget) return false;
                QStyleOptionSpinBox option;
                option.initFrom(spinBoxWidget);
                option.subControls = QStyle::SC_All;
                const QRect upButton = spinBoxWidget->style()->subControlRect(
                    QStyle::CC_SpinBox, &option, QStyle::SC_SpinBoxUp,
                    spinBoxWidget);
                QMouseEvent moveEvent(
                    QEvent::MouseMove, QPointF(upButton.center()),
                    spinBoxWidget->mapToGlobal(upButton.center()),
                    Qt::NoButton, Qt::NoButton, Qt::NoModifier);
                QApplication::sendEvent(spinBoxWidget, &moveEvent);
                return true;
            });
    }

    bool captureSelectionPulse() const
    {
        QCheckBox *checkBox =
            new QCheckBox(QStringLiteral("Selected option"));
        checkBox->setProperty("_ngstdAnimationPolicy",
                              QStringLiteral("enabled"));
        return capturePrepared(
            QStringLiteral("check-box-pulse"), checkBox, QSize(180, 32),
            [this](QWidget *widget) {
                QCheckBox *checkBoxWidget = qobject_cast<QCheckBox *>(widget);
                if (!checkBoxWidget || !m_proxyStyle) return false;
                checkBoxWidget->setChecked(true);
                QVariantAnimation *selectionAnimation = nullptr;
                QVariantAnimation *feedbackAnimation = nullptr;
                const QList<QVariantAnimation *> animations =
                    m_proxyStyle->findChildren<QVariantAnimation *>();
                for (QVariantAnimation *animation : animations) {
                    if (animation->property("_ngstdAnimationWidget")
                            .value<QObject *>() != checkBoxWidget) {
                        continue;
                    }
                    if (animation->objectName() == QStringLiteral(
                                                      "_ngstdSelectionStateAnimation")) {
                        selectionAnimation = animation;
                    }
                    else if (animation->objectName() == QStringLiteral(
                                                           "_ngstdSelectionFeedbackAnimation")) {
                        feedbackAnimation = animation;
                    }
                }
                if (!selectionAnimation || !feedbackAnimation) return false;
                selectionAnimation->pause();
                selectionAnimation->setCurrentTime(
                    selectionAnimation->duration() / 2);
                feedbackAnimation->pause();
                feedbackAnimation->setCurrentTime(
                    feedbackAnimation->duration() / 4);
                return true;
            });
    }

    bool captureComboPopup(const QString &name, bool hovered) const
    {
        QScopedPointer<QWidget> host(new QWidget);
        host->setProperty("_ngstdRole", QStringLiteral("page"));
        host->setAttribute(Qt::WA_StyledBackground, true);
        host->resize(320, 280);
        ComboBox *comboBox = new ComboBox(host.data());
        comboBox->setGeometry(40, 24, 220, 42);
        comboBox->setProperty("_ngstdAnimationPolicy",
                              QStringLiteral("disabled"));
        comboBox->addItems({QStringLiteral("No gradient"),
                            QStringLiteral("Top to bottom"),
                            QStringLiteral("Bottom to top"),
                            QStringLiteral("From center")});
        host->show();
        comboBox->showPopup();
        QApplication::sendPostedEvents();
        QApplication::processEvents();
        QFrame *popup = comboBox->findChild<QFrame *>(
            QStringLiteral("_ngstdComboBoxPopup"));
        QListView *view = comboBox->findChild<QListView *>(
            QStringLiteral("_ngstdComboBoxPopupView"));
        if (!popup || !view || !popup->isVisible()) return false;
        if (hovered) {
            const QModelIndex index = view->model()->index(1, 0);
            view->setCurrentIndex(index);
            const QPoint itemCenter = view->visualRect(index).center();
            QMouseEvent moveEvent(QEvent::MouseMove, QPointF(itemCenter),
                                  view->viewport()->mapToGlobal(itemCenter),
                                  Qt::NoButton, Qt::NoButton,
                                  Qt::NoModifier);
            QApplication::sendEvent(view->viewport(), &moveEvent);
            QApplication::processEvents();
        }

        const QRect comboGeometry(
            comboBox->mapToGlobal(QPoint()), comboBox->size());
        const QRect popupGeometry = popup->geometry();
        const QRect unionGeometry =
            comboGeometry.united(popupGeometry).adjusted(-4, -4, 4, 4);
        QImage image(unionGeometry.size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(
            DesignTokens::color(ColorRole::PageSoft, ColorScheme::Light));
        QPainter painter(&image);
        painter.drawPixmap(comboGeometry.topLeft() - unionGeometry.topLeft(),
                           comboBox->grab());
        painter.drawPixmap(popupGeometry.topLeft() - unionGeometry.topLeft(),
                           popup->grab());
        const QString path =
            QStringLiteral("%1/%2.png").arg(m_outputDirectory, name);
        comboBox->hidePopup();
        return image.save(path);
    }

    TableWidget *createTableActionWidget() const
    {
        TableWidget *table = new TableWidget(2, 5);
        table->setHorizontalHeaderLabels({
            QStringLiteral("Layer"), QStringLiteral("Enabled"),
            QStringLiteral("Preview"), QStringLiteral("State"),
            QStringLiteral("Action"),
        });
        table->horizontalHeader()->setSectionResizeMode(0,
                                                        QHeaderView::Stretch);
        table->horizontalHeader()->setSectionResizeMode(
            1, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(
            2, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(3,
                                                        QHeaderView::Stretch);
        table->horizontalHeader()->setSectionResizeMode(
            4, QHeaderView::ResizeToContents);
        const QStringList layers = {QStringLiteral("districts"),
                                    QStringLiteral("roads")};
        for (int row = 0; row < layers.size(); ++row) {
            table->setCellContent(row, 0, new QLabel(layers.at(row)));
            QCheckBox *enabled = new QCheckBox;
            enabled->setChecked(row == 0);
            table->setCellContent(row, 1, enabled, Qt::AlignCenter);
            QLabel *preview = new QLabel(QStringLiteral("64 x 34"));
            preview->setFixedSize(64, 34);
            table->setCellContent(row, 2, preview, Qt::AlignCenter);
            Tag *state = new Tag(row == 0 ? QStringLiteral("Ready")
                                          : QStringLiteral("Processing"));
            table->setCellContent(row, 3, state, Qt::AlignCenter);
            Button *openButton = new Button(QStringLiteral("Open"));
            openButton->setVariant(ButtonVariant::Text);
            table->setCellContent(row, 4, openButton, Qt::AlignCenter);
        }
        return table;
    }

    QWidget *createButtonVariants() const
    {
        QWidget *panel = new QWidget;
        panel->setAutoFillBackground(true);
        QGridLayout *layout = new QGridLayout(panel);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setHorizontalSpacing(8);
        layout->setVerticalSpacing(8);
        const QList<QPair<ButtonVariant, QString>> variants = {
            {ButtonVariant::Primary, QStringLiteral("Primary")},
            {ButtonVariant::Secondary, QStringLiteral("Secondary")},
            {ButtonVariant::Text, QStringLiteral("Text")},
            {ButtonVariant::Danger, QStringLiteral("Delete")},
            {ButtonVariant::Hero, QStringLiteral("Promotional")},
            {ButtonVariant::Trial, QStringLiteral("Try Premium")},
            {ButtonVariant::DataFilled, QStringLiteral("Data primary")},
            {ButtonVariant::DataOutline, QStringLiteral("Data secondary")},
        };
        for (int index = 0; index < variants.size(); ++index) {
            const auto &variant = variants.at(index);
            Button *button = new Button(variant.second);
            button->setVariant(variant.first);
            layout->addWidget(button, index / 4, index % 4);
        }
        return panel;
    }

    QTabWidget *createTabWidget() const
    {
        QTabWidget *tabs = new QTabWidget;
        tabs->addTab(new QLabel(QStringLiteral("Layer display settings")),
                     QStringLiteral("Map"));
        tabs->addTab(new QLabel(QStringLiteral("Attribute table")),
                     QStringLiteral("Data"));
        tabs->addTab(new QLabel(QStringLiteral("Visualization rules")),
                     QStringLiteral("Style"));
        return tabs;
    }

    Card *createCard() const
    {
        Card *card = new Card;
        card->setVariant(CardVariant::Panel);
        card->contentLayout()->addWidget(createLabel(
            QStringLiteral("Passive card"), TypographyRole::Heading3));
        card->contentLayout()->addWidget(
            createLabel(QStringLiteral("Structured non-interactive content"),
                        TypographyRole::Body));
        return card;
    }

    CardButton *createCardButton(CardVariant variant, const QString &title,
                                 const QString &description) const
    {
        CardButton *card = new CardButton;
        card->setVariant(variant);
        card->setAccessibleName(title);
        card->setTopWidget(createLabel(title, TypographyRole::Heading3));
        card->setBodyWidget(createLabel(description, TypographyRole::Body));
        return card;
    }

    CardButton *createMediaCard() const
    {
        CardButton *card = new CardButton;
        card->setVariant(CardVariant::Media);
        card->setAccessibleName(QStringLiteral("Card with media"));
        QLabel *preview = new QLabel;
        preview->setPixmap(createPreview(QSize(360, 140)));
        preview->setScaledContents(true);
        QWidget *body = new QWidget;
        QVBoxLayout *bodyLayout = new QVBoxLayout(body);
        bodyLayout->setContentsMargins(16, 16, 16, 16);
        bodyLayout->addWidget(createLabel(QStringLiteral("Card with media"),
                                          TypographyRole::Heading3));
        bodyLayout->addWidget(createLabel(
            QStringLiteral("Media remains separate from the body copy."),
            TypographyRole::Body));
        bodyLayout->addStretch();
        card->setTopWidget(preview);
        card->setBodyWidget(body);
        return card;
    }

    CardButton *createBackgroundCard() const
    {
        CardButton *card = createCardButton(
            CardVariant::Background, QStringLiteral("Background card"),
            QStringLiteral("Text remains legible on a generated preview."));
        card->setBackgroundPixmap(createPreview(QSize(360, 260)));
        const QList<QLabel *> labels = card->findChildren<QLabel *>();
        for (QLabel *label : labels) {
            QPalette palette = label->palette();
            palette.setColor(
                QPalette::WindowText,
                DesignTokens::color(ColorRole::White, ColorScheme::Dark));
            label->setPalette(palette);
        }
        return card;
    }

    QLabel *createLabel(const QString &text, TypographyRole role) const
    {
        QLabel *label = new QLabel(text);
        label->setWordWrap(true);
        WidgetStyle::setTypographyRole(label, role);
        return label;
    }

    QPixmap createPreview(const QSize &size) const
    {
        QPixmap preview(size);
        preview.fill(
            DesignTokens::color(ColorRole::SurfaceMuted, ColorScheme::Dark));
        QPainter painter(&preview);
        QLinearGradient gradient(preview.rect().topLeft(),
                                 preview.rect().bottomRight());
        gradient.setColorAt(
            0.0, DesignTokens::color(ColorRole::Brand, ColorScheme::Dark));
        gradient.setColorAt(1.0, DesignTokens::color(ColorRole::SurfaceBrand,
                                                     ColorScheme::Dark));
        painter.fillRect(preview.rect(), gradient);
        painter.setPen(
            DesignTokens::color(ColorRole::Link, ColorScheme::Dark));
        for (int offset = 24; offset < size.width(); offset += 48)
            painter.drawLine(offset, 0, offset - 80, size.height());
        return preview;
    }

    RevealWidget *createRevealWidget() const
    {
        RevealWidget *reveal = new RevealWidget;
        QLabel *content =
            new QLabel(QStringLiteral("Content managed by RevealWidget"));
        content->setAlignment(Qt::AlignCenter);
        content->setFixedHeight(64);
        reveal->setContentWidget(content);
        reveal->setExpanded(true);
        return reveal;
    }

    ExpandableSection *createExpandableSection(bool expanded) const
    {
        ExpandableSection *section = new ExpandableSection(
            QStringLiteral("Components"),
            QStringLiteral("Reusable controls and patterns."),
            IconRole::Check);
        section->setProperty("_ngstdAnimationPolicy",
                             QStringLiteral("disabled"));
        QLabel *content = new QLabel(
            QStringLiteral("Assemble corporate screens from public widgets."));
        content->setFixedHeight(32);
        section->contentLayout()->addWidget(content);
        section->setExpanded(expanded);
        return section;
    }

    PageBackground *createPageBackground() const
    {
        PageBackground *background = new PageBackground;
        background->setVariant(PageBackgroundVariant::Main);
        background->setDecorationVisible(true);
        background->setGridVisible(true);
        background->setGradient(PageBackgroundGradient::Down);
        QVBoxLayout *layout = new QVBoxLayout(background);
        layout->setContentsMargins(32, 32, 32, 32);
        QLabel *label = createLabel(QStringLiteral("Page background"),
                                    TypographyRole::Heading2);
        layout->addWidget(label);
        layout->addStretch();
        return background;
    }

    QListWidget *createListWidget() const
    {
        QListWidget *list = new QListWidget;
        list->addItems({QStringLiteral("Basemap"),
                        QStringLiteral("Satellite imagery"),
                        QStringLiteral("Digital elevation model")});
        list->setCurrentRow(0);
        return list;
    }

    QTreeWidget *createTreeWidget() const
    {
        QTreeWidget *tree = new QTreeWidget;
        tree->setHeaderHidden(true);
        QTreeWidgetItem *project =
            new QTreeWidgetItem(tree, QStringList(QStringLiteral("Project")));
        new QTreeWidgetItem(
            project, QStringList(QStringLiteral("District boundaries")));
        new QTreeWidgetItem(project,
                            QStringList(QStringLiteral("Road network")));
        project->setExpanded(true);
        tree->setCurrentItem(project->child(0));
        return tree;
    }

    TableWidget *createTableWidget() const
    {
        TableWidget *table = new TableWidget(2, 3);
        table->setHorizontalHeaderLabels({QStringLiteral("Layer"),
                                          QStringLiteral("Enabled"),
                                          QStringLiteral("State")});
        table->horizontalHeader()->setSectionResizeMode(0,
                                                        QHeaderView::Stretch);
        table->horizontalHeader()->setSectionResizeMode(
            1, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(2,
                                                        QHeaderView::Stretch);
        const QStringList layers = {QStringLiteral("districts"),
                                    QStringLiteral("roads")};
        for (int row = 0; row < layers.size(); ++row) {
            table->setCellContent(row, 0, new QLabel(layers.at(row)));
            QCheckBox *enabled = new QCheckBox;
            enabled->setChecked(row == 0);
            table->setCellContent(row, 1, enabled, Qt::AlignCenter);
            Tag *state = new Tag(row == 0 ? QStringLiteral("Ready")
                                          : QStringLiteral("Processing"));
            state->setTone(row == 0 ? SemanticTone::Success
                                    : SemanticTone::Warning);
            table->setCellContent(row, 2, state, Qt::AlignCenter);
        }
        return table;
    }

    bool captureToast() const
    {
        QScopedPointer<QWidget> host(new QWidget);
        host->resize(420, 100);
        host->show();
        Toast *toast = new Toast(host.data());
        toast->showMessage(QStringLiteral("Canonical notification"), 5000);
        QApplication::sendPostedEvents();
        QApplication::processEvents();
        return save(QStringLiteral("toast"), toast);
    }

    QString m_outputDirectory;
    ThemeController *m_themeController;
    NextgisProxyStyle *m_proxyStyle;
};

int main(int argumentCount, char *argumentValues[])
{
    QApplication application(argumentCount, argumentValues);
    application.setApplicationName(
        QStringLiteral("ngstd_widgets_visual_harness"));
    application.setOrganizationName(QStringLiteral("NextGIS"));
    NextgisProxyStyle *proxyStyle = NextgisProxyStyle::create();
    application.setStyle(proxyStyle);

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Canonical visual harness for ngstd::widgets"));
    parser.addHelpOption();
    const QCommandLineOption outputOption(
        QStringLiteral("output-directory"),
        QStringLiteral("Directory for canonical widget screenshots."),
        QStringLiteral("directory"));
    parser.addOption(outputOption);
    parser.process(application);
    const QString outputDirectory = parser.value(outputOption);
    if (outputDirectory.isEmpty()) parser.showHelp(2);

    ThemeOptions options;
    options.setThemeMode(ThemeMode::Dark);
    options.setAnimationPolicy(AnimationPolicy::Disabled);
    ThemeController *themeController =
        ThemeController::applyToApplication(&application, options);
    if (!themeController) return 1;

    WidgetVisualHarness harness(outputDirectory, themeController, proxyStyle);
    return harness.run();
}
