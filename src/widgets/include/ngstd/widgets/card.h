/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Passive and interactive corporate card components
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_CARD_H
#define NGSTD_WIDGETS_CARD_H

#include <ngstd/widgets/types.h>

#include <QAbstractButton>
#include <QFrame>
#include <QPixmap>
#include <QPointF>
#include <QScopedPointer>

class QPaintEvent;
class QEvent;
class QVBoxLayout;

namespace ngstd {
namespace widgets {

class CardPrivate;
class CardButtonPrivate;

class NGSTD_WIDGETS_EXPORT Card : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(
        ngstd::widgets::CardVariant variant
        READ variant
        WRITE setVariant
        RESET resetVariant
        NOTIFY variantChanged)

public:
    explicit Card(QWidget *parent = nullptr);
    ~Card() override;

    CardVariant variant() const;
    void setVariant(CardVariant variant);
    void resetVariant();

    QVBoxLayout *contentLayout() const;
    QWidget *topWidget() const;
    void setTopWidget(QWidget *widget);
    QWidget *takeTopWidget();
    QWidget *bodyWidget() const;
    void setBodyWidget(QWidget *widget);
    QWidget *takeBodyWidget();

    QPixmap backgroundPixmap() const;
    void setBackgroundPixmap(const QPixmap &pixmap);
    void clearBackgroundPixmap();

signals:
    void variantChanged(ngstd::widgets::CardVariant variant);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Q_DISABLE_COPY(Card)
    QScopedPointer<CardPrivate> d;
};

class NGSTD_WIDGETS_EXPORT CardButton : public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(
        ngstd::widgets::CardVariant variant
        READ variant
        WRITE setVariant
        RESET resetVariant
        NOTIFY variantChanged)

public:
    explicit CardButton(QWidget *parent = nullptr);
    ~CardButton() override;

    CardVariant variant() const;
    void setVariant(CardVariant variant);
    void resetVariant();

    QVBoxLayout *contentLayout() const;
    QWidget *topWidget() const;
    void setTopWidget(QWidget *widget);
    QWidget *takeTopWidget();
    QWidget *bodyWidget() const;
    void setBodyWidget(QWidget *widget);
    QWidget *takeBodyWidget();

    QPixmap backgroundPixmap() const;
    void setBackgroundPixmap(const QPixmap &pixmap);
    void clearBackgroundPixmap();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void variantChanged(ngstd::widgets::CardVariant variant);

protected:
    bool event(QEvent *event) override;
    void changeEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    NGSTD_WIDGETS_LOCAL void startRipple(const QPointF &position);
    NGSTD_WIDGETS_LOCAL void finishRipple();

    Q_DISABLE_COPY(CardButton)
    QScopedPointer<CardButtonPrivate> d;
};

} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_CARD_H
