/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Reusable corporate button component
 *****************************************************************************/
#include <ngstd/widgets/button.h>

#include "accessibility_p.h"
#include "button_label_p.h"
#include "component_utils_p.h"
#include "motion_controller_p.h"

#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/icons.h>
#include <ngstd/widgets/view_components.h>
#include <ngstd/widgets/widget_style.h>

#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QResizeEvent>
#include <QStyle>
#include <QStyleOptionButton>
#include <QVariant>
#include <QVariantAnimation>
#include <QtMath>

namespace ngstd {
namespace widgets {

namespace {

class TrialButtonEffect final : public QGraphicsDropShadowEffect
{
public:
    explicit TrialButtonEffect(QObject *parent = nullptr)
        : QGraphicsDropShadowEffect(parent)
    {}

    void setLift(qreal lift)
    {
        if (qFuzzyCompare(m_lift + 1.0, lift + 1.0)) return;
        m_lift = lift;
        updateBoundingRect();
        update();
    }

protected:
    QRectF boundingRectFor(const QRectF &sourceRectangle) const override
    {
        return QGraphicsDropShadowEffect::boundingRectFor(sourceRectangle)
            .translated(0.0, -m_lift);
    }

    void draw(QPainter *painter) override
    {
        painter->save();
        painter->translate(0.0, -m_lift);
        QGraphicsDropShadowEffect::draw(painter);
        painter->restore();
    }

private:
    qreal m_lift = 0.0;
};

bool isPromotional(ButtonVariant variant)
{
    return variant == ButtonVariant::Hero || variant == ButtonVariant::Trial;
}

qreal trialEffectMetric(const QString &name)
{
    return internal::tokenNumber(
        QStringLiteral("product.trial.desktopShadow.%1").arg(name));
}

QColor buttonContentColor(ButtonVariant variant, bool enabled, bool checked,
                          bool down, bool pointerInside, ColorScheme scheme)
{
    if (!enabled) return DesignTokens::color(ColorRole::TextDisabled, scheme);
    if (checked) return DesignTokens::color(ColorRole::White, scheme);

    switch (variant) {
    case ButtonVariant::Primary:
    case ButtonVariant::Hero:
        return DesignTokens::color(ColorRole::White, scheme);
    case ButtonVariant::Danger:
        return DesignTokens::color(down ? ColorRole::White : ColorRole::Danger,
                                   scheme);
    case ButtonVariant::Trial:
        return internal::cssColor(
            internal::tokenString(QStringLiteral("product.trial.text")));
    case ButtonVariant::DataFilled:
        return DesignTokens::color(ColorRole::DataActionText, scheme);
    case ButtonVariant::DataOutline:
        return DesignTokens::color(ColorRole::DataActionBackground, scheme);
    case ButtonVariant::OnBrand:
        return DesignTokens::color(pointerInside || down
                                       ? ColorRole::CorporateText
                                       : ColorRole::CorporateActionText,
                                   scheme);
    case ButtonVariant::OnBrandSecondary:
        return DesignTokens::color(ColorRole::CorporateText, scheme);
    case ButtonVariant::Photo:
    case ButtonVariant::PhotoText:
        return DesignTokens::color(ColorRole::FieldworkText, scheme);
    case ButtonVariant::Ghost:
        return DesignTokens::color(
            down ? ColorRole::BrandActive
                 : (pointerInside ? ColorRole::LinkHover : ColorRole::Link),
            scheme);
    case ButtonVariant::Icon:
        if (down) return DesignTokens::color(ColorRole::BrandActive, scheme);
        Q_FALLTHROUGH();
    case ButtonVariant::Default:
    case ButtonVariant::Secondary:
    case ButtonVariant::Text:
        return DesignTokens::color(
            down ? ColorRole::BrandActive
                 : (pointerInside ? ColorRole::LinkHover : ColorRole::Text),
            scheme);
    }
    return DesignTokens::color(ColorRole::Text, scheme);
}

struct ButtonBevel
{
    QBrush background = Qt::transparent;
    QColor border = Qt::transparent;
    qreal borderWidth = 1.0;
    qreal radius = 0.0;
};

QBrush diagonalGradient(const QRectF &rectangle, const QString &startToken,
                        const QString &endToken)
{
    QLinearGradient gradient(rectangle.bottomLeft(), rectangle.topRight());
    gradient.setColorAt(
        0.0, internal::cssColor(internal::tokenString(startToken)));
    gradient.setColorAt(
        1.0, internal::cssColor(internal::tokenString(endToken)));
    return QBrush(gradient);
}

QColor dataProductColor(ColorScheme scheme, const QString &name)
{
    const QString schemeName = scheme == ColorScheme::Dark
                                   ? QStringLiteral("dark")
                                   : QStringLiteral("light");
    return internal::cssColor(internal::tokenString(
        QStringLiteral("product.data.%1.%2").arg(schemeName, name)));
}

ButtonBevel buttonBevel(const QRectF &rectangle, ButtonVariant variant,
                        QStyle::State state, ColorScheme scheme)
{
    ButtonBevel bevel;
    bevel.radius = DesignTokens::radius(
        variant == ButtonVariant::DataFilled ||
                variant == ButtonVariant::DataOutline
            ? RadiusRole::Field
            : RadiusRole::Button);
    const bool enabled = state & QStyle::State_Enabled;
    const bool checked = state & QStyle::State_On;
    const bool down = state & QStyle::State_Sunken;
    const bool hovered = state & QStyle::State_MouseOver;
    if (!enabled) {
        bevel.background =
            DesignTokens::color(ColorRole::SurfaceMuted, scheme);
        bevel.border = DesignTokens::color(ColorRole::Border, scheme);
        return bevel;
    }
    if (checked) {
        const ColorRole role = down ? ColorRole::BrandActive
                                    : (hovered ? ColorRole::BrandHover
                                               : ColorRole::Brand);
        bevel.background = DesignTokens::color(role, scheme);
        bevel.border = DesignTokens::color(role, scheme);
        return bevel;
    }

    const QColor surface = DesignTokens::color(ColorRole::Surface, scheme);
    const QColor borderStrong =
        DesignTokens::color(ColorRole::BorderStrong, scheme);
    const QColor hoverSurface =
        DesignTokens::color(ColorRole::SurfaceBrand, scheme);
    const QColor pressedSurface =
        DesignTokens::color(ColorRole::BrandSoft, scheme);
    const QColor hoverBorder =
        DesignTokens::color(ColorRole::BrandHover, scheme);
    const QColor pressedBorder =
        DesignTokens::color(ColorRole::BrandActive, scheme);
    bevel.background = down ? pressedSurface : (hovered ? hoverSurface
                                                        : surface);
    bevel.border = down ? pressedBorder : (hovered ? hoverBorder
                                                   : borderStrong);

    switch (variant) {
    case ButtonVariant::Primary: {
        const ColorRole role = down ? ColorRole::BrandActive
                                    : (hovered ? ColorRole::BrandHover
                                               : ColorRole::Brand);
        bevel.background = DesignTokens::color(role, scheme);
        bevel.border = DesignTokens::color(role, scheme);
        break;
    }
    case ButtonVariant::Secondary:
    case ButtonVariant::Default:
    case ButtonVariant::Icon:
        break;
    case ButtonVariant::Text:
        bevel.background = down ? pressedSurface : Qt::transparent;
        bevel.border = down ? pressedBorder
                            : (hovered ? borderStrong : Qt::transparent);
        break;
    case ButtonVariant::Ghost:
        bevel.background = down ? pressedSurface
                                : (hovered ? hoverSurface : Qt::transparent);
        bevel.border = Qt::transparent;
        break;
    case ButtonVariant::Danger:
        bevel.background =
            down ? DesignTokens::color(ColorRole::DangerAction, scheme)
                 : (hovered
                        ? DesignTokens::color(ColorRole::DangerSoft, scheme)
                        : surface);
        bevel.border =
            down ? DesignTokens::color(ColorRole::DangerAction, scheme)
                 : DesignTokens::color(ColorRole::Danger, scheme);
        break;
    case ButtonVariant::Hero: {
        const QString stateName = down ? QStringLiteral("Active")
                                      : (hovered ? QStringLiteral("Hover")
                                                 : QString());
        const QString suffix = stateName.isEmpty() ? QString() : stateName;
        bevel.background = diagonalGradient(
            rectangle,
            QStringLiteral("product.hero.gradient%1Start").arg(suffix),
            QStringLiteral("product.hero.gradient%1End").arg(suffix));
        bevel.borderWidth = 0.0;
        break;
    }
    case ButtonVariant::Trial:
        bevel.background = diagonalGradient(
            rectangle,
            hovered || down
                ? QStringLiteral("product.trial.gradientHoverStart")
                : QStringLiteral("product.trial.gradientStart"),
            hovered || down
                ? QStringLiteral("product.trial.gradientHoverEnd")
                : QStringLiteral("product.trial.gradientEnd"));
        bevel.borderWidth = 0.0;
        break;
    case ButtonVariant::DataFilled:
        bevel.background =
            down ? dataProductColor(scheme,
                                    QStringLiteral("actionPressedBackground"))
                 : (hovered
                        ? dataProductColor(
                              scheme,
                              QStringLiteral("actionHoverBackground"))
                        : DesignTokens::color(
                              ColorRole::DataActionBackground, scheme));
        bevel.borderWidth = 0.0;
        break;
    case ButtonVariant::DataOutline:
        bevel.background =
            down ? dataProductColor(
                       scheme, QStringLiteral("outlinePressedBackground"))
                 : (hovered
                        ? dataProductColor(
                              scheme,
                              QStringLiteral("outlineHoverBackground"))
                        : QColor(Qt::transparent));
        bevel.border =
            DesignTokens::color(ColorRole::DataActionBackground, scheme);
        break;
    case ButtonVariant::OnBrand:
        bevel.background =
            down ? DesignTokens::color(ColorRole::CorporateSurfaceHover,
                                       scheme)
                 : (hovered
                        ? QColor(Qt::transparent)
                        : DesignTokens::color(ColorRole::CorporateText,
                                              scheme));
        bevel.border =
            DesignTokens::color(ColorRole::CorporateText, scheme);
        break;
    case ButtonVariant::OnBrandSecondary:
        bevel.background =
            down ? DesignTokens::color(ColorRole::CorporateSurfaceActive,
                                       scheme)
                 : (hovered
                        ? DesignTokens::color(
                              ColorRole::CorporateSurfaceHover, scheme)
                        : QColor(Qt::transparent));
        bevel.border = DesignTokens::color(
            down || hovered ? ColorRole::CorporateText
                            : ColorRole::CorporateBorderSecondary,
            scheme);
        break;
    case ButtonVariant::Photo:
        bevel.background =
            down ? DesignTokens::color(ColorRole::FieldworkSurfaceActive,
                                       scheme)
                 : (hovered
                        ? DesignTokens::color(
                              ColorRole::FieldworkSurfaceHover, scheme)
                        : QColor(Qt::transparent));
        bevel.border = hovered || down
                           ? QColor(Qt::transparent)
                           : DesignTokens::color(ColorRole::FieldworkText,
                                                 scheme);
        break;
    case ButtonVariant::PhotoText:
        bevel.background =
            down ? DesignTokens::color(ColorRole::FieldworkSurfaceHover,
                                       scheme)
                 : (hovered
                        ? internal::cssColor(internal::tokenString(
                              QStringLiteral(
                                  "product.fieldwork.textSurfaceHover")))
                        : QColor(Qt::transparent));
        bevel.border = Qt::transparent;
        break;
    }
    return bevel;
}

void paintButtonBevel(QPainter *painter, const QStyleOptionButton &option,
                      ButtonVariant variant, const QWidget *widget)
{
    if (!painter) return;
    const ButtonBevel bevel = buttonBevel(
        QRectF(option.rect), variant, option.state,
        internal::colorSchemeFor(widget));
    const qreal inset = bevel.borderWidth * 0.5;
    const QRectF bounds =
        QRectF(option.rect).adjusted(inset, inset, -inset, -inset);
    QPainterPath path;
    path.addRoundedRect(bounds, bevel.radius, bevel.radius);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->fillPath(path, bevel.background);
    if (bevel.borderWidth > 0.0 && bevel.border.alpha() > 0) {
        painter->setPen(QPen(bevel.border, bevel.borderWidth));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(path);
    }
    painter->restore();
}

} // namespace

class ButtonPrivate final
{
public:
    Spinner *loadingSpinner = nullptr;
    QVariantAnimation *stateAnimation = nullptr;
    QVariantAnimation *shineAnimation = nullptr;
    QVariantAnimation *trialAnimation = nullptr;
    QVariantAnimation *feedbackAnimation = nullptr;
    QVariantAnimation *rippleAnimation = nullptr;
    QVariantAnimation *rippleOpacityAnimation = nullptr;
    QPointer<TrialButtonEffect> trialEffect;
    ButtonVariant variant = ButtonVariant::Default;
    IconRole iconRole = IconRole::Information;
    qreal shineProgress = 0.0;
    qreal shineTarget = 0.0;
    qreal stateProgress = 1.0;
    qreal trialProgress = 0.0;
    qreal trialTarget = 0.0;
    qreal feedbackProgress = 1.0;
    qreal rippleProgress = 1.0;
    qreal rippleOpacity = 0.0;
    QPointF rippleOrigin;
    bool hasIconRole = false;
    bool loading = false;
    bool pointerInside = false;
    QStyle::State startState = QStyle::State_None;
    QStyle::State targetState = QStyle::State_None;
    QAccessible::Id accessibleIdentifier = 0;
};

Button::Button(QWidget *parent) : QPushButton(parent), d(new ButtonPrivate)
{
    initialize();
}

Button::Button(const QString &text, QWidget *parent)
    : QPushButton(text, parent), d(new ButtonPrivate)
{
    initialize();
}

Button::~Button()
{
    internal::unregisterAccessibility(d->accessibleIdentifier);
}

ButtonVariant Button::variant() const
{
    return d->variant;
}

void Button::setVariant(ButtonVariant variant)
{
    if (d->variant == variant) return;
    if (d->trialEffect && graphicsEffect() == d->trialEffect)
        setGraphicsEffect(nullptr);
    d->trialEffect.clear();
    d->shineAnimation->stop();
    d->trialAnimation->stop();
    d->shineProgress = 0.0;
    d->shineTarget = 0.0;
    d->trialProgress = 0.0;
    d->trialTarget = 0.0;
    d->variant = variant;
    WidgetStyle::setButtonVariant(this, variant);
    updatePromotionalMotion();
    updateRoleIcon();
    update();
    emit variantChanged(variant);
}

void Button::resetVariant()
{
    setVariant(ButtonVariant::Default);
}

IconRole Button::iconRole() const
{
    return d->iconRole;
}

bool Button::hasIconRole() const
{
    return d->hasIconRole;
}

void Button::setIconRole(IconRole role)
{
    if (d->hasIconRole && d->iconRole == role) return;
    d->iconRole = role;
    d->hasIconRole = true;
    updateRoleIcon();
}

void Button::clearIconRole()
{
    if (!d->hasIconRole) return;
    d->hasIconRole = false;
    setIcon(QIcon());
}

bool Button::isLoading() const
{
    return d->loading;
}

void Button::setLoading(bool loading)
{
    if (d->loading == loading) return;
    d->loading = loading;
    setProperty("_ngstdBusy", loading);
    d->loadingSpinner->setRunning(loading &&
                                  internal::animationsEnabled(this));
    d->loadingSpinner->setVisible(loading);
    updateLoadingGeometry();
    updateGeometry();
    update();
    QAccessible::State changedState;
    changedState.busy = true;
    QAccessibleStateChangeEvent accessibilityEvent(this, changedState);
    QAccessible::updateAccessibility(&accessibilityEvent);
    emit loadingChanged(loading);
}

void Button::resetLoading()
{
    setLoading(false);
}

QSize Button::sizeHint() const
{
    QSize result = QPushButton::sizeHint();
    if (d->variant != ButtonVariant::Icon && !text().isEmpty()) {
        int minimumLabelWidth = fontMetrics().horizontalAdvance(text()) + 2;
        if (d->hasIconRole) {
            minimumLabelWidth += DesignTokens::controlIconSize() +
                                 internal::buttonIconTextGap();
        }
        const int minimumButtonWidth =
            minimumLabelWidth + DesignTokens::controlPadding() * 2 +
            DesignTokens::spacing(2) + 2;
        result.setWidth(qMax(result.width(), minimumButtonWidth));
    }
    if (d->hasIconRole && !text().isEmpty()) {
        result.rwidth() += internal::buttonIconTextGap() -
                           internal::nativeButtonIconTextGap();
    }
    if (!d->loading || !d->loadingSpinner) return result;
    const QSize spinnerSize = d->loadingSpinner->sizeHint();
    result.rwidth() += spinnerSize.width() + internal::buttonIconTextGap();
    result.setHeight(qMax(result.height(), spinnerSize.height()));
    return result;
}

QSize Button::minimumSizeHint() const
{
    return sizeHint();
}

bool Button::event(QEvent *event)
{
    if (event->type() == QEvent::Enter)
        d->pointerInside = isEnabled();
    else if (event->type() == QEvent::Leave)
        d->pointerInside = false;
    else if (event->type() == QEvent::EnabledChange)
        d->pointerInside = isEnabled() && underMouse();

    if (event->type() == QEvent::MouseButtonPress) {
        const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        startDataRipple(mouseEvent->position());
#else
        startDataRipple(mouseEvent->localPos());
#endif
    }
    else if (event->type() == QEvent::MouseButtonRelease ||
             event->type() == QEvent::Leave) {
        finishDataRipple();
    }

    if (d->loading && (event->type() == QEvent::MouseButtonPress ||
                       event->type() == QEvent::MouseButtonRelease ||
                       event->type() == QEvent::MouseButtonDblClick ||
                       event->type() == QEvent::KeyPress ||
                       event->type() == QEvent::KeyRelease)) {
        event->accept();
        return true;
    }
    const bool handled = QPushButton::event(event);
    if (event->type() == QEvent::PaletteChange ||
        event->type() == QEvent::StyleChange ||
        event->type() == QEvent::EnabledChange) {
        updateRoleIcon();
    }
    if (event->type() == QEvent::LayoutDirectionChange ||
        event->type() == QEvent::StyleChange) {
        updateRoleIcon();
        updateLoadingGeometry();
    }
    if (event->type() == QEvent::Enter || event->type() == QEvent::Leave ||
        event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonRelease ||
        event->type() == QEvent::KeyPress ||
        event->type() == QEvent::KeyRelease ||
        event->type() == QEvent::EnabledChange ||
        event->type() == QEvent::FocusIn ||
        event->type() == QEvent::FocusOut) {
        updateRoleIcon();
        updateStateMotion();
        updatePromotionalMotion();
    }
    return handled;
}

void Button::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QStyleOptionButton currentOption;
    initStyleOption(&currentOption);
    const QStyle::State animatedStateMask =
        QStyle::State_Enabled | QStyle::State_MouseOver |
        QStyle::State_Sunken | QStyle::State_Raised | QStyle::State_On |
        QStyle::State_Off | QStyle::State_HasFocus;
    const auto optionForState =
        [this, &currentOption,
         animatedStateMask](QStyle::State state) -> QStyleOptionButton {
        QStyleOptionButton option = currentOption;
        option.state &= ~animatedStateMask;
        option.state |= state;
        if (d->hasIconRole) {
            const QColor contentColor = buttonContentColor(
                d->variant, state.testFlag(QStyle::State_Enabled),
                state.testFlag(QStyle::State_On),
                state.testFlag(QStyle::State_Sunken),
                state.testFlag(QStyle::State_MouseOver),
                internal::colorSchemeFor(this));
            option.palette.setColor(QPalette::ButtonText, contentColor);
            option.palette.setColor(QPalette::WindowText, contentColor);
            option.palette.setColor(QPalette::Text, contentColor);
            const int iconSize = DesignTokens::controlIconSize();
            option.icon = iconPixmap(d->iconRole, QSize(iconSize, iconSize),
                                     devicePixelRatioF(), contentColor);
            option.iconSize = QSize(iconSize, iconSize);
        }
        else {
            const QColor contentColor = buttonContentColor(
                d->variant, state.testFlag(QStyle::State_Enabled),
                state.testFlag(QStyle::State_On),
                state.testFlag(QStyle::State_Sunken),
                state.testFlag(QStyle::State_MouseOver),
                internal::colorSchemeFor(this));
            option.palette.setColor(QPalette::ButtonText, contentColor);
            option.palette.setColor(QPalette::WindowText, contentColor);
            option.palette.setColor(QPalette::Text, contentColor);
        }
        return option;
    };
    const QStyleOptionButton startOption = optionForState(d->startState);
    const QStyleOptionButton targetOption = optionForState(d->targetState);
    const auto labelOption = [this](QStyleOptionButton option) {
        if (!d->loading) return option;
        const int reservedWidth =
            d->loadingSpinner->width() + internal::buttonIconTextGap();
        if (layoutDirection() == Qt::RightToLeft)
            option.rect.adjust(0, 0, -reservedWidth, 0);
        else
            option.rect.adjust(reservedWidth, 0, 0, 0);
        return option;
    };
    const QStyleOptionButton startLabelOption = labelOption(startOption);
    const QStyleOptionButton targetLabelOption = labelOption(targetOption);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    const auto drawControl = [this, &painter](QStyle::ControlElement element,
                                              const QStyleOptionButton &option,
                                              qreal opacity) {
        if (opacity <= 0.0) return;
        painter.save();
        painter.setOpacity(opacity);
        if (element == QStyle::CE_PushButtonLabel)
            internal::drawButtonLabel(option, &painter, this);
        else
            paintButtonBevel(&painter, option, d->variant, this);
        painter.restore();
    };
    const qreal transition = qBound(0.0, d->stateProgress, 1.0);
    drawControl(QStyle::CE_PushButtonBevel, startOption, 1.0);
    if (transition > 0.0)
        drawControl(QStyle::CE_PushButtonBevel, targetOption, transition);

