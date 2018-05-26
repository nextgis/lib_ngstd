/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: Core Library
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
#ifndef NGSTD_COREAPPLICATION_H
#define NGSTD_COREAPPLICATION_H

#include "core/core.h"

#if QT_VERSION >= 0x050000

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QString>
#include <QVector>

/**
 * @brief The NextGIS Core application class. This is for non GUI applications
 */
class NGCORE_EXPORT NGCoreApplication
{
public:
    NGCoreApplication(const QString &applicationName, const QString &version);
    virtual ~NGCoreApplication();
    virtual void init(int &argc, char **argv);
    virtual int exec();
    virtual QString configDirectory() const;

    // Static
public:
    static NGCoreApplication *instance();

protected:
    virtual void createApplication(int &argc, char **argv);
    virtual void loadTranslation();
    virtual void prepareCommandLineParser(QCommandLineParser &parser);
    virtual void processCommandLine(const QCommandLineParser &parser);

protected:
    QCoreApplication *m_app;
    QString m_applicationName, m_version, m_organization, m_organizationDomain;
    QVector<QTranslator*> m_translations;
};

#endif // QT_VERSION >= 0x050000

#endif // NGSTD_COREAPPLICATION_H
