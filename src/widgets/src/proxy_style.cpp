/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Optional token-backed proxy style for standalone applications
 *****************************************************************************/
#include <ngstd/widgets/proxy_style.h>

#include "button_label_p.h"
#include "component_utils_p.h"
#include "design_tokens_p.h"
#include "motion_controller_p.h"

#include <ngstd/widgets/button.h>
#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/icons.h>

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QDynamicPropertyChangeEvent>
#include <QEvent>
#include <QFocusFrame>
#include <QFontMetrics>
#include <QHash>
#include <QLineEdit>
#include <QItemSelectionModel>
#include <QListView>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QRadioButton>
#include <QStyleFactory>
#include <QStyleOptionButton>
#include <QStyleOptionComboBox>
#include <QStyleOptionFrame>
#include <QStyleOptionSpinBox>
#include <QStyleOptionTab>
#include <QStyleOptionViewItem>
#include <QTabBar>
#include <QTreeView>
#include <QVariantAnimation>
#include <QtMath>

namespace ngstd {
namespace widgets {

namespace {

constexpr auto errorProperty = "ngstdError";
constexpr auto comboArrowProgressProperty = "_ngstdArrowProgress";
constexpr auto iconRoleProperty = "_ngstdIconRole";

QColor semanticButtonTextColor(const QStyleOptionButton *option,
                               const QWidget *widget)
{
    const ColorScheme scheme = internal::colorSchemeFor(widget);
    if (!option || !(option->state & QStyle::State_Enabled))
        return DesignTokens::color(ColorRole::TextDisabled, scheme);
    if (option->state & QStyle::State_On)
        return DesignTokens::color(ColorRole::White, scheme);

    const bool down = option->state & QStyle::State_Sunken;
    const bool hovered = option->state & QStyle::State_MouseOver;
    const QString variant =
        widget ? widget->property("ngstdButtonVariant").toString() : QString();
    if (variant == QStringLiteral("primary") ||
        variant == QStringLiteral("hero")) {
        return DesignTokens::color(ColorRole::White, scheme);
    }
    if (variant == QStringLiteral("danger")) {
        return DesignTokens::color(down ? ColorRole::White : ColorRole::Danger,
                                   scheme);
    }
    if (variant == QStringLiteral("trial")) {
        return internal::cssColor(
            internal::tokenString(QStringLiteral("product.trial.text")));
    }
    if (variant == QStringLiteral("dataFilled"))
        return DesignTokens::color(ColorRole::DataActionText, scheme);
    if (variant == QStringLiteral("dataOutline"))
        return DesignTokens::color(ColorRole::DataActionBackground, scheme);
    if (variant == QStringLiteral("onBrand")) {
        return DesignTokens::color(
            hovered || down ? ColorRole::CorporateText
                            : ColorRole::CorporateActionText,
            scheme);
    }
    if (variant == QStringLiteral("onBrandSecondary"))
        return DesignTokens::color(ColorRole::CorporateText, scheme);
    if (variant == QStringLiteral("photo") ||
        variant == QStringLiteral("photoText")) {
        return DesignTokens::color(ColorRole::FieldworkText, scheme);
    }
    if (variant == QStringLiteral("ghost")) {
        return DesignTokens::color(
            down ? ColorRole::BrandActive
                 : (hovered ? ColorRole::LinkHover : ColorRole::Link),
            scheme);
    }
    return DesignTokens::color(
        down ? ColorRole::BrandActive
             : (hovered ? ColorRole::LinkHover : ColorRole::Text),
        scheme);
}

void configureProgressAnimation(QVariantAnimation *animation,
                                const QString &objectName, QObject *context,
                                qreal *progress, QWidget *widget)
{
    animation->setObjectName(objectName);
    animation->setProperty("_ngstdAnimationWidget",
                           QVariant::fromValue(static_cast<QObject *>(widget)));
    const QPointer<QWidget> guardedWidget(widget);
    QObject::connect(animation, &QVariantAnimation::valueChanged, context,
                     [progress, guardedWidget](const QVariant &value) {
                         *progress = value.toReal();
                         if (guardedWidget) guardedWidget->update();
                     });
}

class SelectionAnimationState final : public QObject
{
public:
    explicit SelectionAnimationState(QAbstractButton *button, QObject *parent)
        : QObject(parent), m_button(button),
          m_selectionAnimation(new QVariantAnimation(this)),
          m_hoverAnimation(new QVariantAnimation(this)),
          m_feedbackAnimation(new QVariantAnimation(this)),
          m_selectionProgress(button->isChecked() ? 1.0 : 0.0),
          m_hoverProgress(button->underMouse() ? 1.0 : 0.0)
    {
        button->setAttribute(Qt::WA_Hover, true);
        button->installEventFilter(this);
        configureProgressAnimation(
            m_selectionAnimation,
            QStringLiteral("_ngstdSelectionStateAnimation"), this,
            &m_selectionProgress, button);
        configureProgressAnimation(
            m_hoverAnimation, QStringLiteral("_ngstdSelectionHoverAnimation"),
            this, &m_hoverProgress, button);
        configureProgressAnimation(
            m_feedbackAnimation,
            QStringLiteral("_ngstdSelectionFeedbackAnimation"), this,
            &m_feedbackProgress, button);
        connect(button, &QAbstractButton::toggled, this, [this](bool checked) {
            animate(m_selectionAnimation, m_selectionProgress,
                    checked ? 1.0 : 0.0,
                    {MotionDuration::Fast, MotionEasing::Standard});
            m_feedbackAnimation->stop();
            m_feedbackProgress = 0.0;
            animate(m_feedbackAnimation, 0.0, 1.0,
                    {MotionDuration::Slow, MotionEasing::Enter});
        });
    }

    qreal selectionProgress() const { return m_selectionProgress; }
    qreal hoverProgress() const { return m_hoverProgress; }
    qreal feedbackProgress() const { return m_feedbackProgress; }
    bool isPressed() const { return m_pressed; }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched != m_button) return QObject::eventFilter(watched, event);
        switch (event->type()) {
        case QEvent::Enter:
            animate(m_hoverAnimation, m_hoverProgress, 1.0,
                    {MotionDuration::Fast, MotionEasing::Standard});
            break;
        case QEvent::Leave:
            m_pressed = false;
            animate(m_hoverAnimation, m_hoverProgress, 0.0,
                    {MotionDuration::Fast, MotionEasing::Standard});
            break;
        case QEvent::MouseButtonPress:
        case QEvent::KeyPress:
            m_pressed = true;
            m_button->update();
            break;
        case QEvent::MouseButtonRelease:
        case QEvent::KeyRelease:
            m_pressed = false;
            m_button->update();
            break;
        case QEvent::EnabledChange:
        case QEvent::PaletteChange:
            m_button->update();
            break;
        default:
            break;
        }
        return QObject::eventFilter(watched, event);
    }

private:
    void animate(QVariantAnimation *animation, qreal start, qreal target,
                 MotionSpec motion)
    {
        animation->stop();
        animation->setStartValue(start);
        animation->setEndValue(target);
        if (internal::MotionController::configure(animation, m_button,
                                                  motion)) {
            animation->start();
            return;
        }
        if (animation == m_selectionAnimation)
            m_selectionProgress = target;
        else if (animation == m_hoverAnimation)
            m_hoverProgress = target;
        else
            m_feedbackProgress = target;
        if (m_button) m_button->update();
    }

