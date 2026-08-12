/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Reversible journal for theme-owned Qt state
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_STATE_JOURNAL_P_H
#define NGSTD_WIDGETS_STATE_JOURNAL_P_H

#include <QByteArray>
#include <QCursor>
#include <QFont>
#include <QHash>
#include <QIcon>
#include <QMetaObject>
#include <QPalette>
#include <QPointer>
#include <QSizePolicy>
#include <QString>
#include <QVariant>
#include <QVector>

class QApplication;
class QObject;
class QWidget;

namespace ngstd {
namespace widgets {
namespace internal {

class StateJournal final
{
public:
    StateJournal() = default;
    ~StateJournal();

    StateJournal(const StateJournal &) = delete;
    StateJournal &operator=(const StateJournal &) = delete;

    void captureApplication(QApplication *application);
    void captureWidget(QWidget *widget);
    bool containsWidget(QWidget *widget) const;
    void setProperty(QObject *object, const QByteArray &name,
                     const QVariant &value);
    void addConnection(const QMetaObject::Connection &connection);
    void restore();

private:
    struct ApplicationState
    {
        QPointer<QApplication> application;
        QString styleSheet;
        QPalette palette;
        QFont font;
    };

    struct WidgetState
    {
        QPointer<QWidget> widget;
        QString styleSheet;
        QPalette palette;
        QFont font;
        QSizePolicy sizePolicy;
        QCursor cursor;
        QIcon buttonIcon;
        QSize buttonIconSize;
        bool autoFillBackground = false;
        bool hadPalette = false;
        bool hadFont = false;
        bool hadCursor = false;
        bool abstractButton = false;
    };

    struct PropertyState
    {
        QPointer<QObject> object;
        QByteArray name;
        QVariant value;
        bool existed = false;
    };

    bool m_hasApplicationState = false;
    ApplicationState m_applicationState;
    QVector<WidgetState> m_widgetStates;
    QVector<PropertyState> m_propertyStates;
    QVector<QMetaObject::Connection> m_connections;
    QHash<QWidget *, qsizetype> m_widgetIndexes;
};

} // namespace internal
} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_STATE_JOURNAL_P_H
