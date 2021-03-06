/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: Core library
*  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
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
#include "core/application.h"

#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

#include "core/version.h"

static NGCoreApplication *gCoreApplication = nullptr;

NGCoreApplication::NGCoreApplication(const QString &applicationName,
                                     const QString &version) :
    m_app(nullptr),
    m_applicationName(applicationName),
    m_version(version),
    m_organization(VENDOR),
    m_organizationDomain(VENDOR_DOMAIN)
{
}

NGCoreApplication::~NGCoreApplication()
{
    delete(m_app);
    qDeleteAll(m_translations);
}

void NGCoreApplication::init(int &argc, char **argv)
{
    QString qtVer("5");
#if QT_VERSION < 0x050000
    qtVer = "4";
#endif

    QString exeDir = QFileInfo(argv[0]).absoluteDir().absolutePath();

#ifdef Q_OS_WIN
    QDir defaultPrefixDir(exeDir + QLatin1String("/.."));
    QCoreApplication::addLibraryPath(defaultPrefixDir.absolutePath() +
        QString("/lib/qt%1/plugins").arg(qtVer));
#elif defined(Q_OS_MAC)
    QDir defaultPrefixDir(exeDir + QLatin1String("/../../../.."));
    QCoreApplication::addLibraryPath(defaultPrefixDir.absolutePath() +
        QString("/Library/Plugins/Qt%1").arg(qtVer));
#else
    QDir defaultPrefixDir("/usr");
    QCoreApplication::addLibraryPath(defaultPrefixDir.absolutePath() +
        QString("/%1/qt%2/plugins").arg(INSTALL_LIB_DIR).arg(qtVer));
#endif
    m_prefixPath = defaultPrefixDir.absolutePath();

    createApplication(argc, argv);

    //TODO: create breakpad handler here.

    m_app->setOrganizationName(m_organization);
    m_app->setApplicationName(m_applicationName);
    m_app->setApplicationVersion(m_version);
    m_app->setOrganizationDomain(m_organizationDomain);

    loadTranslation();

    gCoreApplication = this;
}

int NGCoreApplication::exec()
{
    if(nullptr != m_app)
        return m_app->exec();
    return 0;
}

void NGCoreApplication::createApplication(int &argc, char **argv)
{
    if(nullptr != m_app)
        return;
    m_app = new QCoreApplication(argc, argv);
}

static void translationPath(const QString &basePath,
                               QList<QString> &localePaths)
{
    QDir baseDir(basePath);
    QStringList filters;
    filters << QStringLiteral("ngstd_*.framework");
    QStringList list = baseDir.entryList(filters);
    foreach (QString subPath, list) {
        const QString &libTrPath = basePath + "/" + subPath +
                "/Resources/translations";
        localePaths.append(libTrPath);
    }
}

void NGCoreApplication::loadTranslation()
{
    QStringList uiLanguages = QLocale::system().uiLanguages();

    QSettings settings;
    settings.beginGroup(QLatin1String("Common"));
    QString overrideLanguage = settings.value("lang").toString();
    settings.endGroup();

    if (!overrideLanguage.isEmpty())
        uiLanguages.prepend(overrideLanguage);

    QList<QString> localePaths;
    QString qtVer("5");
#if QT_VERSION < 0x050000
    qtVer = "4";
#endif

#ifdef Q_OS_MACOS
    const QString &libTrPath = m_app->applicationDirPath()
            + QLatin1String("/Contents/Resources/translations/");
    localePaths.append(libTrPath);
    localePaths.append(m_prefixPath + QString("/Library/Translations/Qt%1").arg(qtVer));
    translationPath(m_app->applicationDirPath() +
                       "/Contents/Frameworks/", localePaths);
    translationPath(m_prefixPath + QLatin1String("/Library/Frameworks/"), localePaths);
#else
    localePaths.append(m_prefixPath + QLatin1String("/share/translations"));
    localePaths.append(m_prefixPath + QString("/share/qt%1/translations").arg(qtVer));
#endif

    localePaths.append(QLibraryInfo::location(QLibraryInfo::TranslationsPath));

    QString exeName = QFileInfo(m_app->applicationFilePath()).baseName();

    foreach (QString locale, uiLanguages) {
        locale = QLocale(locale).name();
        QString localeShort = locale.left(2);

        // get qm files list in libTrPath
        QStringList filters;
        filters << QStringLiteral("ngstd_*_%1*").arg(localeShort);
        filters << QStringLiteral("qt_%1*").arg(localeShort);
        filters << QStringLiteral("qtbase_%1*").arg(localeShort);
        filters << QStringLiteral("%1_%2*").arg(exeName).arg(localeShort);

        foreach(QString localePath, localePaths) {
            QDir localeDir(localePath);
            QStringList libTrList = localeDir.entryList(filters);
            foreach (QString trFileName, libTrList) {
                QTranslator *translator = new QTranslator;
                if (translator->load(trFileName, localePath)) {
                    m_app->installTranslator(translator);
                    m_translations.append(translator);
                }
                else {
                    delete translator;
                }
            }
        }

        if(!m_translations.isEmpty() ||
            locale == QLatin1String("C") ||
            locale.startsWith(QLatin1String("en"))) {
            m_app->setProperty("qtc_locale", locale);
            return;
        }
    }
}

void NGCoreApplication::prepareCommandLineParser(QCommandLineParser &parser)
{
    // Parse input arguments. Some predifined keys maybe proceed, i.e --help, --about, --version.
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
}

void NGCoreApplication::processCommandLine(const QCommandLineParser &/*parser*/)
{

}

QString NGCoreApplication::configDirectory() const
{
    QString config;
#ifdef Q_OS_MACOS
    config = QLatin1String("Library/Application Support");
#else
    config = QLatin1String(".config");
#endif
    return QDir::homePath() + QDir::separator() + config + QDir::separator() +
            m_organization + QDir::separator() + m_applicationName;
}

NGCoreApplication *NGCoreApplication::instance()
{
    return gCoreApplication;
}
