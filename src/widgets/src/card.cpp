/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Passive and interactive corporate card components
 *****************************************************************************/
#include <ngstd/widgets/card.h>

#include "component_utils_p.h"
#include "motion_controller_p.h"

#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/widget_style.h>

#include <QButtonGroup>
#include <QElapsedTimer>
#include <QEvent>
#include <QImage>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QTimer>
#include <QVBoxLayout>
#include <QVariant>
#include <QVariantAnimation>
#include <QtMath>

namespace ngstd {
namespace widgets {

namespace {

class CardRenderer
{
public:
    virtual ~CardRenderer() = default;

    virtual void paint(QPainter *painter, const QRectF &rectangle,
                       const QPixmap &background, ColorScheme scheme,
                       qreal interactionProgress,
                       qreal selectionProgress) const = 0;
};

void drawBackground(QPainter *painter, const QPainterPath &clipPath,
                    const QRectF &rectangle, const QPixmap &background)
{
    if (background.isNull()) return;
    painter->save();
    painter->setClipPath(clipPath);
    const QRectF destination = internal::coverRect(
        background.size() / background.devicePixelRatio(), rectangle);
    painter->drawPixmap(destination, background, QRectF(background.rect()));
    painter->restore();
}

QColor cardStateColor(const QColor &idle, const QColor &hovered,
                      const QColor &pressed, qreal interactionProgress)
{
    const qreal hoverProgress = qBound(0.0, interactionProgress, 1.0);
    const qreal pressProgress = qBound(0.0, interactionProgress - 1.0, 1.0);
    return internal::interpolateColor(
        internal::interpolateColor(idle, hovered, hoverProgress), pressed,
        pressProgress);
}

class SurfaceCardRenderer final : public CardRenderer
{
public:
    explicit SurfaceCardRenderer(RadiusRole radiusRole = RadiusRole::Card,
                                 bool imageOverlay = false)
        : m_radiusRole(radiusRole), m_imageOverlay(imageOverlay)
    {}

    void paint(QPainter *painter, const QRectF &rectangle,
               const QPixmap &background, ColorScheme scheme,
               qreal interactionProgress,
               qreal selectionProgress) const override
    {
        const qreal radius = DesignTokens::radius(m_radiusRole);
        QPainterPath path;
        path.addRoundedRect(rectangle, radius, radius);
        const qreal selected = qBound(0.0, selectionProgress, 1.0);
        const QColor idleSurface = internal::interpolateColor(
            DesignTokens::color(ColorRole::Surface, scheme),
            DesignTokens::color(ColorRole::SurfaceBrand, scheme), selected);
        const QColor idleBorder = internal::interpolateColor(
            DesignTokens::color(ColorRole::Border, scheme),
            DesignTokens::color(ColorRole::Brand, scheme), selected);
        painter->fillPath(
            path, cardStateColor(
                      idleSurface,
                      DesignTokens::color(ColorRole::SurfaceBrand, scheme),
                      DesignTokens::color(ColorRole::BrandSoft, scheme),
                      interactionProgress));
        drawBackground(painter, path, rectangle, background);
        if (m_imageOverlay && !background.isNull()) {
            QLinearGradient gradient(rectangle.bottomLeft(),
                                     rectangle.topLeft());
            gradient.setColorAt(0.0, QColor(0, 0, 0, 205));
            gradient.setColorAt(0.58, QColor(0, 0, 0, 28));
            gradient.setColorAt(1.0, QColor(0, 0, 0, 0));
            painter->fillPath(path, gradient);
        }
        const QColor border = cardStateColor(
            idleBorder, DesignTokens::color(ColorRole::BrandHover, scheme),
            DesignTokens::color(ColorRole::BrandActive, scheme),
            interactionProgress);
        QPen borderPen(border, 1.0 + selected);
        painter->setPen(borderPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(path);
    }

private:
    RadiusRole m_radiusRole;
    bool m_imageOverlay;
};

class DataCardRenderer final : public CardRenderer
{
public:
    void paint(QPainter *painter, const QRectF &rectangle,
               const QPixmap &background, ColorScheme scheme,
               qreal interactionProgress,
               qreal selectionProgress) const override
    {
        Q_UNUSED(selectionProgress)
        const qreal radius = DesignTokens::radius(RadiusRole::Field);
        QPainterPath path;
        path.addRoundedRect(rectangle, radius, radius);
        painter->fillPath(path,
                          DesignTokens::color(ColorRole::DataSurface, scheme));
        drawBackground(painter, path, rectangle, background);
        const qreal hoverProgress = qBound(0.0, interactionProgress, 1.0);
        const qreal pressProgress =
            qBound(0.0, interactionProgress - 1.0, 1.0);
        const qreal hoverOpacity = internal::tokenNumber(
            QStringLiteral("desktop.component.card.hoverOverlayOpacity"));
        const qreal pressedOpacity = internal::tokenNumber(
            QStringLiteral("desktop.component.card.dataFocusOverlayOpacity"));
        const qreal opacity =
            hoverOpacity * hoverProgress * (1.0 - pressProgress) +
            pressedOpacity * pressProgress;
        if (opacity > 0.0) {
            const QString theme = scheme == ColorScheme::Dark
                                      ? QStringLiteral("dark")
                                      : QStringLiteral("light");
            QColor overlay = internal::cssColor(internal::tokenString(
                QStringLiteral("product.data.%1.cardText").arg(theme)));
            overlay.setAlphaF(static_cast<float>(overlay.alphaF() * opacity));
            painter->fillPath(path, overlay);
        }
        painter->setPen(
            QPen(DesignTokens::color(ColorRole::DataBorder, scheme), 1.0));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(path);
    }
};

class ToolboxCardRenderer final : public CardRenderer
{
public:
    explicit ToolboxCardRenderer(bool highlighted) : m_highlighted(highlighted)
    {}

