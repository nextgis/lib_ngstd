/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Reusable reveal and disclosure components
 *****************************************************************************/
#include <ngstd/widgets/disclosure.h>

#include "accessibility_p.h"
#include "component_utils_p.h"
#include "motion_controller_p.h"

#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/icons.h>

#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QStyle>
#include <QStyleOptionToolButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariant>
#include <QVariantAnimation>
#include <QtMath>

namespace ngstd {
namespace widgets {

namespace {

class DisclosureHeader final : public QToolButton
{
public:
    explicit DisclosureHeader(QWidget *parent) : QToolButton(parent) {}

    qreal chevronProgress() const
    {
        return m_chevronProgress;
    }

    void setChevronProgress(qreal progress)
    {
        const qreal boundedProgress = qBound(0.0, progress, 1.0);
        if (qFuzzyCompare(m_chevronProgress, boundedProgress)) return;
        m_chevronProgress = boundedProgress;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)
        QStyleOptionToolButton option;
        initStyleOption(&option);
        const QString title = option.text;
        option.text.clear();
        option.icon = QIcon();
        option.arrowType = Qt::NoArrow;

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);
        style()->drawComplexControl(QStyle::CC_ToolButton, &option, &painter,
                                    this);

        const int padding = internal::tokenInteger(QStringLiteral(
            "desktop.component.disclosure.paddingHorizontalPx"));
        const int iconSize = DesignTokens::controlIconSize();
        const int spacing = DesignTokens::spacing(2);
        const QRect logicalTextRect(
            padding, 0, qMax(0, width() - padding * 2 - iconSize - spacing),
            height());
        const QRect textRect =
            style()->visualRect(layoutDirection(), rect(), logicalTextRect);
        const Qt::Alignment textAlignment = QStyle::visualAlignment(
            layoutDirection(), Qt::AlignLeft | Qt::AlignVCenter);
        style()->drawItemText(&painter, textRect, textAlignment,
                              option.palette, isEnabled(), title,
                              QPalette::ButtonText);

        const QRect logicalIconRect(width() - padding - iconSize,
                                    (height() - iconSize) / 2, iconSize,
                                    iconSize);
        const QRect iconRect =
            style()->visualRect(layoutDirection(), rect(), logicalIconRect);
        painter.save();
        painter.translate(iconRect.center());
        painter.rotate(m_chevronProgress * 180.0);
        QPainterPath chevron;
        chevron.moveTo(-5.0, -2.5);
        chevron.lineTo(0.0, 2.5);
        chevron.lineTo(5.0, -2.5);
        painter.setPen(QPen(option.palette.color(QPalette::ButtonText), 1.5,
                            Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(chevron);
        painter.restore();
    }

private:
    qreal m_chevronProgress = 0.0;
};

class DisclosureBody final : public QWidget
{
public:
    explicit DisclosureBody(QWidget *parent = nullptr) : QWidget(parent)
    {
        setProperty("_ngstdRole", QStringLiteral("disclosureContent"));
        setAttribute(Qt::WA_StyledBackground, false);
        setAttribute(Qt::WA_NoSystemBackground);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(DesignTokens::color(
            ColorRole::Border, internal::colorSchemeFor(this))));
        painter.drawLine(QPointF(0.0, 0.5), QPointF(width(), 0.5));
    }
};

} // namespace

class RevealWidgetPrivate final
{
public:
    QPointer<QWidget> contentWidget;
    QPropertyAnimation *animation = nullptr;
    qreal progress = 0.0;
    int contentHeight = 0;
    bool expanded = false;
};

RevealWidget::RevealWidget(QWidget *parent)
    : QWidget(parent), d(new RevealWidgetPrivate)
{
    setProperty("_ngstdRole", QStringLiteral("reveal"));
    setAttribute(Qt::WA_NoSystemBackground);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    d->animation = new QPropertyAnimation(this, "revealProgress", this);
    d->animation->setObjectName(QStringLiteral("_ngstdRevealAnimation"));
}

RevealWidget::~RevealWidget()
{
    if (d->contentWidget) d->contentWidget->removeEventFilter(this);
}

QWidget *RevealWidget::contentWidget() const
{
    return d->contentWidget.data();
}

void RevealWidget::setContentWidget(QWidget *contentWidget)
{
    if (d->contentWidget == contentWidget) return;
    QWidget *previousWidget = takeContentWidget();
    if (previousWidget) previousWidget->deleteLater();
    d->contentWidget = contentWidget;
    if (contentWidget) {
        contentWidget->setParent(this);
        contentWidget->installEventFilter(this);
        contentWidget->show();
    }
    refreshContentGeometry();
}

QWidget *RevealWidget::takeContentWidget()
{
    QWidget *contentWidget = d->contentWidget.data();
    if (!contentWidget) return nullptr;
    contentWidget->removeEventFilter(this);
    contentWidget->setParent(nullptr);
    d->contentWidget.clear();
    d->contentHeight = 0;
    setFixedHeight(0);
    updateGeometry();
    return contentWidget;
}

bool RevealWidget::isExpanded() const
{
    return d->expanded;
}

void RevealWidget::setExpanded(bool expanded)
{
    if (d->expanded == expanded) return;
    d->expanded = expanded;
    d->animation->stop();
    d->animation->setStartValue(d->progress);
    d->animation->setEndValue(expanded ? 1.0 : 0.0);
    if (!internal::MotionController::configure(
            d->animation, this,
            {MotionDuration::Slow,
             expanded ? MotionEasing::Enter : MotionEasing::Exit})) {
        setRevealProgress(expanded ? 1.0 : 0.0);
    }
    else {
        d->animation->start();
    }
    emit expandedChanged(expanded);
}

void RevealWidget::collapse()
{
    setExpanded(false);
}

qreal RevealWidget::revealProgress() const
{
    return d->progress;
}

QSize RevealWidget::sizeHint() const
{
    const QSize contentSize =
        d->contentWidget ? d->contentWidget->sizeHint() : QSize();
    return QSize(contentSize.width(), qRound(d->contentHeight * d->progress));
}

QSize RevealWidget::minimumSizeHint() const
{
    return QSize(0, 0);
}

bool RevealWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->contentWidget &&
        (event->type() == QEvent::LayoutRequest ||
         event->type() == QEvent::Resize || event->type() == QEvent::Show)) {
        refreshContentGeometry();
    }
    return QWidget::eventFilter(watched, event);
}

void RevealWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    refreshContentGeometry();
}

void RevealWidget::setRevealProgress(qreal progress)
{
    const qreal boundedProgress = qBound(0.0, progress, 1.0);
    if (qFuzzyCompare(d->progress, boundedProgress)) return;
    d->progress = boundedProgress;
    refreshContentGeometry();
    emit revealProgressChanged(d->progress);
}

void RevealWidget::refreshContentGeometry()
{
    if (!d->contentWidget) {
        d->contentHeight = 0;
        updateGeometry();
        return;
    }
    const int previousHeight = d->contentHeight;
    const int targetWidth = qMax(0, width());
    const int heightForWidth =
        d->contentWidget->hasHeightForWidth()
            ? d->contentWidget->heightForWidth(targetWidth)
            : -1;
    const int widgetHeight =
        qMax(0, heightForWidth >= 0 ? heightForWidth
                                    : d->contentWidget->sizeHint().height());
    d->contentHeight = widgetHeight;
    d->contentWidget->setGeometry(0, 0, targetWidth, widgetHeight);
    setFixedHeight(qRound(d->contentHeight * d->progress));
    if (previousHeight != d->contentHeight)
        emit contentHeightChanged(d->contentHeight);
    updateGeometry();
}

class DisclosurePrivate final
{
public:
    DisclosureHeader *headerButton = nullptr;
    RevealWidget *revealWidget = nullptr;
    DisclosureBody *bodyWidget = nullptr;
    QVBoxLayout *bodyLayout = nullptr;
    QPointer<QWidget> contentWidget;
    QVariantAnimation *chevronAnimation = nullptr;
    QAccessible::Id accessibleIdentifier = 0;
};

