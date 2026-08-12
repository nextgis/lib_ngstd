/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Optional token-backed proxy style for standalone applications
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_PROXY_STYLE_H
#define NGSTD_WIDGETS_PROXY_STYLE_H

#include <ngstd/widgets/widgets.h>

#include <QProxyStyle>
#include <QScopedPointer>

class QPainter;

namespace ngstd {
namespace widgets {

class NextgisProxyStylePrivate;

class NGSTD_WIDGETS_EXPORT NextgisProxyStyle : public QProxyStyle
{
    Q_OBJECT

public:
    explicit NextgisProxyStyle(QStyle *baseStyle = nullptr);
    explicit NextgisProxyStyle(const QString &baseStyleName);
    ~NextgisProxyStyle() override;

    static NextgisProxyStyle *create(
        const QString &baseStyleName = QStringLiteral("Fusion"));

    void polish(QWidget *widget) override;
    void unpolish(QWidget *widget) override;
    int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr,
                    const QWidget *widget = nullptr) const override;
    QRect subElementRect(SubElement element, const QStyleOption *option,
                         const QWidget *widget = nullptr) const override;
    QRect subControlRect(ComplexControl control,
                         const QStyleOptionComplex *option,
                         SubControl subControl,
                         const QWidget *widget = nullptr) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
                           const QSize &contentsSize,
                           const QWidget *widget = nullptr) const override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter,
                       const QWidget *widget = nullptr) const override;
    void drawComplexControl(
        ComplexControl control, const QStyleOptionComplex *option,
        QPainter *painter,
        const QWidget *widget = nullptr) const override;
    void drawControl(ControlElement element, const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const override;

private:
    Q_DISABLE_COPY(NextgisProxyStyle)
    QScopedPointer<NextgisProxyStylePrivate> d;
};

} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_PROXY_STYLE_H
