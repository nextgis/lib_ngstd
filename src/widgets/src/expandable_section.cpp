/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Reusable expandable corporate section
 *****************************************************************************/
#include <ngstd/widgets/expandable_section.h>

#include "component_utils_p.h"
#include "motion_controller_p.h"

#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/disclosure.h>
#include <ngstd/widgets/icons.h>

#include <QAbstractButton>
#include <QCheckBox>
#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QStyle>
#include <QVariantAnimation>
#include <QVBoxLayout>

namespace ngstd {
namespace widgets {

namespace {

int sectionMetric(ComponentMetric metric)
{
    return DesignTokens::componentMetric(metric);
}

class ExpandableSectionHeader final : public QAbstractButton
{
public:
    explicit ExpandableSectionHeader(QWidget *parent)
        : QAbstractButton(parent), m_hoverAnimation(new QVariantAnimation(this))
    {
        setObjectName(QStringLiteral("_ngstdExpandableSectionHeader"));
        setProperty("_ngstdRole", QStringLiteral("expandableSectionHeader"));
        setAttribute(Qt::WA_Hover, true);
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::StrongFocus);
        setCheckable(true);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setFixedHeight(
            sectionMetric(ComponentMetric::ExpandableSectionHeaderHeight));
        m_selection = new QCheckBox(this);
        m_selection->setObjectName(
            QStringLiteral("_ngstdExpandableSectionSelection"));
        m_selection->setText(QString());
        m_selection->setCursor(Qt::PointingHandCursor);
        m_selection->setVisible(false);
        m_hoverAnimation->setObjectName(
            QStringLiteral("_ngstdExpandableSectionHoverAnimation"));
        connect(m_hoverAnimation, &QVariantAnimation::valueChanged, this,
                [this](const QVariant &value) {
                    m_hoverProgress = value.toReal();
                    update();
                });
    }

    QString title() const
    {
        return m_title;
    }

    void setTitle(const QString &title)
    {
        if (m_title == title) return;
        m_title = title;
        updateAccessibleName();
        update();
    }

    QString description() const
    {
        return m_description;
    }

    void setDescription(const QString &description)
    {
        if (m_description == description) return;
        m_description = description;
        updateAccessibleName();
        update();
    }

    IconRole iconRole() const
    {
        return m_iconRole;
    }

    void setIconRole(IconRole iconRole)
    {
        if (m_iconRole == iconRole) return;
        m_iconRole = iconRole;
        update();
    }

    QString iconText() const
    {
        return m_iconText;
    }

    void setIconText(const QString &iconText)
    {
        if (m_iconText == iconText) return;
        m_iconText = iconText;
        update();
    }

    qreal chevronProgress() const
    {
        return m_chevronProgress;
    }

    QCheckBox *selectionControl() const
    {
        return m_selection;
    }

    void setSelectionVisible(bool visible)
    {
        if (m_selection->isVisible() == visible) return;
        m_selection->setVisible(visible);
        updateSelectionGeometry();
        update();
    }

    void setChevronProgress(qreal progress)
    {
        const qreal boundedProgress = qBound(0.0, progress, 1.0);
        if (qFuzzyCompare(m_chevronProgress, boundedProgress)) return;
        m_chevronProgress = boundedProgress;
        update();
    }

protected:
    bool event(QEvent *event) override
    {
        if (isEnabled() &&
            (event->type() == QEvent::Enter ||
             event->type() == QEvent::Leave)) {
            const qreal target =
                event->type() == QEvent::Enter ? 1.0 : 0.0;
            m_hoverAnimation->stop();
            m_hoverAnimation->setStartValue(m_hoverProgress);
            m_hoverAnimation->setEndValue(target);
            if (internal::MotionController::configure(
                    m_hoverAnimation, this,
                    {MotionDuration::Fast, MotionEasing::Standard})) {
                m_hoverAnimation->start();
            }
            else {
                m_hoverProgress = target;
                update();
            }
        }
        return QAbstractButton::event(event);
    }

    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)
        const ColorScheme scheme = internal::colorSchemeFor(this);
        const qreal radius = DesignTokens::radius(RadiusRole::Panel);
        const QRectF fillBounds =
            QRectF(rect()).adjusted(1.0, 1.0, -1.0, 0.0);
        QPainterPath headerPath;
        headerPath.setFillRule(Qt::WindingFill);
        headerPath.addRoundedRect(fillBounds, qMax(0.0, radius - 1.0),
                                  qMax(0.0, radius - 1.0));
        if (isChecked()) {
            headerPath.addRect(
                QRectF(fillBounds.left(), fillBounds.bottom() - radius,
                       fillBounds.width(), radius + 1.0));
        }

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);
        const qreal stateProgress = isDown() ? 1.0 : m_hoverProgress;
        if (stateProgress > 0.0) {
            painter.save();
            painter.setOpacity(stateProgress);
            painter.fillPath(
                headerPath,
                DesignTokens::color(isDown() ? ColorRole::BrandSoft
                                             : ColorRole::SurfaceNested,
                                    scheme));
            painter.restore();
        }

