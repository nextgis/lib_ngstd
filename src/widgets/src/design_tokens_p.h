/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Private access to generated design-token data
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_DESIGN_TOKENS_P_H
#define NGSTD_WIDGETS_DESIGN_TOKENS_P_H

#include <QString>
#include <QStringList>

namespace ngstd {
namespace widgets {
namespace internal {

QString tokenString(const QString &path);
double tokenNumber(const QString &path);
int tokenInteger(const QString &path);
bool tokenBoolean(const QString &path);
QStringList tokenStringList(const QString &path);
QStringList tokenStrings(const QString &path);

} // namespace internal
} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_DESIGN_TOKENS_P_H
