/******************************************************************************
 * Project: NextGIS widgets gallery
 * Purpose: Corporate widgets style gallery
 *****************************************************************************/
#include "gallery_window.h"

#include "gallery_tokens.h"

#include "demo_wizard.h"
#include "reference_widgets.h"

#include <ngstd/widgets/components.h>
#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/icons.h>
#include <ngstd/widgets/theme.h>
#include <ngstd/widgets/theme_switch.h>
#include <ngstd/widgets/widget_style.h>

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QEasingCurve>
#include <QFile>
#include <QFontMetrics>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QRadioButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QShowEvent>
#include <QSizePolicy>
#include <QSpinBox>
#include <QStyle>
#include <QTabWidget>
#include <QTimer>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <functional>

using namespace ngstd::widgets;

namespace {

QString galleryStyleSheet(ColorScheme scheme)
{
    QFile file(QStringLiteral(":/ngstd/widgets-gallery/nextgis-gallery-%1.qss")
                   .arg(scheme == ColorScheme::Dark
                            ? QStringLiteral("dark")
                            : QStringLiteral("light")));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
    return QString::fromUtf8(file.readAll());
}

QLabel *makeLabel(const QString &text, const QString &role = QString(),
                  bool wordWrap = false)
{
    QLabel *label = new QLabel(text);
    if (!role.isEmpty()) label->setProperty("_ngstdRole", role);
    label->setWordWrap(wordWrap);
    return label;
}

QLabel *makeTypographyLabel(const QString &text, TypographyRole role,
                            bool wordWrap = false)
{
    QLabel *label = makeLabel(text, QString(), wordWrap);
    WidgetStyle::setTypographyRole(label, role);
    return label;
}

QFrame *createProductActionPanel(const QString &title,
                                 const QString &description,
                                 const QList<QWidget *> &actions)
{
    QFrame *panel = new QFrame;
    panel->setProperty("_ngstdRole", QStringLiteral("privateShowcase"));
    panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout *panelLayout = new QHBoxLayout(panel);
    const int horizontalPadding = GalleryTokens::metric(
        GalleryMetric::ProductActionPanelPaddingHorizontal);
    const int verticalPadding = GalleryTokens::metric(
        GalleryMetric::ProductActionPanelPaddingVertical);
    panelLayout->setContentsMargins(horizontalPadding, verticalPadding,
                                    horizontalPadding, verticalPadding);
    panelLayout->setSpacing(
        GalleryTokens::metric(GalleryMetric::ProductActionPanelContentGap));

    QWidget *copyWidget = new QWidget;
    copyWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QVBoxLayout *copyLayout = new QVBoxLayout(copyWidget);
    copyLayout->setContentsMargins(0, 0, 0, 0);
    copyLayout->setSpacing(
        GalleryTokens::metric(GalleryMetric::ProductActionPanelCopySpacing));
    copyLayout->addWidget(makeTypographyLabel(title, TypographyRole::Control));
    copyLayout->addWidget(
        makeTypographyLabel(description, TypographyRole::Caption, true));

    QWidget *actionsWidget = new QWidget;
    actionsWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(
        GalleryTokens::metric(GalleryMetric::ProductActionPanelActionGap));
    for (QWidget *action : actions)
        actionsLayout->addWidget(action);

    panelLayout->addWidget(copyWidget, 1, Qt::AlignVCenter);
    panelLayout->addWidget(actionsWidget, 0, Qt::AlignVCenter);
    return panel;
}

void addSectionIntro(QVBoxLayout *layout, const QString &text)
{
    const int margin =
        GalleryTokens::metric(GalleryMetric::SectionIntroMarginVertical);
    QLabel *intro = makeLabel(text, QStringLiteral("secondary"), true);
    WidgetStyle::setTypographyRole(intro, TypographyRole::Body);
    intro->setMaximumWidth(820);
    const QFontMetrics metrics(DesignTokens::font(TypographyRole::Body));
    const QRect textBounds = metrics.boundingRect(
        QRect(0, 0, 820, QWIDGETSIZE_MAX),
        Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, text);
    const int naturalLineHeight = qMax(1, metrics.lineSpacing());
    const int lineCount =
        qMax(1, qRound(qreal(textBounds.height()) / naturalLineHeight));
    intro->setMinimumHeight(
        lineCount *
        GalleryTokens::metric(GalleryMetric::SectionIntroLineHeight));
    layout->addSpacing(margin);
    layout->addWidget(intro);
    layout->addSpacing(margin);
}

class ActionFrame : public QFrame
{
public:
    explicit ActionFrame(const std::function<void()> &callback,
                         QWidget *parent = nullptr)
        : QFrame(parent), m_callback(callback), m_pressed(false)
    {
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::StrongFocus);
        setProperty("pressed", false);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            setPressed(true);
            event->accept();
            return;
        }
        QFrame::mousePressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            const bool activate = m_pressed && rect().contains(event->pos());
            setPressed(false);
            if (activate && m_callback) m_callback();
            event->accept();
            return;
        }
        QFrame::mouseReleaseEvent(event);
    }

    void leaveEvent(QEvent *event) override
    {
        setPressed(false);
        QFrame::leaveEvent(event);
    }

    void keyPressEvent(QKeyEvent *event) override
    {
        if (!event->isAutoRepeat() &&
            (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ||
             event->key() == Qt::Key_Space)) {
            setPressed(true);
            event->accept();
            return;
        }
        QFrame::keyPressEvent(event);
    }

    void keyReleaseEvent(QKeyEvent *event) override
    {
        if (!event->isAutoRepeat() &&
            (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ||
             event->key() == Qt::Key_Space)) {
            const bool activate = m_pressed;
            setPressed(false);
            if (activate && m_callback) m_callback();
            event->accept();
            return;
        }
        QFrame::keyReleaseEvent(event);
    }

private:
    void setPressed(bool pressed)
    {
        if (m_pressed == pressed) return;
        m_pressed = pressed;
        setProperty("pressed", m_pressed);
        WidgetStyle::refresh(this);
    }

    std::function<void()> m_callback;
    bool m_pressed;
};

class NavigationLink final : public ActionFrame
{
public:
    NavigationLink(int index, const QString &title,
                   const std::function<void()> &callback,
                   QWidget *parent = nullptr)
        : ActionFrame(callback, parent)
    {
        setProperty("_ngstdRole", QStringLiteral("quickLink"));
        setFixedHeight(GalleryTokens::value(
                           {
                               QStringLiteral("desktop"),
                               QStringLiteral("gallery"),
                               QStringLiteral("quickNavigationLinkHeightPx"),
                           })
                           .toInt());
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(12, 0, 12, 0);
        layout->setSpacing(9);
        QLabel *number =
            makeLabel(QStringLiteral("%1").arg(index, 2, 10, QLatin1Char('0')),
                      QStringLiteral("navigationNumber"));
        QLabel *text = makeLabel(title, QStringLiteral("navigationText"));
        QLabel *arrow = makeLabel(QString::fromUtf8("↘"),
                                  QStringLiteral("navigationArrow"));
        number->setAttribute(Qt::WA_TransparentForMouseEvents);
        text->setAttribute(Qt::WA_TransparentForMouseEvents);
        arrow->setAttribute(Qt::WA_TransparentForMouseEvents);
        layout->addWidget(number);
        layout->addWidget(text);
        layout->addStretch();
        layout->addWidget(arrow);
    }
};

class CoverImage final : public QWidget
{
public:
    CoverImage(const QString &imagePath, int height, bool roundedTop,
               QWidget *parent = nullptr)
        : QWidget(parent), m_pixmap(imagePath), m_roundedTop(roundedTop)
    {
        setFixedHeight(height);
        setMinimumWidth(0);
        setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)
        if (m_pixmap.isNull()) return;
        QPainter painter(this);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        if (m_roundedTop) {
            const qreal radius = DesignTokens::radius(RadiusRole::Card);
            QPainterPath clip;
            clip.setFillRule(Qt::WindingFill);
            clip.addRoundedRect(rect(), radius, radius);
            clip.addRect(QRectF(0.0, radius, width(), height() - radius));
            painter.setClipPath(clip);
        }
        const qreal scale = qMax(qreal(width()) / m_pixmap.width(),
                                 qreal(height()) / m_pixmap.height());
        const QSizeF renderedSize(m_pixmap.width() * scale,
                                  m_pixmap.height() * scale);
        const QRectF target((width() - renderedSize.width()) * 0.5,
                            (height() - renderedSize.height()) * 0.5,
                            renderedSize.width(), renderedSize.height());
        painter.drawPixmap(target, m_pixmap, QRectF(m_pixmap.rect()));
    }

private:
    QPixmap m_pixmap;
    bool m_roundedTop;
};

Disclosure *nestedPanel(const QString &title, const QStringList &items)
{
    Disclosure *panel = new Disclosure;
    panel->setTitle(title);
    QWidget *content = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    for (const QString &item : items) {
        QLabel *label = makeLabel(QString::fromUtf8("• ") + item,
                                  QStringLiteral("secondary"), true);
        label->setFont(DesignTokens::font(TypographyRole::Control));
        layout->addWidget(label);
    }
    panel->setContentWidget(content);
    return panel;
}

QWidget *imageCard(const QString &title, const QString &description,
                   const QString &imagePath)
{
    if (imagePath.endsWith(QStringLiteral("sat_card.webp"))) {
        CardButton *card = new CardButton;
        card->setAccessibleName(title);
        card->setAccessibleDescription(description);
        card->setVariant(CardVariant::Background);
        card->setBackgroundPixmap(QPixmap(imagePath));
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        card->setFixedHeight(320);
        QVBoxLayout *layout = card->contentLayout();
        layout->setContentsMargins(20, 20, 20, 20);
        layout->setSpacing(8);
        layout->addStretch();
        QHBoxLayout *tags = new QHBoxLayout;
        tags->setSpacing(8);
        Tag *satellite = new Tag(QObject::tr("Satellite"));
        satellite->setTone(SemanticTone::Information);
        Tag *newTag = new Tag(QObject::tr("New"));
        newTag->setTone(SemanticTone::Warning);
        newTag->setIconRole(IconRole::Flame);
        tags->addWidget(satellite);
        tags->addWidget(newTag);
        tags->addStretch();
        layout->addLayout(tags);
        layout->addWidget(makeLabel(title, QStringLiteral("onImageTitle")));
        layout->addWidget(
            makeLabel(description, QStringLiteral("onImageBody"), true));
        return card;
    }

    CardButton *card = new CardButton;
    card->setAccessibleName(title);
    card->setAccessibleDescription(description);
    if (imagePath.isEmpty()) {
        card->setVariant(CardVariant::Default);
        QHBoxLayout *layout = new QHBoxLayout;
        layout->setContentsMargins(20, 20, 20, 20);
        layout->setSpacing(16);
        QFrame *iconFrame = new QFrame;
        iconFrame->setProperty("_ngstdRole", QStringLiteral("cardIcon"));
        iconFrame->setFixedSize(52, 52);
        QVBoxLayout *iconLayout = new QVBoxLayout(iconFrame);
        iconLayout->setContentsMargins(8, 8, 8, 8);
        QLabel *iconLabel = new QLabel;
        iconLabel->setPixmap(
            QPixmap(
                QStringLiteral(":/ngstd/widgets/assets/pictograms/data.png"))
                .scaled(36, 36, Qt::KeepAspectRatio,
                        Qt::SmoothTransformation));
        iconLayout->addWidget(iconLabel);
        QVBoxLayout *copy = new QVBoxLayout;
        copy->setSpacing(8);
        copy->addWidget(makeLabel(title, QStringLiteral("panelHeading")));
        copy->addWidget(
            makeLabel(description, QStringLiteral("secondary"), true));
        QHBoxLayout *tags = new QHBoxLayout;
        tags->setSpacing(8);
        Tag *catalog = new Tag(QObject::tr("Catalog"));
        catalog->setTone(SemanticTone::Information);
        tags->addWidget(catalog);
        tags->addWidget(new Tag(QObject::tr("API")));
        tags->addStretch();
        copy->addLayout(tags);
        layout->addWidget(iconFrame, 0, Qt::AlignTop);
        layout->addLayout(copy, 1);
        card->contentLayout()->setContentsMargins(0, 0, 0, 0);
        card->contentLayout()->addLayout(layout);
        return card;
    }

    QVBoxLayout *layout = card->contentLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    CoverImage *image = new CoverImage(imagePath, 142, true);
    QWidget *body = new QWidget;
    QVBoxLayout *bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(20, 20, 20, 20);
    bodyLayout->setSpacing(0);
    bodyLayout->addWidget(makeLabel(title, QStringLiteral("panelHeading")));
    bodyLayout->addSpacing(8);
    bodyLayout->addWidget(
        makeLabel(description, QStringLiteral("secondary"), true));
    bodyLayout->addSpacing(16);
    QHBoxLayout *tags = new QHBoxLayout;
    tags->setSpacing(8);
    Tag *popular = new Tag(QObject::tr("Popular"));
    popular->setTone(SemanticTone::Warning);
    popular->setIconRole(IconRole::Flame);
    tags->addWidget(popular);
    tags->addWidget(new Tag(QObject::tr("Vector")));
    tags->addStretch();
    bodyLayout->addLayout(tags);
    card->setVariant(CardVariant::Media);
    card->setTopWidget(image);
    card->setBodyWidget(body);
    return card;
}