        const int horizontalPadding = sectionMetric(
            ComponentMetric::ExpandableSectionHeaderPaddingHorizontal);
        const int contentSpacing = sectionMetric(
            ComponentMetric::ExpandableSectionHeaderContentSpacing);
        const int iconSize =
            sectionMetric(ComponentMetric::ExpandableSectionIconSize);
        const int glyphSize =
            sectionMetric(ComponentMetric::ExpandableSectionIconGlyphSize);
        const int chevronSize =
            sectionMetric(ComponentMetric::ExpandableSectionChevronSize);
        const int selectionExtent = m_selection->isVisible()
            ? sectionMetric(ComponentMetric::SelectionIndicatorSize) +
                  contentSpacing
            : 0;
        const QRect logicalIconRect(
            horizontalPadding + selectionExtent, (height() - iconSize) / 2,
            iconSize, iconSize);
        const QRect iconRect =
            style()->visualRect(layoutDirection(), rect(), logicalIconRect);
        painter.setPen(Qt::NoPen);
        painter.setBrush(DesignTokens::color(ColorRole::SurfaceBrand, scheme));
        painter.drawRoundedRect(QRectF(iconRect),
                                DesignTokens::radius(RadiusRole::Card),
                                DesignTokens::radius(RadiusRole::Card));
        const QColor iconColor = DesignTokens::color(
            isEnabled() ? ColorRole::Link : ColorRole::TextDisabled, scheme);
        if (m_iconText.isEmpty()) {
            const QPixmap pixmap = iconPixmap(
                m_iconRole, QSize(glyphSize, glyphSize), devicePixelRatioF(),
                iconColor);
            painter.drawPixmap(
                QPointF(iconRect.center().x() - glyphSize * 0.5,
                        iconRect.center().y() - glyphSize * 0.5),
                pixmap);
        }
        else {
            QFont iconFont = DesignTokens::font(TypographyRole::Heading2);
            iconFont.setPixelSize(glyphSize);
            painter.setFont(iconFont);
            painter.setPen(iconColor);
            painter.drawText(iconRect, Qt::AlignCenter, m_iconText);
        }

        const int textLeft = horizontalPadding + selectionExtent + iconSize +
            contentSpacing;
        const int textWidth = qMax(
            0, width() - textLeft - horizontalPadding - chevronSize -
                   contentSpacing);
        const int titleHeight = sectionMetric(
            ComponentMetric::ExpandableSectionTitleLineHeight);
        const int descriptionHeight = sectionMetric(
            ComponentMetric::ExpandableSectionDescriptionLineHeight);
        const int titleSpacing = sectionMetric(
            ComponentMetric::ExpandableSectionTitleDescriptionSpacing);
        const int textHeight = titleHeight + titleSpacing + descriptionHeight;
        const int textTop = (height() - textHeight) / 2;
        const QRect logicalTitleRect(textLeft, textTop, textWidth,
                                     titleHeight);
        const QRect logicalDescriptionRect(
            textLeft, textTop + titleHeight + titleSpacing, textWidth,
            descriptionHeight);
        const QRect titleRect =
            style()->visualRect(layoutDirection(), rect(), logicalTitleRect);
        const QRect descriptionRect = style()->visualRect(
            layoutDirection(), rect(), logicalDescriptionRect);
        const Qt::Alignment alignment = QStyle::visualAlignment(
            layoutDirection(), Qt::AlignLeft | Qt::AlignVCenter);
        painter.setFont(DesignTokens::font(TypographyRole::Heading4));
        painter.setPen(DesignTokens::color(
            isEnabled() ? ColorRole::Text : ColorRole::TextDisabled, scheme));
        painter.drawText(titleRect, alignment,
                         painter.fontMetrics().elidedText(
                             m_title, Qt::ElideRight, titleRect.width()));
        painter.setFont(DesignTokens::font(TypographyRole::BodySmall));
        painter.setPen(DesignTokens::color(
            isEnabled() ? ColorRole::TextMuted : ColorRole::TextDisabled,
            scheme));
        painter.drawText(descriptionRect, alignment,
                         painter.fontMetrics().elidedText(
                             m_description, Qt::ElideRight,
                             descriptionRect.width()));

