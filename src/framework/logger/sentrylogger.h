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

#ifndef NGFRAMEWORK_SENTRYLOGGER_H
#define NGFRAMEWORK_SENTRYLOGGER_H

#include "logger/loggerdecorator.h"

#include <QMutex>
#include <QTimer>

class NGFRAMEWORK_EXPORT SentryLogger : public LoggerDecorator
{
    Q_OBJECT

public:
    explicit SentryLogger(std::shared_ptr<BaseLogger> wrapped, QObject *parent = nullptr);
    ~SentryLogger() override;

    void flush() override;

protected:
    void log(LogLevel level, const QString &msg) override;

private:
    void appendMessage(LogLevel level, const QString &formattedMessage);
    void sendBuffered(const QString &payload, LogLevel level);

    static constexpr int kMaxBufferedLines = 1000;
    static constexpr int kFlushIntervalMs = 10 * 1000;

    QTimer m_flushTimer;
    QMutex m_mutex;
    QString m_buffer;
    int m_lineCount = 0;
    LogLevel m_highestBufferedLevel = LogLevel::Debug;
};

#endif // NGFRAMEWORK_SENTRYLOGGER_H

