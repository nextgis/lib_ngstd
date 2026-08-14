/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Basic field and feedback components
 *****************************************************************************/
#include <ngstd/widgets/basic_components.h>

#include "component_utils_p.h"
#include "design_tokens_p.h"
#include "motion_controller_p.h"

#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/icons.h>
#include <ngstd/widgets/widget_style.h>

#include <QAction>
#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QSizePolicy>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
#include <QVariant>

namespace ngstd {
namespace widgets {

class IconLabelPrivate final
{
public:
    IconRole iconRole = IconRole::Information;
    ColorRole colorRole = ColorRole::TextSecondary;
    int iconSize = 0;
    bool hasIconRole = false;
};

IconLabel::IconLabel(QWidget *parent)
    : QLabel(parent), d(new IconLabelPrivate)
{
    setAlignment(Qt::AlignCenter);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    updateIcon();
}

IconLabel::IconLabel(IconRole iconRole, QWidget *parent)
    : IconLabel(parent)
{
    setIconRole(iconRole);
}

IconLabel::~IconLabel() = default;

IconRole IconLabel::iconRole() const
{
    return d->iconRole;
}

bool IconLabel::hasIconRole() const
{
    return d->hasIconRole;
}

void IconLabel::setIconRole(IconRole iconRole)
{
    if (d->hasIconRole && d->iconRole == iconRole) return;
    const bool wasVisible = d->hasIconRole;
    d->iconRole = iconRole;
    d->hasIconRole = true;
    updateIcon();
    emit iconRoleChanged(d->iconRole);
    if (!wasVisible) emit iconVisibilityChanged(true);
}

void IconLabel::clearIconRole()
{
    if (!d->hasIconRole) return;
    d->hasIconRole = false;
    updateIcon();
    emit iconRoleChanged(d->iconRole);
    emit iconVisibilityChanged(false);
}

ColorRole IconLabel::colorRole() const
{
    return d->colorRole;
}

void IconLabel::setColorRole(ColorRole colorRole)
{
    if (d->colorRole == colorRole) return;
    d->colorRole = colorRole;
    updateIcon();
    emit colorRoleChanged(d->colorRole);
}

int IconLabel::iconSize() const
{
    return d->iconSize > 0 ? d->iconSize : DesignTokens::controlIconSize();
}

void IconLabel::setIconSize(int iconSize)
{
    const int normalizedSize = qMax(1, iconSize);
    if (d->iconSize == normalizedSize) return;
    d->iconSize = normalizedSize;
    updateIcon();
    emit iconSizeChanged(d->iconSize);
}

void IconLabel::resetIconSize()
{
    if (d->iconSize == 0) return;
    d->iconSize = 0;
    updateIcon();
    emit iconSizeChanged(iconSize());
}

QSize IconLabel::sizeHint() const
{
    const int size = iconSize();
    return QSize(size, size);
}

QSize IconLabel::minimumSizeHint() const
{
    return sizeHint();
}

void IconLabel::changeEvent(QEvent *event)
{
    QLabel::changeEvent(event);
    if (event->type() == QEvent::PaletteChange ||
        event->type() == QEvent::StyleChange ||
        event->type() == QEvent::EnabledChange ||
        event->type() == QEvent::DevicePixelRatioChange) {
        updateIcon();
    }
}

void IconLabel::updateIcon()
{
    if (!d->hasIconRole) {
        setPixmap(QPixmap());
        updateGeometry();
        return;
    }
    const int size = iconSize();
    const ColorRole role = isEnabled() ? d->colorRole : ColorRole::TextDisabled;
    const QColor color =
        DesignTokens::color(role, internal::colorSchemeFor(this));
    setPixmap(iconPixmap(d->iconRole, QSize(size, size),
                         devicePixelRatioF(), color));
    updateGeometry();
}

class SearchFieldPrivate final
{
public:
    QAction *searchAction = nullptr;
    QPixmap searchPixmap;
};

SearchField::SearchField(QWidget *parent)
    : QLineEdit(parent), d(new SearchFieldPrivate)
{
    setProperty("_ngstdRole", QStringLiteral("searchField"));
    setMinimumHeight(DesignTokens::controlHeight(ControlSize::Medium));
    setClearButtonEnabled(true);
    d->searchAction = addAction(QIcon(), QLineEdit::LeadingPosition);
    const QList<QToolButton *> actionButtons = findChildren<QToolButton *>();
    for (QToolButton *actionButton : actionButtons) {
        actionButton->setProperty("_ngstdRole",
                                  QStringLiteral("fieldAction"));
        actionButton->setCursor(Qt::IBeamCursor);
    }
    connect(d->searchAction, &QAction::triggered, this,
            [this]() { setFocus(Qt::MouseFocusReason); });
    updateSearchIcon();
}

SearchField::~SearchField() = default;

void SearchField::changeEvent(QEvent *event)
{
    QLineEdit::changeEvent(event);
    if (event->type() == QEvent::PaletteChange ||
        event->type() == QEvent::StyleChange ||
        event->type() == QEvent::EnabledChange ||
        event->type() == QEvent::LayoutDirectionChange) {
        updateSearchIcon();
    }
}

void SearchField::paintEvent(QPaintEvent *event)
{
    QLineEdit::paintEvent(event);
    if (d->searchPixmap.isNull()) return;
    const int iconOffset =
        DesignTokens::componentMetric(ComponentMetric::SearchFieldIconOffset);
    const int iconSize =
        DesignTokens::componentMetric(ComponentMetric::SearchFieldIconSize);
    const QRect logicalIconRect(iconOffset, (height() - iconSize) / 2,
                                iconSize, iconSize);
    const QRect iconRect =
        style()->visualRect(layoutDirection(), rect(), logicalIconRect);
    QPainter painter(this);
    painter.drawPixmap(iconRect, d->searchPixmap);
}

void SearchField::updateSearchIcon()
{
    if (!d->searchAction) return;
    const QColor color = DesignTokens::color(
        isEnabled() ? ColorRole::TextMuted : ColorRole::TextDisabled,
        internal::colorSchemeFor(this));
    const int iconSize =
        DesignTokens::componentMetric(ComponentMetric::SearchFieldIconSize);
    const qreal devicePixelRatio = devicePixelRatioF();
    d->searchPixmap = iconPixmap(IconRole::Search, QSize(iconSize, iconSize),
                                 devicePixelRatio, color);
    QPixmap spacer(QSize(qRound(iconSize * devicePixelRatio),
                         qRound(iconSize * devicePixelRatio)));
    spacer.setDevicePixelRatio(devicePixelRatio);
    spacer.fill(Qt::transparent);
    d->searchAction->setIcon(QIcon(spacer));
    update();
}

class ToastPrivate final
{
public:
    QLabel *label = nullptr;
    QGraphicsOpacityEffect *opacityEffect = nullptr;
    QPropertyAnimation *opacityAnimation = nullptr;
    QPropertyAnimation *positionAnimation = nullptr;
    QTimer *hideTimer = nullptr;
};

Toast::Toast(QWidget *parent) : QFrame(parent), d(new ToastPrivate)
{
    d->label = new QLabel(this);
    d->opacityEffect = new QGraphicsOpacityEffect(this);
    d->opacityAnimation =
        new QPropertyAnimation(d->opacityEffect, "opacity", this);
    d->positionAnimation = new QPropertyAnimation(this, "pos", this);
    d->hideTimer = new QTimer(this);
    setProperty("_ngstdRole", QStringLiteral("toast"));
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setGraphicsEffect(d->opacityEffect);
    d->opacityEffect->setOpacity(0.0);
    QHBoxLayout *layout = new QHBoxLayout(this);
    const QString tokenPath = QStringLiteral("desktop.component.toast.");
    const int horizontalPadding = internal::tokenInteger(
        tokenPath + QStringLiteral("paddingHorizontalPx"));
    const int verticalPadding = internal::tokenInteger(
        tokenPath + QStringLiteral("paddingVerticalPx"));
    layout->setContentsMargins(horizontalPadding, verticalPadding,
                               horizontalPadding, verticalPadding);
    layout->addWidget(d->label);
    setMinimumWidth(
        internal::tokenInteger(tokenPath + QStringLiteral("minimumWidthPx")));
    d->label->setAlignment(Qt::AlignCenter);
    d->hideTimer->setSingleShot(true);
    connect(d->opacityAnimation, &QPropertyAnimation::finished, this,
            [this]() {
                if (d->opacityEffect->opacity() <= 0.001) hide();
            });
    connect(d->hideTimer, &QTimer::timeout, this, [this]() {
        internal::MotionController::configure(d->opacityAnimation, this,
                                              {MotionDuration::Normal,
                                               MotionEasing::Exit});
        internal::MotionController::configure(d->positionAnimation, this,
                                              {MotionDuration::Normal,
                                               MotionEasing::Exit});
        d->opacityAnimation->stop();
        d->positionAnimation->stop();
        d->opacityAnimation->setStartValue(d->opacityEffect->opacity());
        d->opacityAnimation->setEndValue(0.0);
        d->positionAnimation->setStartValue(pos());
        d->positionAnimation->setEndValue(pos() + QPoint(0, 8));
        d->opacityAnimation->start();
        d->positionAnimation->start();
    });
    if (parent) parent->installEventFilter(this);
    hide();
}

Toast::~Toast()
{
    if (parentWidget()) parentWidget()->removeEventFilter(this);
}

QString Toast::text() const
{
    return d->label->text();
}

void Toast::setText(const QString &text)
{
    if (d->label->text() == text) return;
    d->label->setText(text);
    adjustSize();
    updatePosition();
    emit textChanged(text);
}

void Toast::showMessage(const QString &text, int durationMs)
{
    setText(text);
    if (!parentWidget()) return;
    const int defaultDuration = internal::tokenInteger(
        QStringLiteral("desktop.component.toast.durationMs"));
    const int resolvedDuration =
        durationMs >= 0 ? durationMs : defaultDuration;
    const QPoint targetPosition = pos();
    move(targetPosition + QPoint(0, 8));
    show();
    raise();
    d->opacityAnimation->stop();
    d->positionAnimation->stop();
    internal::MotionController::configure(d->opacityAnimation, this,
                                          {MotionDuration::Normal,
                                           MotionEasing::Enter});
    internal::MotionController::configure(d->positionAnimation, this,
                                          {MotionDuration::Normal,
                                           MotionEasing::Enter});
    d->opacityAnimation->setStartValue(d->opacityEffect->opacity());
    d->opacityAnimation->setEndValue(1.0);
    d->positionAnimation->setStartValue(pos());
    d->positionAnimation->setEndValue(targetPosition);
    d->opacityAnimation->start();
    d->positionAnimation->start();
    d->hideTimer->start(resolvedDuration);
}

bool Toast::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parentWidget() &&
        (event->type() == QEvent::Resize || event->type() == QEvent::Show)) {
        updatePosition();
    }
    return QFrame::eventFilter(watched, event);
}

