/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Reusable table and progress components
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_VIEW_COMPONENTS_H
#define NGSTD_WIDGETS_VIEW_COMPONENTS_H

#include <ngstd/widgets/widgets.h>

#include <QScopedPointer>
#include <QTableWidget>
#include <QWidget>

class QEvent;
class QResizeEvent;
class QTimerEvent;

namespace ngstd {
namespace widgets {

class TableWidgetPrivate;
class SpinnerPrivate;

class NGSTD_WIDGETS_EXPORT TableWidget : public QTableWidget
{
    Q_OBJECT

public:
    explicit TableWidget(QWidget *parent = nullptr);
    TableWidget(int rows, int columns, QWidget *parent = nullptr);
    ~TableWidget() override;

    void setCellContent(int row, int column, QWidget *widget,
                        Qt::Alignment alignment = Qt::AlignLeft |
                                                  Qt::AlignVCenter);
    QWidget *cellContent(int row, int column) const;
    QWidget *takeCellContent(int row, int column);

protected:
    bool event(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    int sizeHintForColumn(int column) const override;

private:
    NGSTD_WIDGETS_LOCAL void initialize();
    NGSTD_WIDGETS_LOCAL void updateCellGeometry();

    Q_DISABLE_COPY(TableWidget)
    QScopedPointer<TableWidgetPrivate> d;
};

class NGSTD_WIDGETS_EXPORT Spinner : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(
        bool running
        READ isRunning
        WRITE setRunning
        RESET stop
        NOTIFY runningChanged)
    Q_PROPERTY(
        qreal phase
        READ phase
        WRITE setPhase
        RESET resetPhase
        NOTIFY phaseChanged)

public:
    explicit Spinner(QWidget *parent = nullptr);
    ~Spinner() override;

    bool isRunning() const;
    void setRunning(bool running);
    void stop();

    qreal phase() const;
    void setPhase(qreal phase);
    void resetPhase();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void runningChanged(bool running);
    void phaseChanged(qreal phase);

protected:
    void paintEvent(QPaintEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private:
    Q_DISABLE_COPY(Spinner)
    QScopedPointer<SpinnerPrivate> d;
};

} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_VIEW_COMPONENTS_H
