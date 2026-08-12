/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: Core library
*  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
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
#include "core/core.h"

#include "core/version.h"
#include "core/util.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

namespace {

QMap<QString, QVariant> documentToMap(const QJsonDocument &document)
{
    if (!document.isObject()) {
        return QMap<QString, QVariant>();
    }

    return toMap(document.object());
}

} // namespace

const char* getVersion()
{
    return NGLIB_VERSION_STRING;
}

QString getVersionString()
{
    return QString::fromLatin1(NGLIB_VERSION_STRING);
}

QMap<QString, QVariant> memJsonToMap(const QString &str)
{
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(str.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        return QMap<QString, QVariant>();
    }

    return documentToMap(document);
}

QMap<QString, QVariant> jsonToMap(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return QMap<QString, QVariant>();
    }

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        return QMap<QString, QVariant>();
    }

    return documentToMap(document);
}

QString fromBase64(const QString &str)
{
    QByteArray encoded = str.toUtf8();
    encoded.replace('-', '+');
    encoded.replace('_', '/');
    while (encoded.size() % 4 != 0) {
        encoded.append('=');
    }

    return QString::fromUtf8(QByteArray::fromBase64(encoded));
}

QString toBase64(unsigned char *data, int size)
{
    if (!data || size <= 0) {
        return QString();
    }

    const QByteArray raw(reinterpret_cast<const char *>(data), size);
    return QString::fromLatin1(
        raw.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals)
    );
}

QString unescapeUrl(const QString &str)
{
    return QUrl::fromPercentEncoding(str.toUtf8());
}