    void paint(QPainter *painter, const QRectF &rectangle,
               const QPixmap &background, ColorScheme scheme,
               qreal interactionProgress,
               qreal selectionProgress) const override
    {
        Q_UNUSED(selectionProgress)
        const qreal radius = DesignTokens::radius(RadiusRole::Card);
        QPainterPath path;
        path.addRoundedRect(rectangle, radius, radius);
        painter->fillPath(
            path, DesignTokens::color(ColorRole::ToolboxCardSurface, scheme));
        drawBackground(painter, path, rectangle, background);
        if (m_highlighted) {
            drawBackground(painter, path, rectangle, decoration(scheme));
        }
        const bool pressed = interactionProgress > 1.0;
        const QString theme = scheme == ColorScheme::Dark
                                  ? QStringLiteral("dark")
                                  : QStringLiteral("light");
        const QColor focusOutline = internal::cssColor(internal::tokenString(
            QStringLiteral("product.toolbox.%1.focusOutline").arg(theme)));
        painter->setPen(
            m_highlighted
                ? QPen(DesignTokens::color(ColorRole::ToolboxAccent, scheme),
                       2.0)
            : pressed ? QPen(focusOutline, 2.0)
                      : Qt::NoPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(path);
    }

private:
    static const QPixmap &decoration(ColorScheme scheme)
    {
        static const QPixmap lightDecoration(QStringLiteral(
            ":/ngstd/widgets/assets/backgrounds/gradientiso3.png"));
        static const QPixmap darkDecoration = []() {
            const QImage source(QStringLiteral(
                ":/ngstd/widgets/assets/backgrounds/gradientiso3.png"));
            QImage result(source.size(), QImage::Format_ARGB32);
            result.fill(Qt::transparent);
            const QColor accent = DesignTokens::color(ColorRole::ToolboxAccent,
                                                      ColorScheme::Dark);
            for (int y = 0; y < source.height(); ++y) {
                for (int x = 0; x < source.width(); ++x) {
                    const QColor sourceColor = source.pixelColor(x, y);
                    const int distanceFromWhite =
                        qMax(255 - sourceColor.red(),
                             qMax(255 - sourceColor.green(),
                                  255 - sourceColor.blue()));
                    QColor color = accent;
                    color.setAlpha(qMin(160, distanceFromWhite * 4) *
                                   sourceColor.alpha() / 255);
                    result.setPixelColor(x, y, color);
                }
            }
            return QPixmap::fromImage(result);
        }();
        return scheme == ColorScheme::Dark ? darkDecoration : lightDecoration;
    }

    bool m_highlighted;
};

const CardRenderer &rendererFor(CardVariant variant)
{
    static const SurfaceCardRenderer surfaceRenderer;
    static const SurfaceCardRenderer panelRenderer(RadiusRole::Panel);
    static const SurfaceCardRenderer backgroundRenderer(RadiusRole::Card,
                                                        true);
    static const DataCardRenderer dataRenderer;
    static const ToolboxCardRenderer toolboxRenderer(false);
    static const ToolboxCardRenderer highlightedToolboxRenderer(true);
    switch (variant) {
    case CardVariant::Data:
        return dataRenderer;
    case CardVariant::Toolbox:
        return toolboxRenderer;
    case CardVariant::ToolboxNew:
        return highlightedToolboxRenderer;
    case CardVariant::Background:
        return backgroundRenderer;
    case CardVariant::Panel:
        return panelRenderer;
    case CardVariant::Default:
    case CardVariant::Media:
    case CardVariant::Selectable:
        return surfaceRenderer;
    }
    return surfaceRenderer;
}

void replaceWidget(QVBoxLayout *layout, QPointer<QWidget> *current,
                   QWidget *replacement, int index)
{
    if (*current == replacement) return;
    QWidget *previous = current->data();
    if (previous) {
        layout->removeWidget(previous);
        previous->deleteLater();
    }
    *current = replacement;
    if (!replacement) return;
    replacement->setParent(layout->parentWidget());
    layout->insertWidget(index, replacement);
    layout->setAlignment(replacement, Qt::AlignTop);
}

QWidget *takeWidget(QVBoxLayout *layout, QPointer<QWidget> *current)
{
    QWidget *widget = current->data();
    if (!widget) return nullptr;
    layout->removeWidget(widget);
    widget->setParent(nullptr);
    current->clear();
    return widget;
}

void paintCard(QWidget *widget, CardVariant variant, const QPixmap &background,
               qreal interactionProgress, qreal selectionProgress)
{
    QPainter painter(widget);
    painter.setRenderHint(QPainter::Antialiasing, true);
    const qreal inset = 0.5 + 0.5 * selectionProgress;
    const QRectF rectangle =
        QRectF(widget->rect()).adjusted(inset, inset, -inset, -inset);
    rendererFor(variant).paint(&painter, rectangle, background,
                               internal::colorSchemeFor(widget),
                               interactionProgress, selectionProgress);
}

bool usesRadioIndicator(const CardButton *card)
{
    const QButtonGroup *buttonGroup = card->group();
    return card->autoExclusive() || (buttonGroup && buttonGroup->exclusive());
}

void updateCardButtonMargins(CardButton *card, QVBoxLayout *layout,
                             CardVariant variant)
{
    const int padding =
        DesignTokens::componentMetric(ComponentMetric::CardContentPadding);
    int left = padding;
    int right = padding;
    if (variant == CardVariant::Selectable) {
        const int reservedWidth =
            DesignTokens::componentMetric(
                ComponentMetric::SelectionIndicatorSize) +
            DesignTokens::componentMetric(
                ComponentMetric::SelectionIndicatorGap);
        if (card->layoutDirection() == Qt::RightToLeft)
            right += reservedWidth;
        else
            left += reservedWidth;
    }
    layout->setContentsMargins(left, padding, right, padding);
}

void paintSelectionIndicator(CardButton *card, qreal selectionProgress,
                             qreal feedbackProgress)
{
    if (card->variant() != CardVariant::Selectable) return;
    const int size =
        DesignTokens::componentMetric(ComponentMetric::SelectionIndicatorSize);
    const int padding =
        DesignTokens::componentMetric(ComponentMetric::CardContentPadding);
    int centerY = padding + size / 2;
    QWidget *titleWidget = card->topWidget();
    if (titleWidget && titleWidget->geometry().isValid()) {
        centerY = titleWidget->geometry().center().y();
    }
    else if (card->contentLayout()->count() > 0) {
        const QRect titleGeometry =
            card->contentLayout()->itemAt(0)->geometry();
        if (titleGeometry.isValid()) centerY = titleGeometry.center().y();
    }
    const int top =
        qBound(0, centerY - size / 2, qMax(0, card->height() - size));
    const int left = card->layoutDirection() == Qt::RightToLeft
                         ? card->width() - padding - size
                         : padding;
    const QRectF bounds(left + 0.75, top + 0.75, size - 1.5, size - 1.5);
    const ColorScheme scheme = internal::colorSchemeFor(card);
    const qreal selected = qBound(0.0, selectionProgress, 1.0);
    const bool enabled = card->isEnabled();
    const bool hovered = card->underMouse();
    const QColor idleSurface = DesignTokens::color(
        enabled ? ColorRole::Surface : ColorRole::SurfaceMuted, scheme);
    const QColor selectedSurface = DesignTokens::color(
        enabled ? ColorRole::Brand : ColorRole::SurfaceMuted, scheme);
    const QColor idleBorder = DesignTokens::color(
        enabled ? (hovered ? ColorRole::BrandHover : ColorRole::BorderStrong)
                : ColorRole::Border,
        scheme);
    const QColor selectedBorder = DesignTokens::color(
        enabled ? ColorRole::Brand : ColorRole::Border, scheme);
    const QColor surface =
        internal::interpolateColor(idleSurface, selectedSurface, selected);
    const QColor border =
        internal::interpolateColor(idleBorder, selectedBorder, selected);

    QPainter painter(card);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(border, 1.5));
    painter.setBrush(surface);
    const bool radio = usesRadioIndicator(card);
    if (radio) {
        painter.drawEllipse(bounds);
        if (selected > 0.0) {
            const int dotSize = DesignTokens::componentMetric(
                ComponentMetric::SelectionRadioDotSize);
            const qreal animatedSize = dotSize * selected;
            const QRectF dot(bounds.center().x() - animatedSize * 0.5,
                             bounds.center().y() - animatedSize * 0.5,
                             animatedSize, animatedSize);
            painter.setPen(Qt::NoPen);
            painter.setBrush(
                DesignTokens::color(ColorRole::White, ColorScheme::Light));
            painter.drawEllipse(dot);
        }
    }
    else {
        const qreal radius = DesignTokens::componentMetric(
            ComponentMetric::SelectionIndicatorRadius);
        painter.drawRoundedRect(bounds, radius, radius);
        if (selected > 0.0) {
            const auto point = [&bounds](qreal x, qreal y) {
                return QPointF(bounds.left() + bounds.width() * x / 24.0,
                               bounds.top() + bounds.height() * y / 24.0);
            };
            QPainterPath checkPath;
            checkPath.moveTo(point(5.0, 12.0));
            checkPath.lineTo(point(10.0, 17.0));
            checkPath.lineTo(point(19.0, 7.0));
            painter.setBrush(Qt::NoBrush);
            QColor checkColor =
                DesignTokens::color(ColorRole::White, ColorScheme::Light);
            checkColor.setAlphaF(static_cast<float>(selected));
            painter.setPen(QPen(checkColor, 2.0, Qt::SolidLine, Qt::RoundCap,
                                Qt::RoundJoin));
            painter.drawPath(checkPath);
        }
    }

