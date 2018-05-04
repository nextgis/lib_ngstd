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
#ifndef NGFRAMEWORK_UPDATER_H
#define NGFRAMEWORK_UPDATER_H

#include "framework/framework.h"

#include <QObject>
#include <QProcess>
#include <QString>


class NGFRAMEWORK_EXPORT NGUpdater : public QObject
{
    Q_OBJECT
public:
    explicit NGUpdater( QWidget *parent = nullptr );

    void checkUpdates();
    void startUpdate(const QString &projectPath);

protected:
    virtual const QStringList ignorePackages();
    virtual const QString updaterPath();

signals:
    virtual void checkUpdatesStarted();
    virtual void checkUpdatesFinished(bool updatesAvailable);

private:
    QProcess *m_maintainerProcess;
    bool m_updatesAvailable;

protected slots:
    virtual void maintainerStrated();
    virtual void maintainerErrored(QProcess::ProcessError);
    virtual void maintainerStateChanged(QProcess::ProcessState);
    virtual void maintainerFinished(int code, QProcess::ExitStatus status);
    virtual void maintainerReadyReadStandardOutput();
    virtual void maintainerReadyReadStandardError();
};

#endif // NGFRAMEWORK_UPDATER_H
