/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Table and progress view components
 *****************************************************************************/
#include <ngstd/widgets/view_components.h>

#include "component_utils_p.h"

#include <ngstd/widgets/design_tokens.h>

#include <QElapsedTimer>
#include <QEvent>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPainter>
#include <QPointer>
#include <QTimer>
#include <QTimerEvent>
#include <QVariant>
#include <QVector>
#include <QtMath>

namespace {

class TableBorderOverlay final : public QWidget
{
public:
    explicit TableBorderOverlay(QWidget *parent) : QWidget(parent)
    {
        setProperty("_ngstdRole", QStringLiteral("tableBorderOverlay"));
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)
        using namespace ngstd::widgets;
        const qreal strokeWidth = 1.0;
        const QRectF frameRect =
            QRectF(rect()).adjusted(strokeWidth * 0.5, strokeWidth * 0.5,
                                    -strokeWidth * 0.5, -strokeWidth * 0.5);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(
            QPen(DesignTokens::color(ColorRole::Border,
                                     internal::colorSchemeFor(this)),
                 strokeWidth));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(frameRect,
                                DesignTokens::radius(RadiusRole::Card),
                                DesignTokens::radius(RadiusRole::Card));
    }
};

class TableCellHost final : public QWidget
{
public:
    TableCellHost(QWidget *content, Qt::Alignment alignment, QWidget *parent)
        : QWidget(parent), m_content(content)
    {
        setProperty("_ngstdRole", QStringLiteral("tableCellHost"));
        setAttribute(Qt::WA_StyledBackground, true);
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(
            ngstd::widgets::DesignTokens::componentMetric(
                ngstd::widgets::ComponentMetric::TableCellPaddingHorizontal),
            ngstd::widgets::DesignTokens::componentMetric(
                ngstd::widgets::ComponentMetric::TableCellPaddingVertical),
            ngstd::widgets::DesignTokens::componentMetric(
                ngstd::widgets::ComponentMetric::TableCellPaddingHorizontal),
            ngstd::widgets::DesignTokens::componentMetric(
                ngstd::widgets::ComponentMetric::TableCellPaddingVertical));
        layout->addWidget(content, 0, alignment);
    }

    QWidget *content() const
    {
        return m_content.data();
    }

    QWidget *takeContent()
    {
        QWidget *contentWidget = m_content.data();
        if (!contentWidget) return nullptr;
        layout()->removeWidget(contentWidget);
        contentWidget->setParent(nullptr);
        m_content.clear();
        return contentWidget;
    }

    QSize sizeHint() const override
    {
        return layout() ? layout()->sizeHint() : QWidget::sizeHint();
    }

    QSize minimumSizeHint() const override
    {
        return layout() ? layout()->minimumSize() : QWidget::minimumSizeHint();
    }

private:
    QPointer<QWidget> m_content;
};

} // namespace

