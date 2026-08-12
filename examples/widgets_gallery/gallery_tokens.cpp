/******************************************************************************
 * Project: NextGIS widgets gallery
 * Purpose: Gallery-only token access
 *****************************************************************************/
#include "gallery_tokens.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

class GalleryTokenDocument final
{
public:
    GalleryTokenDocument()
    {
        QFile file(
            QStringLiteral(":/ngstd/widgets-gallery/nextgis-tokens.json"));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        if (document.isObject()) m_root = document.object();
    }

    QJsonValue value(const QStringList &path) const
    {
        QJsonValue current(m_root);
        for (const QString &part : path) {
            if (!current.isObject()) return QJsonValue();
            current = current.toObject().value(part);
        }
        return current;
    }

private:
    QJsonObject m_root;
};

QString metricName(GalleryMetric metric)
{
    static const char *const names[] = {
        "contentMaxWidthPx",
        "viewportMarginPx",
        "headerHeightPx",
        "themeSwitchWidthPx",
        "themeSwitchHeightPx",
        "pagePaddingTopPx",
        "pagePaddingBottomPx",
        "heroPaddingPx",
        "heroContentMaxWidthPx",
        "heroTitleSizePx",
        "heroLeadSizePx",
        "heroActionSpacingPx",
        "heroDesktopHeightPx",
        "quickNavigationMarginTopPx",
        "quickNavigationPaddingPx",
        "sectionMarginTopPx",
        "sectionIntroMarginVerticalPx",
        "sectionIntroLineHeightPx",
        "colorGridSpacingPx",
        "colorSampleHeightPx",
        "colorSamplePaddingPx",
        "colorBodyTitleSpacingPx",
        "componentGridSpacingPx",
        "componentPanelPaddingPx",
        "productActionPanelPaddingHorizontalPx",
        "productActionPanelPaddingVerticalPx",
        "productActionPanelContentGapPx",
        "productActionPanelActionGapPx",
        "productActionPanelCopySpacingPx",
        "typeStagePaddingPx",
        "typeStageDisplaySizePx",
        "typeSpecimenTokenWidthPx",
        "typeSpecimenPaddingHorizontalPx",
        "typeSpecimenPaddingVerticalPx",
        "backgroundDemoHeightPx",
        "backgroundCaptionHeightPx",
        "motionProgressTargetPercent",
    };
    const int index = static_cast<int>(metric);
    const int count = static_cast<int>(sizeof(names) / sizeof(names[0]));
    return index >= 0 && index < count ? QString::fromLatin1(names[index])
                                       : QString();
}

} // namespace

int GalleryTokens::metric(GalleryMetric metric)
{
    return value({
                     QStringLiteral("desktop"),
                     QStringLiteral("gallery"),
                     metricName(metric),
                 })
        .toInt();
}

QJsonValue GalleryTokens::value(const QStringList &path)
{
    static const GalleryTokenDocument document;
    return document.value(path);
}
