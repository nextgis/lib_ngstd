#ifndef LOGGER_H
#define LOGGER_H

#include "logger/baselogger.h"
#include <memory>

NGFRAMEWORK_EXPORT BaseLogger &getLogger();
NGFRAMEWORK_EXPORT void setLogger(const std::shared_ptr<BaseLogger> &logger);

#endif // LOGGER_H