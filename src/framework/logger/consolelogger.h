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

#ifndef NGFRAMEWORK_CONSOLELOGGER_H
#define NGFRAMEWORK_CONSOLELOGGER_H

#include "logger/baselogger.h"

class NGFRAMEWORK_EXPORT ConsoleLogger : public BaseLogger
{
    Q_OBJECT

public:
    using BaseLogger::BaseLogger;

protected:
    void log(LogLevel level, const QString &msg) override;
};

#endif // NGFRAMEWORK_CONSOLELOGGER_H

