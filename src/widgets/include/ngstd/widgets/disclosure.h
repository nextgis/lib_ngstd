/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Reusable reveal and disclosure components
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_DISCLOSURE_H
#define NGSTD_WIDGETS_DISCLOSURE_H

#include <ngstd/widgets/widgets.h>

#include <QFrame>
#include <QScopedPointer>
#include <QWidget>

class QEvent;
class QPaintEvent;
class QResizeEvent;

namespace ngstd {
namespace widgets {

class RevealWidgetPrivate;
class DisclosurePrivate;

class NGSTD_WIDGETS_EXPORT RevealWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(
        bool expanded
        READ isExpanded
        WRITE setExpanded
        RESET collapse
        NOTIFY expandedChanged)
    Q_PROPERTY(
        qreal revealProgress
        READ revealProgress
        WRITE setRevealProgress
        NOTIFY revealProgressChanged)

public:
    explicit RevealWidget(QWidget *parent = nullptr);
    ~RevealWidget() override;

    QWidget *contentWidget() const;
    void setContentWidget(QWidget *contentWidget);
    QWidget *takeContentWidget();

    bool isExpanded() const;
    void setExpanded(bool expanded);
    void collapse();
    qreal revealProgress() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void contentHeightChanged(int height);
    void expandedChanged(bool expanded);
    void revealProgressChanged(qreal progress);

private:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    friend class Disclosure;

    NGSTD_WIDGETS_LOCAL void setRevealProgress(qreal progress);
    NGSTD_WIDGETS_LOCAL void refreshContentGeometry();

    Q_DISABLE_COPY(RevealWidget)
    QScopedPointer<RevealWidgetPrivate> d;
};

class NGSTD_WIDGETS_EXPORT Disclosure : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(
        bool expanded
        READ isExpanded
        WRITE setExpanded
        RESET collapse
        NOTIFY expandedChanged)

public:
    explicit Disclosure(QWidget *parent = nullptr);
    ~Disclosure() override;

    QString title() const;
    void setTitle(const QString &title);

    bool isExpanded() const;
    void setExpanded(bool expanded);
    void collapse();

    QWidget *contentWidget() const;
    void setContentWidget(QWidget *contentWidget);
    QWidget *takeContentWidget();
    RevealWidget *revealWidget() const;

signals:
    void titleChanged(const QString &title);
    void expandedChanged(bool expanded);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Q_DISABLE_COPY(Disclosure)
    QScopedPointer<DisclosurePrivate> d;
};

} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_DISCLOSURE_H
