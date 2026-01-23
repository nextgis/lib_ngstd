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

#include <QByteArray>
#include <memory>

#include "framework/logger/consolelogger.h"

namespace
{
std::shared_ptr<BaseLogger> g_logger;

void applyEnvironmentLogLevel(BaseLogger &logger)
{
    const auto envValue = qgetenv("NGSTD_LOGGING_LEVEL");
    if (envValue.isEmpty())
        return;

    logger.setLevel(QString::fromLocal8Bit(envValue));
}
}

std::shared_ptr<BaseLogger> getLogger()
{
    if (!g_logger)
    {
        g_logger = std::make_shared<ConsoleLogger>();
        applyEnvironmentLogLevel(*g_logger);
    }

    return g_logger;
}

void setLogger(const std::shared_ptr<BaseLogger> &logger)
{
    g_logger = logger;

    if (g_logger)
        applyEnvironmentLogLevel(*g_logger);
}