    const qreal feedback = qBound(0.0, feedbackProgress, 1.0);
    if (feedback >= 1.0) return;
    QColor feedbackColor =
        DesignTokens::color(ColorRole::FeedbackRing, scheme);
    internal::paintSelectionFeedback(
        &painter, bounds,
        DesignTokens::componentMetric(
            ComponentMetric::SelectionIndicatorRadius),
        feedback, feedbackColor, radio);
}

void paintRipple(CardButton *card, const QPointF &origin, qreal progress,
                 qreal opacity)
{
    if (card->variant() != CardVariant::Data || opacity <= 0.0) return;
    const QRectF bounds(card->rect());
    const QPointF center =
        origin + (bounds.center() - origin) * qBound(0.0, progress, 1.0);
    const qreal fullRadius = qSqrt(bounds.width() * bounds.width() +
                                   bounds.height() * bounds.height()) *
                             0.5;
    const qreal startScale = internal::tokenNumber(
        QStringLiteral("desktop.component.card.rippleStartScale"));
    const qreal scale =
        startScale + (1.0 - startScale) * qBound(0.0, progress, 1.0);
    QColor ripple = DesignTokens::color(ColorRole::DataRipple,
                                        internal::colorSchemeFor(card));
    ripple.setAlphaF(static_cast<float>(ripple.alphaF() * opacity));
    QPainterPath clip;
    const qreal radius = DesignTokens::radius(RadiusRole::Field);
    clip.addRoundedRect(bounds, radius, radius);
    QPainter painter(card);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setClipPath(clip);
    painter.setPen(Qt::NoPen);
    painter.setBrush(ripple);
    painter.drawEllipse(center, fullRadius * scale, fullRadius * scale);
}

} // namespace

class CardPrivate final
{
public:
    QVBoxLayout *contentLayout = nullptr;
    QPointer<QWidget> topWidget;
    QPointer<QWidget> bodyWidget;
    QPixmap backgroundPixmap;
    CardVariant variant = CardVariant::Default;
};

Card::Card(QWidget *parent) : QFrame(parent), d(new CardPrivate)
{
    setProperty("_ngstdRole", QStringLiteral("card"));
    setAttribute(Qt::WA_StyledBackground, false);
    d->contentLayout = new QVBoxLayout(this);
    const int padding =
        DesignTokens::componentMetric(ComponentMetric::CardContentPadding);
    d->contentLayout->setContentsMargins(padding, padding, padding, padding);
    d->contentLayout->setSpacing(DesignTokens::spacing(3));
    WidgetStyle::setCardVariant(this, d->variant);
}

Card::~Card() = default;

CardVariant Card::variant() const
{
    return d->variant;
}

void Card::setVariant(CardVariant variant)
{
    if (d->variant == variant) return;
    d->variant = variant;
    WidgetStyle::setCardVariant(this, variant);
    emit variantChanged(variant);
}

void Card::resetVariant()
{
    setVariant(CardVariant::Default);
}

QVBoxLayout *Card::contentLayout() const
{
    return d->contentLayout;
}

QWidget *Card::topWidget() const
{
    return d->topWidget.data();
}

void Card::setTopWidget(QWidget *widget)
{
    replaceWidget(d->contentLayout, &d->topWidget, widget, 0);
}

QWidget *Card::takeTopWidget()
{
    return takeWidget(d->contentLayout, &d->topWidget);
}

QWidget *Card::bodyWidget() const
{
    return d->bodyWidget.data();
}

void Card::setBodyWidget(QWidget *widget)
{
    replaceWidget(d->contentLayout, &d->bodyWidget, widget,
                  d->contentLayout->count());
}

QWidget *Card::takeBodyWidget()
{
    return takeWidget(d->contentLayout, &d->bodyWidget);
}

QPixmap Card::backgroundPixmap() const
{
    return d->backgroundPixmap;
}

void Card::setBackgroundPixmap(const QPixmap &pixmap)
{
    d->backgroundPixmap = pixmap;
    update();
}

void Card::clearBackgroundPixmap()
{
    setBackgroundPixmap(QPixmap());
}

void Card::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    paintCard(this, d->variant, d->backgroundPixmap, 0.0, 0.0);
}

class CardButtonPrivate final
{
public:
    QVBoxLayout *contentLayout = nullptr;
    QPointer<QWidget> topWidget;
    QPointer<QWidget> bodyWidget;
    QPixmap backgroundPixmap;
    QVariantAnimation *interactionAnimation = nullptr;
    QVariantAnimation *selectionAnimation = nullptr;
    QVariantAnimation *feedbackAnimation = nullptr;
    QVariantAnimation *rippleAnimation = nullptr;
    QVariantAnimation *rippleOpacityAnimation = nullptr;
    QElapsedTimer rippleTimer;
    QPointF rippleOrigin;
    qreal interactionProgress = 0.0;
    qreal selectionProgress = 0.0;
    qreal feedbackProgress = 1.0;
    qreal rippleProgress = 1.0;
    qreal rippleOpacity = 0.0;
    quint64 rippleGeneration = 0;
    bool rippleReleaseScheduled = false;
    CardVariant variant = CardVariant::Selectable;
};

CardButton::CardButton(QWidget *parent)
    : QAbstractButton(parent), d(new CardButtonPrivate)
{
    setProperty("_ngstdRole", QStringLiteral("card"));
    setProperty("_ngstdPaintedCard", true);
    setProperty("interactive", true);
    setAttribute(Qt::WA_StyledBackground, false);
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    d->contentLayout = new QVBoxLayout(this);
    d->interactionAnimation = new QVariantAnimation(this);
    connect(d->interactionAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
                d->interactionProgress = value.toReal();
                update();
            });
    d->selectionAnimation = new QVariantAnimation(this);
    d->selectionAnimation->setObjectName(
        QStringLiteral("_ngstdCardSelectionAnimation"));
    connect(d->selectionAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
                d->selectionProgress = value.toReal();
                update();
            });
    d->feedbackAnimation = new QVariantAnimation(this);
    d->feedbackAnimation->setObjectName(
        QStringLiteral("_ngstdCardFeedbackAnimation"));
    connect(d->feedbackAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
                d->feedbackProgress = value.toReal();
                update();
            });
    d->rippleAnimation = new QVariantAnimation(this);
    d->rippleAnimation->setObjectName(
        QStringLiteral("_ngstdCardRippleAnimation"));
    connect(d->rippleAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
                d->rippleProgress = value.toReal();
                update();
            });
    d->rippleOpacityAnimation = new QVariantAnimation(this);
    d->rippleOpacityAnimation->setObjectName(
        QStringLiteral("_ngstdCardRippleOpacityAnimation"));
    connect(d->rippleOpacityAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
                d->rippleOpacity = value.toReal();
                update();
            });
    d->contentLayout->setSpacing(DesignTokens::spacing(3));
    updateCardButtonMargins(this, d->contentLayout, d->variant);
    WidgetStyle::setCardVariant(this, d->variant);
    connect(this, &QAbstractButton::toggled, this, [this](bool checked) {
        WidgetStyle::setSelected(this, checked);
        d->selectionAnimation->stop();
        d->selectionAnimation->setStartValue(d->selectionProgress);
        d->selectionAnimation->setEndValue(checked ? 1.0 : 0.0);
        if (internal::MotionController::configure(
                d->selectionAnimation, this,
                {MotionDuration::Normal, MotionEasing::Standard})) {
            d->selectionAnimation->start();
        }
        else {
            d->selectionProgress = checked ? 1.0 : 0.0;
        }
        d->feedbackAnimation->stop();
        d->feedbackProgress = 0.0;
        d->feedbackAnimation->setStartValue(0.0);
        d->feedbackAnimation->setEndValue(1.0);
        if (internal::MotionController::configure(
                d->feedbackAnimation, this,
                {MotionDuration::Slow, MotionEasing::Enter})) {
            d->feedbackAnimation->start();
        }
        else {
            d->feedbackProgress = 1.0;
        }
        update();
    });
}

