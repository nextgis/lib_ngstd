/******************************************************************************
 * Project: NextGIS widgets gallery
 * Purpose: Reference-specific layout and painted widgets
 *****************************************************************************/
#include "reference_widgets.h"

#include "gallery_tokens.h"

#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/icons.h>
#include <ngstd/widgets/motion_adapter.h>

#include <QApplication>
#include <QClipboard>
#include <QImage>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPropertyAnimation>
#include <QRegularExpression>
#include <QStyle>
#include <QSvgRenderer>
#include <QTimer>
#include <QVBoxLayout>
#include <QVariantAnimation>
#include <QtMath>

using namespace ngstd::widgets;

namespace {

QColor jsonColor(const QStringList &path)
{
    const QString colorText = GalleryTokens::value(path).toString();
    QColor color(colorText);
    if (color.isValid()) return color;

    static const QRegularExpression expression(
        QStringLiteral("^rgba\\(\\s*(\\d+)\\s*,\\s*(\\d+)\\s*,\\s*(\\d+)"
                       "\\s*,\\s*([0-9.]+)\\s*\\)$"));
    const QRegularExpressionMatch match = expression.match(colorText);
    if (!match.hasMatch()) return QColor();
    return QColor(match.captured(1).toInt(), match.captured(2).toInt(),
                  match.captured(3).toInt(),
                  qRound(match.captured(4).toDouble() * 255.0));
}

QFont fontWithPixelSize(const QString &family, int pixelSize,
                        QFont::Weight weight)
{
    QFont font(family);
    font.setPixelSize(pixelSize);
    font.setWeight(weight);
    return font;
}

QString fontFamily(TypographyRole role)
{
    const TypographyToken token = DesignTokens::typography(role);
    return token.families.isEmpty() ? QString() : token.families.first();
}

QColor sampleInk(const QColor &background)
{
    const qreal luminance = 0.2126 * background.redF() +
                            0.7152 * background.greenF() +
                            0.0722 * background.blueF();
    return luminance > 0.62 ? QColor(QStringLiteral("#151B23")) : Qt::white;
}

QColor interpolateColor(const QColor &start, const QColor &end, qreal progress)
{
    const qreal bounded = qBound(0.0, progress, 1.0);
    return QColor::fromRgbF(
        static_cast<float>(start.redF() +
                           (end.redF() - start.redF()) * bounded),
        static_cast<float>(start.greenF() +
                           (end.greenF() - start.greenF()) * bounded),
        static_cast<float>(start.blueF() +
                           (end.blueF() - start.blueF()) * bounded),
        static_cast<float>(start.alphaF() +
                           (end.alphaF() - start.alphaF()) * bounded));
}

} // namespace