namespace ngstd {
namespace widgets {

class TableWidgetPrivate final
{
public:
    QWidget *borderOverlay = nullptr;
};

TableWidget::TableWidget(QWidget *parent)
    : QTableWidget(parent), d(new TableWidgetPrivate)
{
    initialize();
}

TableWidget::TableWidget(int rows, int columns, QWidget *parent)
    : QTableWidget(rows, columns, parent), d(new TableWidgetPrivate)
{
    initialize();
}

TableWidget::~TableWidget() = default;

void TableWidget::initialize()
{
    setProperty("_ngstdRole", QStringLiteral("table"));
    setProperty("_ngstdPaintedTable", true);
    viewport()->setProperty("_ngstdRole", QStringLiteral("tableViewport"));
    setShowGrid(false);
    verticalHeader()->setVisible(false);
    const int rowHeight =
        DesignTokens::componentMetric(ComponentMetric::TableRowHeight);
    verticalHeader()->setMinimumSectionSize(rowHeight);
    verticalHeader()->setDefaultSectionSize(rowHeight);
    horizontalHeader()->setMinimumHeight(
        DesignTokens::componentMetric(ComponentMetric::TableHeaderHeight));
    d->borderOverlay = new TableBorderOverlay(this);
    d->borderOverlay->setGeometry(rect());
    d->borderOverlay->raise();
}

void TableWidget::setCellContent(int row, int column, QWidget *widget,
                                 Qt::Alignment alignment)
{
    if (!widget) {
        removeCellWidget(row, column);
        return;
    }
    TableCellHost *host = new TableCellHost(widget, alignment, this);
    QTableWidgetItem *cellItem = item(row, column);
    if (!cellItem) {
        cellItem = new QTableWidgetItem;
        cellItem->setFlags(Qt::NoItemFlags);
        setItem(row, column, cellItem);
    }
    const QSize hostSize =
        host->sizeHint().expandedTo(host->minimumSizeHint());
    const QSize sectionSize = hostSize + QSize(2, 2);
    cellItem->setSizeHint(sectionSize);
    setCellWidget(row, column, host);
    setRowHeight(row, qMax(DesignTokens::componentMetric(
                               ComponentMetric::TableRowHeight),
                           sectionSize.height()));
    if (horizontalHeader()->sectionResizeMode(column) ==
        QHeaderView::ResizeToContents) {
        resizeColumnToContents(column);
        horizontalHeader()->resizeSection(
            column, qMax(horizontalHeader()->sectionSize(column),
                         sectionSize.width()));
    }
}

QWidget *TableWidget::cellContent(int row, int column) const
{
    QWidget *hostWidget = cellWidget(row, column);
    TableCellHost *host = dynamic_cast<TableCellHost *>(hostWidget);
    return host ? host->content() : hostWidget;
}

QWidget *TableWidget::takeCellContent(int row, int column)
{
    QWidget *hostWidget = cellWidget(row, column);
    TableCellHost *host = dynamic_cast<TableCellHost *>(hostWidget);
    if (!host) return nullptr;
    QWidget *contentWidget = host->takeContent();
    removeCellWidget(row, column);
    return contentWidget;
}

bool TableWidget::event(QEvent *event)
{
    const bool handled = QTableWidget::event(event);
    if (event->type() == QEvent::StyleChange ||
        event->type() == QEvent::FontChange || event->type() == QEvent::Show) {
        QPointer<TableWidget> guardedTable(this);
        QTimer::singleShot(0, this, [guardedTable]() {
            if (guardedTable) guardedTable->updateCellGeometry();
        });
    }
    return handled;
}

void TableWidget::updateCellGeometry()
{
    QVector<int> minimumColumnWidths(columnCount(), 0);
    for (int row = 0; row < rowCount(); ++row) {
        int rowHeight =
            DesignTokens::componentMetric(ComponentMetric::TableRowHeight);
        for (int column = 0; column < columnCount(); ++column) {
            QWidget *host = cellWidget(row, column);
            if (!host) continue;
            if (host->layout()) {
                host->layout()->invalidate();
                host->layout()->activate();
            }
            const QSize cellSize =
                host->sizeHint().expandedTo(host->minimumSizeHint());
            const QSize sectionSize = cellSize + QSize(2, 2);
            minimumColumnWidths[column] =
                qMax(minimumColumnWidths.at(column), sectionSize.width());
            rowHeight = qMax(rowHeight, sectionSize.height());
            QTableWidgetItem *cellItem = item(row, column);
            if (cellItem) cellItem->setSizeHint(sectionSize);
        }
        setRowHeight(row, rowHeight);
    }
    for (int column = 0; column < columnCount(); ++column) {
        if (horizontalHeader()->sectionResizeMode(column) ==
            QHeaderView::ResizeToContents) {
            resizeColumnToContents(column);
            horizontalHeader()->resizeSection(
                column,
                qMax(horizontalHeader()->sectionSize(column),
                     minimumColumnWidths.at(column)));
        }
    }
    QTableWidget::doItemsLayout();
    for (int row = 0; row < rowCount(); ++row) {
        for (int column = 0; column < columnCount(); ++column) {
            QWidget *host = cellWidget(row, column);
            if (!host || !host->layout()) continue;
            host->layout()->invalidate();
            host->layout()->activate();
        }
    }
}

void TableWidget::resizeEvent(QResizeEvent *event)
{
    QTableWidget::resizeEvent(event);
    if (!d->borderOverlay) return;
    d->borderOverlay->setGeometry(rect());
    d->borderOverlay->raise();
}

int TableWidget::sizeHintForColumn(int column) const
{
    int result = QTableWidget::sizeHintForColumn(column);
    for (int row = 0; row < rowCount(); ++row) {
        QWidget *host = cellWidget(row, column);
        if (!host) continue;
        const QSize hostSize =
            host->sizeHint().expandedTo(host->minimumSizeHint());
        result = qMax(result, hostSize.width() + 2);
    }
    return result;
}

class SpinnerPrivate final
{
public:
    QElapsedTimer elapsedTimer;
    qreal phase = 0.0;
    qreal startPhase = 0.0;
    int timerId = 0;
    bool running = false;
};

Spinner::Spinner(QWidget *parent) : QWidget(parent), d(new SpinnerPrivate)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

Spinner::~Spinner()
{
    if (d->timerId != 0) killTimer(d->timerId);
}

bool Spinner::isRunning() const
{
    return d->running;
}

void Spinner::setRunning(bool running)
{
    if (d->running == running) return;
    d->running = running;
    if (d->running) {
        d->startPhase = d->phase;
        d->elapsedTimer.start();
        d->timerId = startTimer(16, Qt::PreciseTimer);
    }
    else if (d->timerId != 0) {
        killTimer(d->timerId);
        d->timerId = 0;
    }
    update();
    emit runningChanged(d->running);
}

void Spinner::stop()
{
    setRunning(false);
}

qreal Spinner::phase() const
{
    return d->phase;
}

void Spinner::setPhase(qreal phase)
{
    qreal normalizedPhase = phase - qFloor(phase);
    if (normalizedPhase < 0.0) normalizedPhase += 1.0;
    if (qFuzzyCompare(d->phase, normalizedPhase)) return;
    d->phase = normalizedPhase;
    update();
    emit phaseChanged(d->phase);
}

void Spinner::resetPhase()
{
    setPhase(0.0);
}

QSize Spinner::sizeHint() const
{
    const int size =
        DesignTokens::componentMetric(ComponentMetric::SpinnerSize);
    return QSize(size, size);
}

QSize Spinner::minimumSizeHint() const
{
    return sizeHint();
}

void Spinner::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    const ColorScheme scheme = internal::colorSchemeFor(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    const qreal strokeWidth =
        DesignTokens::componentMetric(ComponentMetric::SpinnerStrokeWidth);
    const QRectF spinnerRectangle =
        QRectF(rect()).adjusted(strokeWidth * 0.5, strokeWidth * 0.5,
                                -strokeWidth * 0.5, -strokeWidth * 0.5);
    painter.setPen(QPen(DesignTokens::color(ColorRole::BrandSoft, scheme),
                        strokeWidth, Qt::SolidLine, Qt::FlatCap));
    painter.drawEllipse(spinnerRectangle);
    const int accentArc = internal::tokenInteger(
        QStringLiteral("desktop.component.spinner.accentArcDegrees"));
    const int accentCenter = internal::tokenInteger(
        QStringLiteral("desktop.component.spinner.accentCenterDegrees"));
    const QPointF ringCenter = spinnerRectangle.center();
    painter.translate(ringCenter);
    painter.rotate(d->phase * 360.0);
    painter.translate(-ringCenter);
    painter.setPen(QPen(DesignTokens::color(ColorRole::Link, scheme),
                        strokeWidth, Qt::SolidLine, Qt::FlatCap));
    painter.drawArc(spinnerRectangle, (accentCenter - accentArc / 2) * 16,
                    accentArc * 16);
}

void Spinner::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != d->timerId) {
        QWidget::timerEvent(event);
        return;
    }
    if (!internal::animationsEnabled(this)) {
        setPhase(0.0);
        return;
    }
    const qreal period =
        qMax(1, DesignTokens::componentMetric(ComponentMetric::SpinnerPeriod));
    setPhase(d->startPhase +
             static_cast<qreal>(d->elapsedTimer.elapsed()) / period);
}

} // namespace widgets
} // namespace ngstd
