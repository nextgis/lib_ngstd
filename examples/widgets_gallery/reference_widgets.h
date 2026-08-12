/******************************************************************************
 * Project: NextGIS widgets gallery
 * Purpose: Reference-specific layout and painted widgets
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_GALLERY_REFERENCE_WIDGETS_H
#define NGSTD_WIDGETS_GALLERY_REFERENCE_WIDGETS_H

#include <ngstd/widgets/components.h>
#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/widget_style.h>

#include <QAbstractButton>
#include <QFrame>
#include <QLayout>
#include <QList>
#include <QPixmap>
#include <QPointer>
#include <QStyle>
#include <QToolButton>

class QLabel;
class QVariantAnimation;

class FlowLayout final : public QLayout
{
public:
    explicit FlowLayout(QWidget *parent = nullptr, int margin = -1,
                        int horizontalSpacing = -1, int verticalSpacing = -1);
    ~FlowLayout() override;

    void addItem(QLayoutItem *item) override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QLayoutItem *takeAt(int index) override;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rectangle) override;
    QSize sizeHint() const override;

private:
    int doLayout(const QRect &rectangle, bool testOnly) const;
    int effectiveSpacing(QStyle::PixelMetric metric) const;

    QList<QLayoutItem *> m_items;
    int m_horizontalSpacing;
    int m_verticalSpacing;
};

class HeroPanel final : public QFrame
{
public:
    explicit HeroPanel(QWidget *parent = nullptr);

    void setColorScheme(ngstd::widgets::ColorScheme scheme);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    ngstd::widgets::ColorScheme m_scheme;
};

class ColorTokenCard final : public QAbstractButton
{
public:
    ColorTokenCard(const QString &title, const QString &tokenName,
                   ngstd::widgets::ColorRole role, QWidget *parent = nullptr);

    QSize sizeHint() const override;
    void setColorScheme(ngstd::widgets::ColorScheme scheme);

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_title;
    QString m_tokenName;
    ngstd::widgets::ColorRole m_role;
    ngstd::widgets::ColorScheme m_scheme;
    QVariantAnimation *m_stateAnimation;
    qreal m_stateProgress;
};

class SemanticCopyButton final : public QToolButton
{
public:
    SemanticCopyButton(const QString &tokenName,
                       ngstd::widgets::ColorRole colorRole,
                       QWidget *parent = nullptr,
                       bool showValueInTokenColor = true);

    void setColorScheme(ngstd::widgets::ColorScheme scheme);

private:
    void updateValue();

    QLabel *m_valueLabel;
    QLabel *m_copyIcon;
    QString m_tokenName;
    ngstd::widgets::ColorRole m_colorRole;
    ngstd::widgets::ColorScheme m_scheme;
    bool m_showValueInTokenColor;
};

class SemanticStateCard final : public QFrame
{
public:
    SemanticStateCard(const QString &title, const QString &textToken,
                      ngstd::widgets::ColorRole textRole,
                      const QString &backgroundToken,
                      ngstd::widgets::ColorRole backgroundRole,
                      ngstd::widgets::SemanticTone tone,
                      QWidget *parent = nullptr);

    void setColorScheme(ngstd::widgets::ColorScheme scheme);

private:
    QLabel *m_iconLabel;
    SemanticCopyButton *m_textButton;
    SemanticCopyButton *m_backgroundButton;
    ngstd::widgets::SemanticTone m_tone;
    ngstd::widgets::ColorScheme m_scheme;
};

#endif // NGSTD_WIDGETS_GALLERY_REFERENCE_WIDGETS_H