QFrame *createAssetCard(const QString &title, const QPixmap &preview,
                        bool darkPreview = false,
                        const QString &format = QStringLiteral("SVG"),
                        bool adaptivePreview = false)
{
    QFrame *card = new QFrame;
    card->setProperty("_ngstdRole", QStringLiteral("assetCard"));
    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QFrame *stage = new QFrame;
    stage->setProperty("_ngstdRole", QStringLiteral("assetPreview"));
    stage->setProperty("dark", darkPreview);
    stage->setProperty("adaptive", adaptivePreview);
    stage->setMinimumHeight(154);
    QVBoxLayout *stageLayout = new QVBoxLayout(stage);
    stageLayout->setContentsMargins(20, 20, 20, 20);
    QLabel *imageLabel = new QLabel;
    imageLabel->setPixmap(preview);
    imageLabel->setAlignment(Qt::AlignCenter);
    stageLayout->addWidget(imageLabel);

    QWidget *caption = new QWidget;
    QHBoxLayout *captionLayout = new QHBoxLayout(caption);
    captionLayout->setContentsMargins(14, 10, 10, 10);
    captionLayout->setSpacing(8);
    QLabel *titleLabel =
        makeTypographyLabel(title, TypographyRole::Caption, true);
    titleLabel->setProperty("_ngstdRole", QStringLiteral("assetTitle"));
    Button *download = new Button(format);
    download->setVariant(ButtonVariant::Secondary);
    download->setIconRole(IconRole::Download);
    download->setFixedHeight(32);
    captionLayout->addWidget(titleLabel, 1);
    captionLayout->addWidget(download, 0, Qt::AlignVCenter);

    layout->addWidget(stage, 1);
    layout->addWidget(caption);
    return card;
}

Button *createIconPreviewButton(IconRole role)
{
    Button *button = new Button;
    button->setVariant(ButtonVariant::Icon);
    button->setIconRole(role);
    button->setFixedSize(40, 40);
    button->setAccessibleName(iconResourcePath(role));
    return button;
}

} // namespace

GalleryWindow::GalleryWindow(ThemeMode initialTheme,
                             const QString &initialSection,
                             AnimationPolicy animationPolicy,
                             ThemeController *themeController, QWidget *parent)
    : QMainWindow(parent), m_themeRoot(nullptr), m_headerInner(nullptr),
      m_page(nullptr), m_scrollArea(new QScrollArea(this)),
      m_scrollAnimation(nullptr), m_themeController(themeController),
      m_toast(nullptr), m_themeMode(initialTheme),
      m_animationPolicy(animationPolicy), m_colorScheme(ColorScheme::Light),
      m_themeSwitch(nullptr), m_initialSectionTarget(nullptr),
      m_initialLayoutPending(false)
{
    setWindowTitle(tr("ngstd::widgets — style guide"));
    resize(1267, 900);
    setMinimumSize(900, 680);

    QWidget *root = new QWidget;
    QVBoxLayout *rootLayout = new QVBoxLayout(root);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    rootLayout->addWidget(createHeader());

    QWidget *scrollContent = new QWidget;
    scrollContent->setProperty("_ngstdRole", QStringLiteral("page"));
    QHBoxLayout *centeringLayout = new QHBoxLayout(scrollContent);
    const int viewportMargin =
        GalleryTokens::metric(GalleryMetric::ViewportMargin);
    centeringLayout->setContentsMargins(viewportMargin, 0, viewportMargin, 0);
    centeringLayout->setSpacing(0);

    m_page = new QWidget;
    m_page->setMaximumWidth(
        GalleryTokens::metric(GalleryMetric::ContentMaxWidth));
    m_page->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QVBoxLayout *pageLayout = new QVBoxLayout(m_page);
    pageLayout->setContentsMargins(
        0, GalleryTokens::metric(GalleryMetric::PagePaddingTop), 0,
        GalleryTokens::metric(GalleryMetric::PagePaddingBottom));
    pageLayout->setSpacing(0);

    HeroPanel *hero = createHero();
    pageLayout->addWidget(hero);

    QWidget *navigation = createQuickNavigation();
    pageLayout->addSpacing(
        GalleryTokens::metric(GalleryMetric::QuickNavigationMarginTop));
    pageLayout->addWidget(navigation);

    ExpandableSection *colors = createColorsSection();
    ExpandableSection *typography = createTypographySection();
    ExpandableSection *components = createComponentsSection();
    ExpandableSection *backgrounds = createBackgroundsSection();
    ExpandableSection *brand = createBrandSection();
    ExpandableSection *icons = createIconsSection();
    ExpandableSection *motion = createMotionSection();
    ExpandableSection *tokens = createTokensSection();
    m_sections = {
        colors, typography, components, backgrounds,
        brand,  icons,      motion,     tokens,
    };

    QGridLayout *navigationGrid =
        qobject_cast<QGridLayout *>(navigation->layout());
    addNavigationLink(navigationGrid, 1, tr("Colors"), colors);
    addNavigationLink(navigationGrid, 2, tr("Typography"), typography);
    addNavigationLink(navigationGrid, 3, tr("Components"), components);
    addNavigationLink(navigationGrid, 4, tr("Backgrounds"), backgrounds);
    addNavigationLink(navigationGrid, 5, tr("Brand assets"), brand);
    addNavigationLink(navigationGrid, 6, tr("Icons"), icons);
    addNavigationLink(navigationGrid, 7, tr("Motion"), motion);
    addNavigationLink(navigationGrid, 8, tr("Tokens and Qt"), tokens);

    const int sectionMargin =
        GalleryTokens::metric(GalleryMetric::SectionMarginTop);
    for (ExpandableSection *section : m_sections) {
        pageLayout->addSpacing(sectionMargin);
        pageLayout->addWidget(section);
    }
    colors->setExpanded(true, false);

    centeringLayout->addStretch();
    centeringLayout->addWidget(m_page, 0, Qt::AlignTop);
    centeringLayout->addStretch();

    m_scrollArea->setWidget(scrollContent);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    rootLayout->addWidget(m_scrollArea, 1);
    setCentralWidget(root);
    m_themeRoot = root;
    const QList<RevealWidget *> revealViewports =
        m_page->findChildren<RevealWidget *>();
    for (RevealWidget *viewport : revealViewports) {
        connect(viewport, &RevealWidget::revealProgressChanged, this,
                [this](qreal) { synchronizePageHeight(); });
    }
    synchronizePageHeight();
    m_toast = new Toast(m_themeRoot);
    const QList<Button *> actionButtons =
        m_themeRoot->findChildren<Button *>();
    for (Button *button : actionButtons) {
        if (button->property("_ngstdGallerySuppressToast").toBool()) continue;
        connect(button, &QPushButton::clicked, this, [this]() {
            if (m_toast) m_toast->showMessage(tr("Clicked!"));
        });
    }
    for (ColorTokenCard *card : m_colorCards) {
        connect(card, &QAbstractButton::clicked, this, [this]() {
            if (m_toast) {
                m_toast->showMessage(
                    tr("Copied: %1")
                        .arg(QApplication::clipboard()->text()));
            }
        });
    }
    const QList<QToolButton *> copyButtons =
        m_themeRoot->findChildren<QToolButton *>();
    for (QToolButton *button : copyButtons) {
        if (button->property("_ngstdRole").toString() !=
            QStringLiteral("semanticCopy")) {
            continue;
        }
        connect(button, &QToolButton::clicked, this, [this]() {
            if (m_toast) {
                m_toast->showMessage(
                    tr("Copied: %1")
                        .arg(QApplication::clipboard()->text()));
            }
        });
    }
    const QList<CardButton *> interactiveCards =
        m_themeRoot->findChildren<CardButton *>();
    for (CardButton *card : interactiveCards) {
        connect(card, &CardButton::clicked, this, [this]() {
            if (m_toast) m_toast->showMessage(tr("Clicked!"));
        });
    }
    QTimer::singleShot(
        0, this, [this]() { m_scrollArea->setFocus(Qt::OtherFocusReason); });

    m_themeSwitch->setThemeMode(m_themeMode);

    connect(m_themeSwitch, &ThemeSwitch::themeModeChanged, this,
            [this](ThemeMode) { applyTheme(); });
    if (m_themeController) {
        connect(m_themeController, &ThemeController::colorSchemeChanged, this,
                [this](ColorScheme scheme) {
                    m_colorScheme = scheme;
                    updateThemeVisuals();
                });
    }
    applyTheme();

    if (initialSection == QStringLiteral("all")) {
        setAllSectionsExpanded(true);
        m_initialLayoutPending = true;
        return;
    }

    ExpandableSection *requestedTarget = nullptr;
    if (initialSection == QStringLiteral("typography"))
        requestedTarget = typography;
    else if (initialSection == QStringLiteral("components"))
        requestedTarget = components;
    else if (initialSection == QStringLiteral("backgrounds"))
        requestedTarget = backgrounds;
    else if (initialSection == QStringLiteral("brand"))
        requestedTarget = brand;
    else if (initialSection == QStringLiteral("icons"))
        requestedTarget = icons;
    else if (initialSection == QStringLiteral("motion") ||
             initialSection == QStringLiteral("animations"))
        requestedTarget = motion;
    else if (initialSection == QStringLiteral("tokens"))
        requestedTarget = tokens;
    if (requestedTarget) {
        for (ExpandableSection *section : m_sections)
            section->setExpanded(section == requestedTarget, false);
        m_initialSectionTarget = requestedTarget;
        m_initialLayoutPending = true;
    }
}

void GalleryWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    const int maximumWidth =
        GalleryTokens::metric(GalleryMetric::ContentMaxWidth);
    const int margin = GalleryTokens::metric(GalleryMetric::ViewportMargin);
    const int availableWidth = qMax(0, width() - margin * 2);
    const int contentWidth = qMin(maximumWidth, availableWidth);
    if (m_headerInner) m_headerInner->setFixedWidth(contentWidth);
    if (m_page) m_page->setFixedWidth(contentWidth);
    synchronizePageHeight();
}

void GalleryWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (!m_initialLayoutPending) return;
    synchronizePageHeight();
    if (m_initialSectionTarget) {
        positionAtSection(m_initialSectionTarget);
        m_initialSectionTarget = nullptr;
    }
    else {
        m_scrollArea->verticalScrollBar()->setValue(0);
    }
    m_initialLayoutPending = false;
}

void GalleryWindow::synchronizePageHeight()
{
    if (!m_page || !m_page->layout() || !m_scrollArea ||
        !m_scrollArea->widget()) {
        return;
    }

    QLayout *pageLayout = m_page->layout();
    pageLayout->invalidate();
    const int targetPageHeight = pageLayout->sizeHint().height();
    m_page->setFixedHeight(targetPageHeight);
    pageLayout->invalidate();
    pageLayout->activate();

    QWidget *scrollContent = m_scrollArea->widget();
    const int targetScrollHeight =
        qMax(m_scrollArea->viewport()->height(), targetPageHeight);
    scrollContent->setMinimumHeight(targetPageHeight);
    scrollContent->resize(m_scrollArea->viewport()->width(),
                          targetScrollHeight);
    if (scrollContent->layout()) {
        scrollContent->layout()->invalidate();
        scrollContent->layout()->activate();
    }
}

