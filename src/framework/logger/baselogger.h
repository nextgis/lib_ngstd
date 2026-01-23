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

#ifndef NGFRAMEWORK_BASELOGGER_H
#define NGFRAMEWORK_BASELOGGER_H

#include "framework/framework.h"

#include <QObject>
#include <QString>

enum class LogLevel
{
    Debug = 0,
    Info,
    Warning,
    Critical,
    Fatal
};

class NGFRAMEWORK_EXPORT BaseLogger : public QObject
{
    Q_OBJECT

public:

    explicit BaseLogger(QObject *parent = nullptr);
    virtual ~BaseLogger();

    void debug(const QString &msg);
    void info(const QString &msg);
    void warning(const QString &msg);
    void critical(const QString &msg);
    void fatal(const QString &msg);

    void log(LogLevel level, const QString &msg, bool force = false);

    virtual void setLevel(LogLevel level);
    virtual void setLevel(const QString &levelStr);
    LogLevel level() const;

    virtual void flush();

    static QString formatMessage(LogLevel level, const QString &msg);

protected:
    virtual void write(LogLevel level, const QString &msg) = 0;

private:
    bool shouldLog(LogLevel level) const;

    LogLevel m_level;
};

#endif // NGFRAMEWORK_BASELOGGER_H