FlowLayout::FlowLayout(QWidget *parent, int margin, int horizontalSpacing,
                       int verticalSpacing)
    : QLayout(parent), m_horizontalSpacing(horizontalSpacing),
      m_verticalSpacing(verticalSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout()
{
    QLayoutItem *item = nullptr;
    while ((item = takeAt(0)) != nullptr)
        delete item;
}

void FlowLayout::addItem(QLayoutItem *item)
{
    m_items.append(item);
}

int FlowLayout::count() const
{
    return static_cast<int>(m_items.size());
}

QLayoutItem *FlowLayout::itemAt(int index) const
{
    return m_items.value(index);
}

QLayoutItem *FlowLayout::takeAt(int index)
{
    if (index < 0 || index >= m_items.size()) return nullptr;
    return m_items.takeAt(index);
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return {};
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(int width) const
{
    return doLayout(QRect(0, 0, width, 0), true);
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (QLayoutItem *item : m_items)
        size = size.expandedTo(item->minimumSize());
    const QMargins margins = contentsMargins();
    size += QSize(margins.left() + margins.right(),
                  margins.top() + margins.bottom());
    return size;
}

void FlowLayout::setGeometry(const QRect &rectangle)
{
    QLayout::setGeometry(rectangle);
    doLayout(rectangle, false);
}

QSize FlowLayout::sizeHint() const
{
    return minimumSize();
}

int FlowLayout::doLayout(const QRect &rectangle, bool testOnly) const
{
    const QMargins margins = contentsMargins();
    const QRect effectiveRectangle = rectangle.adjusted(
        margins.left(), margins.top(), -margins.right(), -margins.bottom());
    int x = effectiveRectangle.x();
    int y = effectiveRectangle.y();
    int lineHeight = 0;
    const int horizontalSpacing =
        effectiveSpacing(QStyle::PM_LayoutHorizontalSpacing);
    const int verticalSpacing =
        effectiveSpacing(QStyle::PM_LayoutVerticalSpacing);

    for (QLayoutItem *item : m_items) {
        const int nextX = x + item->sizeHint().width() + horizontalSpacing;
        if (nextX - horizontalSpacing > effectiveRectangle.right() &&
            lineHeight > 0) {
            x = effectiveRectangle.x();
            y += lineHeight + verticalSpacing;
            lineHeight = 0;
        }
        if (!testOnly)
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));
        x += item->sizeHint().width() + horizontalSpacing;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rectangle.y() + margins.bottom();
}

int FlowLayout::effectiveSpacing(QStyle::PixelMetric metric) const
{
    const int explicitSpacing = metric == QStyle::PM_LayoutHorizontalSpacing
                                    ? m_horizontalSpacing
                                    : m_verticalSpacing;
    if (explicitSpacing >= 0) return explicitSpacing;
    QObject *layoutParent = parent();
    if (!layoutParent) return -1;
    if (layoutParent->isWidgetType()) {
        QWidget *parentWidget = static_cast<QWidget *>(layoutParent);
        return parentWidget->style()->pixelMetric(metric, nullptr,
                                                  parentWidget);
    }
    QLayout *parentLayout = qobject_cast<QLayout *>(layoutParent);
    return parentLayout ? parentLayout->spacing() : -1;
}

HeroPanel::HeroPanel(QWidget *parent)
    : QFrame(parent), m_scheme(ColorScheme::Light)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void HeroPanel::setColorScheme(ColorScheme scheme)
{
    if (m_scheme == scheme) return;
    m_scheme = scheme;
    update();
}

void HeroPanel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF panelRect = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    QPainterPath clipPath;
    clipPath.addRoundedRect(panelRect, DesignTokens::radius(RadiusRole::Hero),
                            DesignTokens::radius(RadiusRole::Hero));
    painter.setClipPath(clipPath);
    painter.fillPath(clipPath,
                     DesignTokens::color(ColorRole::Surface, m_scheme));

    const QStringList decorationPath = {
        QStringLiteral("desktop"),
        QStringLiteral("gallery"),
        QStringLiteral("heroDecoration"),
    };
    const auto decorationValue = [&decorationPath](const QString &name) {
        QStringList path = decorationPath;
        path.append(name);
        return GalleryTokens::value(path);
    };
    QColor plane =
        jsonColor({QStringLiteral("product"), QStringLiteral("reference"),
                   QStringLiteral("heroPlane")});
    const qreal planeTop =
        decorationValue(QStringLiteral("planeTopPercent")).toDouble() / 100.0;
    const qreal planeBottom =
        decorationValue(QStringLiteral("planeBottomPercent")).toDouble() /
        100.0;
    QPolygonF planeShape;
    planeShape << QPointF(width() * planeTop, 0.0) << QPointF(width(), 0.0)
               << QPointF(width(), height())
               << QPointF(width() * planeBottom, height());
    painter.setPen(Qt::NoPen);
    painter.setBrush(plane);
    painter.drawPolygon(planeShape);

    QColor glow =
        jsonColor({QStringLiteral("product"), QStringLiteral("reference"),
                   QStringLiteral("heroGlow")});
    const qreal glowCenterX =
        decorationValue(QStringLiteral("glowCenterXPercent")).toDouble() /
        100.0;
    const qreal glowCenterY =
        decorationValue(QStringLiteral("glowCenterYPercent")).toDouble() /
        100.0;
    const qreal glowRadius =
        decorationValue(QStringLiteral("glowRadiusPercent")).toDouble() /
        100.0;
    QRadialGradient radial(
        QPointF(width() * glowCenterX, height() * glowCenterY),
        width() * glowRadius);
    radial.setColorAt(0.0, glow);
    glow.setAlpha(0);
    radial.setColorAt(1.0, glow);
    painter.setBrush(radial);
    painter.drawRect(rect());

    const QString assetPath =
        QStringLiteral(":/ngstd/widgets/") +
        decorationValue(QStringLiteral("asset")).toString();
    QSvgRenderer renderer(assetPath);
    if (renderer.isValid()) {
        const qreal decorationWidth =
            decorationValue(QStringLiteral("widthPx")).toDouble();
        const qreal decorationHeight =
            decorationValue(QStringLiteral("heightPx")).toDouble();
        const qreal top =
            height() *
            decorationValue(QStringLiteral("topPercent")).toDouble() / 100.0;
        const qreal right =
            decorationValue(QStringLiteral("rightPx")).toDouble();
        const qreal rotation =
            decorationValue(QStringLiteral("rotationDeg")).toDouble();
        const QPointF decorationCenter(width() - right - decorationWidth * 0.5,
                                       top + decorationHeight * 0.5);
        const QSizeF sourceSize = renderer.defaultSize();
        const qreal sourceScale = qMin(decorationWidth / sourceSize.width(),
                                       decorationHeight / sourceSize.height());
        const QSizeF containedSize(sourceSize.width() * sourceScale,
                                   sourceSize.height() * sourceScale);
        const QRectF containedRect(
            -containedSize.width() * 0.5, -containedSize.height() * 0.5,
            containedSize.width(), containedSize.height());
        painter.save();
        painter.setOpacity(m_scheme == ColorScheme::Dark
                               ? 0.18
                               : DesignTokens::decorationOpacity(m_scheme));
        painter.translate(decorationCenter);
        painter.rotate(rotation);
        renderer.render(&painter, containedRect);
        painter.restore();
    }

    painter.setClipping(false);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(DesignTokens::color(ColorRole::Border, m_scheme));
    painter.drawRoundedRect(panelRect, DesignTokens::radius(RadiusRole::Hero),
                            DesignTokens::radius(RadiusRole::Hero));
}