    if (isPromotional(d->variant) && d->shineProgress > 0.0 &&
        d->shineProgress < 1.0) {
        const QRectF buttonRectangle(currentOption.rect);
        const qreal shineLeft =
            buttonRectangle.left() +
            (2.0 * d->shineProgress - 1.0) * buttonRectangle.width();
        const QRectF shineRectangle(shineLeft, buttonRectangle.top(),
                                    buttonRectangle.width(),
                                    buttonRectangle.height());
        QColor shineColor = internal::cssColor(
            internal::tokenString(QStringLiteral("product.hero.shine")));
        QColor transparentShine = shineColor;
        transparentShine.setAlpha(0);
        QLinearGradient shineGradient(shineRectangle.topLeft(),
                                      shineRectangle.topRight());
        shineGradient.setColorAt(0.0, transparentShine);
        shineGradient.setColorAt(0.5, shineColor);
        shineGradient.setColorAt(1.0, transparentShine);
        QPainterPath clipPath;
        const qreal radius = DesignTokens::radius(RadiusRole::Button);
        clipPath.addRoundedRect(buttonRectangle, radius, radius);
        painter.save();
        painter.setClipPath(clipPath);
        painter.fillRect(shineRectangle, shineGradient);
        painter.restore();
    }

    if ((d->variant == ButtonVariant::DataFilled ||
         d->variant == ButtonVariant::DataOutline) &&
        d->rippleOpacity > 0.0) {
        const QRectF buttonBounds(currentOption.rect);
        const qreal progress = qBound(0.0, d->rippleProgress, 1.0);
        const QPointF center =
            d->rippleOrigin +
            (buttonBounds.center() - d->rippleOrigin) * progress;
        const qreal maximumRadius =
            qSqrt(buttonBounds.width() * buttonBounds.width() +
                  buttonBounds.height() * buttonBounds.height()) *
            0.5;
        const qreal startScale = internal::tokenNumber(
            QStringLiteral("desktop.component.card.rippleStartScale"));
        const qreal scale = startScale + (1.0 - startScale) * progress;
        QColor rippleColor = DesignTokens::color(
            ColorRole::DataRipple, internal::colorSchemeFor(this));
        rippleColor.setAlphaF(static_cast<float>(
            rippleColor.alphaF() * qBound(0.0, d->rippleOpacity, 1.0)));
        QPainterPath rippleClip;
        const qreal radius = DesignTokens::radius(RadiusRole::Field);
        rippleClip.addRoundedRect(buttonBounds, radius, radius);
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setClipPath(rippleClip);
        painter.setPen(Qt::NoPen);
        painter.setBrush(rippleColor);
        painter.drawEllipse(center, maximumRadius * scale,
                            maximumRadius * scale);
        painter.restore();
    }

