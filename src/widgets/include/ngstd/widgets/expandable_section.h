/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Reusable expandable corporate section
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_EXPANDABLE_SECTION_H
#define NGSTD_WIDGETS_EXPANDABLE_SECTION_H

#include <ngstd/widgets/types.h>

#include <QFrame>
#include <QScopedPointer>

class QPaintEvent;
class QVBoxLayout;

namespace ngstd {
namespace widgets {

class ExpandableSectionPrivate;

class NGSTD_WIDGETS_EXPORT ExpandableSection : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(
        QString description
        READ description
        WRITE setDescription
        NOTIFY descriptionChanged)
    Q_PROPERTY(
        ngstd::widgets::IconRole iconRole
        READ iconRole
        WRITE setIconRole
        NOTIFY iconRoleChanged)
    Q_PROPERTY(
        QString iconText
        READ iconText
        WRITE setIconText
        NOTIFY iconTextChanged)
    Q_PROPERTY(
        bool expanded
        READ isExpanded
        WRITE setExpanded
        RESET collapse
        NOTIFY expandedChanged)
    Q_PROPERTY(
        bool selectionVisible
        READ isSelectionVisible
        WRITE setSelectionVisible
        NOTIFY selectionVisibleChanged)
    Q_PROPERTY(
        bool selected
        READ isSelected
        WRITE setSelected
        NOTIFY selectedChanged)

public:
    explicit ExpandableSection(QWidget *parent = nullptr);
    ExpandableSection(const QString &title, const QString &description,
                      IconRole iconRole, QWidget *parent = nullptr);
    ~ExpandableSection() override;

    QString title() const;
    void setTitle(const QString &title);

    QString description() const;
    void setDescription(const QString &description);

    IconRole iconRole() const;
    void setIconRole(IconRole iconRole);

    QString iconText() const;
    void setIconText(const QString &iconText);

    bool isExpanded() const;
    void setExpanded(bool expanded);
    void setExpanded(bool expanded, bool animated);
    void collapse();

    bool isSelectionVisible() const;
    void setSelectionVisible(bool visible);

    bool isSelected() const;
    void setSelected(bool selected);

    QVBoxLayout *contentLayout() const;

signals:
    void titleChanged(const QString &title);
    void descriptionChanged(const QString &description);
    void iconRoleChanged(ngstd::widgets::IconRole iconRole);
    void iconTextChanged(const QString &iconText);
    void expandedChanged(bool expanded);
    void selectionVisibleChanged(bool visible);
    void selectedChanged(bool selected);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Q_DISABLE_COPY(ExpandableSection)
    QScopedPointer<ExpandableSectionPrivate> d;
};

} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_EXPANDABLE_SECTION_H