QWidget *GalleryWindow::createHeader()
{
    QFrame *header = new QFrame;
    header->setProperty("_ngstdRole", QStringLiteral("galleryHeader"));
    header->setFixedHeight(GalleryTokens::metric(GalleryMetric::HeaderHeight));
    QHBoxLayout *outerLayout = new QHBoxLayout(header);
    outerLayout->setContentsMargins(
        GalleryTokens::metric(GalleryMetric::ViewportMargin), 0,
        GalleryTokens::metric(GalleryMetric::ViewportMargin), 0);
    outerLayout->setSpacing(0);

    m_headerInner = new QWidget;
    m_headerInner->setMaximumWidth(
        GalleryTokens::metric(GalleryMetric::ContentMaxWidth));
    m_headerInner->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    QHBoxLayout *layout = new QHBoxLayout(m_headerInner);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(20);

    ActionFrame *brandLink = new ActionFrame([this]() {
        scrollToPosition(m_scrollArea->verticalScrollBar()->minimum());
    });
    brandLink->setFocusPolicy(Qt::NoFocus);
    brandLink->setProperty("_ngstdRole", QStringLiteral("headerBrand"));
    brandLink->setAccessibleName(tr("Back to the top"));
    QHBoxLayout *brandLayout = new QHBoxLayout(brandLink);
    brandLayout->setContentsMargins(0, 0, 0, 0);
    brandLayout->setSpacing(14);

    QLabel *logo = new QLabel;
    logo->setProperty("ngThemeLogo", true);
    logo->setFixedSize(152, 22);
    logo->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QLabel *separator = new QLabel;
    separator->setFixedSize(1, 22);
    separator->setProperty("_ngstdRole", QStringLiteral("separator"));
    QLabel *title =
        makeLabel(tr("Style guide"), QStringLiteral("secondary"));
    WidgetStyle::setTypographyRole(title, TypographyRole::Caption);
    logo->setAttribute(Qt::WA_TransparentForMouseEvents);
    separator->setAttribute(Qt::WA_TransparentForMouseEvents);
    title->setAttribute(Qt::WA_TransparentForMouseEvents);
    brandLayout->addWidget(logo);
    brandLayout->addWidget(separator);
    brandLayout->addWidget(title);

    m_themeSwitch = new ThemeSwitch;
    m_themeSwitch->button(ThemeMode::System)
        ->setToolTip(tr("System theme"));
    m_themeSwitch->button(ThemeMode::Light)->setToolTip(tr("Light theme"));
    m_themeSwitch->button(ThemeMode::Dark)->setToolTip(tr("Dark theme"));

    layout->addWidget(brandLink);
    layout->addStretch();
    layout->addWidget(m_themeSwitch);
    outerLayout->addStretch();
    outerLayout->addWidget(m_headerInner);
    outerLayout->addStretch();
    return header;
}

HeroPanel *GalleryWindow::createHero()
{
    HeroPanel *hero = new HeroPanel;
    hero->setMinimumHeight(
        GalleryTokens::metric(GalleryMetric::HeroDesktopHeight));
    m_heroPanels.append(hero);
    QHBoxLayout *heroLayout = new QHBoxLayout(hero);
    const int padding = GalleryTokens::metric(GalleryMetric::HeroPadding);
    heroLayout->setContentsMargins(padding, padding, padding, padding);
    heroLayout->setSpacing(0);

    QWidget *content = new QWidget;
    content->setMaximumWidth(
        GalleryTokens::metric(GalleryMetric::HeroContentMaxWidth));
    QVBoxLayout *layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QLabel *eyebrow = makeLabel(tr("NEXTGIS STYLE GUIDE"),
                                QStringLiteral("eyebrow"));
    QFont eyebrowFont = DesignTokens::font(TypographyRole::Control);
    eyebrowFont.setPixelSize(13);
    eyebrowFont.setWeight(QFont::Bold);
    eyebrowFont.setLetterSpacing(QFont::AbsoluteSpacing, 1.0);
    eyebrow->setFont(eyebrowFont);
    QLabel *title =
        makeLabel(tr("NextGIS foundational design"),
                  QStringLiteral("heroTitle"));
    QLabel *lead = makeLabel(
        tr("A unified visual system for web and Qt interfaces, from "
           "foundational tokens to components, product backgrounds, and "
           "brand assets."),
        QStringLiteral("heroLead"), true);
    lead->setMaximumWidth(720);
    layout->addWidget(eyebrow);
    layout->addSpacing(12);
    layout->addWidget(title);
    layout->addSpacing(20);
    layout->addWidget(lead);
    layout->addSpacing(28);

    QHBoxLayout *downloads = new QHBoxLayout;
    downloads->setContentsMargins(0, 0, 0, 0);
    downloads->setSpacing(10);
    const QStringList downloadTitles = {
        tr("JSON tokens"),
        tr("QSS · light theme"),
        tr("QSS · dark theme"),
    };
    for (const QString &downloadTitle : downloadTitles) {
        Button *button = new Button(downloadTitle);
        button->setVariant(ButtonVariant::Default);
        button->setIconRole(IconRole::Download);
        downloads->addWidget(button);
    }
    downloads->addStretch();
    layout->addLayout(downloads);
    layout->addSpacing(10);

    QHBoxLayout *sectionActions = new QHBoxLayout;
    sectionActions->setContentsMargins(0, 0, 0, 0);
    sectionActions->setSpacing(10);
    Button *openAll = new Button(tr("Expand all"));
    openAll->setVariant(ButtonVariant::Text);
    openAll->setProperty("_ngstdGallerySuppressToast", true);
    Button *closeAll = new Button(tr("Collapse all"));
    closeAll->setVariant(ButtonVariant::Text);
    closeAll->setProperty("_ngstdGallerySuppressToast", true);
    connect(openAll, &QPushButton::clicked, this,
            [this]() { setAllSectionsExpanded(true); });
    connect(closeAll, &QPushButton::clicked, this,
            [this]() { setAllSectionsExpanded(false); });
    sectionActions->addWidget(openAll);
    sectionActions->addWidget(closeAll);
    sectionActions->addStretch();
    layout->addLayout(sectionActions);

    heroLayout->addWidget(content);
    heroLayout->addStretch();
    return hero;
}

QWidget *GalleryWindow::createQuickNavigation()
{
    QFrame *navigation = new QFrame;
    navigation->setProperty("_ngstdRole", QStringLiteral("quickNavigation"));
    navigation->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QGridLayout *layout = new QGridLayout(navigation);
    const int padding =
        GalleryTokens::metric(GalleryMetric::QuickNavigationPadding);
    layout->setContentsMargins(padding, padding, padding, padding);
    layout->setHorizontalSpacing(4);
    layout->setVerticalSpacing(4);
    QLabel *label = makeLabel(QString::fromUtf8("→  QUICK LINKS"),
                              QStringLiteral("navigationLabel"));
    label->setFixedHeight(30);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    layout->addWidget(label, 0, 0, 1, 3);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 1);
    layout->setColumnStretch(2, 1);
    return navigation;
}

ExpandableSection *GalleryWindow::createColorsSection()
{
    ExpandableSection *section = new ExpandableSection(
        tr("Colors"), tr("Foundational, semantic, and copyable tokens."),
        IconRole::Copy);
    QVBoxLayout *layout = section->contentLayout();
    addSectionIntro(
        layout,
        tr("Brand #0C65AF stays unchanged in both themes. Small text on "
           "dark backgrounds uses the lighter #69B8ED to remain legible."));

    QGridLayout *grid = new QGridLayout;
    const int gridSpacing =
        GalleryTokens::metric(GalleryMetric::ColorGridSpacing);
    grid->setHorizontalSpacing(gridSpacing);
    grid->setVerticalSpacing(gridSpacing);
    const struct ColorEntry
    {
        const char *title;
        const char *token;
        ColorRole role;
    } colors[] = {
        {"Brand", "--ng-brand", ColorRole::Brand},
        {"Page background", "--ng-background", ColorRole::Background},
        {"Soft background", "--ng-page-soft", ColorRole::PageSoft},
        {"Surface", "--ng-surface", ColorRole::Surface},
        {"Muted surface", "--ng-surface-muted",
         ColorRole::SurfaceMuted},
        {"Brand surface", "--ng-surface-brand",
         ColorRole::SurfaceBrand},
        {"Border", "--ng-border", ColorRole::Border},
        {"Primary text", "--ng-text", ColorRole::Text},
        {"Secondary text", "--ng-text-secondary", ColorRole::TextSecondary},
        {"Links and focus", "--ng-link", ColorRole::Link},
    };
    for (int i = 0; i < int(sizeof(colors) / sizeof(colors[0])); ++i) {
        ColorTokenCard *card = new ColorTokenCard(
            tr(colors[i].title), QString::fromLatin1(colors[i].token),
            colors[i].role);
        m_colorCards.append(card);
        grid->addWidget(card, i / 4, i % 4);
    }
    for (int column = 0; column < 4; ++column)
        grid->setColumnStretch(column, 1);
    layout->addLayout(grid);
    layout->addSpacing(28);
    layout->addWidget(makeLabel(tr("Semantic states"),
                                QStringLiteral("panelHeading")));
    layout->addSpacing(8);
    layout->addWidget(makeLabel(
        tr("Text and matching background colors are shown together, as in "
           "notices. Select a value to copy it."),
        QStringLiteral("muted"), true));
    layout->addSpacing(16);

    QGridLayout *semanticGrid = new QGridLayout;
    semanticGrid->setSpacing(12);
    semanticGrid->addWidget(
        createSemanticCard(tr("Information"), QStringLiteral("--ng-link"),
                           ColorRole::Link,
                           QStringLiteral("--ng-surface-brand"),
                           ColorRole::SurfaceBrand, SemanticTone::Information),
        0, 0);
    semanticGrid->addWidget(
        createSemanticCard(tr("Success"), QStringLiteral("--ng-success"),
                           ColorRole::Success,
                           QStringLiteral("--ng-success-soft"),
                           ColorRole::SuccessSoft, SemanticTone::Success),
        0, 1);
    semanticGrid->addWidget(
        createSemanticCard(tr("Warning"),
                           QStringLiteral("--ng-warning"), ColorRole::Warning,
                           QStringLiteral("--ng-warning-soft"),
                           ColorRole::WarningSoft, SemanticTone::Warning),
        0, 2);
    semanticGrid->addWidget(
        createSemanticCard(tr("Error"), QStringLiteral("--ng-danger"),
                           ColorRole::Danger,
                           QStringLiteral("--ng-danger-soft"),
                           ColorRole::DangerSoft, SemanticTone::Danger),
        0, 3);
    layout->addLayout(semanticGrid);
    layout->addSpacing(18);
    layout->addWidget(nestedPanel(
        tr("Implementation rules"),
        {
            tr("#0C65AF remains the primary button fill in both themes."),
            tr("#69B8ED is used for links, focus indicators, and fine icons "
               "on dark backgrounds."),
            tr("Hierarchy comes from backgrounds, borders, and accents; a "
               "permanent shadow does not replace surface levels."),
        }));
    return section;
}