    if (isCheckable() && d->feedbackProgress < 1.0) {
        const qreal feedback = qBound(0.0, d->feedbackProgress, 1.0);
        QColor feedbackColor;
        if (isChecked()) {
            feedbackColor = DesignTokens::color(
                ColorRole::White, internal::colorSchemeFor(this));
            feedbackColor.setAlphaF(static_cast<float>(internal::tokenNumber(
                QStringLiteral("desktop.component.selectionControl."
                               "feedbackOnAccentOpacity"))));
        }
        else {
            feedbackColor = DesignTokens::color(
                ColorRole::FeedbackRing, internal::colorSchemeFor(this));
        }
        internal::paintSelectionFeedback(
            &painter, QRectF(rect()),
            DesignTokens::radius(RadiusRole::Button), feedback,
            feedbackColor);
    }

    QStyleOptionButton blendedLabelOption(targetLabelOption);
    const QColor startContentColor =
        startLabelOption.palette.color(QPalette::ButtonText);
    const QColor targetContentColor =
        targetLabelOption.palette.color(QPalette::ButtonText);
    const QColor contentColor = internal::interpolateColor(
        startContentColor, targetContentColor, transition);
    blendedLabelOption.palette.setColor(QPalette::ButtonText, contentColor);
    blendedLabelOption.palette.setColor(QPalette::WindowText, contentColor);
    blendedLabelOption.palette.setColor(QPalette::Text, contentColor);
    if (d->hasIconRole) {
        const int iconSize = DesignTokens::controlIconSize();
        blendedLabelOption.icon = iconPixmap(
            d->iconRole, QSize(iconSize, iconSize), devicePixelRatioF(),
            contentColor);
        blendedLabelOption.iconSize = QSize(iconSize, iconSize);
    }
    drawControl(QStyle::CE_PushButtonLabel, blendedLabelOption, 1.0);
}

void Button::resizeEvent(QResizeEvent *event)
{
    QPushButton::resizeEvent(event);
    updateLoadingGeometry();
}

void Button::initialize()
{
    d->accessibleIdentifier = internal::registerButtonAccessibility(this);
    d->stateAnimation = new QVariantAnimation(this);
    d->stateAnimation->setObjectName(
        QStringLiteral("_ngstdButtonStateAnimation"));
    connect(d->stateAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
                d->stateProgress = value.toReal();
                update();
            });
    connect(d->stateAnimation, &QVariantAnimation::finished, this, [this]() {
        d->startState = d->targetState;
        d->stateProgress = 1.0;
        update();
    });
    d->shineAnimation = new QVariantAnimation(this);
    connect(d->shineAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
                d->shineProgress = value.toReal();
                update();
            });
    d->trialAnimation = new QVariantAnimation(this);
    connect(d->trialAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
                d->trialProgress = value.toReal();
                updateTrialEffect();
            });
    d->feedbackAnimation = new QVariantAnimation(this);
    d->feedbackAnimation->setObjectName(
        QStringLiteral("_ngstdButtonFeedbackAnimation"));
    connect(d->feedbackAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
                d->feedbackProgress = value.toReal();
                update();
            });
    d->rippleAnimation = new QVariantAnimation(this);
    d->rippleAnimation->setObjectName(
        QStringLiteral("_ngstdButtonRippleAnimation"));
    connect(d->rippleAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
                d->rippleProgress = value.toReal();
                update();
            });
    d->rippleOpacityAnimation = new QVariantAnimation(this);
    d->rippleOpacityAnimation->setObjectName(
        QStringLiteral("_ngstdButtonRippleOpacityAnimation"));
    connect(d->rippleOpacityAnimation,
            &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
                d->rippleOpacity = value.toReal();
                update();
            });
    d->loadingSpinner = new Spinner(this);
    d->loadingSpinner->setAttribute(Qt::WA_TransparentForMouseEvents);
    d->loadingSpinner->hide();
    connect(this, &QAbstractButton::toggled, this, [this](bool) {
        updateRoleIcon();
        updateStateMotion();
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
    });
    setCursor(Qt::PointingHandCursor);
    d->startState =
        QStyle::State_Enabled | QStyle::State_Raised | QStyle::State_Off;
    d->targetState = d->startState;
    WidgetStyle::setButtonVariant(this, d->variant);
    updateLoadingGeometry();
}