        const QRect logicalChevronRect(
            width() - horizontalPadding - chevronSize,
            (height() - chevronSize) / 2, chevronSize, chevronSize);
        const QRect chevronRect = style()->visualRect(
            layoutDirection(), rect(), logicalChevronRect);
        painter.save();
        painter.translate(QRectF(chevronRect).center());
        painter.rotate(m_chevronProgress * 180.0);
        QPainterPath chevron;
        const qreal halfWidth = chevronSize * 0.25;
        const qreal halfHeight = chevronSize * 0.125;
        chevron.moveTo(-halfWidth, -halfHeight);
        chevron.lineTo(0.0, halfHeight);
        chevron.lineTo(halfWidth, -halfHeight);
        painter.setPen(QPen(
            DesignTokens::color(isEnabled() ? ColorRole::TextMuted
                                            : ColorRole::TextDisabled,
                                scheme),
            1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(chevron);
        painter.restore();
    }

    void resizeEvent(QResizeEvent *event) override
    {
        QAbstractButton::resizeEvent(event);
        updateSelectionGeometry();
    }

private:
    void updateAccessibleName()
    {
        setAccessibleName(m_description.isEmpty()
                              ? m_title
                              : QStringLiteral("%1. %2")
                                    .arg(m_title, m_description));
    }

    void updateSelectionGeometry()
    {
        if (!m_selection->isVisible()) return;
        const int size = sectionMetric(
            ComponentMetric::SelectionIndicatorSize);
        const int horizontalPadding = sectionMetric(
            ComponentMetric::ExpandableSectionHeaderPaddingHorizontal);
        const QRect logicalRectangle(horizontalPadding,
                                     (height() - size) / 2, size, size);
        m_selection->setGeometry(style()->visualRect(
            layoutDirection(), rect(), logicalRectangle));
    }

    QString m_title;
    QString m_description;
    QString m_iconText;
    IconRole m_iconRole = IconRole::Information;
    QCheckBox *m_selection = nullptr;
    QVariantAnimation *m_hoverAnimation = nullptr;
    qreal m_hoverProgress = 0.0;
    qreal m_chevronProgress = 0.0;
};

class ExpandableSectionBody final : public QWidget
{
public:
    explicit ExpandableSectionBody(QWidget *parent) : QWidget(parent)
    {
        setAttribute(Qt::WA_NoSystemBackground);
    }
};

} // namespace

class ExpandableSectionPrivate final
{
public:
    ExpandableSectionHeader *header = nullptr;
    RevealWidget *reveal = nullptr;
    ExpandableSectionBody *body = nullptr;
    QVBoxLayout *contentLayout = nullptr;
    QVariantAnimation *chevronAnimation = nullptr;
};

ExpandableSection::ExpandableSection(QWidget *parent)
    : ExpandableSection(QString(), QString(), IconRole::Information, parent)
{}

ExpandableSection::ExpandableSection(const QString &title,
                                     const QString &description,
                                     IconRole iconRole, QWidget *parent)
    : QFrame(parent), d(new ExpandableSectionPrivate)
{
    setProperty("_ngstdRole", QStringLiteral("expandableSection"));
    setAttribute(Qt::WA_StyledBackground, false);
    setFrameShape(QFrame::NoFrame);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QVBoxLayout *sectionLayout = new QVBoxLayout(this);
    sectionLayout->setContentsMargins(0, 0, 0, 0);
    sectionLayout->setSpacing(0);
    d->header = new ExpandableSectionHeader(this);
    d->header->setTitle(title);
    d->header->setDescription(description);
    d->header->setIconRole(iconRole);
    d->reveal = new RevealWidget(this);
    d->body = new ExpandableSectionBody(d->reveal);
    d->contentLayout = new QVBoxLayout(d->body);
    d->contentLayout->setContentsMargins(
        sectionMetric(
            ComponentMetric::ExpandableSectionContentPaddingHorizontal),
        0,
        sectionMetric(
            ComponentMetric::ExpandableSectionContentPaddingHorizontal),
        sectionMetric(
            ComponentMetric::ExpandableSectionContentPaddingBottom));
    d->contentLayout->setSpacing(0);
    d->reveal->setContentWidget(d->body);
    sectionLayout->addWidget(d->header);
    sectionLayout->addWidget(d->reveal);

    d->chevronAnimation = new QVariantAnimation(this);
    d->chevronAnimation->setObjectName(
        QStringLiteral("_ngstdExpandableSectionChevronAnimation"));
    connect(d->chevronAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
                d->header->setChevronProgress(value.toReal());
            });
    connect(d->header, &QAbstractButton::toggled, this,
            [this](bool expanded) {
                d->chevronAnimation->stop();
                d->chevronAnimation->setStartValue(
                    d->header->chevronProgress());
                d->chevronAnimation->setEndValue(expanded ? 1.0 : 0.0);
                if (!internal::MotionController::configure(
                        d->chevronAnimation, this,
                        {MotionDuration::Slow,
                         expanded ? MotionEasing::Enter
                                  : MotionEasing::Exit})) {
                    d->header->setChevronProgress(expanded ? 1.0 : 0.0);
                }
                else {
                    d->chevronAnimation->start();
                }
                d->reveal->setExpanded(expanded);
                update();
                emit expandedChanged(expanded);
            });
    connect(d->header->selectionControl(), &QAbstractButton::toggled, this,
            [this](bool selected) { emit selectedChanged(selected); });
}