ExpandableSection *GalleryWindow::createTypographySection()
{
    ExpandableSection *section = new ExpandableSection(
        tr("Typography"),
        tr("Ubuntu for headings, Roboto for controls and body copy."),
        IconRole::Information);
    section->setIconText(QStringLiteral("Aa"));
    QVBoxLayout *layout = section->contentLayout();
    addSectionIntro(
        layout,
        tr("Ubuntu and Roboto are bundled locally in WOFF2 format. The "
           "document preserves its typography and works entirely offline."));

    QFrame *stage = new QFrame;
    stage->setProperty("_ngstdRole", QStringLiteral("sampleStage"));
    QVBoxLayout *stageLayout = new QVBoxLayout(stage);
    const int stagePadding =
        GalleryTokens::metric(GalleryMetric::TypeStagePadding);
    stageLayout->setContentsMargins(stagePadding, stagePadding, stagePadding,
                                    stagePadding);
    stageLayout->setSpacing(0);
    QLabel *display =
        makeLabel(QStringLiteral("Spatial data\nwithout visual noise"),
                  QStringLiteral("typeStageDisplay"));
    QFont stageDisplayFont = DesignTokens::font(TypographyRole::Display);
    stageDisplayFont.setPixelSize(
        GalleryTokens::metric(GalleryMetric::TypeStageDisplaySize));
    stageDisplayFont.setWeight(QFont::Bold);
    stageDisplayFont.setLetterSpacing(QFont::PercentageSpacing, 97.0);
    display->setFont(stageDisplayFont);
    QLabel *body = makeLabel(
        tr("Headings stay compact and distinctive, while long labels, "
           "tables, forms, and menus use neutral Roboto."),
        QStringLiteral("heroLead"), true);
    body->setMaximumWidth(720);
    stageLayout->addWidget(display);
    stageLayout->addSpacing(18);
    stageLayout->addWidget(body);
    layout->addWidget(stage);
    layout->addSpacing(20);

    Card *scale = new Card;
    QVBoxLayout *scaleLayout = scale->contentLayout();
    scaleLayout->setContentsMargins(1, 1, 1, 1);
    scaleLayout->setSpacing(0);
    const struct TypeEntry
    {
        TypographyRole role;
        const char *token;
        const char *sample;
        const char *metadata;
    } entries[] = {
        {TypographyRole::Display, "display", "Spatial data",
         "48 / 1.12 · 700 · Ubuntu"},
        {TypographyRole::Heading1, "h1", "Dataset catalog",
         "34 / 1.2 · 700 · Ubuntu"},
        {TypographyRole::Heading2, "h2", "Layer settings",
         "24 / 1.25 · 700 · Ubuntu"},
        {TypographyRole::Heading3, "h3", "Sources and filters",
         "18 / 1.35 · 500 · Ubuntu"},
        {TypographyRole::BodyLarge, "body-lg",
         "Publish maps, connect data, and share results.",
         "18 / 1.55 · 400 · Roboto"},
        {TypographyRole::Body, "body",
         "Interface body copy should stay calm and readable beside a map.",
         "16 / 1.5 · 400 · Roboto"},
        {TypographyRole::Control, "control", "Save changes",
         "14 / 1.4 · 500 · Roboto"},
        {TypographyRole::Caption, "caption", "Updated 5 minutes ago",
         "12 / 1.4 · 400 · Roboto"},
        {TypographyRole::Mono, "mono", "districts_2026",
         "12 · 400 · JetBrains Mono"},
    };
    const int entryCount = int(sizeof(entries) / sizeof(entries[0]));
    for (int entryIndex = 0; entryIndex < entryCount; ++entryIndex) {
        const TypeEntry &entry = entries[entryIndex];
        QFrame *row = new QFrame;
        row->setProperty("_ngstdRole", QStringLiteral("typeSpecimen"));
        row->setProperty("last", entryIndex == entryCount - 1);
        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(
            GalleryTokens::metric(
                GalleryMetric::TypeSpecimenPaddingHorizontal),
            GalleryTokens::metric(GalleryMetric::TypeSpecimenPaddingVertical),
            GalleryTokens::metric(
                GalleryMetric::TypeSpecimenPaddingHorizontal),
            GalleryTokens::metric(GalleryMetric::TypeSpecimenPaddingVertical));
        rowLayout->setSpacing(18);
        QLabel *token = makeTypographyLabel(QString::fromLatin1(entry.token),
                                            TypographyRole::Mono);
        token->setFixedWidth(
            GalleryTokens::metric(GalleryMetric::TypeSpecimenTokenWidth));
        QLabel *sample = makeTypographyLabel(tr(entry.sample), entry.role);
        QLabel *metadata = makeTypographyLabel(
            QString::fromUtf8(entry.metadata), TypographyRole::Caption);
        metadata->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        rowLayout->addWidget(token);
        rowLayout->addWidget(sample, 1);
        rowLayout->addWidget(metadata);
        scaleLayout->addWidget(row);
    }
    layout->addWidget(scale);
    layout->addSpacing(14);

    QHBoxLayout *references = new QHBoxLayout;
    references->setSpacing(10);
    const QStringList referenceTitles = {
        tr("Ubuntu ↗"),
        tr("Roboto ↗"),
        tr("Ubuntu license"),
        tr("Roboto license"),
    };
    for (const QString &referenceTitle : referenceTitles)
        references->addWidget(new Button(referenceTitle));
    references->addStretch();
    layout->addLayout(references);
    layout->addSpacing(18);
    layout->addWidget(nestedPanel(
        tr("Fonts and type scale"),
        {
            tr("48–64 px: promotional and onboarding screens on wide "
               "displays."),
            tr("34 px: main window or promotional heading."),
            tr("24/18 px: sections and cards; 16 px: body copy; 14 px: "
               "controls; 12 px: metadata."),
        }));
    return section;
}

ExpandableSection *GalleryWindow::createComponentsSection()
{
    ExpandableSection *section = new ExpandableSection(
        tr("Components"),
        tr("Buttons, fields, selectors, disclosures, notices, and cards."),
        IconRole::Check);
    section->setObjectName(QStringLiteral("componentsExpandableSection"));
    QVBoxLayout *layout = section->contentLayout();
    addSectionIntro(
        layout,
        tr("On hover, standard controls change color, background, and border "
           "without moving. Toggle buttons highlight their active state and "
           "confirm both transitions with an outline pulse. Lift is reserved "
           "for explicitly promotional components."));

    const QList<std::function<void()>> demoRegistry = {
        [this, layout]() { createStandardControlsDemo(layout); },
        [this, layout, section]() { createCardsDemo(layout, section); },
        [this, layout]() { createItemViewsDemo(layout); },
        [this, layout]() { createWizardDemo(layout); },
    };
    for (const std::function<void()> &createDemo : demoRegistry)
        createDemo();
    return section;
}

void GalleryWindow::createStandardControlsDemo(QVBoxLayout *layout)
{
    QGridLayout *board = new QGridLayout;
    const int boardSpacing =
        GalleryTokens::metric(GalleryMetric::ComponentGridSpacing);
    board->setHorizontalSpacing(boardSpacing);
    board->setVerticalSpacing(boardSpacing);

    QVBoxLayout *buttonsLayout = nullptr;
    QFrame *buttonsPanel = createComponentPanel(tr("Buttons"), &buttonsLayout);
    QWidget *buttonFlowWidget = new QWidget;
    buttonFlowWidget->setMinimumHeight(90);
    FlowLayout *buttonFlow = new FlowLayout(buttonFlowWidget, 0, 10, 10);
    const QList<QPair<ButtonVariant, QString>> buttons = {
        {ButtonVariant::Primary, tr("Primary")},
        {ButtonVariant::Secondary, tr("Secondary")},
        {ButtonVariant::Text, tr("Text")},
        {ButtonVariant::Danger, tr("Delete")},
        {ButtonVariant::Hero, tr("Promotional")},
    };
    for (const auto &entry : buttons) {
        Button *button = new Button(entry.second);
        button->setVariant(entry.first);
        if (entry.first == ButtonVariant::Hero)
            button->setObjectName(QStringLiteral("motionHeroButton"));
        if (entry.first == ButtonVariant::Primary)
            button->setIconRole(IconRole::Check);
        else if (entry.first == ButtonVariant::Secondary)
            button->setIconRole(IconRole::Layers);
        buttonFlow->addWidget(button);
    }
    Button *download = new Button;
    download->setVariant(ButtonVariant::Icon);
    download->setIconRole(IconRole::Download);
    buttonFlow->addWidget(download);
    Button *disabled = new Button(tr("Disabled"));
    disabled->setVariant(ButtonVariant::Secondary);
    disabled->setEnabled(false);
    buttonFlow->addWidget(disabled);
    Button *toggle = new Button(tr("Snap to grid"));
    toggle->setProperty("_ngstdGallerySuppressToast", true);
    toggle->setVariant(ButtonVariant::Secondary);
    toggle->setCheckable(true);
    buttonFlow->addWidget(toggle);
    Button *loading = new Button(tr("Loading"));
    loading->setVariant(ButtonVariant::Secondary);
    loading->setLoading(true);
    buttonFlow->addWidget(loading);
    buttonsLayout->addWidget(buttonFlowWidget);
    buttonsLayout->addSpacing(18);

    Button *trialButton = new Button(tr("Try Premium"));
    trialButton->setVariant(ButtonVariant::Trial);
    trialButton->setObjectName(QStringLiteral("motionTrialButton"));
    buttonsLayout->addWidget(createProductActionPanel(
        tr("Premium promotional button"),
        tr("Primary action for a free trial offer."),
        {trialButton}));
    buttonsLayout->addSpacing(10);

    Button *dataFilled = new Button(tr("Primary"));
    dataFilled->setVariant(ButtonVariant::DataFilled);
    Button *dataOutline = new Button(tr("Secondary"));
    dataOutline->setVariant(ButtonVariant::DataOutline);
    buttonsLayout->addWidget(createProductActionPanel(
        tr("NextGIS Data buttons"),
        tr("Primary and secondary actions on a product promotion screen."),
        {dataFilled, dataOutline}));

    QVBoxLayout *fieldsLayout = nullptr;
    QFrame *fieldsPanel = createComponentPanel(tr("Fields"), &fieldsLayout);
    QSpinBox *objectCount = new QSpinBox;
    objectCount->setRange(0, 1000000);
    objectCount->setSingleStep(100);
    objectCount->setValue(12500);
    QDoubleSpinBox *gridStep = new QDoubleSpinBox;
    gridStep->setRange(0.1, 1000.0);
    gridStep->setDecimals(1);
    gridStep->setSingleStep(0.5);
    gridStep->setSuffix(tr(" m"));
    gridStep->setValue(5.0);
    QCheckBox *cacheCheckBox = new QCheckBox(tr("Use local cache"));
    cacheCheckBox->setChecked(true);
    QWidget *radioOptions = new QWidget;
    QHBoxLayout *radioLayout = new QHBoxLayout(radioOptions);
    radioLayout->setContentsMargins(0, 0, 0, 0);
    radioLayout->setSpacing(16);
    QRadioButton *recommended = new QRadioButton(tr("Recommended"));
    QRadioButton *custom = new QRadioButton(tr("Custom"));
    recommended->setChecked(true);
    radioLayout->addWidget(recommended);
    radioLayout->addWidget(custom);
    radioLayout->addStretch();
    QLineEdit *layerName = new QLineEdit(tr("District boundaries"));
    ComboBox *source = new ComboBox;
    source->setObjectName(QStringLiteral("componentsSourceComboBox"));
    SearchField *search = new SearchField;
    const QList<QPair<QString, QWidget *>> fields = {
        {tr("Layer name"), layerName},
        {tr("Source"), source},
        {tr("Search datasets"), search},
        {tr("Feature count"), objectCount},
        {tr("Grid step"), gridStep},
        {tr("Options"), cacheCheckBox},
        {tr("Mode"), radioOptions},
    };
    source->addItems({
        tr("NextGIS Data"),
        tr("Local file"),
        tr("Connected service"),
    });
    search->setPlaceholderText(tr("Address, layer, or category"));
    for (const auto &field : fields) {
        fieldsLayout->addWidget(
            makeTypographyLabel(field.first, TypographyRole::Caption));
        fieldsLayout->addSpacing(6);
        fieldsLayout->addWidget(field.second);
        fieldsLayout->addSpacing(12);
    }
    fieldsLayout->addStretch();

    QVBoxLayout *tagsLayout = nullptr;
    QFrame *tagsPanel =
        createComponentPanel(tr("Tags and tabs"), &tagsLayout);
    QWidget *tagFlowWidget = new QWidget;
    FlowLayout *tagFlow = new FlowLayout(tagFlowWidget, 0, 8, 8);
    const QList<QPair<SemanticTone, QString>> tags = {
        {SemanticTone::Neutral, tr("Vector")},
        {SemanticTone::Information, tr("NextGIS Data")},
        {SemanticTone::Success, tr("Ready")},
        {SemanticTone::Warning, tr("Review")},
        {SemanticTone::Danger, tr("Error")},
        {SemanticTone::Warning, tr("Popular")},
    };
    for (const auto &entry : tags) {
        Tag *tag = new Tag(entry.second);
        tag->setTone(entry.first);
        if (entry.second == tr("Popular"))
            tag->setIconRole(IconRole::Flame);
        tagFlow->addWidget(tag);
    }
    tagsLayout->addWidget(tagFlowWidget);
    tagsLayout->addSpacing(18);
    QTabWidget *tabs = new QTabWidget;
    tabs->setObjectName(QStringLiteral("componentsTabs"));
    tabs->addTab(makeLabel(tr("Layer display settings on the map."),
                           QStringLiteral("secondary")),
                 tr("Map"));
    tabs->addTab(makeLabel(tr("Attribute table and source settings."),
                           QStringLiteral("secondary")),
                 tr("Data"));
    tabs->addTab(makeLabel(tr("Colors, labels, and visualization rules."),
                           QStringLiteral("secondary")),
                 tr("Style"));
    tagsLayout->addWidget(tabs);

    QVBoxLayout *noticesLayout = nullptr;
    QFrame *noticesPanel =
        createComponentPanel(tr("Notices"), &noticesLayout);
    const QList<QPair<SemanticTone, QPair<QString, QString>>> notices = {
        {SemanticTone::Information,
         {tr("Information."), tr("Changes take effect after saving.")}},
        {SemanticTone::Success, {tr("Success."), tr("Layer published.")}},
        {SemanticTone::Warning,
         {tr("Warning."), tr("Check the coordinate reference system.")}},
        {SemanticTone::Danger,
         {tr("Error."), tr("Could not connect to the source.")}},
    };
    for (const auto &entry : notices) {
        Notice *notice = new Notice;
        notice->setTone(entry.first);
        notice->setTitle(entry.second.first);
        notice->setText(entry.second.second);
        noticesLayout->addWidget(notice);
        noticesLayout->addSpacing(10);
    }

    QVBoxLayout *disclosureLayout = nullptr;
    QFrame *disclosurePanel =
        createComponentPanel(tr("Disclosure"), &disclosureLayout);
    disclosureLayout->addWidget(makeTypographyLabel(
        tr("Content expands and collapses smoothly; aria-expanded exposes "
           "the state."),
        TypographyRole::Caption, true));
    disclosureLayout->addSpacing(16);
    Disclosure *disclosure = new Disclosure;
    disclosure->setObjectName(QStringLiteral("componentsDisclosure"));
    disclosure->setTitle(tr("Layer options"));
    disclosure->setContentWidget(
        makeLabel(tr("Coordinate reference system, display scale, and cache "
                     "settings."),
                  QStringLiteral("secondary"), true));
    disclosureLayout->addWidget(disclosure);

    board->addWidget(buttonsPanel, 0, 0);
    board->addWidget(fieldsPanel, 0, 1);
    board->addWidget(tagsPanel, 1, 0);
    board->addWidget(noticesPanel, 1, 1);
    board->addWidget(disclosurePanel, 2, 0, 1, 2);
    board->setColumnStretch(0, 1);
    board->setColumnStretch(1, 1);
    layout->addLayout(board);
    layout->addSpacing(18);
    layout->addWidget(nestedPanel(
        tr("Sizes, radii, and states"),
        {
            tr("Standard control: 40–42 px; compact: 32 px; large: 48 px."),
            tr("Field: 6 px; button: 8 px; card: 10 px; large panel: "
               "12–16 px."),
            tr("A visible focus indicator is required for keyboard "
               "navigation and cannot be replaced by hover state."),
            tr("Lift is allowed only for the trial promotion button; NextGIS "
               "Data and NextGIS Toolbox cards remain stationary."),
        }));
}