void Button::updateStateMotion()
{
    QStyle::State targetState = QStyle::State_None;
    if (isEnabled()) targetState |= QStyle::State_Enabled;
    if (isEnabled() && d->pointerInside)
        targetState |= QStyle::State_MouseOver;
    if (isDown())
        targetState |= QStyle::State_Sunken;
    else
        targetState |= QStyle::State_Raised;
    targetState |= isChecked() ? QStyle::State_On : QStyle::State_Off;
    if (targetState == d->targetState) return;

    if (!isEnabled()) {
        d->stateAnimation->stop();
        d->startState = targetState;
        d->targetState = targetState;
        d->stateProgress = 1.0;
        update();
        return;
    }

    if (d->variant == ButtonVariant::DataFilled ||
        d->variant == ButtonVariant::DataOutline) {
        d->stateAnimation->stop();
        d->startState = targetState;
        d->targetState = targetState;
        d->stateProgress = 1.0;
        update();
        return;
    }

    const bool checkedChanged = targetState.testFlag(QStyle::State_On) !=
                                d->targetState.testFlag(QStyle::State_On);
    d->stateAnimation->stop();
    d->startState = d->stateProgress < 0.5 ? d->startState : d->targetState;
    d->targetState = targetState;
    d->stateProgress = 0.0;
    d->stateAnimation->setStartValue(0.0);
    d->stateAnimation->setEndValue(1.0);
    const MotionDuration duration = checkedChanged ? MotionDuration::Normal
                                                   : MotionDuration::Fast;
    if (internal::MotionController::configure(
            d->stateAnimation, this,
            {duration, MotionEasing::Standard})) {
        d->stateAnimation->start();
    }
    else {
        d->startState = d->targetState;
        d->stateProgress = 1.0;
        update();
    }
}

