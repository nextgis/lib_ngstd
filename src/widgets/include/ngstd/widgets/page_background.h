/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Reusable corporate page background component
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_PAGE_BACKGROUND_H
#define NGSTD_WIDGETS_PAGE_BACKGROUND_H

#include <ngstd/widgets/types.h>

#include <QFrame>
#include <QScopedPointer>

class QPaintEvent;

namespace ngstd {
namespace widgets {

class PageBackgroundPrivate;

class NGSTD_WIDGETS_EXPORT PageBackground : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(
        ngstd::widgets::PageBackgroundVariant variant
        READ variant
        WRITE setVariant
        RESET resetVariant
        NOTIFY variantChanged)
    Q_PROPERTY(
        bool decorationVisible
        READ isDecorationVisible
        WRITE setDecorationVisible
        RESET resetDecorationVisible
        NOTIFY decorationVisibleChanged)
    Q_PROPERTY(
        bool gridVisible
        READ isGridVisible
        WRITE setGridVisible
        RESET resetGridVisible
        NOTIFY gridVisibleChanged)
    Q_PROPERTY(
        ngstd::widgets::PageBackgroundGradient gradient
        READ gradient
        WRITE setGradient
        RESET resetGradient
        NOTIFY gradientChanged)
    Q_PROPERTY(
        ngstd::widgets::PageBackgroundCornerMode cornerMode
        READ cornerMode
        WRITE setCornerMode
        RESET resetCornerMode
        NOTIFY cornerModeChanged)

public:
    explicit PageBackground(QWidget *parent = nullptr);
    ~PageBackground() override;

    PageBackgroundVariant variant() const;
    void setVariant(PageBackgroundVariant variant);
    void resetVariant();

    bool isDecorationVisible() const;
    void setDecorationVisible(bool visible);
    void resetDecorationVisible();

    bool isGridVisible() const;
    void setGridVisible(bool visible);
    void resetGridVisible();

    PageBackgroundGradient gradient() const;
    void setGradient(PageBackgroundGradient gradient);
    void resetGradient();

    PageBackgroundCornerMode cornerMode() const;
    void setCornerMode(PageBackgroundCornerMode cornerMode);
    void resetCornerMode();

signals:
    void variantChanged(ngstd::widgets::PageBackgroundVariant variant);
    void decorationVisibleChanged(bool visible);
    void gridVisibleChanged(bool visible);
    void gradientChanged(ngstd::widgets::PageBackgroundGradient gradient);
    void cornerModeChanged(
        ngstd::widgets::PageBackgroundCornerMode cornerMode);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Q_DISABLE_COPY(PageBackground)
    QScopedPointer<PageBackgroundPrivate> d;
};

} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_PAGE_BACKGROUND_H