void Toast::updatePosition()
{
    QWidget *anchor = parentWidget();
    if (!anchor) return;
    const int bottomOffset = internal::tokenInteger(
        QStringLiteral("desktop.component.toast.bottomOffsetPx"));
    move((anchor->width() - width()) / 2,
         anchor->height() - height() - bottomOffset);
}

class TagPrivate final
{
public:
    SemanticTone tone = SemanticTone::Neutral;
    IconRole iconRole = IconRole::Flame;
    bool hasIconRole = false;
};

Tag::Tag(QWidget *parent) : QLabel(parent), d(new TagPrivate)
{
    initialize();
}

Tag::Tag(const QString &text, QWidget *parent)
    : QLabel(text, parent), d(new TagPrivate)
{
    initialize();
}

Tag::~Tag() = default;

SemanticTone Tag::tone() const
{
    return d->tone;
}

void Tag::setTone(SemanticTone tone)
{
    if (d->tone == tone) return;
    d->tone = tone;
    WidgetStyle::setTone(this, d->tone);
    update();
    emit toneChanged(d->tone);
}

void Tag::resetTone()
{
    setTone(SemanticTone::Neutral);
}

IconRole Tag::iconRole() const
{
    return d->iconRole;
}

bool Tag::isIconVisible() const
{
    return d->hasIconRole;
}

