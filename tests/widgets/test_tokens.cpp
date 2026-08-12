#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/theme_options.h>
#include <ngstd/widgets/types.h>
#include <ngstd/widgets/version.h>

#include <QMetaEnum>
#include <QTest>

#include <cstring>

using namespace ngstd::widgets;

class TokensTest final : public QObject
{
    Q_OBJECT

private slots:
    void exposesLibraryVersion();
    void providesTypedTokens();
    void registersPublicEnums();
    void sharesThemeOptionsImplicitly();
};

void TokensTest::exposesLibraryVersion()
{
    QCOMPARE(std::strcmp(versionString(), "1.0.0"), 0);
    QVERIFY(!DesignTokens::name().isEmpty());
    QVERIFY(!DesignTokens::version().isEmpty());
}

void TokensTest::providesTypedTokens()
{
    QVERIFY(
        DesignTokens::color(ColorRole::Brand, ColorScheme::Light).isValid());
    QVERIFY(DesignTokens::spacing(4) > 0);
    QVERIFY(DesignTokens::radius(RadiusRole::Card) > 0);
    QCOMPARE(DesignTokens::duration(MotionDuration::Instant), 0);
    QVERIFY(DesignTokens::duration(MotionDuration::Normal) > 0);
    QVERIFY(DesignTokens::componentMetric(
                ComponentMetric::ExpandableSectionHeaderHeight) > 0);
    QVERIFY(DesignTokens::font(TypographyRole::Body).pixelSize() > 0);
    QCOMPARE(DesignTokens::typography(TypographyRole::Body).weight,
             QFont::Normal);
    QCOMPARE(DesignTokens::typography(TypographyRole::Heading1).weight,
             QFont::Bold);
}

void TokensTest::registersPublicEnums()
{
    const QMetaEnum themeMode = QMetaEnum::fromType<ThemeMode>();
    const QMetaEnum features = QMetaEnum::fromType<ThemeFeatures>();
    const QMetaEnum cornerMode =
        QMetaEnum::fromType<PageBackgroundCornerMode>();
    QVERIFY(themeMode.isValid());
    QVERIFY(features.isValid());
    QVERIFY(cornerMode.isValid());
    QCOMPARE(QString::fromLatin1(
                 themeMode.valueToKey(static_cast<int>(ThemeMode::Dark))),
             QStringLiteral("Dark"));
}

void TokensTest::sharesThemeOptionsImplicitly()
{
    ThemeOptions original;
    ThemeOptions copy = original;
    copy.setThemeMode(ThemeMode::Dark);
    QCOMPARE(original.themeMode(), ThemeMode::System);
    QCOMPARE(copy.themeMode(), ThemeMode::Dark);
    QVERIFY(original != copy);
}

QTEST_APPLESS_MAIN(TokensTest)

#include "test_tokens.moc"
