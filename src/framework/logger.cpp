/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: Framework library
*  Author:  NextGIS
*******************************************************************************
*  Copyright (C) 2012-2025 NextGIS, info@nextgis.ru
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

#include "logger.h"
#include "sentryreporter.h"

Logger::Logger(const QString& id, const QString& title, QObject* parent)
    : QObject(parent)
    , m_message(QString("%1 %2").arg(title, id))
{
}

void Logger::add(const QString& message)
{
    m_message += QString("\n* \"%1\"").arg(message);
}

void Logger::send()
{
    SentryReporter::instance().sendMessage(m_message);
}
