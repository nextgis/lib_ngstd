/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Widgets library export declarations
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_WIDGETS_H
#define NGSTD_WIDGETS_WIDGETS_H

#include <QtCore/QtGlobal>

#if defined(NGSTD_WIDGETS_STATIC)
#define NGSTD_WIDGETS_EXPORT
#define NGSTD_WIDGETS_LOCAL
#elif defined(NGSTD_WIDGETS_LIBRARY)
#define NGSTD_WIDGETS_EXPORT Q_DECL_EXPORT
#define NGSTD_WIDGETS_LOCAL Q_DECL_HIDDEN
#else
#define NGSTD_WIDGETS_EXPORT Q_DECL_IMPORT
#define NGSTD_WIDGETS_LOCAL Q_DECL_HIDDEN
#endif

#endif // NGSTD_WIDGETS_WIDGETS_H
