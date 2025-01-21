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

#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QObject>
#include "framework.h"

class Logger final : public QObject
{
public:
    Logger(const QString& id, const QString& title, QObject* parent = nullptr);
    ~Logger() = default;

    void add(const QString& message);
    void send();

private:
    QString m_message;
};

#endif LOGGER_H