    QPointer<QWidget> m_button;
    QVariantAnimation *m_selectionAnimation;
    QVariantAnimation *m_hoverAnimation;
    QVariantAnimation *m_feedbackAnimation;
    qreal m_selectionProgress;
    qreal m_hoverProgress;
    qreal m_feedbackProgress = 1.0;
    bool m_pressed = false;
};

class FieldAnimationState final : public QObject
{
public:
    explicit FieldAnimationState(QWidget *field, QObject *parent)
        : QObject(parent), m_field(field),
          m_borderAnimation(new QVariantAnimation(this)),
          m_originalMinimumHeight(field->minimumHeight()),
          m_appliedMinimumHeight(qMax(
              field->minimumHeight(),
              DesignTokens::controlHeight(ControlSize::Medium))),
          m_hovered(field->underMouse())
    {
        field->setMinimumHeight(m_appliedMinimumHeight);
        field->setAttribute(Qt::WA_Hover, true);
        field->installEventFilter(this);
        m_currentBorder = targetBorder();
        m_startBorder = m_currentBorder;
        m_targetBorder = m_currentBorder;
        m_borderAnimation->setObjectName(
            QStringLiteral("_ngstdFieldBorderAnimation"));
        connect(m_borderAnimation, &QVariantAnimation::valueChanged, this,
                [this](const QVariant &value) {
                    m_currentBorder = internal::interpolateColor(
                        m_startBorder, m_targetBorder, value.toReal());
                    if (m_field) m_field->update();
                 });
    }

    ~FieldAnimationState() override
    {
        if (m_field &&
            m_field->minimumHeight() == m_appliedMinimumHeight) {
            m_field->setMinimumHeight(m_originalMinimumHeight);
        }
    }

    QColor border() const { return m_currentBorder; }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched != m_field) return QObject::eventFilter(watched, event);
        bool stateChanged = false;
        switch (event->type()) {
        case QEvent::Enter:
            m_hovered = true;
            stateChanged = true;
            break;
        case QEvent::Leave:
            m_hovered = false;
            stateChanged = true;
            break;
        case QEvent::EnabledChange:
        case QEvent::PaletteChange:
            stateChanged = true;
            break;
        case QEvent::DynamicPropertyChange: {
            const QByteArray propertyName =
                static_cast<QDynamicPropertyChangeEvent *>(event)
                    ->propertyName();
            stateChanged = propertyName == QByteArray(errorProperty) ||
                           propertyName ==
                               QByteArrayLiteral("_ngstdColorScheme");
            break;
        }
        default:
            break;
        }
        if (stateChanged) animateBorder(targetBorder());
        return QObject::eventFilter(watched, event);
    }

private:
    QColor targetBorder() const
    {
        const ColorScheme scheme = internal::colorSchemeFor(m_field);
        if (!m_field || !m_field->isEnabled())
            return DesignTokens::color(ColorRole::Border, scheme);
        if (m_field->property(errorProperty).toBool())
            return DesignTokens::color(ColorRole::Danger, scheme);
        if (m_hovered)
            return DesignTokens::color(ColorRole::BrandHover, scheme);
        return DesignTokens::color(ColorRole::BorderStrong, scheme);
    }

    void animateBorder(const QColor &target)
    {
        if (target == m_targetBorder) return;
        m_borderAnimation->stop();
        m_startBorder = m_currentBorder;
        m_targetBorder = target;
        m_borderAnimation->setStartValue(0.0);
        m_borderAnimation->setEndValue(1.0);
        if (internal::MotionController::configure(
                m_borderAnimation, m_field,
                {MotionDuration::Fast, MotionEasing::Standard})) {
            m_borderAnimation->start();
            return;
        }
        m_currentBorder = m_targetBorder;
        if (m_field) m_field->update();
    }

    QPointer<QWidget> m_field;
    QVariantAnimation *m_borderAnimation;
    QColor m_currentBorder;
    QColor m_startBorder;
    QColor m_targetBorder;
    int m_originalMinimumHeight;
    int m_appliedMinimumHeight;
    bool m_hovered;
};

class TabAnimationState final : public QObject
{
public:
    explicit TabAnimationState(QTabBar *tabBar, QObject *parent)
        : QObject(parent), m_tabBar(tabBar),
          m_animation(new QVariantAnimation(this)),
          m_previousIndex(tabBar->currentIndex()),
          m_targetIndex(tabBar->currentIndex())
    {
        m_animation->setObjectName(QStringLiteral("_ngstdTabStateAnimation"));
        connect(m_animation, &QVariantAnimation::valueChanged, this,
                [this](const QVariant &value) {
                    m_progress = value.toReal();
                    if (m_tabBar) m_tabBar->update();
                });
        connect(m_animation, &QVariantAnimation::finished, this, [this]() {
            m_previousIndex = m_targetIndex;
            m_progress = 1.0;
            if (m_tabBar) m_tabBar->update();
        });
        connect(tabBar, &QTabBar::currentChanged, this, [this](int index) {
            if (index < 0 || index == m_targetIndex) return;
            m_animation->stop();
            m_previousIndex = m_targetIndex;
            m_targetIndex = index;
            m_progress = 0.0;
            m_animation->setStartValue(0.0);
            m_animation->setEndValue(1.0);
            if (internal::MotionController::configure(
                    m_animation, m_tabBar,
                    {MotionDuration::Normal, MotionEasing::Standard})) {
                m_animation->start();
                return;
            }
            m_previousIndex = m_targetIndex;
            m_progress = 1.0;
            if (m_tabBar) m_tabBar->update();
        });
    }

    int previousIndex() const { return m_previousIndex; }
    int targetIndex() const { return m_targetIndex; }
    qreal progress() const { return m_progress; }

private:
    QPointer<QWidget> m_tabBar;
    QVariantAnimation *m_animation;
    int m_previousIndex;
    int m_targetIndex;
    qreal m_progress = 1.0;
};

class ItemViewUpdateState final : public QObject
{
public:
    explicit ItemViewUpdateState(QAbstractItemView *view, QObject *parent)
        : QObject(parent), m_view(view)
    {
        view->installEventFilter(this);
        bindSelectionModel();
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched != m_view)
            return QObject::eventFilter(watched, event);
        if (event->type() == QEvent::Show ||
            event->type() == QEvent::Polish ||
            event->type() == QEvent::StyleChange ||
            event->type() == QEvent::DynamicPropertyChange) {
            bindSelectionModel();
        }
        return QObject::eventFilter(watched, event);
    }