void Tag::setIconRole(IconRole role)
{
    if (d->hasIconRole && d->iconRole == role) return;
    d->iconRole = role;
    d->hasIconRole = true;
    updateIconMargins();
    emit iconRoleChanged(d->iconRole, true);
}

void Tag::clearIconRole()
{
    if (!d->hasIconRole) return;
    d->hasIconRole = false;
    updateIconMargins();
    emit iconRoleChanged(d->iconRole, false);
}

void Tag::changeEvent(QEvent *event)
{
    QLabel::changeEvent(event);
    if (event->type() == QEvent::PaletteChange ||
        event->type() == QEvent::StyleChange) {
        update();
    }
    else if (event->type() == QEvent::LayoutDirectionChange) {
        updateIconMargins();
    }
}

void Tag::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    if (!d->hasIconRole) return;
    const ColorScheme scheme = internal::colorSchemeFor(this);
    const QColor color = DesignTokens::color(
        isEnabled() ? internal::colorRoleForTone(d->tone)
                    : ColorRole::TextDisabled,
        scheme);
    const int iconSize = DesignTokens::controlIconSize();
    const int horizontalPadding =
        DesignTokens::componentMetric(ComponentMetric::TagPaddingHorizontal);
    const int x = layoutDirection() == Qt::RightToLeft
                      ? width() - horizontalPadding - iconSize
                      : horizontalPadding;
    QPainter painter(this);
    painter.drawPixmap(x, (height() - iconSize) / 2,
                       iconPixmap(d->iconRole, QSize(iconSize, iconSize),
                                  devicePixelRatioF(), color));
}