void GalleryWindow::createCardsDemo(QVBoxLayout *layout, QObject *owner)
{
    layout->addSpacing(28);
    layout->addWidget(
        makeLabel(tr("Cards"), QStringLiteral("panelHeading")));
    layout->addSpacing(8);
    layout->addWidget(makeLabel(
        tr("Standard cards communicate state through border and background. "
           "NextGIS Data cards use a color fill and NextGIS Toolbox cards "
           "strengthen their shadow; geometry remains unchanged."),
        QStringLiteral("muted"), true));
    layout->addSpacing(16);

    QGridLayout *cards = new QGridLayout;
    cards->setSpacing(16);
    QWidget *iconCard = imageCard(
        tr("Card with icon"),
        tr("Suitable for a product, tool, or compact setting."),
        QString());
    cards->addWidget(iconCard, 0, 0, Qt::AlignTop);
    QWidget *mediaCard = imageCard(
        tr("Card with image"),
        tr("The image is separated from content to keep text legible."),
        QStringLiteral(":/ngstd/widgets/assets/cards/overture_card.webp"));
    cards->addWidget(mediaCard, 0, 1, Qt::AlignTop);
    QWidget *backgroundCard = imageCard(
        tr("Card with background"),
        tr("A gradient behind text ensures stable contrast."),
        QStringLiteral(":/ngstd/widgets/assets/cards/sat_card.webp"));
    cards->addWidget(backgroundCard, 0, 2, Qt::AlignTop);
    for (int column = 0; column < 3; ++column)
        cards->setColumnStretch(column, 1);
    layout->addLayout(cards);
    layout->addSpacing(28);

    layout->addWidget(
        makeLabel(tr("Selectable cards"), QStringLiteral("panelHeading")));
    layout->addSpacing(8);
    layout->addWidget(
        makeLabel(tr("Checkbox cards allow multiple selection; radio cards "
                     "form a mutually exclusive group."),
                  QStringLiteral("muted"), true));
    layout->addSpacing(16);
    const auto addSelectableContent = [](CardButton *card,
                                         const QString &title,
                                         const QString &description) {
        card->setAccessibleName(title);
        card->setAccessibleDescription(description);
        card->setTopWidget(makeLabel(title, QStringLiteral("panelHeading")));
        card->setBodyWidget(
            makeLabel(description, QStringLiteral("secondary"), true));
        card->contentLayout()->setSpacing(DesignTokens::spacing(2));
        card->contentLayout()->addStretch();
    };
    QGridLayout *selectableCards = new QGridLayout;
    selectableCards->setSpacing(16);
    CardButton *checkCard = new CardButton;
    addSelectableContent(
        checkCard, tr("Cache layer"),
        tr("An independent option that can be enabled with others."));
    checkCard->setChecked(true);
    CardButton *recommendedCard = new CardButton;
    recommendedCard->setAutoExclusive(true);
    addSelectableContent(
        recommendedCard, tr("Recommended installation"),
        tr("Standard component set for most users."));
    recommendedCard->setChecked(true);
    CardButton *customCard = new CardButton;
    customCard->setAutoExclusive(true);
    addSelectableContent(
        customCard, tr("Custom installation"),
        tr("Choose plugins and integrations manually."));
    QButtonGroup *installationGroup = new QButtonGroup(owner);
    installationGroup->setExclusive(true);
    installationGroup->addButton(recommendedCard);
    installationGroup->addButton(customCard);
    selectableCards->addWidget(checkCard, 0, 0);
    selectableCards->addWidget(recommendedCard, 0, 1);
    selectableCards->addWidget(customCard, 0, 2);
    for (int column = 0; column < 3; ++column)
        selectableCards->setColumnStretch(column, 1);
    layout->addLayout(selectableCards);
    layout->addSpacing(28);

    layout->addWidget(
        makeLabel(tr("Product cards"), QStringLiteral("panelHeading")));
    layout->addSpacing(8);
    layout->addWidget(
        makeLabel(tr("These examples reproduce the geometry, states, and "
                     "timing of NextGIS Data and NextGIS Toolbox."),
                  QStringLiteral("muted"), true));
    layout->addSpacing(16);

    const auto createDataCard = [this]() {
        CardButton *card = new CardButton;
        card->setAccessibleName(tr("Basemap"));
        card->setAccessibleDescription(
            tr("Daily updated vector map"));
        card->setVariant(CardVariant::Data);
        card->setMinimumHeight(360);
        QVBoxLayout *cardLayout = card->contentLayout();
        cardLayout->setContentsMargins(0, 0, 0, 0);
        cardLayout->setSpacing(0);

        QWidget *titleContainer = new QWidget;
        QVBoxLayout *titleLayout = new QVBoxLayout(titleContainer);
        titleLayout->setContentsMargins(16, 12, 16, 8);
        titleLayout->addWidget(
            makeLabel(tr("Basemap"), QStringLiteral("dataCardTitle")));
        cardLayout->addWidget(titleContainer);

        CoverImage *preview = new CoverImage(
            QStringLiteral(":/ngstd/widgets/assets/cards/base_card.webp"), 205,
            false);
        cardLayout->addWidget(preview);

        QWidget *descriptionContainer = new QWidget;
        QVBoxLayout *descriptionLayout = new QVBoxLayout(descriptionContainer);
        descriptionLayout->setContentsMargins(16, 16, 16, 16);
        descriptionLayout->addWidget(makeLabel(
            tr("A daily updated vector map with administrative boundaries, "
               "roads, buildings, water features, infrastructure, and other "
               "categories."),
            QStringLiteral("dataCardDescription"), true));
        descriptionLayout->addStretch();
        cardLayout->addWidget(descriptionContainer, 1);
        return card;
    };

    const auto createToolboxCard = [this](bool isNew) {
        CardButton *card = new CardButton;
        card->setAccessibleName(isNew ? tr("Convert vector layer")
                                      : tr("EXIF photos to Web GIS layer"));
        card->setVariant(isNew ? CardVariant::ToolboxNew
                               : CardVariant::Toolbox);
        card->setMinimumHeight(160);
        QVBoxLayout *cardLayout = card->contentLayout();
        cardLayout->setContentsMargins(16, 10, 16, 16);
        cardLayout->setSpacing(8);

        QHBoxLayout *tagLayout = new QHBoxLayout;
        tagLayout->setContentsMargins(0, 0, 0, 0);
        tagLayout->setSpacing(8);
        const QStringList tagTitles =
            isNew ? QStringList({
                        tr("Recommended 🔥"),
                        tr("Conversion"),
                        tr("Vector"),
                    })
                  : QStringList({tr("Photo"), tr("Web GIS")});
        for (int i = 0; i < tagTitles.size(); ++i) {
            QLabel *tag =
                makeLabel(tagTitles.at(i), QStringLiteral("toolboxTag"));
            if (isNew && i == 0) tag->setProperty("featured", true);
            tagLayout->addWidget(tag);
        }
        tagLayout->addStretch();
        cardLayout->addLayout(tagLayout);
        QLabel *titleLabel =
            makeLabel(isNew ? tr("Convert vector layer")
                            : tr("EXIF photos to Web GIS layer"),
                      QStringLiteral("toolboxCardTitle"), true);
        QFont titleFont = DesignTokens::font(TypographyRole::Heading2);
        titleFont.setPixelSize(22);
        titleFont.setWeight(QFont::Bold);
        titleFont.setLetterSpacing(QFont::AbsoluteSpacing, -0.5);
        titleLabel->setFont(titleFont);
        cardLayout->addWidget(titleLabel);
        cardLayout->addWidget(makeLabel(
            isNew ? tr("Converts a vector layer to the selected format.")
                  : tr("Converts georeferenced EXIF photos to a point vector "
                       "layer in NextGIS Web."),
            QStringLiteral("toolboxCardBody"), true));
        cardLayout->addStretch();
        return card;
    };

    QGridLayout *productCards = new QGridLayout;
    productCards->setHorizontalSpacing(16);
    productCards->setVerticalSpacing(10);
    const QStringList productLabels = {
        tr("NextGIS Data"),
        tr("NextGIS Toolbox"),
        tr("NextGIS Toolbox · new tool"),
    };
    for (int column = 0; column < productLabels.size(); ++column) {
        productCards->addWidget(makeLabel(productLabels.at(column),
                                          QStringLiteral("sourceCardLabel")),
                                0, column);
        productCards->setColumnStretch(column, 1);
    }
    productCards->addWidget(createDataCard(), 1, 0, Qt::AlignTop);
    productCards->addWidget(createToolboxCard(false), 1, 1, Qt::AlignTop);
    productCards->addWidget(createToolboxCard(true), 1, 2, Qt::AlignTop);
    layout->addLayout(productCards);
    layout->addSpacing(28);
}

void GalleryWindow::createItemViewsDemo(QVBoxLayout *layout)
{
    layout->addWidget(makeLabel(tr("Lists, trees, and tables"),
                                QStringLiteral("panelHeading")));
    layout->addSpacing(8);
    layout->addWidget(
        makeLabel(tr("Rows share common geometry and scrollbars. Any QWidget "
                     "can be placed in a table cell."),
                  QStringLiteral("muted"), true));
    layout->addSpacing(16);

    QGridLayout *viewGrid = new QGridLayout;
    viewGrid->setSpacing(16);
    QListWidget *listView = new QListWidget;
    listView->addItems({
        tr("Basemap"),
        tr("Satellite imagery"),
        tr("Digital elevation model"),
    });
    listView->setCurrentRow(0);
    listView->setMinimumHeight(150);
    QTreeWidget *treeView = new QTreeWidget;
    treeView->setHeaderHidden(true);
    QTreeWidgetItem *projectItem =
        new QTreeWidgetItem(treeView, QStringList(tr("Project")));
    new QTreeWidgetItem(projectItem, QStringList(tr("District boundaries")));
    new QTreeWidgetItem(projectItem, QStringList(tr("Road network")));
    projectItem->setExpanded(true);
    treeView->setCurrentItem(projectItem->child(0));
    treeView->setMinimumHeight(150);
    QVBoxLayout *listPanelLayout = nullptr;
    QFrame *listPanel =
        createComponentPanel(tr("List view"), &listPanelLayout);
    listPanelLayout->addWidget(listView);
    QVBoxLayout *treePanelLayout = nullptr;
    QFrame *treePanel =
        createComponentPanel(tr("Tree view"), &treePanelLayout);
    treePanelLayout->addWidget(treeView);
    viewGrid->addWidget(listPanel, 0, 0);
    viewGrid->addWidget(treePanel, 0, 1);
    viewGrid->setColumnStretch(0, 1);
    viewGrid->setColumnStretch(1, 1);
    layout->addLayout(viewGrid);
    layout->addSpacing(16);

    TableWidget *table = new TableWidget(2, 5);
    table->setObjectName(QStringLiteral("componentsTable"));
    table->setHorizontalHeaderLabels({
        tr("Layer"),
        tr("Enabled"),
        tr("Preview"),
        tr("State"),
        tr("Action"),
    });
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(
        2, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(
        4, QHeaderView::ResizeToContents);
    table->setMinimumHeight(150);
    const QStringList layerNames = {
        QStringLiteral("districts"),
        QStringLiteral("roads"),
    };
    for (int row = 0; row < layerNames.size(); ++row) {
        QLabel *layer = new QLabel(layerNames.at(row));
        WidgetStyle::setTypographyRole(layer, TypographyRole::Mono);
        table->setCellContent(row, 0, layer);
        QCheckBox *enabled = new QCheckBox;
        enabled->setChecked(row == 0);
        table->setCellContent(row, 1, enabled, Qt::AlignCenter);
        if (row == 0) {
            QLabel *preview = new QLabel;
            preview->setPixmap(
                QPixmap(QStringLiteral(
                            ":/ngstd/widgets/assets/cards/base_card.webp"))
                    .scaled(64, 34, Qt::KeepAspectRatioByExpanding,
                            Qt::SmoothTransformation));
            table->setCellContent(row, 2, preview, Qt::AlignCenter);
        }
        else {
            Spinner *cellSpinner = new Spinner;
            cellSpinner->setRunning(true);
            table->setCellContent(row, 2, cellSpinner, Qt::AlignCenter);
        }
        Tag *state = new Tag(row == 0 ? tr("Ready") : tr("Processing"));
        state->setTone(row == 0 ? SemanticTone::Success
                                : SemanticTone::Warning);
        table->setCellContent(row, 3, state, Qt::AlignCenter);
        Button *open = new Button(tr("Open"));
        open->setVariant(ButtonVariant::Text);
        table->setCellContent(row, 4, open, Qt::AlignCenter);
    }
    layout->addWidget(table);
    layout->addSpacing(28);
}

void GalleryWindow::createWizardDemo(QVBoxLayout *layout)
{
    QVBoxLayout *wizardLayout = nullptr;
    QFrame *wizardPanel = createComponentPanel(
        tr("QWizard, ButtonBox, and navigation"), &wizardLayout);
    wizardLayout->addWidget(makeLabel(
        tr("Back and Cancel use the secondary variant; Next, Commit, and "
           "Finish use primary. Pages created later are handled "
           "automatically."),
        QStringLiteral("secondary"), true));
    wizardLayout->addSpacing(14);
    Button *openWizardButton = new Button(tr("Open QWizard"));
    openWizardButton->setVariant(ButtonVariant::Primary);
    openWizardButton->setProperty("_ngstdGallerySuppressToast", true);
    connect(openWizardButton, &QPushButton::clicked, this,
            &GalleryWindow::openWizard);
    wizardLayout->addWidget(openWizardButton, 0, Qt::AlignLeft);
    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
                             QDialogButtonBox::Apply);
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    buttonBox->button(QDialogButtonBox::Apply)->setText(tr("Apply"));
    wizardLayout->addWidget(buttonBox);
    layout->addWidget(wizardPanel);
}

