/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Explicit adapters for existing Qt widgets
 *****************************************************************************/
#include <ngstd/widgets/adapters.h>

#include "component_utils_p.h"
#include "state_journal_p.h"

#include <ngstd/widgets/widget_style.h>

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QComboBox>
#include <QEvent>
#include <QFrame>
#include <QLayout>
#include <QListView>
#include <QPointer>
#include <QTimer>
#include <QVariant>
#include <QWizard>
#include <QWizardPage>

#include <memory>
#include <utility>

namespace ngstd {
namespace widgets {

namespace {

const char comboAdapterObjectName[] = "_ngstd_combo_box_adapter";
const char wizardAdapterObjectName[] = "_ngstd_wizard_adapter";

bool isPrimaryRole(QWizard::WizardButton role)
{
    return role == QWizard::NextButton || role == QWizard::FinishButton ||
           role == QWizard::CommitButton;
}

QString colorName(const QColor &color)
{
    return color.name(QColor::HexArgb);
}

QString comboPopupStyleSheet(const QWidget *widget)
{
    const ColorScheme scheme = internal::colorSchemeFor(widget);
    const QColor surface = DesignTokens::color(ColorRole::Surface, scheme);
    const QColor border = DesignTokens::color(ColorRole::Border, scheme);
    const QColor text =
        DesignTokens::color(ColorRole::TextSecondary, scheme);
    const QColor disabledText =
        DesignTokens::color(ColorRole::TextDisabled, scheme);
    const QColor selectedSurface =
        DesignTokens::color(ColorRole::SurfaceBrand, scheme);
    const QColor selectedText = DesignTokens::color(ColorRole::Link, scheme);
    const int popupRadius = internal::tokenInteger(
        QStringLiteral("desktop.component.comboBox.popupRadiusPx"));
    const int popupBorder = internal::tokenInteger(
        QStringLiteral("desktop.component.comboBox.popupBorderWidthPx"));
    const int itemHeight = internal::tokenInteger(
        QStringLiteral("desktop.component.comboBox.popupItemHeightPx"));
    const int itemPadding = internal::tokenInteger(QStringLiteral(
        "desktop.component.comboBox.popupItemPaddingHorizontalPx"));
    const int itemRadius = DesignTokens::radius(RadiusRole::Field);
    return QStringLiteral(
               "QFrame[_ngstdAdaptedComboPopup=\"true\"] {"
               "background: %1; border: %2px solid %3; "
               "border-radius: %4px;}"
               "QAbstractItemView[_ngstdComboBoxPopupView=\"true\"] {"
               "background: transparent; border: 0; color: %5; outline: 0; "
               "selection-background-color: transparent; padding: 0;}"
               "QAbstractItemView[_ngstdComboBoxPopupView=\"true\"]::item {"
               "min-height: %6px; padding: 0 %7px; border: 0; "
               "border-radius: %8px; color: %5;}"
               "QAbstractItemView[_ngstdComboBoxPopupView=\"true\"]::item:disabled {"
               "color: %9;}"
               "QAbstractItemView[_ngstdComboBoxPopupView=\"true\"]::item:hover,"
               "QAbstractItemView[_ngstdComboBoxPopupView=\"true\"]::item:selected {"
               "background: %10; color: %11; font-weight: 500;}")
        .arg(colorName(surface))
        .arg(popupBorder)
        .arg(colorName(border))
        .arg(popupRadius)
        .arg(colorName(text))
        .arg(itemHeight)
        .arg(itemPadding)
        .arg(itemRadius)
        .arg(colorName(disabledText))
        .arg(colorName(selectedSurface))
        .arg(colorName(selectedText));
}

} // namespace

class ComboBoxAdapterPrivate final
{
public:
    struct ScrollerState
    {
        QPointer<QWidget> widget;
        bool visible = false;
    };

