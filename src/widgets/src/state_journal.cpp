/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Reversible journal for theme-owned Qt state
 *****************************************************************************/
#include "state_journal_p.h"

#include <QAbstractButton>
#include <QApplication>
#include <QObject>
#include <QVariant>
#include <QWidget>

#include <algorithm>

namespace ngstd {
namespace widgets {
namespace internal {

StateJournal::~StateJournal()
{
    restore();
}

void StateJournal::captureApplication(QApplication *application)
{
    if (!application || m_hasApplicationState) return;
    m_hasApplicationState = true;
    m_applicationState.application = application;
    m_applicationState.styleSheet = application->styleSheet();
    m_applicationState.palette = application->palette();
    m_applicationState.font = application->font();
}

void StateJournal::captureWidget(QWidget *widget)
{
    if (!widget || m_widgetIndexes.contains(widget)) return;
    WidgetState state;
    state.widget = widget;
    state.styleSheet = widget->styleSheet();
    state.palette = widget->palette();
    state.font = widget->font();
    state.sizePolicy = widget->sizePolicy();
    state.cursor = widget->cursor();
    state.autoFillBackground = widget->autoFillBackground();
    state.hadPalette = widget->testAttribute(Qt::WA_SetPalette);
    state.hadFont = widget->testAttribute(Qt::WA_SetFont);
    state.hadCursor = widget->testAttribute(Qt::WA_SetCursor);
    if (QAbstractButton *button = qobject_cast<QAbstractButton *>(widget)) {
        state.abstractButton = true;
        state.buttonIcon = button->icon();
        state.buttonIconSize = button->iconSize();
    }
    m_widgetIndexes.insert(widget, m_widgetStates.size());
    m_widgetStates.append(state);
}

bool StateJournal::containsWidget(QWidget *widget) const
{
    return widget && m_widgetIndexes.contains(widget);
}

void StateJournal::setProperty(QObject *object, const QByteArray &name,
                               const QVariant &value)
{
    if (!object || name.isEmpty()) return;
    const auto duplicate =
        std::find_if(m_propertyStates.cbegin(), m_propertyStates.cend(),
                     [object, &name](const PropertyState &state) {
                         return state.object == object && state.name == name;
                     });
    if (duplicate == m_propertyStates.cend()) {
        PropertyState state;
        state.object = object;
        state.name = name;
        state.value = object->property(name.constData());
        state.existed = object->dynamicPropertyNames().contains(name);
        m_propertyStates.append(state);
    }
    object->setProperty(name.constData(), value);
}

void StateJournal::addConnection(const QMetaObject::Connection &connection)
{
    m_connections.append(connection);
}

void StateJournal::restore()
{
    for (auto iterator = m_connections.crbegin();
         iterator != m_connections.crend(); ++iterator) {
        QObject::disconnect(*iterator);
    }
    m_connections.clear();

    for (auto iterator = m_propertyStates.crbegin();
         iterator != m_propertyStates.crend(); ++iterator) {
        if (!iterator->object) continue;
        iterator->object->setProperty(iterator->name.constData(),
                                      iterator->existed ? iterator->value
                                                        : QVariant());
    }
    m_propertyStates.clear();

    for (auto iterator = m_widgetStates.crbegin();
         iterator != m_widgetStates.crend(); ++iterator) {
        QWidget *widget = iterator->widget.data();
        if (!widget) continue;
        widget->setStyleSheet(iterator->styleSheet);
        if (iterator->hadPalette)
            widget->setPalette(iterator->palette);
        else
            widget->setPalette(QPalette());
        if (iterator->hadFont)
            widget->setFont(iterator->font);
        else
            widget->setFont(QFont());
        if (iterator->hadCursor)
            widget->setCursor(iterator->cursor);
        else
            widget->unsetCursor();
        widget->setSizePolicy(iterator->sizePolicy);
        widget->setAutoFillBackground(iterator->autoFillBackground);
        if (iterator->abstractButton) {
            QAbstractButton *button = qobject_cast<QAbstractButton *>(widget);
            if (button) {
                button->setIcon(iterator->buttonIcon);
                button->setIconSize(iterator->buttonIconSize);
            }
        }
        widget->update();
    }
    m_widgetStates.clear();
    m_widgetIndexes.clear();

    QApplication *application = m_applicationState.application.data();
    if (m_hasApplicationState && application) {
        application->setStyleSheet(m_applicationState.styleSheet);
        application->setPalette(m_applicationState.palette);
        application->setFont(m_applicationState.font);
    }
    m_applicationState = ApplicationState();
    m_hasApplicationState = false;
}

} // namespace internal
} // namespace widgets
} // namespace ngstd