ColorTokenCard::ColorTokenCard(const QString &title, const QString &tokenName,
                               ColorRole role, QWidget *parent)
    : QAbstractButton(parent), m_title(title), m_tokenName(tokenName),
      m_role(role), m_scheme(ColorScheme::Light),
      m_stateAnimation(nullptr), m_stateProgress(0.0)
{
    m_stateAnimation = new QVariantAnimation(this);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::TabFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(m_stateAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
                m_stateProgress = value.toReal();
                update();
            });
    connect(this, &QAbstractButton::clicked, this, [this]() {
        const QString colorName = DesignTokens::color(m_role, m_scheme)
                                      .name(QColor::HexRgb)
                                      .toUpper();
        QApplication::clipboard()->setText(colorName);
    });
}

bool ColorTokenCard::event(QEvent *event)
{
    if (!m_stateAnimation) return QAbstractButton::event(event);
    qreal target = m_stateProgress;
    MotionDuration motionDuration = MotionDuration::Slow;
    if (event->type() == QEvent::Enter)
        target = 0.35;
    else if (event->type() == QEvent::Leave)
        target = 0.0;
    else if (event->type() == QEvent::MouseButtonPress) {
        target = 1.0;
        motionDuration = MotionDuration::Normal;
    }
    else if (event->type() == QEvent::MouseButtonRelease) {
        target = underMouse() ? 0.35 : 0.0;
    }
    if (!qFuzzyCompare(target + 1.0, m_stateProgress + 1.0)) {
        m_stateAnimation->stop();
        m_stateAnimation->setStartValue(m_stateProgress);
        m_stateAnimation->setEndValue(target);
        if (MotionAdapter::configure(
                m_stateAnimation, this,
                {motionDuration, MotionEasing::Enter})) {
            m_stateAnimation->start();
        }
        else {
            m_stateProgress = target;
            update();
        }
    }
    return QAbstractButton::event(event);
}

QSize ColorTokenCard::sizeHint() const
{
    return QSize(250, 184);
}

void ColorTokenCard::setColorScheme(ColorScheme scheme)
{
    if (m_scheme == scheme) return;
    m_scheme = scheme;
    update();
}