ExpandableSection::~ExpandableSection() = default;

QString ExpandableSection::title() const
{
    return d->header->title();
}

void ExpandableSection::setTitle(const QString &title)
{
    if (d->header->title() == title) return;
    d->header->setTitle(title);
    emit titleChanged(title);
}

QString ExpandableSection::description() const
{
    return d->header->description();
}

void ExpandableSection::setDescription(const QString &description)
{
    if (d->header->description() == description) return;
    d->header->setDescription(description);
    emit descriptionChanged(description);
}

IconRole ExpandableSection::iconRole() const
{
    return d->header->iconRole();
}

void ExpandableSection::setIconRole(IconRole iconRole)
{
    if (d->header->iconRole() == iconRole) return;
    d->header->setIconRole(iconRole);
    emit iconRoleChanged(iconRole);
}

QString ExpandableSection::iconText() const
{
    return d->header->iconText();
}

void ExpandableSection::setIconText(const QString &iconText)
{
    if (d->header->iconText() == iconText) return;
    d->header->setIconText(iconText);
    emit iconTextChanged(iconText);
}

bool ExpandableSection::isExpanded() const
{
    return d->header->isChecked();
}

void ExpandableSection::setExpanded(bool expanded)
{
    d->header->setChecked(expanded);
}

void ExpandableSection::setExpanded(bool expanded, bool animated)
{
    if (animated) {
        setExpanded(expanded);
        return;
    }
    const QVariant previousPolicy = property("_ngstdAnimationPolicy");
    setProperty("_ngstdAnimationPolicy", QStringLiteral("disabled"));
    setExpanded(expanded);
    setProperty("_ngstdAnimationPolicy", previousPolicy);
}

void ExpandableSection::collapse()
{
    setExpanded(false);
}

bool ExpandableSection::isSelectionVisible() const
{
    return d->header->selectionControl()->isVisible();
}

void ExpandableSection::setSelectionVisible(bool visible)
{
    if (isSelectionVisible() == visible) return;
    d->header->setSelectionVisible(visible);
    emit selectionVisibleChanged(visible);
}

bool ExpandableSection::isSelected() const
{
    return d->header->selectionControl()->isChecked();
}

void ExpandableSection::setSelected(bool selected)
{
    d->header->selectionControl()->setChecked(selected);
}

QVBoxLayout *ExpandableSection::contentLayout() const
{
    return d->contentLayout;
}

void ExpandableSection::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    const qreal strokeWidth = 1.0;
    const qreal radius = DesignTokens::radius(RadiusRole::Panel);
    const QRectF bounds =
        QRectF(rect()).adjusted(strokeWidth * 0.5, strokeWidth * 0.5,
                                -strokeWidth * 0.5, -strokeWidth * 0.5);
    const ColorScheme scheme = internal::colorSchemeFor(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen borderPen(DesignTokens::color(ColorRole::Border, scheme),
                   strokeWidth);
    borderPen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(borderPen);
    painter.setBrush(DesignTokens::color(ColorRole::Surface, scheme));
    painter.drawRoundedRect(bounds, radius, radius);
    if (d->reveal->height() > 0) {
        painter.setPen(borderPen);
        painter.drawLine(QPointF(strokeWidth, d->header->height() + 0.5),
                         QPointF(width() - strokeWidth,
                                 d->header->height() + 0.5));
    }
}

} // namespace widgets
} // namespace ngstd
