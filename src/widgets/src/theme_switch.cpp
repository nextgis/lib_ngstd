/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Reusable animated theme switch
 *****************************************************************************/
#include <ngstd/widgets/theme_switch.h>

#include "component_utils_p.h"
#include "motion_controller_p.h"

#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/icons.h>

#include <QAbstractButton>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QPainter>
#include <QTimer>
#include <QToolButton>
#include <QVariant>
#include <QVariantAnimation>
#include <QtMath>

namespace ngstd {
namespace widgets {

namespace {

class ThemeSwitchButton final : public QToolButton
{
public:
    explicit ThemeSwitchButton(QWidget *parent) : QToolButton(parent) {}

    void setFeedbackProgress(qreal progress)
    {
        const qreal boundedProgress = qBound(0.0, progress, 1.0);
        if (qFuzzyCompare(m_feedbackProgress + 1.0,
                          boundedProgress + 1.0)) {
            return;
        }
        m_feedbackProgress = boundedProgress;
        update();
    }

    void setColorScheme(ColorScheme scheme)
    {
        if (m_colorScheme == scheme) return;
        m_colorScheme = scheme;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QToolButton::paintEvent(event);
        if (m_feedbackProgress >= 1.0) return;
        QColor feedbackColor =
            DesignTokens::color(ColorRole::FeedbackRing, m_colorScheme);
        const qreal padding = internal::tokenNumber(
            QStringLiteral("desktop.component.themeSwitch.paddingPx"));
        const qreal radius =
            qMax(0.0, DesignTokens::radius(RadiusRole::Card) - padding);
        QPainter painter(this);
        internal::paintSelectionFeedback(&painter, QRectF(rect()), radius,
                                         m_feedbackProgress, feedbackColor);
    }

private:
    ColorScheme m_colorScheme = ColorScheme::Light;
    qreal m_feedbackProgress = 1.0;
};

} // namespace

class ThemeSwitchPrivate final
{
public:
    QButtonGroup *buttonGroup = nullptr;
    QList<ThemeSwitchButton *> buttons;
    QVariantAnimation *animation = nullptr;
    ThemeMode themeMode = ThemeMode::System;
    ColorScheme colorScheme = ColorScheme::Light;
    qreal transitionProgress = 1.0;
    int previousIndex = 0;
    int targetIndex = 0;
    bool initialized = false;
};

ThemeSwitch::ThemeSwitch(QWidget *parent)
    : QFrame(parent), d(new ThemeSwitchPrivate)
{
    d->buttonGroup = new QButtonGroup(this);
    d->animation = new QVariantAnimation(this);
    d->animation->setObjectName(
        QStringLiteral("_ngstdThemeSwitchAnimation"));
    const int padding = internal::tokenInteger(
        QStringLiteral("desktop.component.themeSwitch.paddingPx"));
    const int buttonWidth = internal::tokenInteger(
        QStringLiteral("desktop.component.themeSwitch.buttonWidthPx"));
    const int buttonHeight = internal::tokenInteger(
        QStringLiteral("desktop.component.themeSwitch.buttonHeightPx"));
    const int iconSize = DesignTokens::controlIconSize();
    setProperty("_ngstdRole", QStringLiteral("themeSwitch"));
    setAttribute(Qt::WA_StyledBackground, true);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->setContentsMargins(padding, padding, padding, padding);
    layout->setSpacing(padding);
    d->buttonGroup->setExclusive(true);
    const QList<QPair<IconRole, QString>> definitions = {
        {IconRole::Monitor, tr("System theme")},
        {IconRole::Sun, tr("Light theme")},
        {IconRole::Moon, tr("Dark theme")},
    };
    for (int index = 0; index < definitions.size(); ++index) {
        ThemeSwitchButton *themeButton = new ThemeSwitchButton(this);
        themeButton->setProperty("_ngstdRole", QStringLiteral("themeButton"));
        themeButton->setProperty("_ngstdThemeIcon",
                                 static_cast<int>(definitions[index].first));
        themeButton->setToolTip(definitions[index].second);
        themeButton->setAccessibleName(definitions[index].second);
        themeButton->setCheckable(true);
        themeButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        themeButton->setFixedSize(buttonWidth, buttonHeight);
        themeButton->setIconSize(QSize(iconSize, iconSize));
        d->buttonGroup->addButton(themeButton, index);
        d->buttons.append(themeButton);
        layout->addWidget(themeButton);
        connect(themeButton, &QToolButton::pressed, this,
                [this]() { updateIcons(); });
        connect(themeButton, &QToolButton::released, this, [this]() {
            QTimer::singleShot(0, this, [this]() { updateIcons(); });
        });
    }
    d->buttonGroup->button(0)->setChecked(true);
    connect(d->animation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
                d->transitionProgress = value.toReal();
                if (ThemeSwitchButton *targetButton =
                        d->buttons.value(d->targetIndex)) {
                    targetButton->setFeedbackProgress(
                        d->transitionProgress);
                }
                updateIcons();
                update();
            });
    connect(d->animation, &QVariantAnimation::finished, this, [this]() {
        d->previousIndex = d->targetIndex;
        d->transitionProgress = 1.0;
        if (ThemeSwitchButton *targetButton =
                d->buttons.value(d->targetIndex)) {
            targetButton->setFeedbackProgress(1.0);
        }
        updateIcons();
        update();
    });
    connect(d->buttonGroup, &QButtonGroup::idClicked, this,
            [this](int index) { setThemeMode(modeForIndex(index)); });
    updateIcons();
}

ThemeSwitch::~ThemeSwitch() = default;

ThemeMode ThemeSwitch::themeMode() const
{
    return d->themeMode;
}

void ThemeSwitch::setThemeMode(ThemeMode mode)
{
    const int targetIndex = indexForMode(mode);
    if (d->initialized && d->themeMode == mode) return;
    const ThemeMode previousMode = d->themeMode;
    const int previousIndex = indexForMode(previousMode);
    d->themeMode = mode;
    if (QAbstractButton *targetButton = d->buttonGroup->button(targetIndex))
        targetButton->setChecked(true);
    d->animation->stop();
    for (ThemeSwitchButton *button : d->buttons)
        button->setFeedbackProgress(1.0);
    if (!d->initialized) {
        d->previousIndex = targetIndex;
        d->targetIndex = targetIndex;
        d->transitionProgress = 1.0;
        d->initialized = true;
    }
    else {
        d->previousIndex = previousIndex;
        d->targetIndex = targetIndex;
        d->transitionProgress = 0.0;
        if (ThemeSwitchButton *targetButton = d->buttons.value(targetIndex))
            targetButton->setFeedbackProgress(0.0);
        d->animation->setStartValue(0.0);
        d->animation->setEndValue(1.0);
        if (!internal::MotionController::configure(
                d->animation, this,
                {MotionDuration::Normal, MotionEasing::Standard})) {
            d->transitionProgress = 1.0;
            d->previousIndex = d->targetIndex;
        }
        else {
            d->animation->start();
        }
    }
    updateIcons();
    update();
    if (previousMode != d->themeMode) emit themeModeChanged(d->themeMode);
}

void ThemeSwitch::resetThemeMode()
{
    setThemeMode(ThemeMode::System);
}

ColorScheme ThemeSwitch::colorScheme() const
{
    return d->colorScheme;
}

void ThemeSwitch::setColorScheme(ColorScheme scheme)
{
    if (d->colorScheme == scheme) return;
    d->colorScheme = scheme;
    for (ThemeSwitchButton *button : d->buttons)
        button->setColorScheme(scheme);
    updateIcons();
    update();
    emit colorSchemeChanged(scheme);
}

void ThemeSwitch::resetColorScheme()
{
    setColorScheme(ColorScheme::Light);
}

QToolButton *ThemeSwitch::button(ThemeMode mode) const
{
    return qobject_cast<QToolButton *>(
        d->buttonGroup->button(indexForMode(mode)));
}

void ThemeSwitch::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
    const qreal padding = internal::tokenNumber(
        QStringLiteral("desktop.component.themeSwitch.paddingPx"));
    const qreal radius =
        qMax(0.0, DesignTokens::radius(RadiusRole::Card) - padding);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    const auto drawSelection = [this, &painter, radius](int index,
                                                        qreal opacity) {
        QAbstractButton *themeButton = d->buttonGroup->button(index);
        if (!themeButton || opacity <= 0.0) return;
        QColor color = DesignTokens::color(ColorRole::Brand, d->colorScheme);
        color.setAlphaF(static_cast<float>(color.alphaF() * opacity));
        painter.setBrush(color);
        painter.drawRoundedRect(QRectF(themeButton->geometry()), radius,
                                radius);
    };
    if (d->previousIndex != d->targetIndex)
        drawSelection(d->previousIndex, 1.0 - d->transitionProgress);
    drawSelection(d->targetIndex, d->transitionProgress);
}