private:
    void bindSelectionModel()
    {
        if (!m_view || m_selectionModel == m_view->selectionModel()) return;
        if (m_selectionModel) disconnect(m_selectionModel, nullptr, this, nullptr);
        m_selectionModel = m_view->selectionModel();
        if (!m_selectionModel) return;
        connect(m_selectionModel, &QItemSelectionModel::currentChanged, this,
                [this](const QModelIndex &, const QModelIndex &) {
                    if (m_view && m_view->viewport())
                        m_view->viewport()->update();
                });
        connect(m_selectionModel, &QItemSelectionModel::selectionChanged,
                this, [this](const QItemSelection &, const QItemSelection &) {
                    if (m_view && m_view->viewport())
                        m_view->viewport()->update();
                });
    }

    QPointer<QAbstractItemView> m_view;
    QPointer<QItemSelectionModel> m_selectionModel;
};

QColor fieldBorder(const QStyleOption *option, const QWidget *widget)
{
    const ColorScheme scheme = internal::colorSchemeFor(widget);
    if (!(option->state & QStyle::State_Enabled))
        return DesignTokens::color(ColorRole::Border, scheme);
    if (widget && widget->property(errorProperty).toBool())
        return DesignTokens::color(ColorRole::Danger, scheme);
    if (option->state & QStyle::State_MouseOver)
        return DesignTokens::color(ColorRole::BrandHover, scheme);
    return DesignTokens::color(ColorRole::BorderStrong, scheme);
}

void paintFieldPanel(const QStyleOption *option, QPainter *painter,
                     const QWidget *widget,
                     const FieldAnimationState *animationState)
{
    if (!option || !painter) return;
    const ColorScheme scheme = internal::colorSchemeFor(widget);
    const bool enabled = option->state & QStyle::State_Enabled;
    const bool error = widget && widget->property(errorProperty).toBool();
    const QColor surface = DesignTokens::color(
        !enabled ? ColorRole::SurfaceMuted
                 : (error ? ColorRole::DangerSoft : ColorRole::Surface),
        scheme);
    const QColor border = animationState ? animationState->border()
                                         : fieldBorder(option, widget);
    const qreal radius = DesignTokens::radius(RadiusRole::Field);
    const QRectF bounds =
        QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(border, 1.0));
    painter->setBrush(surface);
    painter->drawRoundedRect(bounds, radius, radius);
    painter->restore();
}

