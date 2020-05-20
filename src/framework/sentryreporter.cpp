#include "sentryreporter.h"

#include "core/version.h"

SentryReporter &SentryReporter::instance()
{
    static SentryReporter inst;
    return inst;
}

SentryReporter::SentryReporter()
{
    m_options = sentry_options_new();
    sentry_options_set_release(m_options, QString::number(NGLIB_VERSION_NUMBER).toLocal8Bit().data());
    sentry_init(m_options);
}

SentryReporter::~SentryReporter()
{
    sentry_shutdown();
}

void SentryReporter::init(bool enabled, const QString &sentryKey)
{
    m_enabled = enabled;

    if (!sentryKey.isEmpty()) {
        sentry_options_set_dsn(m_options, sentryKey.toLocal8Bit().data());
    }
}

void SentryReporter::sendMessage(const QString &message, Level level /* = Level::Info */)
{
    if (!m_enabled) {
        return;
    }

    auto event = sentry_value_new_message_event(toNativeLevel(level), LIB_NAME, message.toLocal8Bit().data());
    sentry_capture_event(event);
}

sentry_level_e SentryReporter::toNativeLevel(SentryReporter::Level level)
{
    switch (level)
    {
    case SentryReporter::Level::Info:
        return SENTRY_LEVEL_INFO;
    case SentryReporter::Level::Warning:
        return SENTRY_LEVEL_WARNING;
    case SentryReporter::Level::Error:
        return SENTRY_LEVEL_ERROR;
    case SentryReporter::Level::Fatal:
        return SENTRY_LEVEL_FATAL;
    }
}