int ThemeSwitch::indexForMode(ThemeMode mode)
{
    if (mode == ThemeMode::Light) return 1;
    if (mode == ThemeMode::Dark) return 2;
    return 0;
}

ThemeMode ThemeSwitch::modeForIndex(int index)
{
    if (index == 1) return ThemeMode::Light;
    if (index == 2) return ThemeMode::Dark;
    return ThemeMode::System;
}

qreal ThemeSwitch::selectionOpacity(int index) const
{
    if (d->previousIndex == d->targetIndex)
        return index == d->targetIndex ? 1.0 : 0.0;
    if (index == d->previousIndex) return 1.0 - d->transitionProgress;
    if (index == d->targetIndex) return d->transitionProgress;
    return 0.0;
}

void ThemeSwitch::updateIcons()
{
    const int iconSize = DesignTokens::controlIconSize();
    const QColor muted =
        DesignTokens::color(ColorRole::TextMuted, d->colorScheme);
    const QColor selected =
        DesignTokens::color(ColorRole::White, d->colorScheme);
    for (QAbstractButton *abstractButton : d->buttonGroup->buttons()) {
        QToolButton *themeButton = qobject_cast<QToolButton *>(abstractButton);
        if (!themeButton) continue;
        const IconRole role = static_cast<IconRole>(
            themeButton->property("_ngstdThemeIcon").toInt());
        const qreal opacity =
            themeButton->isDown()
                ? 1.0
                : selectionOpacity(d->buttonGroup->id(themeButton));
        const QColor color =
            internal::interpolateColor(muted, selected, opacity);
        themeButton->setIcon(QIcon(iconPixmap(role, QSize(iconSize, iconSize),
                                              devicePixelRatioF(), color)));
    }
}

} // namespace widgets
} // namespace ngstd
