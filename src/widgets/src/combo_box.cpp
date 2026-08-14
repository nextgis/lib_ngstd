/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Combo box with an owned, explicitly positioned popup
 *****************************************************************************/
#include <ngstd/widgets/combo_box.h>

#include "accessibility_p.h"
#include "component_utils_p.h"
#include "motion_controller_p.h"

#include <ngstd/widgets/design_tokens.h>

#include <QAbstractItemModel>
#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QKeyEvent>
#include <QListView>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QScrollBar>
#include <QStyle>
#include <QStyleOptionComboBox>
#include <QVBoxLayout>
#include <QVariantAnimation>
#include <QtMath>

namespace ngstd {
namespace widgets {

namespace {

class ComboPopupFrame final : public QFrame
{
public:
    explicit ComboPopupFrame(QWidget *parent) : QFrame(parent, Qt::Popup)
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_NoSystemBackground);
        setAutoFillBackground(false);
        setWindowFlag(Qt::FramelessWindowHint, true);
        setWindowFlag(Qt::NoDropShadowWindowHint, true);
        setFrameShape(QFrame::NoFrame);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)
        const ColorScheme scheme = internal::colorSchemeFor(this);
        const qreal borderWidth = internal::tokenNumber(
            QStringLiteral("desktop.component.comboBox.popupBorderWidthPx"));
        const qreal radius = internal::tokenNumber(
            QStringLiteral("desktop.component.comboBox.popupRadiusPx"));
        const QRectF bounds =
            QRectF(rect()).adjusted(borderWidth * 0.5, borderWidth * 0.5,
                                    -borderWidth * 0.5, -borderWidth * 0.5);
        QPainterPath path;
        path.addRoundedRect(bounds, radius, radius);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillPath(path,
                         DesignTokens::color(ColorRole::Surface, scheme));
        painter.setPen(
            QPen(DesignTokens::color(ColorRole::Border, scheme), borderWidth));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
    }
};

} // namespace

class ComboBoxPrivate final
{
public:
    QFrame *popup = nullptr;
    QListView *popupView = nullptr;
    QGraphicsOpacityEffect *popupEffect = nullptr;
    QVariantAnimation *arrowAnimation = nullptr;
    QVariantAnimation *popupAnimation = nullptr;
    ComboBoxPopupPlacement placement = ComboBoxPopupPlacement::Below;
    ComboBoxPopupAlignment alignment = ComboBoxPopupAlignment::Left;
    qreal arrowProgress = 0.0;
    bool closingPopup = false;
    bool suppressNextShow = false;
    QAccessible::Id accessibleIdentifier = 0;
};

ComboBox::ComboBox(QWidget *parent) : QComboBox(parent), d(new ComboBoxPrivate)
{
    d->accessibleIdentifier = internal::registerComboBoxAccessibility(this);
    setProperty("_ngstdPopupOpen", false);
    setSizeAdjustPolicy(QComboBox::AdjustToContents);
    d->arrowAnimation = new QVariantAnimation(this);
    d->arrowAnimation->setObjectName(
        QStringLiteral("_ngstdComboBoxArrowAnimation"));
    connect(d->arrowAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
                d->arrowProgress = value.toReal();
                setProperty("_ngstdArrowProgress", d->arrowProgress);
                update();
            });
    d->popupAnimation = new QVariantAnimation(this);
    d->popupAnimation->setObjectName(
        QStringLiteral("_ngstdComboBoxPopupAnimation"));
    connect(d->popupAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
                if (d->popupEffect)
                    d->popupEffect->setOpacity(value.toReal());
            });
    connect(d->popupAnimation, &QVariantAnimation::finished, this, [this]() {
        if (!d->closingPopup || !d->popup) return;
        d->popup->hide();
        if (d->popupEffect) d->popupEffect->setOpacity(1.0);
        d->closingPopup = false;
    });
}

ComboBox::~ComboBox()
{
    internal::unregisterAccessibility(d->accessibleIdentifier);
}

