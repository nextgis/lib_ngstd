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

#include "framework/logger/loggerdecorator.h"

LoggerDecorator::LoggerDecorator(std::shared_ptr<BaseLogger> wrapped, QObject *parent)
    : BaseLogger(parent)
    , m_wrapped(std::move(wrapped))
{
    setLevel(m_wrapped->level());
}

void LoggerDecorator::setLevel(const LogLevel level)
{
    if (m_wrapped)
        m_wrapped->setLevel(level);
}

void LoggerDecorator::setLevel(const QString &levelStr)
{
    if (m_wrapped)
        m_wrapped->setLevel(levelStr);
}

void LoggerDecorator::flush()
{
    if (m_wrapped)
        m_wrapped->flush();
}

void LoggerDecorator::write(const LogLevel level, const QString &msg)
{
    if (!m_wrapped)
        return;

    switch (level)
    {
    case LogLevel::Debug:
        m_wrapped->debug(msg);
        break;
    case LogLevel::Info:
        m_wrapped->info(msg);
        break;
    case LogLevel::Warning:
        m_wrapped->warning(msg);
        break;
    case LogLevel::Critical:
        m_wrapped->critical(msg);
        break;
    case LogLevel::Fatal:
        m_wrapped->fatal(msg);
        break;
    }
}

std::shared_ptr<BaseLogger> LoggerDecorator::wrapped() const
{
    return m_wrapped;
}