Disclosure::Disclosure(QWidget *parent)
    : QFrame(parent), d(new DisclosurePrivate)
{
    d->accessibleIdentifier = internal::registerDisclosureAccessibility(this);
    setProperty("_ngstdRole", QStringLiteral("disclosure"));
    setAttribute(Qt::WA_StyledBackground, true);
    setFrameShape(QFrame::NoFrame);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(0);
    d->headerButton = new DisclosureHeader(this);
    d->headerButton->setProperty("_ngstdRole", QStringLiteral("disclosure"));
    d->headerButton->setCheckable(true);
    d->headerButton->setCursor(Qt::PointingHandCursor);
    d->headerButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    d->headerButton->setFixedHeight(DesignTokens::componentMetric(
        ComponentMetric::DisclosureHeaderHeight));
    d->headerButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    d->chevronAnimation = new QVariantAnimation(this);
    d->chevronAnimation->setObjectName(
        QStringLiteral("_ngstdDisclosureChevronAnimation"));
    connect(d->chevronAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
                d->headerButton->setChevronProgress(value.toReal());
            });
    d->revealWidget = new RevealWidget(this);
    d->bodyWidget = new DisclosureBody(d->revealWidget);
    d->bodyLayout = new QVBoxLayout(d->bodyWidget);
    const int contentPadding = DesignTokens::componentMetric(
        ComponentMetric::DisclosureContentPadding);
    d->bodyLayout->setContentsMargins(contentPadding, contentPadding,
                                      contentPadding, contentPadding);
    d->bodyLayout->setSpacing(0);
    d->revealWidget->setContentWidget(d->bodyWidget);
    layout->addWidget(d->headerButton);
    layout->addWidget(d->revealWidget);
    connect(
        d->headerButton, &QToolButton::toggled, this, [this](bool expanded) {
            d->chevronAnimation->stop();
            d->chevronAnimation->setStartValue(
                d->headerButton->chevronProgress());
            d->chevronAnimation->setEndValue(expanded ? 1.0 : 0.0);
            if (!internal::MotionController::configure(
                    d->chevronAnimation, this,
                    {MotionDuration::Slow,
                     expanded ? MotionEasing::Enter : MotionEasing::Exit})) {
                d->headerButton->setChevronProgress(expanded ? 1.0 : 0.0);
            }
            else {
                d->chevronAnimation->start();
            }
            d->revealWidget->setExpanded(expanded);
            QAccessible::State changedState;
            changedState.expanded = true;
            changedState.collapsed = true;
            QAccessibleStateChangeEvent accessibilityEvent(this, changedState);
            QAccessible::updateAccessibility(&accessibilityEvent);
            emit expandedChanged(expanded);
        });
}

Disclosure::~Disclosure()
{
    internal::unregisterAccessibility(d->accessibleIdentifier);
}

void Disclosure::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    const qreal strokeWidth = 1.0;
    const qreal radius = DesignTokens::radius(RadiusRole::Button);
    const QRectF bounds =
        QRectF(rect()).adjusted(strokeWidth * 0.5, strokeWidth * 0.5,
                                -strokeWidth * 0.5, -strokeWidth * 0.5);
    const ColorScheme scheme = internal::colorSchemeFor(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(
        QPen(DesignTokens::color(ColorRole::Border, scheme), strokeWidth));
    painter.setBrush(DesignTokens::color(ColorRole::Surface, scheme));
    painter.drawRoundedRect(bounds, radius, radius);
}

QString Disclosure::title() const
{
    return d->headerButton->text();
}

void Disclosure::setTitle(const QString &title)
{
    if (d->headerButton->text() == title) return;
    d->headerButton->setText(title);
    d->headerButton->setAccessibleName(title);
    emit titleChanged(title);
}

bool Disclosure::isExpanded() const
{
    return d->headerButton->isChecked();
}

void Disclosure::setExpanded(bool expanded)
{
    d->headerButton->setChecked(expanded);
}

void Disclosure::collapse()
{
    setExpanded(false);
}

QWidget *Disclosure::contentWidget() const
{
    return d->contentWidget.data();
}

void Disclosure::setContentWidget(QWidget *contentWidget)
{
    if (d->contentWidget == contentWidget) return;
    QWidget *previousWidget = takeContentWidget();
    if (previousWidget) previousWidget->deleteLater();
    d->contentWidget = contentWidget;
    if (contentWidget) d->bodyLayout->addWidget(contentWidget);
    d->bodyLayout->invalidate();
    d->bodyLayout->activate();
    d->bodyWidget->updateGeometry();
    d->revealWidget->refreshContentGeometry();
}

QWidget *Disclosure::takeContentWidget()
{
    QWidget *contentWidget = d->contentWidget.data();
    if (!contentWidget) return nullptr;
    d->bodyLayout->removeWidget(contentWidget);
    contentWidget->setParent(nullptr);
    d->contentWidget.clear();
    d->bodyLayout->invalidate();
    d->bodyLayout->activate();
    d->bodyWidget->updateGeometry();
    d->revealWidget->refreshContentGeometry();
    return contentWidget;
}

RevealWidget *Disclosure::revealWidget() const
{
    return d->revealWidget;
}

} // namespace widgets
} // namespace ngstd