void paintSelectionIndicator(
    QStyle::PrimitiveElement element, const QStyleOption *option,
    QPainter *painter, const QWidget *widget,
    const SelectionAnimationState *animationState)
{
    if (!option || !painter) return;
    const bool radio = element == QStyle::PE_IndicatorRadioButton;
    const bool enabled = option->state & QStyle::State_Enabled;
    const bool checked = option->state & QStyle::State_On;
    const bool partial = option->state & QStyle::State_NoChange;
    const bool pressed = animationState
                             ? animationState->isPressed()
                             : (option->state & QStyle::State_Sunken);
    const qreal selected = animationState
                               ? animationState->selectionProgress()
                               : ((checked || partial) ? 1.0 : 0.0);
    const qreal hovered = animationState
                              ? animationState->hoverProgress()
                              : ((option->state & QStyle::State_MouseOver)
                                     ? 1.0
                                     : 0.0);
    const qreal feedback = animationState
                               ? animationState->feedbackProgress()
                               : 1.0;
    const ColorScheme scheme = internal::colorSchemeFor(widget);
    const QRectF bounds =
        QRectF(option->rect).adjusted(0.75, 0.75, -0.75, -0.75);
    const QColor idleSurface = DesignTokens::color(
        enabled ? ColorRole::Surface : ColorRole::SurfaceMuted, scheme);
    const QColor selectedSurface = DesignTokens::color(
        enabled ? (pressed ? ColorRole::BrandActive : ColorRole::Brand)
                : ColorRole::SurfaceMuted,
        scheme);
    QColor idleBorder = DesignTokens::color(
        enabled ? ColorRole::BorderStrong : ColorRole::Border, scheme);
    if (enabled && hovered > 0.0) {
        idleBorder = internal::interpolateColor(
            idleBorder, DesignTokens::color(ColorRole::BrandHover, scheme),
            hovered);
    }
    const QColor selectedBorder = DesignTokens::color(
        enabled ? (pressed ? ColorRole::BrandActive : ColorRole::Brand)
                : ColorRole::Border,
        scheme);
    const QColor surface =
        internal::interpolateColor(idleSurface, selectedSurface, selected);
    const QColor border =
        internal::interpolateColor(idleBorder, selectedBorder, selected);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    const auto drawFeedback = [&]() {
        if (feedback >= 1.0) return;
        QColor feedbackColor =
            DesignTokens::color(ColorRole::FeedbackRing, scheme);
        internal::paintSelectionFeedback(
            painter, bounds,
            DesignTokens::componentMetric(
                ComponentMetric::SelectionIndicatorRadius),
            feedback, feedbackColor, radio);
    };

    painter->setPen(QPen(border, 1.5));
    painter->setBrush(surface);
    if (radio)
        painter->drawEllipse(bounds);
    else {
        const qreal radius = DesignTokens::componentMetric(
            ComponentMetric::SelectionIndicatorRadius);
        painter->drawRoundedRect(bounds, radius, radius);
    }
    if (selected <= 0.0) {
        drawFeedback();
        painter->restore();
        return;
    }

    QColor mark = DesignTokens::color(
        enabled ? ColorRole::White : ColorRole::TextDisabled, scheme);
    mark.setAlphaF(static_cast<float>(mark.alphaF() * selected));
    if (radio) {
        const qreal dotSize =
            DesignTokens::componentMetric(
                ComponentMetric::SelectionRadioDotSize) *
            selected;
        painter->setPen(Qt::NoPen);
        painter->setBrush(mark);
        painter->drawEllipse(bounds.center(), dotSize * 0.5, dotSize * 0.5);
    }
    else if (partial) {
        painter->setPen(QPen(mark, 2.0, Qt::SolidLine, Qt::RoundCap));
        const qreal halfWidth = bounds.width() * 0.25 * selected;
        painter->drawLine(QPointF(bounds.center().x() - halfWidth,
                                  bounds.center().y()),
                          QPointF(bounds.center().x() + halfWidth,
                                  bounds.center().y()));
    }
    else {
        const auto point = [&bounds](qreal x, qreal y) {
            return QPointF(bounds.left() + bounds.width() * x / 24.0,
                           bounds.top() + bounds.height() * y / 24.0);
        };
        QPainterPath checkPath;
        checkPath.moveTo(point(5.0, 12.0));
        checkPath.lineTo(point(10.0, 17.0));
        checkPath.lineTo(point(19.0, 7.0));
        painter->setBrush(Qt::NoBrush);
        painter->setPen(
            QPen(mark, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawPath(checkPath);
    }
    drawFeedback();
    painter->restore();
}

void paintArrow(QPainter *painter, const QRect &rectangle, QColor color,
                bool pointsUp, qreal rotation = 0.0)
{
    if (!painter || rectangle.isEmpty()) return;
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->translate(QRectF(rectangle).center());
    painter->rotate(rotation);
    const qreal halfWidth = rectangle.width() * 0.28;
    const qreal halfHeight = rectangle.height() * 0.14;
    QPainterPath path;
    if (pointsUp) {
        path.moveTo(-halfWidth, halfHeight);
        path.lineTo(0.0, -halfHeight);
        path.lineTo(halfWidth, halfHeight);
    }
    else {
        path.moveTo(-halfWidth, -halfHeight);
        path.lineTo(0.0, halfHeight);
        path.lineTo(halfWidth, -halfHeight);
    }
    painter->setPen(
        QPen(color, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);
    painter->restore();
}

bool paintFocusFrame(const QStyleOption *option, QPainter *painter,
                     const QWidget *widget)
{
    const QFocusFrame *focusFrame = qobject_cast<const QFocusFrame *>(widget);
    if (!focusFrame || !option || !painter) return false;
    const QWidget *focusWidget = focusFrame->widget();
    const qreal strokeWidth = DesignTokens::componentMetric(
        ComponentMetric::FocusFrameStrokeWidth);
    const qreal margin = DesignTokens::componentMetric(
        ComponentMetric::FocusFrameMargin);
    qreal radius = DesignTokens::radius(RadiusRole::Button);
    const QString role = focusWidget
                             ? focusWidget->property("_ngstdRole").toString()
                             : QString();
    if (focusWidget &&
        (qobject_cast<const QLineEdit *>(focusWidget) ||
         qobject_cast<const QComboBox *>(focusWidget) ||
         qobject_cast<const QAbstractSpinBox *>(focusWidget))) {
        radius = DesignTokens::radius(RadiusRole::Field);
    }
    else if (role == QStringLiteral("card")) {
        radius = DesignTokens::radius(RadiusRole::Card);
    }
    const QRectF bounds = QRectF(option->rect).adjusted(
        strokeWidth * 0.5, strokeWidth * 0.5, -strokeWidth * 0.5,
        -strokeWidth * 0.5);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    const QWidget *schemeWidget = focusFrame->property("_ngstdColorScheme")
                                      .isValid()
                                      ? focusFrame
                                      : focusWidget;
    painter->setPen(QPen(
        DesignTokens::color(ColorRole::Focus,
                            internal::colorSchemeFor(schemeWidget)),
        strokeWidth));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(bounds, radius + margin, radius + margin);
    painter->restore();
    return true;
}

int tabIndexForRectangle(const QTabBar *tabBar, const QRect &rectangle)
{
    if (!tabBar) return -1;
    for (int i = 0; i < tabBar->count(); ++i) {
        if (tabBar->tabRect(i) == rectangle) return i;
    }
    return -1;
}

} // namespace

class NextgisProxyStylePrivate final : public QObject
{
public:
    explicit NextgisProxyStylePrivate(QObject *parent) : QObject(parent) {}

    void polish(QWidget *widget)
    {
        if (!widget || m_states.contains(widget)) return;
        QObject *state = nullptr;
        if (QCheckBox *checkBox = qobject_cast<QCheckBox *>(widget))
            state = new SelectionAnimationState(checkBox, this);
        else if (QRadioButton *radioButton =
                     qobject_cast<QRadioButton *>(widget))
            state = new SelectionAnimationState(radioButton, this);
        else if (QTabBar *tabBar = qobject_cast<QTabBar *>(widget))
            state = new TabAnimationState(tabBar, this);
        else if (QAbstractItemView *itemView =
                     qobject_cast<QAbstractItemView *>(widget))
            state = new ItemViewUpdateState(itemView, this);
        else if (isField(widget))
            state = new FieldAnimationState(widget, this);
        if (!state) return;

        m_states.insert(widget, state);
        connect(widget, &QObject::destroyed, state, [this, widget]() {
            QObject *removedState = m_states.take(widget);
            if (removedState) removedState->deleteLater();
        });
    }

    void unpolish(QWidget *widget)
    {
        QObject *state = m_states.take(widget);
        delete state;
    }

    template <typename State>
    const State *state(const QWidget *widget) const
    {
        const auto iterator = m_states.constFind(widget);
        if (iterator == m_states.cend()) return nullptr;
        return dynamic_cast<const State *>(iterator.value());
    }

private:
    static bool isField(QWidget *widget)
    {
        if (qobject_cast<QComboBox *>(widget) ||
            qobject_cast<QAbstractSpinBox *>(widget)) {
            return true;
        }
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(widget);
        if (!lineEdit) return false;
        return !qobject_cast<QAbstractSpinBox *>(widget->parentWidget()) &&
               !qobject_cast<QComboBox *>(widget->parentWidget());
    }

    QHash<const QWidget *, QObject *> m_states;
};

NextgisProxyStyle::NextgisProxyStyle(QStyle *baseStyle)
    : QProxyStyle(baseStyle), d(new NextgisProxyStylePrivate(this))
{}

NextgisProxyStyle::NextgisProxyStyle(const QString &baseStyleName)
    : QProxyStyle(baseStyleName), d(new NextgisProxyStylePrivate(this))
{}

NextgisProxyStyle::~NextgisProxyStyle() = default;

NextgisProxyStyle *NextgisProxyStyle::create(const QString &baseStyleName)
{
    QStyle *baseStyle = QStyleFactory::create(baseStyleName);
    if (!baseStyle)
        baseStyle = QStyleFactory::create(QStringLiteral("Fusion"));
    return new NextgisProxyStyle(baseStyle);
}

void NextgisProxyStyle::polish(QWidget *widget)
{
    QProxyStyle::polish(widget);
    d->polish(widget);
}

void NextgisProxyStyle::unpolish(QWidget *widget)
{
    d->unpolish(widget);
    QProxyStyle::unpolish(widget);
}

int NextgisProxyStyle::pixelMetric(PixelMetric metric,
                                   const QStyleOption *option,
                                   const QWidget *widget) const
{
    switch (metric) {
    case QStyle::PM_ScrollBarExtent:
        return DesignTokens::componentMetric(ComponentMetric::ScrollBarExtent);
    case QStyle::PM_ScrollBarSliderMin:
        return DesignTokens::componentMetric(
            ComponentMetric::ScrollBarMinimumThumb);
    case QStyle::PM_DefaultFrameWidth:
    case QStyle::PM_ComboBoxFrameWidth:
    case QStyle::PM_SpinBoxFrameWidth:
        return 1;
    case QStyle::PM_FocusFrameHMargin:
    case QStyle::PM_FocusFrameVMargin:
        return DesignTokens::componentMetric(
            ComponentMetric::FocusFrameMargin);
    case QStyle::PM_ButtonMargin:
        return DesignTokens::controlPadding();
    case QStyle::PM_IndicatorWidth:
    case QStyle::PM_IndicatorHeight:
    case QStyle::PM_ExclusiveIndicatorWidth:
    case QStyle::PM_ExclusiveIndicatorHeight:
        return DesignTokens::componentMetric(
            ComponentMetric::SelectionIndicatorSize);
    case QStyle::PM_CheckBoxLabelSpacing:
    case QStyle::PM_RadioButtonLabelSpacing:
        return DesignTokens::spacing(2);
    case QStyle::PM_TabBarTabHSpace:
        return DesignTokens::componentMetric(
                   ComponentMetric::TabPaddingHorizontal) *
               2;
    case QStyle::PM_TabBarTabVSpace:
        return DesignTokens::componentMetric(
                   ComponentMetric::TabPaddingVertical) *
                   2 +
               DesignTokens::componentMetric(
                   ComponentMetric::TabUnderlineHeight);
    case QStyle::PM_TreeViewIndentation:
        return DesignTokens::componentMetric(
            ComponentMetric::ItemViewIndentation);
    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

QRect NextgisProxyStyle::subElementRect(SubElement element,
                                        const QStyleOption *option,
                                        const QWidget *widget) const
{
    if (option && (element == QStyle::SE_CheckBoxIndicator ||
                   element == QStyle::SE_RadioButtonIndicator)) {
        const int indicatorSize = DesignTokens::componentMetric(
            ComponentMetric::SelectionIndicatorSize);
        return QStyle::alignedRect(
            option->direction, Qt::AlignLeading | Qt::AlignVCenter,
            QSize(indicatorSize, indicatorSize), option->rect);
    }
    const QStyleOptionViewItem *itemOption =
        qstyleoption_cast<const QStyleOptionViewItem *>(option);
    if (itemOption &&
        itemOption->features & QStyleOptionViewItem::HasCheckIndicator) {
        if (element == QStyle::SE_ItemViewItemCheckIndicator) {
            const QTreeView *treeView = qobject_cast<const QTreeView *>(widget);
            if (!treeView && widget)
                treeView = qobject_cast<const QTreeView *>(widget->parentWidget());
            const bool compact = treeView &&
                treeView->property("_ngstdCompactItems").toBool();
            const int indicatorSize =
                DesignTokens::componentMetric(
                    ComponentMetric::SelectionIndicatorSize) -
                (compact ? DesignTokens::spacing(1) : 0);
            const int horizontalPadding = DesignTokens::spacing(compact ? 2 : 3);
            const QRect contentRectangle = itemOption->rect.adjusted(
                horizontalPadding, 0, -horizontalPadding, 0);
            return QStyle::alignedRect(
                itemOption->direction,
                Qt::AlignLeading | Qt::AlignVCenter,
                QSize(indicatorSize, indicatorSize), contentRectangle);
        }
        if (element == QStyle::SE_ItemViewItemText) {
            QRect textRectangle = QProxyStyle::subElementRect(
                element, option, widget);
            const QRect indicatorRectangle = subElementRect(
                QStyle::SE_ItemViewItemCheckIndicator, option, widget);
            const QTreeView *treeView = qobject_cast<const QTreeView *>(widget);
            if (!treeView && widget)
                treeView = qobject_cast<const QTreeView *>(widget->parentWidget());
            const int indicatorSpacing = DesignTokens::spacing(
                treeView && treeView->property("_ngstdCompactItems").toBool()
                    ? 1 : 2);
            if (itemOption->direction == Qt::LeftToRight) {
                textRectangle.setLeft(qMax(
                    textRectangle.left(),
                    indicatorRectangle.right() + 1 + indicatorSpacing));
            }
            else {
                textRectangle.setRight(qMin(
                    textRectangle.right(),
                    indicatorRectangle.left() - 1 - indicatorSpacing));
            }
            return textRectangle;
        }
    }
    if (option && element == QStyle::SE_LineEditContents) {
        if (widget &&
            (qobject_cast<const QAbstractSpinBox *>(widget->parentWidget()) ||
             qobject_cast<const QComboBox *>(widget->parentWidget()))) {
            return option->rect;
        }
        const int frameWidth = pixelMetric(QStyle::PM_DefaultFrameWidth,
                                           option, widget);
        const int horizontalPadding = DesignTokens::spacing(3);
        return option->rect.adjusted(frameWidth + horizontalPadding,
                                     frameWidth,
                                     -frameWidth - horizontalPadding,
                                     -frameWidth);
    }
    return QProxyStyle::subElementRect(element, option, widget);
}

QRect NextgisProxyStyle::subControlRect(ComplexControl control,
                                        const QStyleOptionComplex *option,
                                        SubControl subControl,
                                        const QWidget *widget) const
{
    if (!option) return QRect();
    if (control == QStyle::CC_ComboBox) {
        const int dropDownWidth = DesignTokens::componentMetric(
            ComponentMetric::ComboBoxDropDownWidth);
        const QRect logicalDropDown(option->rect.right() - dropDownWidth + 1,
                                    option->rect.top(), dropDownWidth,
                                    option->rect.height());
        const QRect dropDown = visualRect(option->direction, option->rect,
                                          logicalDropDown);
        if (subControl == QStyle::SC_ComboBoxArrow) return dropDown;
        if (subControl == QStyle::SC_ComboBoxEditField) {
            const int padding = DesignTokens::spacing(3);
            const QRect logicalContents(
                option->rect.left() + padding, option->rect.top() + 1,
                qMax(0, option->rect.width() - dropDownWidth - padding * 2),
                qMax(0, option->rect.height() - 2));
            return visualRect(option->direction, option->rect,
                              logicalContents);
        }
    }
    if (control == QStyle::CC_SpinBox) {
        const int buttonWidth = DesignTokens::componentMetric(
            ComponentMetric::SpinBoxButtonWidth);
        const int inset = DesignTokens::componentMetric(
            ComponentMetric::SpinBoxButtonInset);
        const int availableHeight = qMax(0, option->rect.height() - inset * 2);
        const int tokenButtonHeight = internal::tokenInteger(
            QStringLiteral("desktop.component.spinBox.buttonHeightPx"));
        const int buttonHeight =
            qMin(tokenButtonHeight, availableHeight / 2);
        const int buttonsHeight = buttonHeight * 2;
        const QRect logicalButtons(option->rect.right() - buttonWidth - inset +
                                       1,
                                   option->rect.top() +
                                       (option->rect.height() -
                                        buttonsHeight) /
                                           2,
                                   buttonWidth, buttonsHeight);
        const QRect buttons =
            visualRect(option->direction, option->rect, logicalButtons);
        if (subControl == QStyle::SC_SpinBoxUp)
            return QRect(buttons.left(), buttons.top(), buttons.width(),
                         buttonHeight);
        if (subControl == QStyle::SC_SpinBoxDown)
            return QRect(buttons.left(), buttons.top() + buttonHeight,
                         buttons.width(), buttonHeight);
        if (subControl == QStyle::SC_SpinBoxEditField) {
            const int padding = DesignTokens::spacing(3);
            const QRect logicalContents(
                option->rect.left() + padding, option->rect.top() + 1,
                qMax(0, option->rect.width() - buttonWidth - inset -
                            padding * 2),
                qMax(0, option->rect.height() - 2));
            return visualRect(option->direction, option->rect,
                              logicalContents);
        }
        if (subControl == QStyle::SC_SpinBoxFrame) return option->rect;
    }
    return QProxyStyle::subControlRect(control, option, subControl, widget);
}

QSize NextgisProxyStyle::sizeFromContents(ContentsType type,
                                          const QStyleOption *option,
                                          const QSize &contentsSize,
                                          const QWidget *widget) const
{
    QSize result =
        QProxyStyle::sizeFromContents(type, option, contentsSize, widget);
    switch (type) {
    case QStyle::CT_PushButton:
        if (!qobject_cast<const Button *>(widget)) {
            const QStyleOptionButton *buttonOption =
                qstyleoption_cast<const QStyleOptionButton *>(option);
            if (buttonOption && !buttonOption->icon.isNull() &&
                !buttonOption->text.isEmpty()) {
                result.rwidth() += internal::buttonIconTextGap() -
                                   internal::nativeButtonIconTextGap();
            }
        }
        result.setHeight(qMax(result.height(), DesignTokens::controlHeight(
                                                   ControlSize::Medium)));
        break;
    case QStyle::CT_LineEdit:
        result.setHeight(DesignTokens::controlHeight(ControlSize::Medium));
        break;
    case QStyle::CT_ComboBox:
    {
        const int textSafety = 4;
        result.setWidth(qMax(
            result.width(),
            contentsSize.width() +
                DesignTokens::componentMetric(
                    ComponentMetric::ComboBoxDropDownWidth) +
                DesignTokens::spacing(3) * 2 + textSafety));
        result.setHeight(DesignTokens::controlHeight(ControlSize::Medium));
        break;
    }
    case QStyle::CT_SpinBox:
        result.setHeight(DesignTokens::controlHeight(ControlSize::Medium));
        break;
    case QStyle::CT_TabBarTab:
    {
        const QStyleOptionTab *tabOption =
            qstyleoption_cast<const QStyleOptionTab *>(option);
        if (tabOption) {
            const QFont controlFont =
                DesignTokens::font(TypographyRole::Control);
            const QFont bodyFont =
                DesignTokens::font(TypographyRole::BodySmall);
            const int textWidth = qMax(
                QFontMetrics(controlFont)
                    .boundingRect(tabOption->text)
                    .width(),
                QFontMetrics(bodyFont)
                    .boundingRect(tabOption->text)
                    .width());
            const int horizontalPadding = DesignTokens::componentMetric(
                ComponentMetric::TabPaddingHorizontal);
            result.setWidth(qMax(result.width(),
                                 textWidth + horizontalPadding * 2 + 4));
            const int verticalPadding = DesignTokens::componentMetric(
                ComponentMetric::TabPaddingVertical);
            const int textHeight = qMax(QFontMetrics(controlFont).height(),
                                        QFontMetrics(bodyFont).height());
            result.setHeight(qMax(
                result.height(), textHeight + verticalPadding * 2 +
                                     DesignTokens::componentMetric(
                                         ComponentMetric::TabUnderlineHeight)));
        }
        break;
    }
    case QStyle::CT_CheckBox:
    case QStyle::CT_RadioButton:
        result.setHeight(qMax(
            result.height(), DesignTokens::componentMetric(
                                 ComponentMetric::SelectionIndicatorSize)));
        break;
    case QStyle::CT_ItemViewItem:
        result.setHeight(qMax(
            result.height(), DesignTokens::componentMetric(
                widget && widget->property("_ngstdCompactItems").toBool()
                    ? ComponentMetric::CompactItemViewRowHeight
                    : ComponentMetric::ItemViewRowHeight)));
        break;
    default:
        break;
    }
    return result;
}

void NextgisProxyStyle::drawPrimitive(PrimitiveElement element,
                                      const QStyleOption *option,
                                      QPainter *painter,
                                      const QWidget *widget) const
{
    if (element == QStyle::PE_PanelItemViewRow) {
        const QTreeView *treeView = qobject_cast<const QTreeView *>(widget);
        if (!treeView && widget) {
            treeView =
                qobject_cast<const QTreeView *>(widget->parentWidget());
        }
        if (treeView) return;
    }
    if (element == QStyle::PE_FrameFocusRect) {
        const QAbstractItemView *itemView =
            qobject_cast<const QAbstractItemView *>(widget);
        if (!itemView && widget) {
            itemView = qobject_cast<const QAbstractItemView *>(
                widget->parentWidget());
        }
        if (itemView &&
            itemView->property("_ngstdComboBoxPopupView").toBool()) {
            return;
        }
        if (paintFocusFrame(option, painter, widget)) return;
    }
    if (element == QStyle::PE_IndicatorCheckBox ||
        element == QStyle::PE_IndicatorRadioButton) {
        paintSelectionIndicator(
            element, option, painter, widget,
            d->state<SelectionAnimationState>(widget));
        return;
    }
    if (element == QStyle::PE_IndicatorBranch) {
        if (!option || !(option->state & QStyle::State_Children)) return;
        const int arrowSize = qMin(
            10, qMax(0, qMin(option->rect.width(), option->rect.height()) - 4));
        QRect arrowRectangle = QStyle::alignedRect(
            option->direction, Qt::AlignCenter, QSize(arrowSize, arrowSize),
            option->rect);
        const int inset = qMin(2, qMax(0, option->rect.width() / 8));
        arrowRectangle.translate(option->direction == Qt::RightToLeft
                                     ? -inset
                                     : inset,
                                 0);
        const bool open = option->state & QStyle::State_Open;
        const qreal rotation =
            open ? 0.0
                 : (option->direction == Qt::RightToLeft ? 90.0 : -90.0);
        const ColorScheme scheme = internal::colorSchemeFor(widget);
        const QColor color = DesignTokens::color(
            option->state & QStyle::State_Enabled ? ColorRole::TextSecondary
                                                  : ColorRole::TextDisabled,
            scheme);
        paintArrow(painter, arrowRectangle, color, false, rotation);
        return;
    }
    if (element == QStyle::PE_PanelLineEdit) {
        if (widget &&
            (qobject_cast<const QAbstractSpinBox *>(widget->parentWidget()) ||
             qobject_cast<const QComboBox *>(widget->parentWidget()))) {
            return;
        }
        paintFieldPanel(option, painter, widget,
                        d->state<FieldAnimationState>(widget));
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void NextgisProxyStyle::drawComplexControl(
    ComplexControl control, const QStyleOptionComplex *option,
    QPainter *painter, const QWidget *widget) const
{
    if (control == QStyle::CC_ComboBox) {
        const QStyleOptionComboBox *comboOption =
            qstyleoption_cast<const QStyleOptionComboBox *>(option);
        if (!comboOption) {
            QProxyStyle::drawComplexControl(control, option, painter, widget);
            return;
        }
        paintFieldPanel(comboOption, painter, widget,
                        d->state<FieldAnimationState>(widget));
        if (comboOption->subControls & QStyle::SC_ComboBoxArrow) {
            const QRect dropDown = subControlRect(
                control, comboOption, QStyle::SC_ComboBoxArrow, widget);
            const int arrowSize = DesignTokens::componentMetric(
                ComponentMetric::ComboBoxArrowSize);
            const QRect arrowRectangle = QStyle::alignedRect(
                comboOption->direction, Qt::AlignCenter,
                QSize(arrowSize, arrowSize), dropDown);
            const ColorScheme scheme = internal::colorSchemeFor(widget);
            const QColor color = DesignTokens::color(
                comboOption->state & QStyle::State_Enabled
                    ? ColorRole::Text
                    : ColorRole::TextDisabled,
                scheme);
            qreal progress = comboOption->state & QStyle::State_On ? 1.0 : 0.0;
            if (widget) {
                const QVariant property =
                    widget->property(comboArrowProgressProperty);
                if (property.isValid()) progress = property.toReal();
            }
            paintArrow(painter, arrowRectangle, color, false,
                       qBound(0.0, progress, 1.0) * 180.0);
        }
        return;
    }
    if (control == QStyle::CC_SpinBox) {
        const QStyleOptionSpinBox *spinOption =
            qstyleoption_cast<const QStyleOptionSpinBox *>(option);
        if (!spinOption) {
            QProxyStyle::drawComplexControl(control, option, painter, widget);
            return;
        }
        paintFieldPanel(spinOption, painter, widget,
                        d->state<FieldAnimationState>(widget));
        const ColorScheme scheme = internal::colorSchemeFor(widget);
        const int arrowSize = DesignTokens::componentMetric(
            ComponentMetric::SpinBoxArrowSize);
        const bool enabled = spinOption->state & QStyle::State_Enabled;
        const auto drawSpinArrow = [&](QStyle::SubControl subControl,
                                       bool pointsUp, bool stepEnabled) {
            if (!(spinOption->subControls & subControl)) return;
            const QRect buttonRectangle =
                subControlRect(control, spinOption, subControl, widget);
            if (spinOption->activeSubControls & subControl &&
                spinOption->state & QStyle::State_MouseOver) {
                painter->save();
                QPainterPath fieldClip;
                const QRectF fieldBounds = QRectF(spinOption->rect).adjusted(
                    1.0, 1.0, -1.0, -1.0);
                const qreal radius = qMax(
                    0.0, DesignTokens::radius(RadiusRole::Field) - 1.0);
                fieldClip.addRoundedRect(fieldBounds, radius, radius);
                painter->setClipPath(fieldClip);
                painter->setPen(Qt::NoPen);
                painter->setBrush(DesignTokens::color(
                    ColorRole::SurfaceBrand, scheme));
                painter->drawRect(buttonRectangle.adjusted(0, 0, -1, 0));
                painter->restore();
            }
            const QRect arrowRectangle = QStyle::alignedRect(
                spinOption->direction, Qt::AlignCenter,
                QSize(arrowSize, arrowSize), buttonRectangle);
            const QColor color = DesignTokens::color(
                enabled && stepEnabled ? ColorRole::TextSecondary
                                       : ColorRole::TextDisabled,
                scheme);
            paintArrow(painter, arrowRectangle, color, pointsUp);
        };
        drawSpinArrow(
            QStyle::SC_SpinBoxUp, true,
            spinOption->stepEnabled & QAbstractSpinBox::StepUpEnabled);
        drawSpinArrow(
            QStyle::SC_SpinBoxDown, false,
            spinOption->stepEnabled & QAbstractSpinBox::StepDownEnabled);
        return;
    }
    QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void NextgisProxyStyle::drawControl(ControlElement element,
                                    const QStyleOption *option,
                                    QPainter *painter,
                                    const QWidget *widget) const
{
    if (element == QStyle::CE_FocusFrame &&
        paintFocusFrame(option, painter, widget)) {
        return;
    }
    if (element == QStyle::CE_PushButtonLabel) {
        const QStyleOptionButton *buttonOption =
            qstyleoption_cast<const QStyleOptionButton *>(option);
        if (buttonOption) {
            QStyleOptionButton styledOption(*buttonOption);
            const QColor textColor =
                semanticButtonTextColor(&styledOption, widget);
            styledOption.palette.setColor(QPalette::ButtonText, textColor);
            styledOption.palette.setColor(QPalette::WindowText, textColor);
            styledOption.palette.setColor(QPalette::Text, textColor);
            if (widget) {
                const QVariant iconRole = widget->property(iconRoleProperty);
                if (iconRole.isValid()) {
                    const int iconSize = DesignTokens::controlIconSize();
                    styledOption.icon = iconPixmap(
                        static_cast<IconRole>(iconRole.toInt()),
                        QSize(iconSize, iconSize), widget->devicePixelRatioF(),
                        textColor);
                    styledOption.iconSize = QSize(iconSize, iconSize);
                }
            }
            internal::drawButtonLabel(styledOption, painter, widget);
            return;
        }
    }
    if (element == QStyle::CE_ItemViewItem) {
        const QStyleOptionViewItem *itemOption =
            qstyleoption_cast<const QStyleOptionViewItem *>(option);
        const QTreeView *treeView = qobject_cast<const QTreeView *>(widget);
        if (!treeView && widget)
            treeView = qobject_cast<const QTreeView *>(widget->parentWidget());
        if (itemOption && treeView) {
            QStyleOptionViewItem styledOption(*itemOption);
            const bool selected =
                styledOption.state & QStyle::State_Selected;
            const bool hovered =
                styledOption.state & QStyle::State_MouseOver;
            const ColorScheme scheme = internal::colorSchemeFor(treeView);
            if (selected || hovered) {
                QRectF rowRectangle(styledOption.rect);
                rowRectangle.setLeft(treeView->viewport()->rect().left() +
                                     1.0);
                rowRectangle.setRight(treeView->viewport()->rect().right() -
                                      1.0);
                rowRectangle.adjust(0.0, 1.0, 0.0, -1.0);
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setPen(Qt::NoPen);
                painter->setBrush(DesignTokens::color(
                    selected ? ColorRole::BrandSoft
                             : ColorRole::SurfaceBrand,
                    scheme));
                const qreal radius =
                    DesignTokens::radius(RadiusRole::Field);
                painter->drawRoundedRect(rowRectangle, radius, radius);
                painter->restore();
            }
            const QColor textColor =
                DesignTokens::color(ColorRole::Text, scheme);
            styledOption.palette.setColor(QPalette::Text, textColor);
            styledOption.palette.setColor(QPalette::HighlightedText,
                                          textColor);
            styledOption.state &= ~(QStyle::State_Selected |
                                    QStyle::State_MouseOver |
                                    QStyle::State_HasFocus);
            QProxyStyle::drawControl(element, &styledOption, painter, widget);
            if ((selected || hovered) && styledOption.index.isValid() &&
                styledOption.index.model()->hasChildren(styledOption.index)) {
                const int indentation = DesignTokens::componentMetric(
                    ComponentMetric::ItemViewIndentation);
                const int branchLeft =
                    styledOption.direction == Qt::RightToLeft
                        ? styledOption.rect.right() + 1
                        : styledOption.rect.left() - indentation;
                const QRect branchRectangle(branchLeft,
                                            styledOption.rect.top(),
                                            indentation,
                                            styledOption.rect.height());
                const int arrowSize = qMin(
                    10, qMax(0, qMin(branchRectangle.width(),
                                    branchRectangle.height()) -
                                    4));
                QRect arrowRectangle = QStyle::alignedRect(
                    styledOption.direction, Qt::AlignCenter,
                    QSize(arrowSize, arrowSize), branchRectangle);
                const int inset =
                    qMin(2, qMax(0, branchRectangle.width() / 8));
                arrowRectangle.translate(
                    styledOption.direction == Qt::RightToLeft ? -inset
                                                               : inset,
                    0);
                const bool open = treeView->isExpanded(styledOption.index);
                const qreal rotation =
                    open ? 0.0
                         : (styledOption.direction == Qt::RightToLeft ? 90.0
                                                                      : -90.0);
                paintArrow(painter, arrowRectangle, textColor, false,
                           rotation);
            }
            return;
        }
        const QListView *listView = qobject_cast<const QListView *>(widget);
        if (!listView && widget)
            listView = qobject_cast<const QListView *>(widget->parentWidget());
        if (itemOption && listView &&
            (listView->property("_ngstdStrongSelection").toBool() ||
             listView->property("_ngstdComboBoxPopupView").toBool())) {
            QStyleOptionViewItem styledOption(*itemOption);
            const bool selected =
                styledOption.state & QStyle::State_Selected;
            const bool hovered = styledOption.state & QStyle::State_MouseOver;
            const ColorScheme scheme = internal::colorSchemeFor(listView);
            if (selected || hovered) {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setPen(Qt::NoPen);
                painter->setBrush(DesignTokens::color(
                    ColorRole::SurfaceBrand, scheme));
                painter->drawRoundedRect(
                    QRectF(styledOption.rect).adjusted(1.0, 1.0, -1.0, -1.0),
                    DesignTokens::radius(RadiusRole::Field),
                    DesignTokens::radius(RadiusRole::Field));
                painter->restore();
            }
            const QColor textColor = DesignTokens::color(
                selected ? ColorRole::Text : ColorRole::TextSecondary,
                scheme);
            styledOption.palette.setColor(QPalette::Text, textColor);
            styledOption.palette.setColor(QPalette::HighlightedText,
                                          textColor);
            styledOption.font = DesignTokens::font(TypographyRole::BodySmall);
            styledOption.font.setWeight(selected ? QFont::DemiBold
                                                 : QFont::Normal);
            styledOption.state &= ~(QStyle::State_Selected |
                                    QStyle::State_MouseOver |
                                    QStyle::State_HasFocus);
            QProxyStyle::drawControl(element, &styledOption, painter, widget);
            return;
        }
    }
    if (element == QStyle::CE_TabBarTab) {
        drawControl(QStyle::CE_TabBarTabShape, option, painter, widget);
        drawControl(QStyle::CE_TabBarTabLabel, option, painter, widget);
        return;
    }
    if (element == QStyle::CE_TabBarTabShape) {
        const QStyleOptionTab *tabOption =
            qstyleoption_cast<const QStyleOptionTab *>(option);
        const QTabBar *tabBar = qobject_cast<const QTabBar *>(widget);
        if (!tabOption || !tabBar) return;
        const int index = tabIndexForRectangle(tabBar, tabOption->rect);
        const TabAnimationState *animationState =
            d->state<TabAnimationState>(widget);
        const int targetIndex =
            animationState ? animationState->targetIndex()
                           : tabBar->currentIndex();
        const int previousIndex =
            animationState ? animationState->previousIndex() : targetIndex;
        const qreal progress =
            animationState ? qBound(0.0, animationState->progress(), 1.0)
                           : 1.0;
        const qreal idleScale = internal::tokenNumber(
            QStringLiteral("desktop.component.tab.idleScale"));
        qreal scale = 0.0;
        qreal opacity = 0.0;
        if (index == previousIndex && previousIndex != targetIndex) {
            scale = 1.0 - (1.0 - idleScale) * progress;
            opacity = 1.0 - progress;
        }
        if (index == targetIndex) {
            scale = idleScale + (1.0 - idleScale) * progress;
            opacity = qMax(opacity, progress);
        }
        if (opacity <= 0.0) return;

        const qreal padding = DesignTokens::componentMetric(
            ComponentMetric::TabPaddingHorizontal);
        const qreal height = DesignTokens::componentMetric(
            ComponentMetric::TabUnderlineHeight);
        const qreal availableWidth =
            qMax(0.0, tabOption->rect.width() - padding * 2.0);
        const qreal width = availableWidth * scale;
        const QRectF underline(tabOption->rect.center().x() - width * 0.5,
                               tabOption->rect.bottom() - height + 1.0, width,
                               height);
        QColor color = DesignTokens::color(
            ColorRole::Brand, internal::colorSchemeFor(widget));
        color.setAlphaF(static_cast<float>(color.alphaF() * opacity));
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(color);
        painter->drawRoundedRect(underline, height * 0.5, height * 0.5);
        painter->restore();
        return;
    }
    if (element == QStyle::CE_TabBarTabLabel) {
        const QStyleOptionTab *tabOption =
            qstyleoption_cast<const QStyleOptionTab *>(option);
        if (tabOption) {
            QStyleOptionTab styledOption(*tabOption);
            const ColorScheme scheme = internal::colorSchemeFor(widget);
            const ColorRole textRole =
                !(styledOption.state & QStyle::State_Enabled)
                    ? ColorRole::TextDisabled
                    : (styledOption.state & QStyle::State_Selected)
                          ? ColorRole::Text
                          : (styledOption.state & QStyle::State_MouseOver)
                                ? ColorRole::LinkHover
                                : ColorRole::TextMuted;
            const QColor textColor = DesignTokens::color(textRole, scheme);
            const QFont tabFont = DesignTokens::font(
                styledOption.state & QStyle::State_Selected
                    ? TypographyRole::Control
                    : TypographyRole::BodySmall);
            styledOption.fontMetrics = QFontMetrics(tabFont);
            styledOption.palette.setColor(QPalette::WindowText, textColor);
            styledOption.palette.setColor(QPalette::ButtonText, textColor);
            styledOption.palette.setColor(QPalette::Text, textColor);
            painter->save();
            painter->setFont(tabFont);
            QProxyStyle::drawControl(element, &styledOption, painter, widget);
            painter->restore();
            return;
        }
    }
    QProxyStyle::drawControl(element, option, painter, widget);
}

} // namespace widgets
} // namespace ngstd