CardButton::~CardButton() = default;

CardVariant CardButton::variant() const
{
    return d->variant;
}

void CardButton::setVariant(CardVariant variant)
{
    if (d->variant == variant) return;
    d->variant = variant;
    const bool selectable = d->variant == CardVariant::Selectable;
    if (!selectable && isChecked()) setChecked(false);
    setCheckable(selectable);
    updateCardButtonMargins(this, d->contentLayout, d->variant);
    WidgetStyle::setCardVariant(this, variant);
    emit variantChanged(variant);
}

void CardButton::resetVariant()
{
    setVariant(CardVariant::Selectable);
}

QVBoxLayout *CardButton::contentLayout() const
{
    return d->contentLayout;
}

QWidget *CardButton::topWidget() const
{
    return d->topWidget.data();
}

void CardButton::setTopWidget(QWidget *widget)
{
    replaceWidget(d->contentLayout, &d->topWidget, widget, 0);
}

QWidget *CardButton::takeTopWidget()
{
    return takeWidget(d->contentLayout, &d->topWidget);
}

QWidget *CardButton::bodyWidget() const
{
    return d->bodyWidget.data();
}

void CardButton::setBodyWidget(QWidget *widget)
{
    replaceWidget(d->contentLayout, &d->bodyWidget, widget,
                  d->contentLayout->count());
}

