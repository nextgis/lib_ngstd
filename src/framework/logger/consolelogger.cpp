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

#include "logger/consolelogger.h"

#include <QTextStream>

void ConsoleLogger::log(const BaseLogger::LogLevel level, const QString &msg)
{
    QTextStream stream(
        (level == LogLevel::Warning || level == LogLevel::Critical) ? stderr : stdout,
        QIODevice::WriteOnly);
    stream << formatMessage(level, msg) << Qt::endl;
}

