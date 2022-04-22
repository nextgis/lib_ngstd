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

#include "updater.h"

#include <QApplication>
#include <QWidget>
#include <QXmlStreamReader>

#if defined Q_OS_WIN
constexpr const char *updater = "nextgisupdater.exe";
#elif defined(Q_OS_MAC)
constexpr const char *updater = "nextgisupdater.app/Contents/MacOS/nextgisupdater";
#else
constexpr const char *updater = "";
#endif //

NGUpdater::NGUpdater( QWidget *parent ) : QObject( parent )
{
    m_maintainerProcess = new QProcess(this);

    connect(m_maintainerProcess, SIGNAL(started()), this, SLOT(maintainerStrated()) );
    connect(m_maintainerProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(maintainerErrored(QProcess::ProcessError)));
    connect(m_maintainerProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(maintainerStateChanged(QProcess::ProcessState)));
    connect(m_maintainerProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(maintainerFinished(int, QProcess::ExitStatus)));
    connect(m_maintainerProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(maintainerReadyReadStandardOutput()));
    connect(m_maintainerProcess, SIGNAL(readyReadStandardError()), this, SLOT(maintainerReadyReadStandardError()));
}

const QString NGUpdater::updaterPath()
{
#ifdef Q_OS_LINUX
    #warning "Linux is not supported yet!"
#endif
    return QLatin1String(updater);
}

void NGUpdater::checkUpdates()
{
    if (m_maintainerProcess->state() != QProcess::NotRunning) {
        return;
    }

    // Show available packages on Ubuntu without root access
    // apt list --upgradable

    const QString path = updaterPath();
    QStringList args;
    args << "check-updates";
    m_maintainerProcess->start(path, args);
}

void NGUpdater::maintainerStrated()
{
    this->checkUpdatesStarted();
}

void NGUpdater::maintainerErrored(QProcess::ProcessError)
{
}

void NGUpdater::maintainerStateChanged(QProcess::ProcessState)
{
}

void NGUpdater::maintainerFinished(int /*code*/, QProcess::ExitStatus /*status*/)
{
    QProcess *prc = static_cast<QProcess*>(sender());

    QByteArray data = prc->readAllStandardOutput();
    QXmlStreamReader xml(QString::fromUtf8(data));
    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartDocument) {
            continue;
        }
        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == "update") {
                if (!ignorePackages().contains(xml.attributes().value("name").toString(),
                    Qt::CaseInsensitive)) {
                    this->checkUpdatesFinished(true);
                    return;
                }
            }
        }
    }
    this->checkUpdatesFinished(false);
}

void NGUpdater::maintainerReadyReadStandardOutput()
{
}

void NGUpdater::maintainerReadyReadStandardError()
{
}

void NGUpdater::startUpdate(const QString &projectPath)
{
  // TODO: pkexec or update-manager
	QString program = updaterPath();
	QStringList arguments;
	arguments << "--updater";
	arguments << "--launch";
	arguments << qApp->applicationFilePath();
	if(!projectPath.isEmpty()) {
		arguments << "--launch-options";
		arguments << projectPath;
	}

	QProcess::startDetached( program, arguments	);
}

const QStringList NGUpdater::ignorePackages()
{
    QStringList packages;
    return packages;
}
