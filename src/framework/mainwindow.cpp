/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: Framework library
*  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
*  Copyright (C) 2012-2016 NextGIS, info@nextgis.ru
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
#include "mainwindow.h"

#include "core/application.h"

#include <QStatusBar>
#include <QSettings>
#include <QCloseEvent>
#include <QApplication>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenuBar>
#include <QToolBar>
#include <QMessageBox>

//FIXME: 5. add properties dialog (dynamic)

NGMainWindow::NGMainWindow(QWidget *parent) : QMainWindow(parent)
{
    statusBar()->showMessage(tr("Ready"));
}

void NGMainWindow::open()
{

}

void NGMainWindow::about()
{
}

void NGMainWindow::quit()
{
    QCoreApplication::quit();
}

void NGMainWindow::preferences()
{

}

void NGMainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    }
    else {
        event->ignore();
    }
}

QAction *NGMainWindow::commandByKey(const QString& key) const
{
    return m_commands[key];
}

bool NGMainWindow::maybeSave()
{
    return true;
}

void NGMainWindow::writeSettings()
{
    bool isStatusBarVisible = statusBar()->isVisible();
    QSettings settings;

    settings.beginGroup("MainWindow");
    if(isMaximized()){
        settings.setValue("frame.maximized", true);
    }
    else{
        settings.setValue("frame.size", size());
        settings.setValue("frame.pos", pos());
    }
    settings.setValue("frame.state", saveState());
    qDebug("Write to settings status bar is visible: %s", isStatusBarVisible ? "true" : "false");
    settings.setValue("frame.statusbar.shown", isStatusBarVisible);
    settings.endGroup();
}

void NGMainWindow::readSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    if(settings.value("frame.maximized", false).toBool()) {
        showMaximized();
    }
    else {
        resize(settings.value("frame.size", QSize(400, 400)).toSize());
        move(settings.value("frame.pos", QPoint(200, 200)).toPoint());
    }
    restoreState(settings.value("frame_state").toByteArray());
    qDebug("Status bar is shown: %s", settings.value("frame.statusbar.shown", true).toBool() ? "true" : "false");
    statusBar()->setVisible(settings.value("frame.statusbar.shown", true).toBool());
    settings.endGroup();
}

void NGMainWindow::init()
{
    createCommands();
    loadInterface();

    readSettings();
}

void NGMainWindow::createCommands()
{
    QAction *openAct = new QAction(QIcon(":/icons/open.svg"), tr("&Open"), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open project file"));
    connect(openAct, &QAction::triggered, this, &NGMainWindow::open);

    m_commands["file.open"] = openAct;

    QAction *quitAct = new QAction(tr("&Quit"), this);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit program"));
    quitAct->setMenuRole(QAction::QuitRole);
    connect(quitAct, &QAction::triggered, this, &NGMainWindow::quit);

    m_commands["file.quit"] = quitAct;

    QAction *aboutAct = new QAction(windowIcon(),
                                    tr("About") + " " +
                                    QCoreApplication::applicationName(), this);
    aboutAct->setStatusTip(tr("About program"));
    aboutAct->setMenuRole(QAction::AboutRole);
    connect(aboutAct, &QAction::triggered, this, &NGMainWindow::about);

    m_commands["help.about"] = aboutAct;

    QAction *prefAct = new QAction(QIcon(":/icons/settings.svg"), tr("&Preferences"), this);
    prefAct->setShortcuts(QKeySequence::Preferences);
    prefAct->setStatusTip(tr("Application preferences"));
    prefAct->setMenuRole(QAction::PreferencesRole);
    connect(prefAct, &QAction::triggered, this, &NGMainWindow::preferences);
    m_commands["options.preferences"] = prefAct;
}

void NGMainWindow::loadInterface()
{
    // Check if interface.json file exists
    NGCoreApplication *app = NGCoreApplication::instance();
    QString interfaceConfig = app->configDirectory() + QDir::separator() +
            "interface.json";

    // If not exists load from resources
    if(!QFileInfo::exists(interfaceConfig)) {
        interfaceConfig = ":/interface.json";
    }

    QFile jsonFile(interfaceConfig);
    jsonFile.open(QFile::ReadOnly);
    QJsonDocument iConfigDoc = QJsonDocument::fromJson(jsonFile.readAll());
    if(iConfigDoc.isObject()) {
        QJsonObject root = iConfigDoc.object();
        loadMenus(root[QLatin1String("menus")].toArray());
        loadToolbars(root[QLatin1String("toolbars")].toArray());
    }
}

void NGMainWindow::loadMenus(const QJsonArray& array)
{
    for(const QJsonValue &item : array) {
        QJsonObject itemObject = item.toObject();
        QString menuName = tr(qPrintable(itemObject[QLatin1String("name")].toString()));
        QMenu *menu = menuBar()->addMenu(menuName);
        QJsonArray actions = itemObject[QLatin1String("actions")].toArray();
        loadMenuActions(menu, actions);
    }
}

void NGMainWindow::loadMenuActions(QMenu *menu, const QJsonArray &array)
{
    for(const QJsonValue& action : array ) {
        QJsonObject actionObject = action.toObject();
        QString actionType = actionObject[QLatin1String("type")].toString();
        if(actionType == "separator") {
            menu->addSeparator();
        }
        else if(actionType == "menu") {
            QMenu *subMenu = new QMenu(tr(qPrintable(actionObject[QLatin1String("name")].toString())));
            QJsonArray subActions = actionObject[QLatin1String("actions")].toArray();
            loadMenuActions(subMenu, subActions);
            menu->addMenu(subMenu);
        }
        else if(actionType == "action") {
            QAction *command = commandByKey(actionObject[QLatin1String("key")].toString());
            if(nullptr != command) {
                menu->addAction(command);
            }
        }
    }
}

void NGMainWindow::loadToolbars(const QJsonArray& array)
{
    for(const QJsonValue &item : array) {
        QJsonObject itemObject = item.toObject();
        QString menuName = tr(qPrintable(itemObject[QLatin1String("name")].toString()));
        QJsonArray allowedAreas = itemObject[QLatin1String("allowedAreas")].toArray();
        Qt::ToolBarAreas allowedAreasMask = Qt::TopToolBarArea;
        if(allowedAreas.contains(QLatin1String("left"))) {
            allowedAreasMask |= Qt::LeftToolBarArea;
        }
        else if(allowedAreas.contains(QLatin1String("right"))) {
            allowedAreasMask |= Qt::RightToolBarArea;
        }
        else if(allowedAreas.contains(QLatin1String("bottom"))) {
            allowedAreasMask |= Qt::BottomToolBarArea;
        }
        QToolBar *toolBar = addToolBar(menuName);
        toolBar->setAllowedAreas(allowedAreasMask);
        QJsonArray actions = itemObject[QLatin1String("actions")].toArray();
        for(const QJsonValue &action : actions ) {
            QJsonObject actionObject = action.toObject();
            QString actionType = actionObject[QLatin1String("type")].toString();
            if(actionType == "separator") {
                toolBar->addSeparator();
            }
            else if(actionType == "action") {
                QAction *command = commandByKey(actionObject[QLatin1String("key")].toString());
                if(nullptr != command) {
                    toolBar->addAction(command);
                }
            }
        }
    }
}