void Button::updatePromotionalMotion()
{
    if (!isPromotional(d->variant)) return;
    if (!isEnabled()) {
        d->shineAnimation->stop();
        d->trialAnimation->stop();
        d->shineProgress = 0.0;
        d->shineTarget = 0.0;
        d->trialProgress = 0.0;
        d->trialTarget = 0.0;
        updateTrialEffect();
        update();
        return;
    }
    const qreal target = isEnabled() && d->pointerInside ? 1.0 : 0.0;
    if (!qFuzzyCompare(d->shineTarget + 1.0, target + 1.0)) {
        d->shineTarget = target;
        d->shineAnimation->stop();
        d->shineAnimation->setStartValue(d->shineProgress);
        d->shineAnimation->setEndValue(target);
        if (internal::MotionController::configure(
                d->shineAnimation, this,
                {MotionDuration::Slow, MotionEasing::Enter})) {
            d->shineAnimation->start();
        }
        else {
            d->shineProgress = target;
            update();
        }
    }

    if (d->variant != ButtonVariant::Trial) return;
    if (!d->trialEffect && !graphicsEffect()) {
        d->trialEffect = new TrialButtonEffect(this);
        setGraphicsEffect(d->trialEffect);
    }
    if (!qFuzzyCompare(d->trialTarget + 1.0, target + 1.0)) {
        d->trialTarget = target;
        d->trialAnimation->stop();
        d->trialAnimation->setStartValue(d->trialProgress);
        d->trialAnimation->setEndValue(target);
        if (internal::MotionController::configure(
                d->trialAnimation, this,
                {MotionDuration::Slow, MotionEasing::Enter})) {
            d->trialAnimation->start();
        }
        else {
            d->trialProgress = target;
            updateTrialEffect();
        }
    }
    updateTrialEffect();
}

