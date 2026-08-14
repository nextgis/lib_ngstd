/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Reusable navigation panel and list components
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_NAVIGATION_COMPONENTS_H
#define NGSTD_WIDGETS_NAVIGATION_COMPONENTS_H

#include <ngstd/widgets/widgets.h>

#include <QFrame>
#include <QListWidget>

class QPaintEvent;
class QEvent;
class QListWidgetItem;

namespace ngstd {
namespace widgets {

class NGSTD_WIDGETS_EXPORT NavigationPanel : public QFrame
{
    Q_OBJECT

public:
    explicit NavigationPanel(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Q_DISABLE_COPY(NavigationPanel)
};

class NGSTD_WIDGETS_EXPORT NavigationListWidget : public QListWidget
{
    Q_OBJECT

public:
    enum class StepState : quint8
    {
        Pending,
        Current,
        Complete,
        Available
    };
    Q_ENUM(StepState)

    explicit NavigationListWidget(QWidget *parent = nullptr);

    StepState stepState(const QListWidgetItem *item) const;
    void setStepState(QListWidgetItem *item, StepState state);

protected:
    void changeEvent(QEvent *event) override;

private:
    NGSTD_WIDGETS_LOCAL void updateStepAppearance(QListWidgetItem *item);
    NGSTD_WIDGETS_LOCAL void updateStepAppearances();

    Q_DISABLE_COPY(NavigationListWidget)
};

} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_NAVIGATION_COMPONENTS_H
