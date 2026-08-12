/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Combo box with an owned, explicitly positioned popup
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_COMBO_BOX_H
#define NGSTD_WIDGETS_COMBO_BOX_H

#include <ngstd/widgets/types.h>

#include <QComboBox>
#include <QScopedPointer>

namespace ngstd {
namespace widgets {

class ComboBoxPrivate;

class NGSTD_WIDGETS_EXPORT ComboBox : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY(
        ngstd::widgets::ComboBoxPopupPlacement popupPlacement
        READ popupPlacement
        WRITE setPopupPlacement
        RESET resetPopupPlacement
        NOTIFY popupPlacementChanged)
    Q_PROPERTY(
        ngstd::widgets::ComboBoxPopupAlignment popupAlignment
        READ popupAlignment
        WRITE setPopupAlignment
        RESET resetPopupAlignment
        NOTIFY popupAlignmentChanged)

public:
    explicit ComboBox(QWidget *parent = nullptr);
    ~ComboBox() override;

    ComboBoxPopupPlacement popupPlacement() const;
    void setPopupPlacement(ComboBoxPopupPlacement placement);
    void resetPopupPlacement();

    ComboBoxPopupAlignment popupAlignment() const;
    void setPopupAlignment(ComboBoxPopupAlignment alignment);
    void resetPopupAlignment();

public slots:
    void hidePopup() override;
    void showPopup() override;

signals:
    void popupPlacementChanged(
        ngstd::widgets::ComboBoxPopupPlacement placement);
    void popupAlignmentChanged(
        ngstd::widgets::ComboBoxPopupAlignment alignment);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    NGSTD_WIDGETS_LOCAL void animateArrow(qreal target);
    NGSTD_WIDGETS_LOCAL void animatePopup(qreal target, bool hideWhenDone);

    Q_DISABLE_COPY(ComboBox)
    QScopedPointer<ComboBoxPrivate> d;
};

} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_COMBO_BOX_H