void Button::updateTrialEffect()
{
    if (!d->trialEffect) return;
    const qreal progress = qBound(0.0, d->trialProgress, 1.0);
    const qreal blur = trialEffectMetric(QStringLiteral("blurPx")) +
                       (trialEffectMetric(QStringLiteral("hoverBlurPx")) -
                        trialEffectMetric(QStringLiteral("blurPx"))) *
                           progress;
    const qreal offset = trialEffectMetric(QStringLiteral("offsetYPx")) +
                         (trialEffectMetric(QStringLiteral("hoverOffsetYPx")) -
                          trialEffectMetric(QStringLiteral("offsetYPx"))) *
                             progress;
    const qreal opacity = trialEffectMetric(QStringLiteral("opacity")) +
                          (trialEffectMetric(QStringLiteral("hoverOpacity")) -
                           trialEffectMetric(QStringLiteral("opacity"))) *
                              progress;
    QColor shadowColor = internal::cssColor(internal::tokenString(
        QStringLiteral("product.trial.desktopShadow.color")));
    shadowColor.setAlphaF(static_cast<float>(opacity));
    d->trialEffect->setBlurRadius(blur);
    d->trialEffect->setOffset(0.0, offset);
    d->trialEffect->setColor(shadowColor);
    const qreal lift =
        isDown() ? 0.0
                 : trialEffectMetric(QStringLiteral("hoverLiftPx")) * progress;
    d->trialEffect->setLift(lift);
}

