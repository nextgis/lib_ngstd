/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: Framework library
*  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
*  Copyright (C) 2012-2019 NextGIS, info@nextgis.ru
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
#include "framework/application.h"

#include <QApplication>
#include <QDir>
#include <QPainter>
#include <QSettings>
#include <QStandardPaths>
#include <QThread>
#include <QTimer>

static QString gTheme = "light";

NGGUIApplication::NGGUIApplication(const QString &applicationName,
                                   const QString &version) :
    NGCoreApplication(applicationName, version),
    m_wnd(nullptr)
{

}

NGGUIApplication::~NGGUIApplication()
{
    delete m_wnd;
}

void NGGUIApplication::init(int &argc, char **argv)
{
    Q_INIT_RESOURCE(framework);

    NGCoreApplication::init(argc, argv);

    QApplication *pApp = static_cast<QApplication*>(m_app);
    pApp->setApplicationDisplayName(m_applicationName);
    pApp->setAttribute(Qt::AA_UseHighDpiPixmaps);

    // Set theme and style.
    QSettings settings;
    settings.beginGroup(QLatin1String("Common"));
    QString themeName = settings.value("theme", QLatin1String("light")).toString();//"dark" //
    settings.endGroup();
    setStyle(themeName);

    // Create main window.
    createMainWindow();

    QCommandLineParser parser;
    prepareCommandLineParser(parser);
    parser.process(*pApp);
    processCommandLine(parser);

    m_wnd->show();
}

QString NGGUIApplication::style()
{
    return gTheme;
}

const NGTheme *NGGUIApplication::theme()
{
    NGGUIApplication *app = static_cast<NGGUIApplication *>(NGCoreApplication::instance());
    QApplication *guiApp = static_cast<QApplication*>(app->m_app);
    NGStyle *style = static_cast<NGStyle *>(guiApp->style());
    return style->theme();
}

void NGGUIApplication::createApplication(int &argc, char **argv)
{
    if(m_app != nullptr)
        return;
    m_app = new QApplication(argc, argv);
}

void NGGUIApplication::createMainWindow()
{
    if(m_wnd) // only one main window
        return;
    m_wnd = new NGMainWindow();
    m_wnd->init();
}

QPixmap NGGUIApplication::createSplash(const QColor& bkColor)
{
    QPixmap splashPixmap(":/images/splash.svg");

    QPainter painter(&splashPixmap);

    setSplashBackground(painter, bkColor);

    QImage ng_logo(":/images/ng_logo_bw.svg");
    int nTextFieldWidth = 440;
    int nIconFieldWidth = 160;
    int nOff = 24;
    int ng_logo_h = 22;
    int ng_logo_w = static_cast<int>(float(ng_logo.width()) *
                                     float(ng_logo_h) / ng_logo.height());
    int nXLogoOff = nIconFieldWidth + (nTextFieldWidth - ng_logo_w) / 2;
    QRect ng_logo_rect(nXLogoOff,nOff, ng_logo_w, ng_logo_h);
    painter.drawImage(ng_logo_rect, ng_logo);

    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(font.pointSize() * 3);
    painter.setFont(font);
    painter.drawText(nIconFieldWidth, nOff + ng_logo_h, nTextFieldWidth, 70,
                     Qt::AlignCenter, m_applicationName);

    font.setPointSize(static_cast<int>(float(font.pointSize()) * 0.4f));
    painter.setFont(font);
    painter.drawText(nIconFieldWidth, nOff / 2 + ng_logo_h + 70,
                     nTextFieldWidth, nOff * 2, Qt::AlignCenter,
                     m_app->tr("Version") + ": " + m_version);

    painter.setPen(QColor(0,0,0,100));
    painter.setBrush(QBrush(QColor(0,0,0,100)));
    painter.drawRect(QRect(0,0,nIconFieldWidth,nIconFieldWidth));
    QImage logo(":/images/main_logo.svg");

    int nIcoOff = 34;
    int w = nIconFieldWidth - (nIcoOff + nIcoOff);
    painter.drawImage(QRect(nIcoOff,nIcoOff,w,w), logo);

    return splashPixmap;
}


void NGGUIApplication::setSplashBackground(QPainter& painter, const QColor &bkColor)
{
    if(bkColor.alpha() == 0)
        return;
    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    painter.setPen(QColor(0,0,0,100));
    painter.setBrush(QBrush(bkColor));
    //painter.setBrush(QBrush(QColor(255,106,0,100)));
    //painter.setBrush(QBrush(QColor(0,148,255,100)));
    painter.drawRect(QRect(0,0,600,160));

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
}

void NGGUIApplication::setStyle(const QString& themeName)
{
    gTheme = themeName;
    QString sBaseName("fusion");
    QString themeURI = QStringLiteral(":/themes/%1.theme").arg(themeName);
    QSettings themeSettings(themeURI, QSettings::IniFormat);
    NGTheme *theme = new NGTheme(themeName);
    theme->readSettings(themeSettings);

    NGStyle *pStyle = new NGStyle(sBaseName, theme);
    QApplication* pApp = static_cast<QApplication*>(m_app);

    if (theme->flag(NGTheme::ApplyThemePaletteGlobally)){
        pApp->setPalette(theme->palette());
    }

    pApp->setStyle(pStyle);
}
