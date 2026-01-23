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

#include "framework/logger/baselogger.h"

#include <QDateTime>

namespace
{
LogLevel parseLevel(const QString &value, bool *ok)
{
    const auto normalized = value.trimmed().toLower();
    auto level = LogLevel::Warning;
    auto parsed = true;

    if (normalized == QLatin1String("debug"))
        level = LogLevel::Debug;
    else if (normalized == QLatin1String("info"))
        level = LogLevel::Info;
    else if (normalized == QLatin1String("warning"))
        level = LogLevel::Warning;
    else if (normalized == QLatin1String("critical"))
        level = LogLevel::Critical;
    else if (normalized == QLatin1String("fatal"))
        level = LogLevel::Critical;
    else
        parsed = false;

    if (ok)
        *ok = parsed;

    return level;
}

QString levelToString(const LogLevel level)
{
    switch (level)
    {
    case LogLevel::Info:
        return QStringLiteral("INFO");
    case LogLevel::Warning:
        return QStringLiteral("WARNING");
    case LogLevel::Critical:
        return QStringLiteral("CRITICAL");
    case LogLevel::Fatal:
        return QStringLiteral("FATAL");
    default:
        return QStringLiteral("DEBUG");
    }
}
}

BaseLogger::BaseLogger(QObject *parent)
    : QObject(parent)
    , m_level(LogLevel::Critical)
{
}

BaseLogger::~BaseLogger()
{
    flush();
}

void BaseLogger::debug(const QString &msg)
{
    log(LogLevel::Debug, msg);
}

void BaseLogger::info(const QString &msg)
{
    log(LogLevel::Info, msg);
}

void BaseLogger::warning(const QString &msg)
{
    log(LogLevel::Warning, msg);
}

void BaseLogger::critical(const QString &msg)
{
    log(LogLevel::Critical, msg);
}

void BaseLogger::fatal(const QString &msg)
{
    log(LogLevel::Fatal, msg);
}

void BaseLogger::setLevel(const LogLevel level)
{
    m_level = level;
}

void BaseLogger::setLevel(const QString &levelStr)
{
    auto ok = false;
    const auto parsed = parseLevel(levelStr, &ok);
    if (ok)
    {
        setLevel(parsed);
        return;
    }

    if (!levelStr.trimmed().isEmpty())
    {
        log(LogLevel::Warning,
              QStringLiteral("Unknown log level \"%1\". Keeping \"%2\".")
                  .arg(levelStr, levelToString(m_level)),
              true);
    }
}

LogLevel BaseLogger::level() const
{
    return m_level;
}

void BaseLogger::flush()
{
    // no implementation
}

QString BaseLogger::formatMessage(const LogLevel level, const QString &msg)
{
    const auto timestamp = QDateTime::currentDateTime()
                                  .toString(QStringLiteral("yyyy-MM-ddTHH:mm:ss.zzz"));
    return QStringLiteral("%1 ngstd [%2] %3").arg(timestamp, levelToString(level), msg);
}

void BaseLogger::log(const LogLevel level, const QString &msg, const bool force)
{
    if (force || shouldLog(level))
        write(level, msg);
}

bool BaseLogger::shouldLog(const LogLevel level) const
{
    return level >= m_level;
}
