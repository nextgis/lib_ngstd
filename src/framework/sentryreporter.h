/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: Framework library
*******************************************************************************
*  Copyright (C) 2012-2020 NextGIS, info@nextgis.ru
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
#ifndef NGFRAMEWORK_SENTRY_REPORTER_H
#define NGFRAMEWORK_SENTRY_REPORTER_H

#include <QString>
#include <sentry.h>

class SentryReporter
{
    Q_DISABLE_COPY(SentryReporter)
public:
    enum class Level { Info, Warning, Error, Fatal };

    static SentryReporter &instance();

    void init(bool enabled, const QString &sentryKey);
    void sendMessage(const QString &message, Level level = Level::Info);

private:
    SentryReporter() = default;
    ~SentryReporter();

    sentry_level_e toNativeLevel(Level level);
    QString getConfigPath(const QString &sentryKey) const;

    bool m_enabled = false;
    bool m_initialized = false;
    sentry_options_t *m_options;
};

#endif // NGFRAMEWORK_SENTRY_REPORTER_H