ExpandableSection *GalleryWindow::createBackgroundsSection()
{
    ExpandableSection *section = new ExpandableSection(
        tr("Page backgrounds"),
        tr("Production-ready backgrounds and decoration rules."),
        IconRole::Layers);
    QVBoxLayout *layout = section->contentLayout();
    addSectionIntro(
        layout,
        tr("Each background preserves its product's visual language. Use "
           "the controls below each example to inspect decorations, grids, "
           "and gradient direction."));

    QGridLayout *grid = new QGridLayout;
    grid->setSpacing(16);
    grid->addWidget(
        createBackgroundCard(
            tr("NextGIS home page"),
            tr("Create maps and\nwork with geospatial data"),
            tr("Cartographic lines and a coordinate grid create the "
               "recognizable NextGIS home page background."),
            PageBackgroundVariant::Main),
        0, 0);
    grid->addWidget(
        createBackgroundCard(
            tr("NextGIS Toolbox"), tr("Geospatial processing\ntools"),
            tr("A calm backdrop, topographic lines, and a soft gradient keep "
               "attention on finding the right tool."),
            PageBackgroundVariant::Toolbox),
        0, 1);
    grid->addWidget(
        createBackgroundCard(
            tr("NextGIS Data"), tr("Ready-to-use data for your GIS projects"),
            tr("A catalog of datasets for analysis, visualization, and map "
               "publishing."),
            PageBackgroundVariant::Data),
        1, 0, 1, 2);
    grid->addWidget(
        createBackgroundCard(
            tr("Workspace"), tr("Workspace"),
            tr("A neutral surface for catalogs, filters, and information-"
               "dense screens."),
            PageBackgroundVariant::Workspace),
        2, 0);
    grid->addWidget(
        createBackgroundCard(
            tr("Corporate panel"), tr("Corporate panel"),
            tr("A contrasting brand surface for corporate offers and key "
               "actions."),
            PageBackgroundVariant::Corporate),
        2, 1);
    grid->addWidget(
        createBackgroundCard(
            tr("Field data collection"), tr("Field data collection"),
            tr("A mobile app, configurable forms, and online or offline "
               "operation."),
            PageBackgroundVariant::Fieldwork),
        3, 0, 1, 2);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);
    layout->addLayout(grid);
    return section;
}

ExpandableSection *GalleryWindow::createBrandSection()
{
    ExpandableSection *section = new ExpandableSection(
        tr("Logos and brand assets"),
        tr("Primary mark, monochrome variants, and decorative assets."),
        IconRole::Download);
    QVBoxLayout *layout = section->contentLayout();
    addSectionIntro(
        layout,
        tr("Choose logos by surface contrast without changing colors or "
           "proportions. The library ships these assets through typed roles."));

    layout->addWidget(
        makeLabel(tr("Logos"), QStringLiteral("panelHeading")));
    layout->addSpacing(8);
    layout->addWidget(makeLabel(
        tr("Light and dark surfaces use dedicated variants so the mark "
           "retains its intended contrast."),
        QStringLiteral("muted"), true));
    layout->addSpacing(16);

    QGridLayout *logoGrid = new QGridLayout;
    logoGrid->setSpacing(16);
    const qreal ratio = devicePixelRatioF();
    logoGrid->addWidget(createAssetCard(tr("Primary two-color"),
                                        logoPixmap(LogoRole::Horizontal,
                                                   QSize(270, 72), ratio)),
                        0, 0);
    logoGrid->addWidget(createAssetCard(tr("Two-color on dark"),
                                        logoPixmap(LogoRole::HorizontalOnDark,
                                                   QSize(270, 72), ratio),
                                        true),
                        0, 1);
    logoGrid->addWidget(createAssetCard(tr("Monochrome blue"),
                                        logoPixmap(LogoRole::MonoBrand,
                                                   QSize(270, 72), ratio)),
                        0, 2);
    logoGrid->addWidget(
        createAssetCard(tr("Monochrome dark"),
                        logoPixmap(LogoRole::MonoDark, QSize(270, 72), ratio)),
        1, 0);
    logoGrid->addWidget(
        createAssetCard(tr("Monochrome light"),
                        logoPixmap(LogoRole::MonoLight, QSize(270, 72), ratio),
                        true),
        1, 1);
    for (int column = 0; column < 3; ++column)
        logoGrid->setColumnStretch(column, 1);
    layout->addLayout(logoGrid);
    layout->addSpacing(28);

    layout->addWidget(makeLabel(tr("Symbol and application icons"),
                                QStringLiteral("panelHeading")));
    layout->addSpacing(8);
    layout->addWidget(makeLabel(
        tr("The symbol follows the same contrast rules as the horizontal "
           "logo and always retains clear space around it."),
        QStringLiteral("muted"), true));
    layout->addSpacing(16);
    QGridLayout *symbolGrid = new QGridLayout;
    symbolGrid->setSpacing(16);
    symbolGrid->addWidget(
        createAssetCard(tr("Primary symbol"),
                        logoPixmap(LogoRole::Symbol, QSize(84, 84), ratio)),
        0, 0);
    symbolGrid->addWidget(createAssetCard(tr("Symbol on dark"),
                                          logoPixmap(LogoRole::SymbolOnDark,
                                                     QSize(84, 84), ratio),
                                          true),
                          0, 1);
    symbolGrid->addWidget(
        createAssetCard(tr("Light application icon"),
                        logoPixmap(LogoRole::Symbol, QSize(70, 70), ratio)),
        0, 2);
    symbolGrid->addWidget(createAssetCard(tr("Dark application icon"),
                                          logoPixmap(LogoRole::SymbolOnDark,
                                                     QSize(70, 70), ratio),
                                          true),
                          0, 3);
    for (int column = 0; column < 4; ++column)
        symbolGrid->setColumnStretch(column, 1);
    layout->addLayout(symbolGrid);
    layout->addSpacing(28);

    layout->addWidget(
        makeLabel(tr("Decorative graphics"),
                  QStringLiteral("panelHeading")));
    layout->addSpacing(8);
    layout->addWidget(makeLabel(
        tr("Contours, grids, and routes support product context without "
           "carrying standalone meaning."),
        QStringLiteral("muted"), true));
    layout->addSpacing(16);
    QGridLayout *decorationGrid = new QGridLayout;
    decorationGrid->setSpacing(16);
    const QList<QPair<QString, QString>> decorations = {
        {tr("Home page contours"),
         QStringLiteral(
             ":/ngstd/widgets/assets/backgrounds/for-mainpage-isolines.svg")},
        {tr("Toolbox topographic lines"),
         QStringLiteral(":/ngstd/widgets/assets/backgrounds/topo-bg.svg")},
        {tr("NextGIS Data route"),
         QStringLiteral(":/ngstd/widgets/assets/backgrounds/data-route.svg")},
        {tr("Home page illustration"),
         QStringLiteral(
             ":/ngstd/widgets/assets/backgrounds/main-right-pic.webp")},
    };
    for (int index = 0; index < decorations.size(); ++index) {
        const auto &decoration = decorations.at(index);
        decorationGrid->addWidget(
            createAssetCard(decoration.first,
                            QPixmap(decoration.second)
                                .scaled(240, 110, Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation),
                            false,
                            decoration.second.endsWith(QStringLiteral("webp"))
                                ? QStringLiteral("WEBP")
                                : QStringLiteral("SVG"),
                            true),
            index / 4, index % 4);
        decorationGrid->setColumnStretch(index % 4, 1);
    }
    layout->addLayout(decorationGrid);
    return section;
}

ExpandableSection *GalleryWindow::createIconsSection()
{
    ExpandableSection *section = new ExpandableSection(
        tr("Icon sets"),
        tr("Local examples and references to the source collections."),
        IconRole::Grid);
    QVBoxLayout *layout = section->contentLayout();
    addSectionIntro(
        layout,
        tr("Components receive icons through IconRole, allowing themes to "
           "recolor and select assets without string paths in application "
           "code."));

    struct IconSet
    {
        QString title;
        QString description;
        QList<IconRole> roles;
    };
    const QList<IconSet> iconSets = {
        {tr("Material Design Icons"),
         tr("Primary NextGIS Data icon set."),
         {IconRole::Menu, IconRole::Search, IconRole::Download,
          IconRole::ChevronDown}},
        {tr("Ant Design Icons"),
         tr("Compact utility icons for Toolbox."),
         {IconRole::Search, IconRole::Ellipsis, IconRole::CloseCircle,
          IconRole::Information}},
        {tr("Lucide"),
         tr("Neutral outline icons for the interface."),
         {IconRole::Copy, IconRole::Layers, IconRole::CheckCircle,
          IconRole::Flame}},
    };
    QGridLayout *grid = new QGridLayout;
    grid->setSpacing(16);
    for (int column = 0; column < iconSets.size(); ++column) {
        const IconSet &iconSet = iconSets.at(column);
        QVBoxLayout *panelLayout = nullptr;
        QFrame *panel = createComponentPanel(iconSet.title, &panelLayout);
        panelLayout->addWidget(
            makeLabel(iconSet.description, QStringLiteral("muted"), true));
        panelLayout->addSpacing(16);
        QHBoxLayout *icons = new QHBoxLayout;
        icons->setSpacing(10);
        for (IconRole role : iconSet.roles)
            icons->addWidget(createIconPreviewButton(role));
        icons->addStretch();
        panelLayout->addLayout(icons);
        panelLayout->addSpacing(16);
        Button *catalog = new Button(tr("Catalog"));
        catalog->setVariant(ButtonVariant::Secondary);
        panelLayout->addWidget(catalog, 0, Qt::AlignLeft);
        grid->addWidget(panel, 0, column);
        grid->setColumnStretch(column, 1);
    }
    layout->addLayout(grid);
    layout->addSpacing(16);
    Disclosure *rules = new Disclosure;
    rules->setTitle(tr("Size and color rules"));
    rules->setContentWidget(makeLabel(
        tr("Working size is 18 px and the interactive area is at least 40 px. "
           "Color comes from the current theme's semantic text role."),
        QStringLiteral("secondary"), true));
    layout->addWidget(rules);
    return section;
}

