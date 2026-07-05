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

#include "framework/logger/consolelogger.h"

#include <QIODevice>
#include <QTextStream>

#ifdef Q_OS_WIN
#include <cstdio>
#include <mutex>
#include <windows.h>
#endif

namespace
{
void ensureConsoleAttached()
{
#ifdef Q_OS_WIN
    static std::once_flag attachFlag;
    std::call_once(attachFlag, []()
    {
        if (GetConsoleWindow() == nullptr) {
            AttachConsole(ATTACH_PARENT_PROCESS);
        }

        if (GetConsoleWindow() == nullptr) {
            return;
        }

        FILE *stream = nullptr;
        freopen_s(&stream, "CONOUT$", "w", stdout);
        freopen_s(&stream, "CONOUT$", "w", stderr);
        setvbuf(stdout, nullptr, _IONBF, 0);
        setvbuf(stderr, nullptr, _IONBF, 0);
    });
#endif
}
}

void ConsoleLogger::write(const LogLevel level, const QString &msg)
{
    ensureConsoleAttached();

    bool use_stderr = (
        level == LogLevel::Warning
        || level == LogLevel::Critical
        || level == LogLevel::Fatal
    );

    QTextStream stream(
        use_stderr ? stderr : stdout,
        QIODevice::WriteOnly);
    stream << formatMessage(level, msg) << Qt::endl;
}
