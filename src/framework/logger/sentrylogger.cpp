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

#include "logger/sentrylogger.h"

#include <QMutexLocker>

#include "logger/baselogger.h"
#include "logger/loggerdecorator.h"
#include "sentryreporter.h"

namespace
{
SentryReporter::Level toSentryLevel(const BaseLogger::LogLevel level)
{
    switch (level)
    {
    case BaseLogger::LogLevel::Critical:
        return SentryReporter::Level::Fatal;
    case BaseLogger::LogLevel::Warning:
        return SentryReporter::Level::Warning;
    case BaseLogger::LogLevel::Info:
        return SentryReporter::Level::Info;
    case BaseLogger::LogLevel::Debug:
    default:
        return SentryReporter::Level::Debug;
    }
}
}

SentryLogger::SentryLogger(std::shared_ptr<BaseLogger> wrapped, QObject *parent)
    : LoggerDecorator(std::move(wrapped), parent)
    , m_flushTimer(this)
{
    m_flushTimer.setInterval(kFlushIntervalMs);
    m_flushTimer.setTimerType(Qt::CoarseTimer);

    QObject::connect(&m_flushTimer, &QTimer::timeout, this, [this]() {
        if (m_lineCount == 0)
            return;

        flush();
    });

    m_flushTimer.start();
}

SentryLogger::~SentryLogger()
{
    m_flushTimer.stop();
    flush();
}

void SentryLogger::flush()
{
    QString payload;
    auto payloadLevel = LogLevel::Debug;

    {
        QMutexLocker locker(&m_mutex);
        if (m_buffer.isEmpty())
        {
            LoggerDecorator::flush();
            return;
        }

        payload = m_buffer;
        payloadLevel = m_highestBufferedLevel;
        m_buffer.clear();
        m_lineCount = 0;
        m_highestBufferedLevel = LogLevel::Debug;
    }

    sendBuffered(payload, payloadLevel);
    LoggerDecorator::flush();
}

void SentryLogger::log(const BaseLogger::LogLevel level, const QString &msg)
{
    LoggerDecorator::log(level, msg);
    appendMessage(level, BaseLogger::formatMessage(level, msg));
}

void SentryLogger::appendMessage(const BaseLogger::LogLevel level, const QString &formattedMessage)
{
    QString payload;
    auto payloadLevel = LogLevel::Debug;

    {
        QMutexLocker locker(&m_mutex);
        if (m_lineCount >= kMaxBufferedLines && !m_buffer.isEmpty())
        {
            payload = m_buffer;
            payloadLevel = m_highestBufferedLevel;
            m_buffer.clear();
            m_lineCount = 0;
            m_highestBufferedLevel = LogLevel::Debug;
        }

        if (!m_buffer.isEmpty())
            m_buffer.append(QLatin1Char('\n'));

        m_buffer.append(formattedMessage);
        ++m_lineCount;

        if (m_lineCount == 1 || level >= m_highestBufferedLevel)
            m_highestBufferedLevel = level;
    }

    if (!payload.isEmpty())
        sendBuffered(payload, payloadLevel);
}

void SentryLogger::sendBuffered(const QString &payload, BaseLogger::LogLevel level)
{
    if (payload.isEmpty())
        return;

    SentryReporter::instance().sendMessage(payload, toSentryLevel(level));
}