void ColorTokenCard::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QColor sample = DesignTokens::color(m_role, m_scheme);
    const QColor idleSurface =
        DesignTokens::color(ColorRole::Surface, m_scheme);
    const QColor hoverSurface =
        DesignTokens::color(ColorRole::SurfaceBrand, m_scheme);
    const QColor pressedSurface =
        DesignTokens::color(ColorRole::BrandSoft, m_scheme);
    const QColor idleBorder =
        DesignTokens::color(ColorRole::Border, m_scheme);
    const QColor hoverBorder =
        DesignTokens::color(ColorRole::BrandHover, m_scheme);
    const QColor pressedBorder =
        DesignTokens::color(ColorRole::BrandActive, m_scheme);
    const qreal hoverProgress = qMin(m_stateProgress / 0.35, 1.0);
    const qreal pressedProgress = qMax(0.0, (m_stateProgress - 0.35) / 0.65);
    const QColor surface = interpolateColor(
        interpolateColor(idleSurface, hoverSurface, hoverProgress),
        pressedSurface, pressedProgress);
    const QColor border = interpolateColor(
        interpolateColor(idleBorder, hoverBorder, hoverProgress),
        pressedBorder, pressedProgress);
    const int radius = DesignTokens::radius(RadiusRole::Card);
    const int sampleHeight =
        GalleryTokens::metric(GalleryMetric::ColorSampleHeight);
    const QRectF outerRect = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    QPainterPath outerPath;
    outerPath.addRoundedRect(outerRect, radius, radius);
    painter.fillPath(outerPath, surface);
    painter.save();
    painter.setClipPath(outerPath);
    painter.fillRect(QRect(0, 0, width(), sampleHeight), sample);
    painter.restore();
    painter.setBrush(Qt::NoBrush);
    painter.setPen(border);
    painter.drawPath(outerPath);

    const int padding =
        GalleryTokens::metric(GalleryMetric::ColorSamplePadding);
    painter.setPen(sampleInk(sample));
    painter.setFont(fontWithPixelSize(fontFamily(TypographyRole::Heading2), 28,
                                      QFont::Bold));
    painter.drawText(
        QRect(padding, sampleHeight - 48, width() - padding * 2, 36),
        Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("Aa"));

    const int bodyLeft = 14;
    int textY = sampleHeight + 12;
    painter.setPen(DesignTokens::color(ColorRole::Text, m_scheme));
    painter.setFont(
        fontWithPixelSize(fontFamily(TypographyRole::Body), 14, QFont::Bold));
    painter.drawText(QRect(bodyLeft, textY, width() - 28, 20),
                     Qt::AlignLeft | Qt::AlignVCenter, m_title);
    textY += 20 + GalleryTokens::metric(GalleryMetric::ColorBodyTitleSpacing);
    painter.setPen(DesignTokens::color(ColorRole::TextMuted, m_scheme));
    painter.setFont(DesignTokens::font(TypographyRole::Mono));
    painter.drawText(QRect(bodyLeft, textY, width() - 28, 17),
                     Qt::AlignLeft | Qt::AlignVCenter, m_tokenName);
    textY += 21;
    painter.setPen(DesignTokens::color(ColorRole::TextSecondary, m_scheme));
    painter.setFont(DesignTokens::font(TypographyRole::Mono));
    painter.drawText(QRect(bodyLeft, textY, width() - 28, 18),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     sample.name(QColor::HexRgb).toUpper());
}

SemanticCopyButton::SemanticCopyButton(const QString &tokenName,
                                       ColorRole colorRole, QWidget *parent,
                                       bool showValueInTokenColor)
    : QToolButton(parent), m_valueLabel(new QLabel(this)),
      m_copyIcon(new QLabel(this)), m_tokenName(tokenName),
      m_colorRole(colorRole), m_scheme(ColorScheme::Light),
      m_showValueInTokenColor(showValueInTokenColor)
{
    setProperty("_ngstdRole", QStringLiteral("semanticCopy"));
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setToolButtonStyle(Qt::ToolButtonTextOnly);
    setAccessibleName(QObject::tr("Copy %1").arg(m_tokenName));
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 4, 6, 4);
    layout->setSpacing(8);
    QLabel *tokenLabel = new QLabel(m_tokenName, this);
    tokenLabel->setProperty("_ngstdRole", QStringLiteral("semanticToken"));
    tokenLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_valueLabel->setProperty("_ngstdRole", QStringLiteral("semanticValue"));
    m_valueLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_copyIcon->setFixedSize(14, 14);
    m_copyIcon->setAlignment(Qt::AlignCenter);
    m_copyIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    layout->addWidget(tokenLabel, 1);
    layout->addWidget(m_valueLabel);
    layout->addWidget(m_copyIcon);
    connect(this, &QToolButton::clicked, this, [this]() {
        QApplication::clipboard()->setText(m_valueLabel->text());
    });
    updateValue();
}