void Tag::initialize()
{
    setProperty("_ngstdRole", QStringLiteral("tag"));
    setIndent(0);
    setAlignment(Qt::AlignLeading | Qt::AlignVCenter);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
}

void Tag::updateIconMargins()
{
    const int margin = d->hasIconRole
                           ? DesignTokens::controlIconSize() +
                                 DesignTokens::componentMetric(
                                     ComponentMetric::TagContentSpacing)
                           : 0;
    if (layoutDirection() == Qt::RightToLeft)
        setContentsMargins(0, 0, margin, 0);
    else
        setContentsMargins(margin, 0, 0, 0);
    updateGeometry();
    update();
}

class NoticePrivate final
{
public:
    QLabel *iconLabel = nullptr;
    QLabel *textLabel = nullptr;
    QString title;
    QString text;
    SemanticTone tone = SemanticTone::Information;
};

Notice::Notice(QWidget *parent) : QFrame(parent), d(new NoticePrivate)
{
    d->iconLabel = new QLabel(this);
    d->textLabel = new QLabel(this);
    setProperty("_ngstdRole", QStringLiteral("notice"));
    setProperty("_ngstdPaintedNotice", true);
    setFrameShape(QFrame::NoFrame);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(
        DesignTokens::componentMetric(ComponentMetric::NoticeContentSpacing));
    layout->setAlignment(Qt::AlignVCenter);
    d->iconLabel->setFixedSize(DesignTokens::controlIconSize(),
                               DesignTokens::controlIconSize());
    d->iconLabel->setAlignment(Qt::AlignCenter);
    d->iconLabel->setAttribute(Qt::WA_StyledBackground, false);
    d->textLabel->setWordWrap(true);
    d->textLabel->setTextFormat(Qt::RichText);
    d->textLabel->setAttribute(Qt::WA_StyledBackground, false);
    layout->addWidget(d->iconLabel, 0, Qt::AlignVCenter);
    layout->addWidget(d->textLabel, 1);
    WidgetStyle::setTone(this, d->tone);
    updateIcon();
}

