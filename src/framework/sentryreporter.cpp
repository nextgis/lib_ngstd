#include "sentryreporter.h"

#include <QDir>
#include <QCryptographicHash>
#include "core/version.h"

SentryReporter &SentryReporter::instance()
{
    static SentryReporter inst;
    return inst;
}

SentryReporter::~SentryReporter()
{
    sentry_shutdown();
}

void SentryReporter::init(bool enabled, const QString &sentryKey)
{
    m_enabled = enabled;

    if (!sentryKey.isEmpty()) {
        m_options = sentry_options_new();
        sentry_options_set_release(m_options, QString::number(NGLIB_VERSION_NUMBER).toLocal8Bit().data());
        sentry_options_set_dsn(m_options, sentryKey.toLocal8Bit().data());
        sentry_options_set_database_path(m_options, getConfigPath(sentryKey).toLocal8Bit().data());
        m_initialized = !sentry_init(m_options); // sentry_init returns 0 on success
    }
}

void SentryReporter::sendMessage(const QString &message, Level level /* = Level::Info */)
{
    if (!m_enabled || !m_initialized) {
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

QString SentryReporter::getConfigPath(const QString &sentryKey) const
{
    QString config;
#if defined(Q_OS_MACOS) || defined(Q_OS_MAC) // In Qt 4.8 Q_OS_MAC
    config = QLatin1String("Library/Application Support");
#else
    config = QLatin1String(".config");
#endif

    auto configPath = QString("%1%2sentry-native%3")
        .arg(QDir::homePath() + QDir::separator() + config + QDir::separator())
        .arg(QDir::separator() + QLatin1String(VENDOR) + QDir::separator())
        .arg(QDir::separator() + QString(QCryptographicHash::hash(
            sentryKey.toLatin1(),
            QCryptographicHash::Md5).toHex()));

    return configPath;
}
