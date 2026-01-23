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

#include <QCryptographicHash>
#include <QDir>
#include <sentry.h>

#include "core/version.h"

#include "framework/logger/baselogger.h"
#include "framework/logger/loggerdecorator.h"
#include "framework/logger/sentrylogger.h"

namespace
{
sentry_level_e toSentryLevel(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Info:
        return SENTRY_LEVEL_INFO;
    case LogLevel::Warning:
        return SENTRY_LEVEL_WARNING;
    case LogLevel::Critical:
        return SENTRY_LEVEL_ERROR;
    case LogLevel::Fatal:
        return SENTRY_LEVEL_FATAL;
    case LogLevel::Debug:
    default:
        return SENTRY_LEVEL_DEBUG;
    }
}

}

SentryLogger::SentryLogger(
    std::shared_ptr<BaseLogger> wrapped,
    const QString &sentryKey,
    const QString &softwareVersion,
    QObject *parent
) :
    LoggerDecorator(std::move(wrapped), parent),
    m_sentryKey(sentryKey),
    m_softwareVersion(softwareVersion)
{
    if (m_sentryKey.isEmpty())
    {
        return;
    }

    m_options = sentry_options_new();
    sentry_options_set_dsn(m_options, m_sentryKey.toUtf8().constData());
    sentry_options_set_release(m_options, m_softwareVersion.toUtf8().constData());
    sentry_options_set_database_path(
        m_options,
        configPath(m_sentryKey).toUtf8().constData()
    );

    if (sentry_init(m_options) == 0)
    {
        m_isInitialized = true;
    }
}

SentryLogger::~SentryLogger()
{
    if (!m_isInitialized)
        return;

    sentry_options_free(m_options);
    sentry_close();
}

void SentryLogger::write(LogLevel level, const QString &msg)
{
    LoggerDecorator::write(level, msg);

    if (!m_isInitialized)
        return;

    auto event = sentry_value_new_message_event(
        toSentryLevel(level),
        LIB_NAME,
        formatMessage(level, msg).toLocal8Bit().constData()
    );
    sentry_capture_event(event);
}

QString SentryLogger::configPath(const QString &sentryKey) const
{
    QString configRoot;
#if defined(Q_OS_MACOS) || defined(Q_OS_MAC) // In Qt 4.8 Q_OS_MAC
    configRoot = QLatin1String("Library/Application Support");
#else
    configRoot = QLatin1String(".config");
#endif

    const QByteArray keyHash = QCryptographicHash::hash(
        sentryKey.toLatin1(),
        QCryptographicHash::Md5
    );

    QDir path(QDir::homePath());
    path = QDir(path.filePath(configRoot));
    path = QDir(path.filePath(QLatin1String(VENDOR)));
    path = QDir(path.filePath(QLatin1String("sentry-native")));

    return path.filePath(QString::fromLatin1(keyHash.toHex()));
}