QWidget *CardButton::takeBodyWidget()
{
    return takeWidget(d->contentLayout, &d->bodyWidget);
}

QPixmap CardButton::backgroundPixmap() const
{
    return d->backgroundPixmap;
}

void CardButton::setBackgroundPixmap(const QPixmap &pixmap)
{
    d->backgroundPixmap = pixmap;
    update();
}

void CardButton::clearBackgroundPixmap()
{
    setBackgroundPixmap(QPixmap());
}

QSize CardButton::sizeHint() const
{
    return d->contentLayout->sizeHint().expandedTo(QSize(120, 64));
}

QSize CardButton::minimumSizeHint() const
{
    return d->contentLayout->minimumSize().expandedTo(QSize(64, 48));
}

bool CardButton::event(QEvent *event)
{
    const bool handled = QAbstractButton::event(event);
    qreal targetProgress = d->interactionProgress;
    MotionSpec motion{MotionDuration::Fast, MotionEasing::Standard};
    switch (event->type()) {
    case QEvent::Enter:
        targetProgress = 1.0;
        break;
    case QEvent::Leave:
        targetProgress = 0.0;
        motion = {MotionDuration::Slow, MotionEasing::Exit};
        finishRipple();
        break;
    case QEvent::MouseButtonPress: {
        if (isDown()) {
            targetProgress = 2.0;
            motion = {MotionDuration::Normal, MotionEasing::Enter};
            const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            startRipple(mouseEvent->position());
#else
            startRipple(mouseEvent->localPos());
#endif
        }
        break;
    }
    case QEvent::KeyPress:
        if (isDown() && !static_cast<QKeyEvent *>(event)->isAutoRepeat()) {
            targetProgress = 2.0;
            motion = {MotionDuration::Normal, MotionEasing::Enter};
            startRipple(rect().center());
        }
        break;
    case QEvent::MouseButtonRelease:
    case QEvent::KeyRelease:
        targetProgress = underMouse() ? 1.0 : 0.0;
        motion = {MotionDuration::Slow, MotionEasing::Exit};
        finishRipple();
        break;
    case QEvent::FocusOut:
        finishRipple();
        break;
    default:
        break;
    }
    if (!qFuzzyCompare(targetProgress + 1.0, d->interactionProgress + 1.0)) {
        d->interactionAnimation->stop();
        d->interactionAnimation->setStartValue(d->interactionProgress);
        d->interactionAnimation->setEndValue(targetProgress);
        if (!internal::MotionController::configure(
                d->interactionAnimation, this, motion)) {
            d->interactionProgress = targetProgress;
            update();
        }
        else {
            d->interactionAnimation->start();
        }
    }
    return handled;
}

