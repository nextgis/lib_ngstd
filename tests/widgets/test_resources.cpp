#include <ngstd/widgets/icons.h>
#include <ngstd/widgets/theme.h>

#include <QFile>
#include <QImage>
#include <QPixmap>
#include <QTest>

using namespace ngstd::widgets;

class ResourcesTest final : public QObject
{
    Q_OBJECT

private slots:
    void publicAssetsExist();
    void horizontalLogoKeepsAspectRatio();
    void coreResourceCollectionUsesAllowlist();
    void styleSheetUsesStableProperties();
};

void ResourcesTest::publicAssetsExist()
{
    QVERIFY(QFile::exists(iconResourcePath(IconRole::Search)));
    QVERIFY(QFile::exists(logoResourcePath(LogoRole::Data)));
    QVERIFY(QFile::exists(logoResourcePath(LogoRole::Horizontal)));
    QVERIFY(!logoPixmap(LogoRole::Data, QSize(42, 42), 1.0).isNull());
}

void ResourcesTest::horizontalLogoKeepsAspectRatio()
{
    const QPixmap pixmap =
        logoPixmap(LogoRole::Horizontal, QSize(270, 72), 1.0);
    QVERIFY(!pixmap.isNull());
    const QImage image = pixmap.toImage();
    QRect paintedBounds;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (image.pixelColor(x, y).alpha() == 0) continue;
            paintedBounds |= QRect(x, y, 1, 1);
        }
    }
    QVERIFY(!paintedBounds.isEmpty());
    QVERIFY(paintedBounds.width() >= 265);
    QVERIFY(paintedBounds.height() >= 34);
    QVERIFY(paintedBounds.height() <= 40);
    QVERIFY(qAbs(paintedBounds.center().y() - image.rect().center().y()) <= 1);
}

void ResourcesTest::coreResourceCollectionUsesAllowlist()
{
    QVERIFY(!QFile::exists(
        QStringLiteral(":/ngstd/widgets/tokens/nextgis-tokens.json")));
    QVERIFY(!QFile::exists(
        QStringLiteral(":/ngstd/widgets/assets/cards/base_card.webp")));
    QVERIFY(!QFile::exists(
        QStringLiteral(":/ngstd/widgets/assets/pictograms/data.png")));
    QVERIFY(!QFile::exists(QStringLiteral(
        ":/ngstd/widgets/assets/fonts/roboto-latin-400.woff2")));
}

void ResourcesTest::styleSheetUsesStableProperties()
{
    const QString styleSheet = ThemeController::styleSheet(ColorScheme::Light);
    QVERIFY(styleSheet.contains(QStringLiteral("ngstdButtonVariant")));
    QVERIFY(styleSheet.contains(QStringLiteral("ngstdCardVariant")));
    QVERIFY(styleSheet.contains(QStringLiteral("ngstdError")));
    QVERIFY(styleSheet.contains(QStringLiteral("ngstdSelected")));
    QVERIFY(styleSheet.contains(QStringLiteral("wizardPageStack")));
    QVERIFY(styleSheet.contains(QStringLiteral("wizardNavigation")));
    QVERIFY(styleSheet.contains(QStringLiteral("nextGisShellArea")));
    QVERIFY(styleSheet.contains(QStringLiteral(
        "QWizard[_ngstdRole=\"wizard\"] {\nbackground-color: #FFFFFF;")));
    QVERIFY(styleSheet.contains(QStringLiteral(
        "QWizard QPushButton[ngstdButtonVariant=\"icon\"]")));
    QVERIFY(styleSheet.contains(QStringLiteral("max-width: 38px;")));
    QVERIFY(styleSheet.contains(QStringLiteral("background-color: #F2F8FC")));
    QVERIFY(styleSheet.contains(QStringLiteral("QToolTip")));
    QVERIFY(!styleSheet.contains(QStringLiteral("ngState")));
    QVERIFY(!styleSheet.contains(QStringLiteral("ngVariant")));
}

QTEST_MAIN(ResourcesTest)

#include "test_resources.moc"