ComboBoxPopupPlacement ComboBox::popupPlacement() const
{
    return d->placement;
}

void ComboBox::setPopupPlacement(ComboBoxPopupPlacement placement)
{
    if (d->placement == placement) return;
    d->placement = placement;
    emit popupPlacementChanged(placement);
}

void ComboBox::resetPopupPlacement()
{
    setPopupPlacement(ComboBoxPopupPlacement::Below);
}

ComboBoxPopupAlignment ComboBox::popupAlignment() const
{
    return d->alignment;
}

void ComboBox::setPopupAlignment(ComboBoxPopupAlignment alignment)
{
    if (d->alignment == alignment) return;
    d->alignment = alignment;
    emit popupAlignmentChanged(alignment);
}

void ComboBox::resetPopupAlignment()
{
    setPopupAlignment(ComboBoxPopupAlignment::Left);
}

QSize ComboBox::sizeHint() const
{
    return contentAwareSizeHint(QComboBox::sizeHint());
}

QSize ComboBox::minimumSizeHint() const
{
    return contentAwareSizeHint(QComboBox::minimumSizeHint());
}

bool ComboBox::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->popup && event->type() == QEvent::Hide) {
        if (rect().contains(mapFromGlobal(QCursor::pos())))
            d->suppressNextShow = true;
        if (!d->closingPopup) {
            setProperty("_ngstdPopupOpen", false);
            animateArrow(0.0);
        }
        return QComboBox::eventFilter(watched, event);
    }
    if (watched == d->popupView && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            hidePopup();
            return true;
        }
        if (keyEvent->key() == Qt::Key_Return ||
            keyEvent->key() == Qt::Key_Enter) {
            const QModelIndex index = d->popupView->currentIndex();
            if (index.isValid()) {
                setCurrentIndex(index.row());
                QMetaObject::invokeMethod(this, "activated",
                                          Qt::DirectConnection,
                                          Q_ARG(int, index.row()));
            }
            hidePopup();
            return true;
        }
    }
    return QComboBox::eventFilter(watched, event);
}

void ComboBox::hidePopup()
{
    if (!d->popup || !d->popup->isVisible()) return;
    setProperty("_ngstdPopupOpen", false);
    d->popup->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    animateArrow(0.0);
    animatePopup(0.0, true);
    QAccessible::State changedState;
    changedState.expanded = true;
    changedState.collapsed = true;
    QAccessibleStateChangeEvent accessibilityEvent(this, changedState);
    QAccessible::updateAccessibility(&accessibilityEvent);
}

