/******************************************************************************
*  Project: NextGIS common lib
*  Purpose: GIS libraries set.
*  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
*  Copyright (C) 2009-2019 Dmitry Baryshnikov, bishop.dev@gmail.com
*  Copyright (C) 2012-2019 NextGIS, info@nextgis.ru
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

#ifndef NGLIB_MAJOR_VERSION
#define NGLIB_MAJOR_VERSION     0
#define NGLIB_MINOR_VERSION     12
#define NGLIB_PATCH_NUMBER      2
#endif // NGLIB_MAJOR_VERSION

#ifndef NGLIB_COMPUTE_VERSION
#define NGLIB_COMPUTE_VERSION(maj,min,rev) ((maj)*10000+(min)*100+(rev))
#endif // NGLIB_COMPUTE_VERSION

#ifndef NGLIB_VERSION_NUMBER
#define NGLIB_VERSION_NUMBER (NGLIB_COMPUTE_VERSION(NGLIB_MAJOR_VERSION,NGLIB_MINOR_VERSION,NGLIB_PATCH_NUMBER))
#endif // NGLIB_VERSION_NUMBER

#ifndef NGSTD_VERSION_H
#define NGSTD_VERSION_H

#define VENDOR "NextGIS"
#define VENDOR_DOMAIN "nextgis.com"
#define LIB_COMMENT "Standart NextGIS library for desktop applications."
#define LIB_NAME "ngstd" // for settings

#define STRINGIZE(x)  #x
#define MAKE_VERSION_DOT_STRING(x, y, z) STRINGIZE(x) "." STRINGIZE(y) "." \
    STRINGIZE(z)

#define NGLIB_VERSION_STRING MAKE_VERSION_DOT_STRING(NGLIB_MAJOR_VERSION, \
     NGLIB_MINOR_VERSION,  NGLIB_PATCH_NUMBER)

/*  check if the current version is at least major.minor.release */
#define NGLIB_CHECK_VERSION(major,minor,patch) \
    ( NGLIB_MAJOR_VERSION > (major) || \
    ( NGLIB_MAJOR_VERSION == (major) &&  NGLIB_MINOR_VERSION > (minor)) || \
    ( NGLIB_MAJOR_VERSION == (major) &&  NGLIB_MINOR_VERSION == (minor) && \
      NGLIB_PATCH_NUMBER >= (patch)))

#endif // NGSTD_VERSION_H
