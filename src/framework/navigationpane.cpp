/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: Framework library
*  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
*  Copyright (C) 2012-2018 NextGIS, info@nextgis.ru
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 2 of the License, or
*   (at your option) any later version.
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "navigationpane.h"

#include <QHBoxLayout>
#include <QToolButton>

////////////////////////////////////////////////////////////////////////////////
// NGNavigationPaneHolder
////////////////////////////////////////////////////////////////////////////////

NGNavigationPaneHolder::NGNavigationPaneHolder(NGNavigationWidget *parent) :
    QWidget(parent),
    m_currentWidget(nullptr)
{
    m_navigationComboBox = new QComboBox(this);
    m_navigationComboBox->setSizePolicy(QSizePolicy::Ignored,
                                        QSizePolicy::Ignored);
    m_navigationComboBox->setFocusPolicy(Qt::TabFocus);
    m_navigationComboBox->setMinimumContentsLength(0);
    m_navigationComboBox->setProperty("panelwidget", true);
    foreach (INGNavigationPane *pane, parent->panes()) {
        m_navigationComboBox->addItem(pane->icon(), pane->name());
    }

    NGStyledBar *toolBar = new NGStyledBar(this);
    QHBoxLayout *toolBarLayout = new QHBoxLayout;
    toolBarLayout->setMargin(0);
    toolBarLayout->setSpacing(0);
    toolBar->setLayout(toolBarLayout);
    toolBarLayout->addWidget(m_navigationComboBox);

    QToolButton *splitAction = new QToolButton();
    splitAction->setIcon(QIcon(":/icons/pagebreak.svg"));
    splitAction->setToolTip(tr("Split"));
    splitAction->setPopupMode(QToolButton::InstantPopup);
    splitAction->setProperty("noArrow", true);
    m_splitMenu = new QMenu(splitAction);
    splitAction->setMenu(m_splitMenu);
    connect(m_splitMenu, &QMenu::aboutToShow, this,
            &NGNavigationPaneHolder::populateSplitMenu);

    QToolButton *closeButton = new QToolButton();
    closeButton->setIcon(QIcon(":/icons/multiply.svg"));
    closeButton->setToolTip(tr("Close"));

    toolBarLayout->addWidget(splitAction);
    toolBarLayout->addWidget(closeButton);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);
    layout->addWidget(toolBar);

#if QT_VERSION >= 0x050900
    connect(m_navigationComboBox,
            QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
            this, &NGNavigationPaneHolder::comboBoxIndexChanged);
#endif
    connect(closeButton, &QAbstractButton::clicked, this,
            &NGNavigationPaneHolder::onClose);

    selectPane(m_navigationComboBox->currentText());
}

void NGNavigationPaneHolder::selectPane(const QString &name)
{
    NGNavigationWidget *navParent = static_cast<NGNavigationWidget *>(parent());
    INGNavigationPane *pane = navParent->paneByName(name);
    if(nullptr != pane) {
        QLayout *vbLayout = layout();
        QWidget *newWidget = pane->widget();
        if(m_currentWidget) {
            vbLayout->replaceWidget(m_currentWidget, newWidget);
            m_currentWidget->setParent(nullptr);
            delete m_currentWidget;
        }
        else {
            vbLayout->addWidget(newWidget);
        }

        m_currentWidget = newWidget;
        m_currentWidgetName = pane->name();
    }
}

void NGNavigationPaneHolder::selectComboboxItem(const QString &name)
{
    m_navigationComboBox->setCurrentText(name);
}

void NGNavigationPaneHolder::populateSplitMenu()
{
    m_splitMenu->clear();
    NGNavigationWidget *navParent = static_cast<NGNavigationWidget *>(parent());
    foreach (INGNavigationPane *pane, navParent->panes()) {
        QAction *action = m_splitMenu->addAction(pane->icon(), pane->name());
        action->setObjectName(pane->name()); // This need to work onSplit
        connect(action, &QAction::triggered, this, &NGNavigationPaneHolder::onSplit);
    }
}

void NGNavigationPaneHolder::onSplit()
{
    QObject *obj = sender();
    QString objName = obj->objectName();

    NGNavigationWidget *navParent = static_cast<NGNavigationWidget *>(parent());
    NGNavigationPaneHolder *holder = navParent->addPaneHolder();
    holder->selectComboboxItem(objName);
}

void NGNavigationPaneHolder::onClose()
{
    NGNavigationWidget *navParent = static_cast<NGNavigationWidget *>(parent());
    navParent->removePaneHolder(this);
}

void NGNavigationPaneHolder::comboBoxIndexChanged(const QString &text)
{
    selectPane(text);
}

QWidget *NGNavigationPaneHolder::currentWidget() const
{
    return m_currentWidget;
}

QString NGNavigationPaneHolder::currentWidgetName() const
{
    return m_currentWidgetName;
}