void ComboBox::showPopup()
{
    if (d->suppressNextShow) {
        d->suppressNextShow = false;
        return;
    }
    if (d->popup && d->popup->isVisible()) {
        hidePopup();
        return;
    }
    if (!model() || count() == 0) return;
    if (!d->popup) {
        d->popup = new ComboPopupFrame(this);
        d->popup->setObjectName(QStringLiteral("_ngstdComboBoxPopup"));
        d->popup->installEventFilter(this);
        d->popupEffect = new QGraphicsOpacityEffect(d->popup);
        d->popupEffect->setOpacity(1.0);
        d->popup->setGraphicsEffect(d->popupEffect);
        QVBoxLayout *layout = new QVBoxLayout(d->popup);
        const int popupPadding = DesignTokens::componentMetric(
            ComponentMetric::ComboBoxPopupPadding);
        layout->setContentsMargins(popupPadding, popupPadding, popupPadding,
                                   popupPadding);
        d->popupView = new QListView(d->popup);
        d->popupView->setObjectName(QStringLiteral("_ngstdComboBoxPopupView"));
        d->popupView->setProperty("_ngstdComboBoxPopupView", true);
        d->popupView->setFrameShape(QFrame::NoFrame);
        d->popupView->setAutoFillBackground(false);
        d->popupView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        d->popupView->setSelectionMode(QAbstractItemView::SingleSelection);
        d->popupView->setSpacing(DesignTokens::componentMetric(
            ComponentMetric::ComboBoxPopupItemSpacing));
        d->popupView->installEventFilter(this);
        layout->addWidget(d->popupView);
        connect(d->popupView, &QListView::clicked, this,
                [this](const QModelIndex &index) {
                    if (!index.isValid()) return;
                    setCurrentIndex(index.row());
                    QMetaObject::invokeMethod(this, "activated",
                                              Qt::DirectConnection,
                                              Q_ARG(int, index.row()));
                    hidePopup();
                });
    }

    d->popupView->setModel(model());
    d->popupView->setRootIndex(rootModelIndex());
    d->popupView->setModelColumn(modelColumn());
    d->popupView->setCurrentIndex(
        model()->index(currentIndex(), modelColumn(), rootModelIndex()));
    const QPalette popupPalette = internal::comboBoxPopupPalette(this);
    d->popup->setPalette(popupPalette);
    d->popupView->setPalette(popupPalette);
    d->popupView->viewport()->setPalette(popupPalette);

    const int rowHeight =
        qMax(DesignTokens::componentMetric(ComponentMetric::ItemViewRowHeight),
             d->popupView->sizeHintForRow(0));
    const int visibleRows = qMin(maxVisibleItems(), count());
    const bool allRowsVisible = visibleRows == count();
    d->popupView->setVerticalScrollBarPolicy(
        allRowsVisible ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded);
    const int popupPadding = DesignTokens::componentMetric(
        ComponentMetric::ComboBoxPopupPadding);
    const int popupSpacing = DesignTokens::componentMetric(
        ComponentMetric::ComboBoxPopupItemSpacing);
    const int popupBorder = qCeil(internal::tokenNumber(
        QStringLiteral("desktop.component.comboBox.popupBorderWidthPx")));
    const int popupHeight =
        qMax(rowHeight,
             visibleRows * (rowHeight + popupSpacing * 2) +
                 popupPadding * 2 + popupBorder * 2);
    const int popupItemPadding = internal::tokenInteger(QStringLiteral(
        "desktop.component.comboBox.popupItemPaddingHorizontalPx"));
    const int contentWidth =
        d->popupView->sizeHintForColumn(modelColumn()) +
        (allRowsVisible
             ? 0
             : d->popupView->verticalScrollBar()->sizeHint().width()) +
        popupItemPadding * 2 + popupPadding * 2 + popupBorder * 2;
    const QSize popupSize(qMax(width(), contentWidth), popupHeight);

    QScreen *targetScreen = screen();
    if (!targetScreen) targetScreen = QGuiApplication::primaryScreen();
    const QRect availableGeometry = targetScreen
                                        ? targetScreen->availableGeometry()
                                        : QRect(mapToGlobal(QPoint()), size());
    const int popupOffset = DesignTokens::componentMetric(
        ComponentMetric::ComboBoxPopupOffset);
    const QPoint belowLeft =
        mapToGlobal(QPoint(0, height() + popupOffset));
    const QPoint aboveLeft =
        mapToGlobal(QPoint(0, -popupSize.height() - popupOffset));
    const bool fitsBelow =
        belowLeft.y() + popupSize.height() <= availableGeometry.bottom() + 1;
    const bool fitsAbove = aboveLeft.y() >= availableGeometry.top();
    bool placeAbove = d->placement == ComboBoxPopupPlacement::Above;
    if (placeAbove && !fitsAbove && fitsBelow)
        placeAbove = false;
    else if (!placeAbove && !fitsBelow && fitsAbove)
        placeAbove = true;

    int x = belowLeft.x();
    if (d->alignment == ComboBoxPopupAlignment::Center)
        x += (width() - popupSize.width()) / 2;
    else if (d->alignment == ComboBoxPopupAlignment::Right)
        x += width() - popupSize.width();
    if (layoutDirection() == Qt::RightToLeft) {
        if (d->alignment == ComboBoxPopupAlignment::Left)
            x += width() - popupSize.width();
        else if (d->alignment == ComboBoxPopupAlignment::Right)
            x = belowLeft.x();
    }
    x = qBound(availableGeometry.left(), x,
               availableGeometry.right() - popupSize.width() + 1);
    int y = placeAbove ? aboveLeft.y() : belowLeft.y();
    y = qBound(availableGeometry.top(), y,
               availableGeometry.bottom() - popupSize.height() + 1);

    d->popup->setGeometry(QRect(QPoint(x, y), popupSize));
    d->closingPopup = false;
    d->popupAnimation->stop();
    d->popup->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    d->popupEffect->setOpacity(0.0);
    d->popup->show();
    d->popup->raise();
    d->popupView->setFocus(Qt::PopupFocusReason);
    d->popupView->scrollTo(d->popupView->currentIndex());
    setProperty("_ngstdPopupOpen", true);
    animateArrow(1.0);
    animatePopup(1.0, false);
    QAccessible::State changedState;
    changedState.expanded = true;
    changedState.collapsed = true;
    QAccessibleStateChangeEvent accessibilityEvent(this, changedState);
    QAccessible::updateAccessibility(&accessibilityEvent);
}