void Button::updateRoleIcon()
{
    if (!d->hasIconRole) return;
    const int iconSize = DesignTokens::controlIconSize();
    const ColorScheme scheme = internal::colorSchemeFor(this);
    const QColor color = buttonContentColor(
        d->variant, isEnabled(), isChecked(), isDown(), d->pointerInside,
        scheme);
    setIcon(iconPixmap(d->iconRole, QSize(iconSize, iconSize),
                       devicePixelRatioF(), color));
    setIconSize(QSize(iconSize, iconSize));
}

void Button::updateLoadingGeometry()
{
    if (!d->loadingSpinner) return;
    const QSize spinnerSize = d->loadingSpinner->sizeHint();
    QStyleOptionButton option;
    initStyleOption(&option);
    const QRect contents =
        style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);
    const QRect logicalGeometry(
        contents.left(),
        contents.top() + (contents.height() - spinnerSize.height()) / 2,
        spinnerSize.width(), spinnerSize.height());
    d->loadingSpinner->setGeometry(
        style()->visualRect(layoutDirection(), contents, logicalGeometry));
    d->loadingSpinner->raise();
}

void Button::startDataRipple(const QPointF &origin)
{
    if (!isEnabled() || d->loading ||
        (d->variant != ButtonVariant::DataFilled &&
         d->variant != ButtonVariant::DataOutline)) {
        return;
    }
    d->rippleAnimation->stop();
    d->rippleOpacityAnimation->stop();
    d->rippleOrigin = origin;
    d->rippleProgress = 0.0;
    d->rippleOpacity = internal::tokenNumber(
        QStringLiteral("desktop.component.card.rippleOpacity"));
    d->rippleAnimation->setStartValue(0.0);
    d->rippleAnimation->setEndValue(1.0);
    if (internal::MotionController::configure(
            d->rippleAnimation, this,
            {MotionDuration::Normal, MotionEasing::Enter})) {
        d->rippleAnimation->start();
    }
    else {
        d->rippleProgress = 1.0;
        d->rippleOpacity = 0.0;
    }
    update();
}

void Button::finishDataRipple()
{
    if (d->rippleOpacity <= 0.0) return;
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
}

} // namespace widgets
} // namespace ngstd