Notice::~Notice() = default;

QString Notice::title() const
{
    return d->title;
}

void Notice::setTitle(const QString &title)
{
    if (d->title == title) return;
    d->title = title;
    updateText();
    emit titleChanged(d->title);
}

QString Notice::text() const
{
    return d->text;
}

void Notice::setText(const QString &text)
{
    if (d->text == text) return;
    d->text = text;
    updateText();
    emit textChanged(d->text);
}

SemanticTone Notice::tone() const
{
    return d->tone;
}

void Notice::setTone(SemanticTone tone)
{
    if (d->tone == tone) return;
    d->tone = tone;
    WidgetStyle::setTone(this, d->tone);
    updateIcon();
    emit toneChanged(d->tone);
}

void Notice::resetTone()
{
    setTone(SemanticTone::Information);
}

void Notice::changeEvent(QEvent *event)
{
    QFrame::changeEvent(event);
    if (event->type() == QEvent::PaletteChange ||
        event->type() == QEvent::EnabledChange) {
        updateIcon();
        update();
    }
}

void Notice::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    const ColorScheme scheme = internal::colorSchemeFor(this);
    ColorRole backgroundRole = ColorRole::SurfaceBrand;
    if (!isEnabled())
        backgroundRole = ColorRole::SurfaceMuted;
    else if (d->tone == SemanticTone::Success)
        backgroundRole = ColorRole::SuccessSoft;
    else if (d->tone == SemanticTone::Warning)
        backgroundRole = ColorRole::WarningSoft;
    else if (d->tone == SemanticTone::Danger)
        backgroundRole = ColorRole::DangerSoft;

    constexpr qreal borderWidth = 1.0;
    const qreal radius = DesignTokens::radius(RadiusRole::Button);
    const QRectF bounds = QRectF(rect()).adjusted(
        borderWidth * 0.5, borderWidth * 0.5,
        -borderWidth * 0.5, -borderWidth * 0.5);
    QPainterPath shape;
    shape.addRoundedRect(bounds, radius, radius);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillPath(shape, DesignTokens::color(backgroundRole, scheme));
    painter.setPen(QPen(DesignTokens::color(ColorRole::Border, scheme),
                        borderWidth));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(shape);

    painter.save();
    painter.setClipPath(shape);
    painter.setPen(Qt::NoPen);
    painter.setBrush(DesignTokens::color(
        isEnabled() ? internal::colorRoleForTone(d->tone)
                    : ColorRole::TextDisabled,
        scheme));
    const qreal accentWidth = DesignTokens::componentMetric(
        ComponentMetric::NoticeAccentWidth);
    const QRectF accent(layoutDirection() == Qt::RightToLeft
                            ? rect().right() - accentWidth + 1.0
                            : rect().left(),
                        rect().top(), accentWidth, rect().height());
    painter.drawRect(accent);
    painter.restore();
}

void Notice::updateText()
{
    const QString titleText = d->title.toHtmlEscaped();
    const QString bodyText = d->text.toHtmlEscaped();
    d->textLabel->setText(
        titleText.isEmpty()
            ? bodyText
            : QStringLiteral("<b>%1</b> %2").arg(titleText, bodyText));
}

void Notice::updateIcon()
{
    const ColorScheme scheme = internal::colorSchemeFor(this);
    const QColor color =
        DesignTokens::color(internal::colorRoleForTone(d->tone), scheme);
    d->iconLabel->setPixmap(iconPixmap(internal::iconForTone(d->tone),
                                       QSize(DesignTokens::controlIconSize(),
                                             DesignTokens::controlIconSize()),
                                       devicePixelRatioF(), color));
}

} // namespace widgets
} // namespace ngstd