void ComboBox::animateArrow(qreal target)
{
    const qreal boundedTarget = qBound(0.0, target, 1.0);
    d->arrowAnimation->stop();
    d->arrowAnimation->setStartValue(d->arrowProgress);
    d->arrowAnimation->setEndValue(boundedTarget);
    if (internal::MotionController::configure(
            d->arrowAnimation, this,
            {MotionDuration::Normal, MotionEasing::Standard})) {
        d->arrowAnimation->start();
    }
    else {
        d->arrowProgress = boundedTarget;
        update();
    }
}

QSize ComboBox::contentAwareSizeHint(const QSize &base) const
{
    const auto textWidth = [this](const QString &text) {
        const QFontMetrics metrics = fontMetrics();
        return qMax(metrics.horizontalAdvance(text),
                    metrics.boundingRect(text).width());
    };
    int widestContent = textWidth(placeholderText());
    for (int index = 0; index < count(); ++index) {
        int itemWidth = textWidth(itemText(index));
        if (!itemIcon(index).isNull())
            itemWidth += iconSize().width() + DesignTokens::spacing(2);
        const QModelIndex itemIndex = model()->index(
            index, modelColumn(), rootModelIndex());
        if (itemIndex.flags() & Qt::ItemIsUserCheckable) {
            itemWidth += DesignTokens::componentMetric(
                             ComponentMetric::SelectionIndicatorSize) +
                         DesignTokens::spacing(2);
        }
        widestContent = qMax(widestContent, itemWidth);
    }

    QStyleOptionComboBox option;
    initStyleOption(&option);
    const QSize fitted = style()->sizeFromContents(
        QStyle::CT_ComboBox, &option,
        QSize(widestContent, fontMetrics().height()), this);
    QSize result = base.expandedTo(fitted);
    const int textSafety = 4;
    result.setWidth(qMax(
        result.width(),
        widestContent + DesignTokens::componentMetric(
                            ComponentMetric::ComboBoxDropDownWidth) +
            DesignTokens::spacing(3) * 2 + textSafety));
    return result;
}

void ComboBox::animatePopup(qreal target, bool hideWhenDone)
{
    if (!d->popup || !d->popupEffect) return;
    d->popupAnimation->stop();
    d->closingPopup = hideWhenDone;
    d->popupAnimation->setStartValue(d->popupEffect->opacity());
    d->popupAnimation->setEndValue(qBound(0.0, target, 1.0));
    if (internal::MotionController::configure(
            d->popupAnimation, this,
            {MotionDuration::Fast,
             hideWhenDone ? MotionEasing::Exit : MotionEasing::Enter})) {
        d->popupAnimation->start();
    }
    else {
        d->popupEffect->setOpacity(target);
        if (hideWhenDone) {
            d->popup->hide();
            d->popupEffect->setOpacity(1.0);
            d->closingPopup = false;
        }
    }
}

} // namespace widgets
} // namespace ngstd
