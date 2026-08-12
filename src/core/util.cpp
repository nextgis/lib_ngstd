#include "core/util.h"

#include <QJsonObject>

QMap<QString, QVariant> toMap(const QJsonObject &root)
{
    return root.toVariantMap();
}