void CardButton::changeEvent(QEvent *event)
{
    QAbstractButton::changeEvent(event);
    if (event->type() == QEvent::LayoutDirectionChange)
        updateCardButtonMargins(this, d->contentLayout, d->variant);
}

void CardButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    paintCard(this, d->variant, d->backgroundPixmap,
              isEnabled() ? d->interactionProgress : 0.0,
              isCheckable() ? d->selectionProgress : 0.0);
    paintRipple(this, d->rippleOrigin, d->rippleProgress, d->rippleOpacity);
    paintSelectionIndicator(this, d->selectionProgress, d->feedbackProgress);
}

void CardButton::startRipple(const QPointF &position)
{
    if (d->variant != CardVariant::Data || !isEnabled()) return;
    ++d->rippleGeneration;
    d->rippleReleaseScheduled = false;
    d->rippleTimer.restart();
    d->rippleOrigin = position;
    d->rippleProgress = 0.0;
    d->rippleOpacity = 0.0;
    d->rippleAnimation->stop();
    d->rippleOpacityAnimation->stop();
    d->rippleAnimation->setStartValue(0.0);
    d->rippleAnimation->setEndValue(1.0);
    d->rippleOpacityAnimation->setStartValue(0.0);
    d->rippleOpacityAnimation->setEndValue(internal::tokenNumber(
        QStringLiteral("desktop.component.card.rippleOpacity")));
    const bool scaleAnimated = internal::MotionController::configure(
        d->rippleAnimation, this,
        {MotionDuration::Normal, MotionEasing::Enter});
    const bool opacityAnimated = internal::MotionController::configure(
        d->rippleOpacityAnimation, this,
        {MotionDuration::Fast, MotionEasing::Enter});
    if (!scaleAnimated || !opacityAnimated) {
        d->rippleProgress = 1.0;
        d->rippleOpacity = 0.0;
        update();
        return;
    }
    d->rippleAnimation->start();
    d->rippleOpacityAnimation->start();
}

void CardButton::finishRipple()
{
    if (!d->rippleTimer.isValid() || d->rippleReleaseScheduled) return;
    d->rippleReleaseScheduled = true;
    const quint64 generation = d->rippleGeneration;
    const int minimumHold =
        DesignTokens::duration(MotionDuration::Normal);
    const int delay =
        qMax(0, minimumHold - static_cast<int>(d->rippleTimer.elapsed()));
    QTimer::singleShot(delay, this, [this, generation]() {
        if (generation != d->rippleGeneration) return;
        d->rippleOpacityAnimation->stop();
        d->rippleOpacityAnimation->setStartValue(d->rippleOpacity);
        d->rippleOpacityAnimation->setEndValue(0.0);
        if (internal::MotionController::configure(
                d->rippleOpacityAnimation, this,
                {MotionDuration::Slow, MotionEasing::Exit})) {
            d->rippleOpacityAnimation->start();
        }
        else {
            d->rippleOpacity = 0.0;
            update();
        }
    });
}

} // namespace widgets
} // namespace ngstd
