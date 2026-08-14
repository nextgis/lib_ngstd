/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Reusable corporate button component
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_BUTTON_H
#define NGSTD_WIDGETS_BUTTON_H

#include <ngstd/widgets/types.h>

#include <QPushButton>
#include <QScopedPointer>

class QEvent;
class QPaintEvent;
class QResizeEvent;

namespace ngstd {
namespace widgets {

class ButtonPrivate;

class NGSTD_WIDGETS_EXPORT Button : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(
        ngstd::widgets::ButtonVariant variant
        READ variant
        WRITE setVariant
        RESET resetVariant
        NOTIFY variantChanged)
    Q_PROPERTY(
        bool loading
        READ isLoading
        WRITE setLoading
        RESET resetLoading
        NOTIFY loadingChanged)
    Q_PROPERTY(
        bool iconOnly
        READ iconOnly
        WRITE setIconOnly
        RESET resetIconOnly
        NOTIFY iconOnlyChanged)
    Q_PROPERTY(
        ngstd::widgets::ButtonIconPlacement iconPlacement
        READ iconPlacement
        WRITE setIconPlacement
        RESET resetIconPlacement
        NOTIFY iconPlacementChanged)

public:
    explicit Button(QWidget *parent = nullptr);
    explicit Button(const QString &text, QWidget *parent = nullptr);
    ~Button() override;

    ButtonVariant variant() const;
    void setVariant(ButtonVariant variant);
    void resetVariant();

    bool iconOnly() const;
    void setIconOnly(bool iconOnly);
    void resetIconOnly();

    IconRole iconRole() const;
    bool hasIconRole() const;
    void setIconRole(IconRole role);
    void clearIconRole();

    ButtonIconPlacement iconPlacement() const;
    void setIconPlacement(ButtonIconPlacement placement);
    void resetIconPlacement();

    LogoRole logoRole() const;
    QSize logoSize() const;
    bool hasLogoRole() const;
    void setLogoRole(LogoRole role, const QSize &logicalSize);
    void clearLogoRole();

    bool isLoading() const;
    void setLoading(bool loading);
    void resetLoading();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void variantChanged(ngstd::widgets::ButtonVariant variant);
    void iconOnlyChanged(bool iconOnly);
    void iconPlacementChanged(ngstd::widgets::ButtonIconPlacement placement);
    void loadingChanged(bool loading);

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    NGSTD_WIDGETS_LOCAL void initialize();
    NGSTD_WIDGETS_LOCAL void updateStateMotion();
    NGSTD_WIDGETS_LOCAL void updatePromotionalMotion();
    NGSTD_WIDGETS_LOCAL void updateTrialEffect();
    NGSTD_WIDGETS_LOCAL void updateIconSizePolicy();
    NGSTD_WIDGETS_LOCAL void updateRoleIcon();
    NGSTD_WIDGETS_LOCAL void updateLoadingGeometry();
    NGSTD_WIDGETS_LOCAL void startDataRipple(const QPointF &origin);
    NGSTD_WIDGETS_LOCAL void finishDataRipple();

    Q_DISABLE_COPY(Button)
    QScopedPointer<ButtonPrivate> d;
};

} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_BUTTON_H