    QPointer<QComboBox> comboBox;
    QPointer<QAbstractItemView> view;
    QPointer<QWidget> popup;
    std::unique_ptr<internal::StateJournal> journal;
    QMargins popupLayoutMargins;
    QVector<ScrollerState> scrollers;
    Qt::ScrollBarPolicy horizontalScrollBarPolicy = Qt::ScrollBarAsNeeded;
    bool popupLayoutMarginsCaptured = false;
    bool horizontalScrollBarPolicyCaptured = false;
    bool viewMouseTracking = false;
    bool viewportMouseTracking = false;
    bool viewportHover = false;
    bool popupTranslucent = false;
    bool viewportTranslucent = false;
    bool attached = true;
};

ComboBoxAdapter *ComboBoxAdapter::attach(QComboBox *comboBox)
{
    if (!comboBox) return nullptr;
    ComboBoxAdapter *adapter = comboBox->findChild<ComboBoxAdapter *>(
        QString::fromLatin1(comboAdapterObjectName),
        Qt::FindDirectChildrenOnly);
    if (adapter) {
        adapter->apply();
        return adapter;
    }
    return new ComboBoxAdapter(comboBox);
}

ComboBoxAdapter::ComboBoxAdapter(QComboBox *comboBox)
    : QObject(comboBox), d(new ComboBoxAdapterPrivate)
{
    setObjectName(QString::fromLatin1(comboAdapterObjectName));
    d->comboBox = comboBox;
    apply();
}

ComboBoxAdapter::~ComboBoxAdapter()
{
    detach();
}

QComboBox *ComboBoxAdapter::comboBox() const
{
    return d->comboBox.data();
}

bool ComboBoxAdapter::isAttached() const
{
    return d->attached;
}

void ComboBoxAdapter::apply()
{
    if (!d->attached || !d->comboBox || !d->comboBox->view()) return;
    if (d->journal) {
        refreshPopup();
        return;
    }
    d->journal.reset(new internal::StateJournal);
    QAbstractItemView *view = d->comboBox->view();
    d->view = view;
    d->horizontalScrollBarPolicy = view->horizontalScrollBarPolicy();
    d->horizontalScrollBarPolicyCaptured = true;
    d->viewMouseTracking = view->hasMouseTracking();
    d->viewportMouseTracking = view->viewport()->hasMouseTracking();
    d->viewportHover = view->viewport()->testAttribute(Qt::WA_Hover);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setMouseTracking(true);
    view->viewport()->setMouseTracking(true);
    view->viewport()->setAttribute(Qt::WA_Hover, true);
    d->comboBox->installEventFilter(this);
    view->installEventFilter(this);
    d->journal->captureWidget(view);
    d->journal->captureWidget(view->viewport());
    d->journal->setProperty(d->comboBox.data(),
                            QByteArrayLiteral("_ngstdComboBoxAdapter"), true);
    d->journal->setProperty(view, QByteArrayLiteral("_ngstdComboBoxPopupView"),
                            true);
    refreshPopup();
}

bool ComboBoxAdapter::eventFilter(QObject *watched, QEvent *event)
{
    if (!d->attached || !event) return QObject::eventFilter(watched, event);
    bool position = false;
    bool refresh = false;
    if (watched == d->comboBox &&
        (event->type() == QEvent::PaletteChange ||
         event->type() == QEvent::StyleChange ||
         event->type() == QEvent::ChildAdded)) {
        refresh = true;
    }
    else if (watched == d->view &&
             (event->type() == QEvent::Show ||
              event->type() == QEvent::ParentChange)) {
        refresh = true;
        position = event->type() == QEvent::Show;
    }
    else if (watched == d->popup && event->type() == QEvent::Show) {
        refresh = true;
        position = true;
    }
    if (refresh) {
        QTimer::singleShot(0, this, [this, position]() {
            refreshPopup(position);
        });
    }
    return QObject::eventFilter(watched, event);
}

void ComboBoxAdapter::refreshPopup(bool position)
{
    if (!d->attached || !d->comboBox || !d->journal) return;
    QAbstractItemView *view = d->comboBox->view();
    if (!view) return;
    if (d->view != view) {
        if (d->view) {
            d->view->removeEventFilter(this);
            if (d->horizontalScrollBarPolicyCaptured) {
                d->view->setHorizontalScrollBarPolicy(
                    d->horizontalScrollBarPolicy);
            }
            d->view->setMouseTracking(d->viewMouseTracking);
            d->view->viewport()->setMouseTracking(d->viewportMouseTracking);
            d->view->viewport()->setAttribute(Qt::WA_Hover,
                                              d->viewportHover);
        }
        d->view = view;
        d->horizontalScrollBarPolicy = view->horizontalScrollBarPolicy();
        d->horizontalScrollBarPolicyCaptured = true;
        d->viewMouseTracking = view->hasMouseTracking();
        d->viewportMouseTracking = view->viewport()->hasMouseTracking();
        d->viewportHover = view->viewport()->testAttribute(Qt::WA_Hover);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setMouseTracking(true);
        view->viewport()->setMouseTracking(true);
        view->viewport()->setAttribute(Qt::WA_Hover, true);
        d->view->installEventFilter(this);
        d->journal->captureWidget(view);
        d->journal->captureWidget(view->viewport());
        d->journal->setProperty(
            view, QByteArrayLiteral("_ngstdComboBoxPopupView"), true);
    }

    const ColorScheme scheme = internal::colorSchemeFor(d->comboBox.data());
    const QString schemeValue = scheme == ColorScheme::Dark
                                    ? QStringLiteral("dark")
                                    : QStringLiteral("light");
    const QPalette popupPalette =
        internal::comboBoxPopupPalette(d->comboBox.data());
    view->setPalette(popupPalette);
    view->viewport()->setPalette(popupPalette);
    d->journal->setProperty(view, QByteArrayLiteral("_ngstdColorScheme"),
                            schemeValue);
    d->journal->setProperty(view->viewport(),
                            QByteArrayLiteral("_ngstdColorScheme"),
                            schemeValue);

    QWidget *popup = view->window();
    if (!popup || popup == d->comboBox->window()) {
        view->update();
        return;
    }
    if (d->popup != popup) {
        if (d->popup) d->popup->removeEventFilter(this);
        d->popup = popup;
        d->popup->installEventFilter(this);
        d->journal->captureWidget(popup);
        d->popupTranslucent =
            popup->testAttribute(Qt::WA_TranslucentBackground);
        d->viewportTranslucent =
            view->viewport()->testAttribute(Qt::WA_TranslucentBackground);
        if (popup->layout()) {
            d->popupLayoutMargins = popup->layout()->contentsMargins();
            d->popupLayoutMarginsCaptured = true;
        }
    }
    d->journal->setProperty(popup,
                            QByteArrayLiteral("_ngstdAdaptedComboPopup"),
                            true);
    d->journal->setProperty(popup, QByteArrayLiteral("_ngstdColorScheme"),
                            schemeValue);
    popup->setPalette(popupPalette);
    popup->setAttribute(Qt::WA_TranslucentBackground, true);
    popup->setAutoFillBackground(false);
    view->viewport()->setAttribute(Qt::WA_TranslucentBackground, true);
    view->viewport()->setAutoFillBackground(false);
    const int popupPadding = DesignTokens::componentMetric(
        ComponentMetric::ComboBoxPopupPadding);
    if (popup->layout()) {
        popup->layout()->setContentsMargins(popupPadding, popupPadding,
                                            popupPadding, popupPadding);
    }
    popup->setStyleSheet(comboPopupStyleSheet(d->comboBox.data()));
    popup->ensurePolished();
    const int visibleRows =
        qMin(d->comboBox->maxVisibleItems(), d->comboBox->count());
    const int nativeRowHeight = view->sizeHintForRow(0);
    const int rowHeight =
        nativeRowHeight > 0
            ? nativeRowHeight
            : internal::tokenInteger(QStringLiteral(
                  "desktop.component.comboBox.popupItemHeightPx"));
    const QListView *listView = qobject_cast<const QListView *>(view);
    const int spacing = listView ? listView->spacing() : 0;
    const int popupBorder = internal::tokenInteger(
        QStringLiteral("desktop.component.comboBox.popupBorderWidthPx"));
    const int popupHeight =
        qMax(rowHeight, visibleRows * rowHeight +
                            qMax(0, visibleRows - 1) * spacing +
                            popupPadding * 2 + popupBorder * 2);
    popup->resize(popup->width(), popupHeight);
    if (popup->layout()) popup->layout()->activate();
    const QList<QWidget *> popupChildren =
        popup->findChildren<QWidget *>(QString(),
                                       Qt::FindChildrenRecursively);
    for (QWidget *child : popupChildren) {
        if (QString::fromLatin1(child->metaObject()->className()) !=
            QStringLiteral("QComboBoxPrivateScroller")) {
            continue;
        }
        d->journal->captureWidget(child);
        child->setPalette(popupPalette);
        child->setStyleSheet(
            QStringLiteral("background: %1; color: %2; border: 0;")
                .arg(colorName(DesignTokens::color(ColorRole::Surface,
                                                   scheme)),
                     colorName(DesignTokens::color(
                         ColorRole::TextSecondary, scheme))));
        if (visibleRows >= d->comboBox->count()) {
            const auto known = std::find_if(
                d->scrollers.cbegin(), d->scrollers.cend(),
                [child](const ComboBoxAdapterPrivate::ScrollerState &state) {
                    return state.widget == child;
                });
            if (known == d->scrollers.cend()) {
                d->scrollers.append({child, !child->isHidden()});
            }
            child->hide();
        }
    }
    view->update();
    popup->update();

    if (!position || !popup->isVisible()) return;
    const QRect comboRectangle(d->comboBox->mapToGlobal(QPoint()),
                               d->comboBox->size());
    QRect popupGeometry = popup->geometry();
    const int offset = DesignTokens::componentMetric(
        ComponentMetric::ComboBoxPopupOffset);
    const bool below = popupGeometry.center().y() >=
                       comboRectangle.center().y();
    popupGeometry.moveTop(
        below ? comboRectangle.bottom() + 1 + offset
              : comboRectangle.top() - offset - popupGeometry.height());
    popup->setGeometry(popupGeometry);
}

void ComboBoxAdapter::detach()
{
    if (!d->attached) return;
    d->attached = false;
    if (d->comboBox) d->comboBox->removeEventFilter(this);
    if (d->view) {
        d->view->removeEventFilter(this);
        if (d->horizontalScrollBarPolicyCaptured) {
            d->view->setHorizontalScrollBarPolicy(
                d->horizontalScrollBarPolicy);
        }
        d->view->setMouseTracking(d->viewMouseTracking);
        d->view->viewport()->setMouseTracking(d->viewportMouseTracking);
        d->view->viewport()->setAttribute(Qt::WA_Hover, d->viewportHover);
    }
    if (d->popup) {
        d->popup->removeEventFilter(this);
        d->popup->setAttribute(Qt::WA_TranslucentBackground,
                               d->popupTranslucent);
        if (d->popupLayoutMarginsCaptured && d->popup->layout()) {
            d->popup->layout()->setContentsMargins(d->popupLayoutMargins);
        }
    }
    if (d->view && d->view->viewport()) {
        d->view->viewport()->setAttribute(Qt::WA_TranslucentBackground,
                                          d->viewportTranslucent);
    }
    for (const ComboBoxAdapterPrivate::ScrollerState &state :
         std::as_const(d->scrollers)) {
        if (state.widget) state.widget->setVisible(state.visible);
    }
    d->scrollers.clear();
    d->journal.reset();
    emit attachedChanged(false);
}

class WizardAdapterPrivate final
{
public:
    QPointer<QWizard> wizard;
    std::unique_ptr<internal::StateJournal> journal;
    QWizard::WizardStyle originalStyle = QWizard::ClassicStyle;
    QMetaObject::Connection pageAddedConnection;
    bool changeWizardStyle = false;
    bool attached = true;
};

WizardAdapter *WizardAdapter::attach(QWizard *wizard)
{
    if (!wizard) return nullptr;
    WizardAdapter *adapter = wizard->findChild<WizardAdapter *>(
        QString::fromLatin1(wizardAdapterObjectName),
        Qt::FindDirectChildrenOnly);
    if (adapter) {
        adapter->apply();
        return adapter;
    }
    return new WizardAdapter(wizard);
}

WizardAdapter::WizardAdapter(QWizard *wizard)
    : QObject(wizard), d(new WizardAdapterPrivate)
{
    setObjectName(QString::fromLatin1(wizardAdapterObjectName));
    d->wizard = wizard;
    d->originalStyle = wizard->wizardStyle();
    d->pageAddedConnection =
        connect(wizard, &QWizard::pageAdded, this, [this](int) { apply(); });
    apply();
}

WizardAdapter::~WizardAdapter()
{
    detach();
}

QWizard *WizardAdapter::wizard() const
{
    return d->wizard.data();
}

bool WizardAdapter::changeWizardStyle() const
{
    return d->changeWizardStyle;
}

void WizardAdapter::setChangeWizardStyle(bool enabled)
{
    if (d->changeWizardStyle == enabled) return;
    d->changeWizardStyle = enabled;
    apply();
    emit changeWizardStyleChanged(enabled);
}

void WizardAdapter::resetChangeWizardStyle()
{
    setChangeWizardStyle(false);
}

bool WizardAdapter::isAttached() const
{
    return d->attached;
}

void WizardAdapter::apply()
{
    if (!d->attached || !d->wizard) return;
    d->journal.reset(new internal::StateJournal);
    if (d->changeWizardStyle)
        d->wizard->setWizardStyle(QWizard::ClassicStyle);
    else
        d->wizard->setWizardStyle(d->originalStyle);
    d->journal->setProperty(d->wizard.data(),
                            QByteArrayLiteral("_ngstdWizardAdapter"), true);
    const QWizard::WizardButton roles[] = {
        QWizard::BackButton,    QWizard::NextButton,    QWizard::CommitButton,
        QWizard::FinishButton,  QWizard::CancelButton,  QWizard::HelpButton,
        QWizard::CustomButton1, QWizard::CustomButton2, QWizard::CustomButton3,
    };
    for (QWizard::WizardButton role : roles) {
        QAbstractButton *button = d->wizard->button(role);
        if (!button || button->dynamicPropertyNames().contains(
                           QByteArrayLiteral("ngstdButtonVariant"))) {
            continue;
        }
        d->journal->setProperty(
            button, QByteArrayLiteral("ngstdButtonVariant"),
            isPrimaryRole(role) ? QStringLiteral("primary")
                                : QStringLiteral("secondary"));
        WidgetStyle::refresh(button);
    }
    for (int pageId : d->wizard->pageIds()) {
        QWizardPage *page = d->wizard->page(pageId);
        if (!page || !page->accessibleName().isEmpty() ||
            page->title().isEmpty()) {
            continue;
        }
        d->journal->setProperty(page, QByteArrayLiteral("accessibleName"),
                                page->title());
    }
}

void WizardAdapter::detach()
{
    if (!d->attached) return;
    d->attached = false;
    QObject::disconnect(d->pageAddedConnection);
    d->journal.reset();
    if (d->wizard && d->changeWizardStyle)
        d->wizard->setWizardStyle(d->originalStyle);
    emit attachedChanged(false);
}

} // namespace widgets
} // namespace ngstd