void SemanticCopyButton::setColorScheme(ColorScheme scheme)
{
    if (m_scheme == scheme) return;
    m_scheme = scheme;
    updateValue();
}

void SemanticCopyButton::updateValue()
{
    m_valueLabel->setText(DesignTokens::color(m_colorRole, m_scheme)
                              .name(QColor::HexRgb)
                              .toUpper());
    m_valueLabel->setProperty("ngSemanticColor",
                              DesignTokens::color(m_colorRole, m_scheme));
    const ColorRole valueTextRole =
        m_showValueInTokenColor
            ? (m_scheme == ColorScheme::Dark ? ColorRole::Text : m_colorRole)
            : ColorRole::TextSecondary;
    QPalette palette = m_valueLabel->palette();
    palette.setColor(
        QPalette::WindowText,
        DesignTokens::color(valueTextRole, m_scheme));
    m_valueLabel->setPalette(palette);
    m_copyIcon->setPixmap(
        iconPixmap(IconRole::Copy, QSize(14, 14), devicePixelRatioF(),
                   DesignTokens::color(ColorRole::TextSecondary, m_scheme)));
}

SemanticStateCard::SemanticStateCard(const QString &title,
                                     const QString &textToken,
                                     ColorRole textRole,
                                     const QString &backgroundToken,
                                     ColorRole backgroundRole,
                                     SemanticTone tone, QWidget *parent)
    : QFrame(parent), m_iconLabel(new QLabel(this)),
      m_textButton(new SemanticCopyButton(textToken, textRole, this)),
      m_backgroundButton(new SemanticCopyButton(backgroundToken,
                                                backgroundRole, this, false)),
      m_tone(tone), m_scheme(ColorScheme::Light)
{
    setProperty("_ngstdRole", QStringLiteral("semanticState"));
    WidgetStyle::setTone(this, m_tone);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 16);
    layout->setSpacing(6);
    QHBoxLayout *previewLayout = new QHBoxLayout;
    previewLayout->setContentsMargins(0, 0, 0, 0);
    previewLayout->setSpacing(9);
    m_iconLabel->setFixedSize(20, 20);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    QLabel *titleLabel = new QLabel(title, this);
    titleLabel->setProperty("_ngstdRole", QStringLiteral("semanticTitle"));
    WidgetStyle::setTone(titleLabel, m_tone);
    previewLayout->addWidget(m_iconLabel);
    previewLayout->addWidget(titleLabel);
    previewLayout->addStretch();
    layout->addLayout(previewLayout);
    layout->addWidget(m_textButton);
    layout->addWidget(m_backgroundButton);
    setColorScheme(ColorScheme::Light);
}

void SemanticStateCard::setColorScheme(ColorScheme scheme)
{
    m_scheme = scheme;
    m_textButton->setColorScheme(m_scheme);
    m_backgroundButton->setColorScheme(m_scheme);
    IconRole iconRole = IconRole::Information;
    ColorRole colorRole = ColorRole::Link;
    if (m_tone == SemanticTone::Success) {
        iconRole = IconRole::CheckCircle;
        colorRole = ColorRole::Success;
    }
    else if (m_tone == SemanticTone::Warning) {
        iconRole = IconRole::AlertTriangle;
        colorRole = ColorRole::Warning;
    }
    else if (m_tone == SemanticTone::Danger) {
        iconRole = IconRole::CloseCircle;
        colorRole = ColorRole::Danger;
    }
    m_iconLabel->setPixmap(
        iconPixmap(iconRole, QSize(20, 20), devicePixelRatioF(),
                   DesignTokens::color(colorRole, m_scheme)));
}