ExpandableSection *GalleryWindow::createMotionSection()
{
    ExpandableSection *section =
        new ExpandableSection(tr("Motion and states"),
                              tr("Brief feedback without shifting elements."),
                              IconRole::Gradient);
    QVBoxLayout *layout = section->contentLayout();
    addSectionIntro(
        layout,
        tr("Base durations cover hover, state changes, and disclosure. "
           "Reduced motion preserves the final state while removing "
           "intermediate frames."));

    QGridLayout *grid = new QGridLayout;
    grid->setSpacing(16);

    QVBoxLayout *stateLayout = nullptr;
    QFrame *statePanel =
        createComponentPanel(tr("Color state"), &stateLayout);
    stateLayout->addWidget(makeLabel(
        tr("Hover changes background and border without changing geometry."),
        QStringLiteral("muted"), true));
    stateLayout->addSpacing(16);
    Button *stateButton = new Button(tr("Check state"));
    stateButton->setObjectName(QStringLiteral("motionStateButton"));
    stateButton->setVariant(ButtonVariant::Primary);
    stateButton->setCheckable(true);
    stateLayout->addWidget(stateButton);
    grid->addWidget(statePanel, 0, 0);

    QVBoxLayout *progressLayout = nullptr;
    QFrame *progressPanel =
        createComponentPanel(tr("Progress"), &progressLayout);
    progressLayout->addWidget(makeLabel(
        tr("Starts from a user action and is never merely decorative."),
        QStringLiteral("muted"), true));
    progressLayout->addSpacing(16);
    QProgressBar *progress = new QProgressBar;
    progress->setObjectName(QStringLiteral("motionProgress"));
    progress->setRange(0, 100);
    progress->setValue(0);
    progress->setTextVisible(false);
    progressLayout->addWidget(progress);
    progressLayout->addSpacing(12);
    Button *start = new Button(tr("Start"));
    start->setVariant(ButtonVariant::Secondary);
    progressLayout->addWidget(start, 0, Qt::AlignLeft);
    QPropertyAnimation *progressAnimation =
        new QPropertyAnimation(progress, "value", progress);
    progressAnimation->setObjectName(
        QStringLiteral("motionProgressAnimation"));
    progressAnimation->setStartValue(0);
    progressAnimation->setEndValue(
        GalleryTokens::metric(GalleryMetric::MotionProgressTarget));
    MotionAdapter::configure(
        progressAnimation, progress,
        {MotionDuration::Slow, MotionEasing::Standard});
    connect(start, &QPushButton::clicked, progressAnimation,
            [progress, progressAnimation]() {
                progressAnimation->stop();
                if (MotionAdapter::configure(
                        progressAnimation, progress,
                        {MotionDuration::Slow, MotionEasing::Standard})) {
                    progressAnimation->start();
                }
                else {
                    progress->setValue(progressAnimation->endValue().toInt());
                }
            });
    grid->addWidget(progressPanel, 0, 1);

    QVBoxLayout *spinnerLayout = nullptr;
    QFrame *spinnerPanel =
        createComponentPanel(tr("Loading indicator"), &spinnerLayout);
    spinnerLayout->addWidget(makeLabel(
        tr("Use it beside a clear description of the current operation."),
        QStringLiteral("muted"), true));
    spinnerLayout->addSpacing(16);
    QHBoxLayout *spinnerRow = new QHBoxLayout;
    spinnerRow->setSpacing(
        DesignTokens::componentMetric(ComponentMetric::SpinnerLabelSpacing));
    Spinner *spinner = new Spinner;
    spinner->setObjectName(QStringLiteral("motionSpinner"));
    spinner->setRunning(true);
    spinnerRow->addStretch();
    spinnerRow->addWidget(spinner);
    spinnerRow->addWidget(makeLabel(tr("Loading layer…")));
    spinnerRow->addStretch();
    spinnerLayout->addLayout(spinnerRow);
    grid->addWidget(spinnerPanel, 0, 2);

    QVBoxLayout *disclosureLayout = nullptr;
    QFrame *disclosurePanel =
        createComponentPanel(tr("Disclosure"), &disclosureLayout);
    Disclosure *disclosure = new Disclosure;
    disclosure->setObjectName(QStringLiteral("motionDisclosure"));
    disclosure->setTitle(tr("Advanced options"));
    disclosure->setContentWidget(
        makeLabel(tr("RevealWidget changes height without rebuilding content "
                     "on every frame."),
                  QStringLiteral("secondary"), true));
    disclosureLayout->addWidget(disclosure);
    grid->addWidget(disclosurePanel, 1, 0);

    QVBoxLayout *comboLayout = nullptr;
    QFrame *comboPanel =
        createComponentPanel(tr("Combo box"), &comboLayout);
    ComboBox *combo = new ComboBox;
    combo->addItems(
        {tr("Open menu"), tr("First option"), tr("Second option")});
    comboLayout->addWidget(combo);
    grid->addWidget(comboPanel, 1, 1);

    QVBoxLayout *reducedLayout = nullptr;
    QFrame *reducedPanel =
        createComponentPanel(tr("Reduced motion"), &reducedLayout);
    reducedLayout->addWidget(
        makeLabel(tr("ThemeOptions defines the policy inherited by the entire "
                     "themed subtree."),
                  QStringLiteral("muted"), true));
    reducedLayout->addSpacing(16);
    Tag *supported = new Tag(tr("Supported"));
    supported->setTone(SemanticTone::Information);
    reducedLayout->addWidget(supported, 0, Qt::AlignLeft);
    grid->addWidget(reducedPanel, 1, 2);

    for (int column = 0; column < 3; ++column)
        grid->setColumnStretch(column, 1);
    layout->addLayout(grid);
    layout->addSpacing(16);
    Disclosure *forbidden = new Disclosure;
    forbidden->setTitle(tr("Forbidden patterns"));
    forbidden->setContentWidget(makeLabel(
        tr("Do not change layout, style sheet, or a component's primary icon "
           "on every animation frame."),
        QStringLiteral("secondary"), true));
    layout->addWidget(forbidden);
    return section;
}

ExpandableSection *GalleryWindow::createTokensSection()
{
    ExpandableSection *section = new ExpandableSection(
        tr("Tokens and Qt"),
        tr("Shared names for CSS variables, components, and Qt properties."),
        IconRole::Copy);
    QVBoxLayout *layout = section->contentLayout();
    addSectionIntro(
        layout,
        tr("The public system uses stable typed enums and semantic properties. "
           "Internal dynamic properties always start with _ngstd."));

    const auto createDefinitionPanel =
        [this](const QString &title, const QString &description,
               const QList<QPair<QString, QString>> &rows) {
            QVBoxLayout *panelLayout = nullptr;
            QFrame *panel = createComponentPanel(title, &panelLayout);
            panelLayout->addWidget(
                makeLabel(description, QStringLiteral("muted"), true));
            panelLayout->addSpacing(14);
            for (const auto &row : rows) {
                QFrame *line = new QFrame;
                line->setProperty("_ngstdRole", QStringLiteral("tokenRow"));
                QHBoxLayout *lineLayout = new QHBoxLayout(line);
                lineLayout->setContentsMargins(0, 10, 0, 10);
                lineLayout->setSpacing(12);
                QLabel *name =
                    makeTypographyLabel(row.first, TypographyRole::Caption);
                name->setProperty("_ngstdRole",
                                  QStringLiteral("tokenRowName"));
                name->setFixedWidth(170);
                QLabel *value = makeTypographyLabel(
                    row.second, TypographyRole::Mono, true);
                value->setProperty("_ngstdRole",
                                   QStringLiteral("tokenRowValue"));
                lineLayout->addWidget(name);
                lineLayout->addWidget(value, 1);
                panelLayout->addWidget(line);
            }
            panelLayout->addStretch();
            return panel;
        };

    QGridLayout *definitions = new QGridLayout;
    definitions->setSpacing(16);
    definitions->addWidget(
        createDefinitionPanel(
            tr("CSS variables"),
            tr("Names are grouped by role, not by a specific screen."),
            {{tr("Color"),
              QStringLiteral("--ng-brand, --ng-surface, --ng-text")},
             {tr("Typography"),
              QStringLiteral("--ng-font-heading, --ng-type-control-line")},
             {tr("Geometry"), QStringLiteral("--ng-space-*, --ng-radius-*")},
             {tr("Motion"),
              QStringLiteral("--ng-motion-fast, --ng-easing-standard")}}),
        0, 0);
    definitions->addWidget(
        createDefinitionPanel(
            tr("Components"),
            tr("Public names do not depend on gallery layout."),
            {{tr("Button"), QStringLiteral("Button, ButtonVariant")},
             {tr("Field"), QStringLiteral("SearchField, ComboBox")},
             {tr("Selection"), QStringLiteral("CardButton, QButtonGroup")},
             {tr("Disclosure"), QStringLiteral("Disclosure, RevealWidget")},
             {tr("Feedback"), QStringLiteral("Toast, Notice, Tag")}}),
        0, 1);
    definitions->addWidget(
        createDefinitionPanel(
            tr("Qt properties"),
            tr("QSS uses independent semantic properties."),
            {{QStringLiteral("ngstdButtonVariant"),
              QStringLiteral("primary, secondary, danger, icon")},
             {QStringLiteral("ngstdCardVariant"),
              QStringLiteral("data, toolbox, media, selectable")},
             {QStringLiteral("ngstdError / ngstdSelected"),
              tr("Independent logical states")},
             {QStringLiteral("ngstdTone"),
              QStringLiteral("info, success, warning, danger")}}),
        1, 0);
    definitions->addWidget(
        createDefinitionPanel(
            tr("Export"),
            tr("JSON remains canonical; runtime uses generated tables."),
            {{tr("Library version"), QStringLiteral("1.0.0")},
             {tr("JSON"), QStringLiteral("nextgis-tokens.json")},
             {tr("Qt light"), QStringLiteral("nextgis-light.qss")},
             {tr("Qt dark"), QStringLiteral("nextgis-dark.qss")}}),
        1, 1);
    definitions->setColumnStretch(0, 1);
    definitions->setColumnStretch(1, 1);
    layout->addLayout(definitions);
    layout->addSpacing(16);

    QGridLayout *codeGrid = new QGridLayout;
    codeGrid->setSpacing(16);
    const QList<QPair<QString, QString>> snippets = {
        {QStringLiteral("C++"),
         QStringLiteral("ThemeOptions options;\n"
                        "options.setThemeMode(ThemeMode::System);\n"
                        "ThemeController::attach(root, options);\n"
                        "WidgetStyle::setButtonVariant(button, "
                        "ButtonVariant::Primary);")},
        {QStringLiteral("Qt properties"),
         QStringLiteral(
             "button->setProperty(\"ngstdButtonVariant\", \"primary\");\n"
             "card->setProperty(\"ngstdSelected\", true);\n"
             "notice->setProperty(\"ngstdTone\", \"warning\");")},
    };
    for (int column = 0; column < snippets.size(); ++column) {
        QPlainTextEdit *code = new QPlainTextEdit(snippets.at(column).second);
        code->setProperty("_ngstdRole", QStringLiteral("codeBlock"));
        code->setReadOnly(true);
        code->setLineWrapMode(QPlainTextEdit::NoWrap);
        code->setFixedHeight(190);
        code->setAccessibleName(snippets.at(column).first);
        codeGrid->addWidget(code, 0, column);
        codeGrid->setColumnStretch(column, 1);
    }
    layout->addLayout(codeGrid);
    return section;
}

QFrame *GalleryWindow::createComponentPanel(const QString &title,
                                            QVBoxLayout **contentLayout)
{
    QFrame *panel = new QFrame;
    panel->setProperty("_ngstdRole", QStringLiteral("componentPanel"));
    QVBoxLayout *layout = new QVBoxLayout(panel);
    const int padding =
        GalleryTokens::metric(GalleryMetric::ComponentPanelPadding);
    layout->setContentsMargins(padding, padding, padding, padding);
    layout->setSpacing(0);
    layout->setAlignment(Qt::AlignTop);
    QLabel *heading = makeLabel(title, QStringLiteral("panelHeading"));
    heading->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    layout->addWidget(heading);
    layout->addSpacing(16);
    if (contentLayout) *contentLayout = layout;
    return panel;
}

QWidget *GalleryWindow::createSemanticCard(const QString &title,
                                           const QString &textToken,
                                           ColorRole textRole,
                                           const QString &backgroundToken,
                                           ColorRole backgroundRole,
                                           SemanticTone tone)
{
    SemanticStateCard *card = new SemanticStateCard(
        title, textToken, textRole, backgroundToken, backgroundRole, tone);
    m_semanticCards.append(card);
    return card;
}

QWidget *GalleryWindow::createBackgroundCard(const QString &title,
                                             const QString &heading,
                                             const QString &description,
                                             PageBackgroundVariant variant)
{
    Card *card = new Card;
    card->setVariant(CardVariant::Panel);
    QVBoxLayout *cardLayout = card->contentLayout();
    cardLayout->setContentsMargins(1, 1, 1, 1);
    cardLayout->setSpacing(0);

    PageBackground *background = new PageBackground;
    background->setVariant(variant);
    background->setCornerMode(PageBackgroundCornerMode::RoundedTop);
    background->setFixedHeight(
        GalleryTokens::metric(GalleryMetric::BackgroundDemoHeight));
    QVBoxLayout *backgroundLayout = new QVBoxLayout(background);
    backgroundLayout->setContentsMargins(
        variant == PageBackgroundVariant::Data ? 96 : 64, 32, 32, 32);
    backgroundLayout->setSpacing(0);
    backgroundLayout->addStretch(2);
    QLabel *headingLabel =
        makeTypographyLabel(heading, TypographyRole::Heading2, true);
    headingLabel->setMaximumWidth(430);
    const bool usesOnImageText = variant == PageBackgroundVariant::Corporate ||
                                 variant == PageBackgroundVariant::Fieldwork;
    QLabel *descriptionLabel =
        makeLabel(description,
                  usesOnImageText ? QStringLiteral("onImageBody")
                                  : QStringLiteral("secondary"),
                  true);
    descriptionLabel->setMaximumWidth(430);
    backgroundLayout->addWidget(headingLabel);
    backgroundLayout->addSpacing(10);
    backgroundLayout->addWidget(descriptionLabel);
    backgroundLayout->addSpacing(20);
    QHBoxLayout *actions = new QHBoxLayout;
    actions->setSpacing(10);
    QString primaryText = tr("Primary action");
    QString secondaryText = tr("Secondary action");
    if (variant == PageBackgroundVariant::Main) {
        primaryText = tr("Open map");
        secondaryText = tr("Documentation");
    }
    else if (variant == PageBackgroundVariant::Toolbox) {
        primaryText = tr("Find a tool");
        secondaryText = tr("All categories");
    }
    else if (variant == PageBackgroundVariant::Data) {
        primaryText = tr("Primary");
        secondaryText = tr("Secondary");
    }
    else if (variant == PageBackgroundVariant::Workspace) {
        primaryText = tr("Create dataset");
        secondaryText = tr("Open catalog");
    }
    else if (variant == PageBackgroundVariant::Corporate) {
        primaryText = tr("Learn more");
        secondaryText = tr("Contact us");
    }
    else if (variant == PageBackgroundVariant::Fieldwork) {
        primaryText = tr("Learn more");
        secondaryText = tr("View example");
    }
    Button *primary = new Button(primaryText);
    Button *secondary = new Button(secondaryText);
    if (variant == PageBackgroundVariant::Data) {
        primary->setVariant(ButtonVariant::DataFilled);
        secondary->setVariant(ButtonVariant::DataOutline);
    }
    else if (variant == PageBackgroundVariant::Corporate) {
        primary->setVariant(ButtonVariant::OnBrand);
        secondary->setVariant(ButtonVariant::OnBrandSecondary);
    }
    else if (variant == PageBackgroundVariant::Fieldwork) {
        primary->setVariant(ButtonVariant::Photo);
        secondary->setVariant(ButtonVariant::PhotoText);
    }
    else {
        primary->setVariant(ButtonVariant::Primary);
        secondary->setVariant(ButtonVariant::Secondary);
    }
    actions->addWidget(primary);
    actions->addWidget(secondary);
    actions->addStretch();
    backgroundLayout->addLayout(actions);
    backgroundLayout->addStretch(3);

    QFrame *caption = new QFrame;
    caption->setProperty("_ngstdRole", QStringLiteral("backgroundCaption"));
    caption->setFixedHeight(
        GalleryTokens::metric(GalleryMetric::BackgroundCaptionHeight));
    QHBoxLayout *captionLayout = new QHBoxLayout(caption);
    captionLayout->setContentsMargins(16, 0, 16, 0);
    QLabel *captionTitle = makeTypographyLabel(title, TypographyRole::Control);
    captionLayout->addWidget(captionTitle);
    captionLayout->addStretch();

    const auto addToggle = [captionLayout,
                            background](IconRole iconRole, bool controlsGrid) {
        Button *button = new Button;
        button->setProperty("_ngstdGallerySuppressToast", true);
        button->setVariant(ButtonVariant::Icon);
        button->setIconRole(iconRole);
        button->setCheckable(true);
        QObject::connect(button, &QPushButton::toggled, background,
                         [background, controlsGrid](bool visible) {
                             if (controlsGrid)
                                 background->setGridVisible(visible);
                             else
                                 background->setDecorationVisible(visible);
                         });
        button->setChecked(controlsGrid ? background->isGridVisible()
                                        : background->isDecorationVisible());
        captionLayout->addWidget(button);
    };
    const auto addGradientSelector = [captionLayout, background]() {
        ComboBox *selector = new ComboBox;
        selector->setFixedWidth(168);
        selector->addItems({
            QObject::tr("No gradient"),
            QObject::tr("Top to bottom"),
            QObject::tr("Bottom to top"),
            QObject::tr("From center"),
            QObject::tr("To center"),
        });
        selector->setCurrentIndex(static_cast<int>(background->gradient()));
        selector->setPopupPlacement(ComboBoxPopupPlacement::Above);
        selector->setPopupAlignment(ComboBoxPopupAlignment::Right);
        QObject::connect(selector,
                         qOverload<int>(&QComboBox::currentIndexChanged),
                         background, [background](int index) {
                             background->setGradient(
                                 static_cast<PageBackgroundGradient>(index));
                         });
        captionLayout->addWidget(selector);
    };

    if (variant == PageBackgroundVariant::Main) {
        addToggle(IconRole::Layers, false);
        addToggle(IconRole::Grid, true);
        addGradientSelector();
    }
    else if (variant == PageBackgroundVariant::Toolbox) {
        addToggle(IconRole::Layers, false);
        addGradientSelector();
    }
    else if (variant == PageBackgroundVariant::Data) {
        addToggle(IconRole::Layers, false);
    }
    else if (variant == PageBackgroundVariant::Workspace) {
        addToggle(IconRole::Grid, true);
        addGradientSelector();
    }
    else if (variant == PageBackgroundVariant::Corporate) {
        addToggle(IconRole::Grid, true);
        addGradientSelector();
    }

    cardLayout->addWidget(background);
    cardLayout->addWidget(caption);
    return card;
}

void GalleryWindow::addNavigationLink(QGridLayout *layout, int index,
                                      const QString &title,
                                      ExpandableSection *target)
{
    if (!layout) return;
    NavigationLink *link = new NavigationLink(
        index, title, [this, target]() { scrollToSection(target); });
    if (!target) {
        link->setEnabled(false);
        link->setCursor(Qt::ArrowCursor);
        link->setAccessibleDescription(
            tr("This section is outside the current gallery."));
    }
    const int itemIndex = index - 1;
    layout->addWidget(link, 1 + itemIndex / 3, itemIndex % 3);
}

void GalleryWindow::scrollToPosition(int value)
{
    QScrollBar *scrollBar = m_scrollArea->verticalScrollBar();
    const int targetValue =
        qBound(scrollBar->minimum(), value, scrollBar->maximum());
    if (m_scrollAnimation) {
        m_scrollAnimation->stop();
        m_scrollAnimation->deleteLater();
    }
    m_scrollAnimation = new QPropertyAnimation(scrollBar, "value", this);
    m_scrollAnimation->setStartValue(scrollBar->value());
    m_scrollAnimation->setEndValue(targetValue);
    const bool animated = MotionAdapter::configure(
        m_scrollAnimation, this,
        {MotionDuration::Normal, MotionEasing::Enter});
    connect(m_scrollAnimation, &QPropertyAnimation::finished, this, [this]() {
        if (!m_scrollAnimation) return;
        m_scrollAnimation->deleteLater();
        m_scrollAnimation = nullptr;
    });
    if (animated) {
        m_scrollAnimation->start();
    }
    else {
        scrollBar->setValue(targetValue);
        m_scrollAnimation->deleteLater();
        m_scrollAnimation = nullptr;
    }
}

void GalleryWindow::scrollToSection(ExpandableSection *target)
{
    if (!target) return;
    target->setExpanded(true, false);
    target->updateGeometry();
    if (m_page->layout()) m_page->layout()->activate();
    QTimer::singleShot(0, this,
                       [this, target]() { positionAtSection(target); });
}

void GalleryWindow::positionAtSection(ExpandableSection *target)
{
    if (!target || !m_scrollArea->widget()) return;
    if (m_page->layout()) m_page->layout()->activate();
    const int offset =
        GalleryTokens::value({
                                 QStringLiteral("desktop"),
                                 QStringLiteral("gallery"),
                                 QStringLiteral("navigationScrollOffsetPx"),
                             })
            .toInt();
    const int sectionTop =
        target->mapTo(m_scrollArea->widget(), QPoint(0, 0)).y();
    if (m_initialLayoutPending)
        m_scrollArea->verticalScrollBar()->setValue(sectionTop - offset);
    else
        scrollToPosition(sectionTop - offset);
}

void GalleryWindow::applyTheme()
{
    if (!m_themeRoot) return;
    m_themeMode = m_themeSwitch->themeMode();

    if (m_themeController) {
        m_themeController->setThemeMode(m_themeMode);
        m_colorScheme = m_themeController->colorScheme();
        updateThemeVisuals();
        return;
    }

    ThemeOptions options;
    options.setThemeMode(m_themeMode);
    options.setAnimationPolicy(m_animationPolicy);
    m_themeController = ThemeController::attach(this, options);
    if (!m_themeController) return;
    m_colorScheme = m_themeController->colorScheme();
    connect(m_themeController, &ThemeController::colorSchemeChanged, this,
            [this](ColorScheme scheme) {
                m_colorScheme = scheme;
                updateThemeVisuals();
            });
    updateThemeVisuals();
}

void GalleryWindow::updateThemeVisuals()
{
    if (m_themeRoot) {
        m_themeRoot->ensurePolished();
        const QString marker =
            QStringLiteral("\n/* ngstd::widgets gallery theme */\n");
        QString styleSheet = m_themeRoot->styleSheet();
        const auto markerPosition = styleSheet.indexOf(marker);
        if (markerPosition >= 0) styleSheet.truncate(markerPosition);
        m_themeRoot->setStyleSheet(styleSheet + marker +
                                   galleryStyleSheet(m_colorScheme));
    }
    const QList<QLabel *> labels = findChildren<QLabel *>();
    for (QLabel *label : labels) {
        if (!label->property("ngThemeLogo").toBool()) continue;
        label->setPixmap(logoPixmap(m_colorScheme == ColorScheme::Dark
                                        ? LogoRole::HorizontalOnDark
                                        : LogoRole::Horizontal,
                                    QSize(152, 21), devicePixelRatioF()));
    }
    for (HeroPanel *hero : m_heroPanels)
        hero->setColorScheme(m_colorScheme);
    for (ColorTokenCard *card : m_colorCards)
        card->setColorScheme(m_colorScheme);
    for (SemanticStateCard *card : m_semanticCards)
        card->setColorScheme(m_colorScheme);
    const QList<QAction *> actions = findChildren<QAction *>();
    for (QAction *action : actions) {
        const QVariant iconProperty = action->property("ngThemeIcon");
        if (!iconProperty.isValid()) continue;
        action->setIcon(QIcon(iconPixmap(
            static_cast<IconRole>(iconProperty.toInt()), QSize(18, 18),
            devicePixelRatioF(),
            DesignTokens::color(ColorRole::TextMuted, m_colorScheme))));
    }
    updateThemeButtons();
}

void GalleryWindow::updateThemeButtons()
{
    if (m_themeSwitch) m_themeSwitch->setColorScheme(m_colorScheme);
}

void GalleryWindow::openWizard()
{
    DemoWizard *wizard = new DemoWizard(this);
    wizard->setAttribute(Qt::WA_DeleteOnClose, true);
    WizardAdapter::attach(wizard);
    wizard->open();
}

void GalleryWindow::setAllSectionsExpanded(bool expanded)
{
    if (!m_scrollArea) return;
    if (m_scrollAnimation) {
        m_scrollAnimation->stop();
        m_scrollAnimation->deleteLater();
        m_scrollAnimation = nullptr;
    }
    QScrollBar *scrollBar = m_scrollArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->minimum());
    for (ExpandableSection *section : m_sections)
        section->setExpanded(expanded, true);
    scrollBar->setValue(scrollBar->minimum());
    const int duration = DesignTokens::duration(MotionDuration::Slow);
    QPointer<QScrollBar> guardedScrollBar(scrollBar);
    QTimer::singleShot(duration + 20, this, [guardedScrollBar]() {
        if (guardedScrollBar)
            guardedScrollBar->setValue(guardedScrollBar->minimum());
    });
}
