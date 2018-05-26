/******************************************************************************
**  Project: NextGIS GIS libraries
**  Purpose: Framework library
**  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
**  Copyright (C) 2012-2016 NextGIS, info@nextgis.ru
**
**   This program is free software: you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation, either version 2 of the License, or
**   (at your option) any later version.
**   This program is distributed in the hope that it will be useful,
**   but WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**   GNU General Public License for more details.
**
**   You should have received a copy of the GNU General Public License
**   along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#ifndef QNGSTYLE_H
#define QNGSTYLE_H

#include "framework/framework.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))

#include <QProxyStyle>
#include <QSettings>

/**
 * @brief Main theme for NextGIS Qt applications
 */

class NGTheme : public QObject
{
    Q_OBJECT

    Q_ENUMS(Color)
    Q_ENUMS(ImageFile)
    Q_ENUMS(Gradient)
    Q_ENUMS(Flag)
    Q_ENUMS(WidgetStyle)

public:
    NGTheme(const QString &name, QObject *parent = nullptr);
    virtual ~NGTheme();

    enum Color {
        BackgroundColorAlternate,
        BackgroundColorDark,
        BackgroundColorHover,
        BackgroundColorNormal,
        BackgroundColorSelected,
        BadgeLabelBackgroundColorChecked,
        BadgeLabelBackgroundColorUnchecked,
        BadgeLabelTextColorChecked,
        BadgeLabelTextColorUnchecked,
        CanceledSearchTextColor,
        ComboBoxArrowColor,
        ComboBoxArrowColorDisabled,
        ComboBoxTextColor,
        DetailsButtonBackgroundColorHover,
        DetailsWidgetBackgroundColor,
        DockWidgetResizeHandleColor,
        DoubleTabWidget1stEmptyAreaBackgroundColor,
        DoubleTabWidget1stSeparatorColor,
        DoubleTabWidget1stTabActiveTextColor,
        DoubleTabWidget1stTabBackgroundColor,
        DoubleTabWidget1stTabInactiveTextColor,
        DoubleTabWidget2ndSeparatorColor,
        DoubleTabWidget2ndTabActiveTextColor,
        DoubleTabWidget2ndTabBackgroundColor,
        DoubleTabWidget2ndTabInactiveTextColor,
        EditorPlaceholderColor,
        FancyTabBarBackgroundColor,
        FancyTabWidgetDisabledSelectedTextColor,
        FancyTabWidgetDisabledUnselectedTextColor,
        FancyTabWidgetEnabledSelectedTextColor,
        FancyTabWidgetEnabledUnselectedTextColor,
        FancyToolButtonHoverColor,
        FancyToolButtonSelectedColor,
        FutureProgressBackgroundColor,
        IconsDisabledColor,
        InfoBarBackground,
        InfoBarText,
        MenuBarEmptyAreaBackgroundColor,
        MenuBarItemBackgroundColor,
        MenuBarItemTextColorDisabled,
        MenuBarItemTextColorNormal,
        MenuItemTextColorDisabled,
        MenuItemTextColorNormal,
        MiniProjectTargetSelectorBackgroundColor,
        MiniProjectTargetSelectorBorderColor,
        MiniProjectTargetSelectorSummaryBackgroundColor,
        MiniProjectTargetSelectorTextColor,
        OutputPaneButtonFlashColor,
        OutputPaneToggleButtonTextColorChecked,
        OutputPaneToggleButtonTextColorUnchecked,
        PanelButtonToolBackgroundColorHover,
        PanelButtonToolBackgroundColorSelected,
        PanelStatusBarBackgroundColor,
        PanelsWidgetSeparatorLineColor,
        PanelTextColorDark,
        PanelTextColorLight,
        ProgressBarColorError,
        ProgressBarColorFinished,
        ProgressBarColorNormal,
        ProgressBarTitleColor,
        SplitterColor,
        TextColorDisabled,
        TextColorError,
        TextColorHighlight,
        TextColorLink,
        TextColorLinkVisited,
        TextColorNormal,
        TodoItemTextColor,
        ToggleButtonBackgroundColor,
        ToolBarBackgroundColor,
        TreeViewArrowColorNormal,
        TreeViewArrowColorSelected,
        ToolTipBase,
        ToolTipText
    };

    enum Gradient {
        DetailsWidgetHeaderGradient,
        Welcome_Button_GradientNormal,
        Welcome_Button_GradientPressed
    };

    enum ImageFile {
        StandardPixmapFileIcon,
        StandardPixmapDirIcon,
        BuildStepDisable,
        BuildStepRemove,
        BuildStepMoveDown,
        BuildStepMoveUp
    };

    enum Flag {
        DrawTargetSelectorBottom,
        DrawSearchResultWidgetFrame,
        DrawProgressBarSunken,
        DrawIndicatorBranch,
        ComboBoxDrawTextShadow,
        DerivePaletteFromTheme,
        ApplyThemePaletteGlobally
    };

    enum WidgetStyle {
        StyleDefault,
        StyleFlat
    };

    WidgetStyle widgetStyle() const;
    bool flag(Flag f) const;
    QColor color(Color role) const;
    QString imageFile(ImageFile imageFile, const QString &fallBack) const;
    QGradientStops gradient(Gradient role) const;
    QPalette palette() const;
    QStringList preferredStyles() const;

    QString filePath() const;
    QString name() const;
    void setName(const QString &name);

    QVariantHash values() const;

    void writeSettings(const QString &filename) const;
    void readSettings(QSettings &settings);

    static QPalette initialPalette();

    static QColor readColor(const QString &color);
    static QString writeColor(const QColor &color);
protected:
    QString m_sName;
    QString m_sFileName;
    WidgetStyle m_eWidgetStyle;
    QStringList m_lsPreferredStyles;
    QVector<QPair<QColor, QString> > m_aoColors;
    QVector<QString> m_aoImageFiles;
    QVector<QGradientStops> m_aoGradients;
    QVector<bool> m_abFlags;
    QMap<QString, QColor> m_moPalette;

private:
    QPair<QColor, QString> readNamedColor(const QString &color) const;
};

/**
 * @brief Main style for NextGIS Qt applications
 */

class NGStyle : public QProxyStyle
{
public:
    NGStyle(const QString &baseStyleName, NGTheme *theme = nullptr);
    ~NGStyle() override;
    int pixelMetric(PixelMetric metric, const QStyleOption *option,
                    const QWidget *widget) const override;
    QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt,
                           const QWidget *widget) const override;
    void drawButtonSeparator(QPainter *painter, const QRect &rect, bool reverse) const;
    void polish(QWidget *widget) override;
    void unpolish(QWidget *widget) override;
    int styleHint(StyleHint hint, const QStyleOption *option,
                  const QWidget *widget,
                  QStyleHintReturn *returnData = nullptr) const override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget) const override;
    void drawControl(ControlElement element, const QStyleOption *option,
                     QPainter *painter, const QWidget *widget) const override;
    void drawComplexControl(ComplexControl control,
                            const QStyleOptionComplex *option, QPainter *painter,
                            const QWidget *widget) const override;
    virtual QSize sizeFromContents(ContentsType ct, const QStyleOption *opt,
                                   const QSize &contentsSize,
                                   const QWidget *w) const override;
    void setTheme(NGTheme* theme);
    const NGTheme *getTheme(void) const;
public:
    int navigationWidgetHeight()  const { return 22;/*4;*/ }
protected:
    //QLinearGradient statusBarGradient(const QRect &statusBarRect) const;
    QColor borderColor(bool lightColored = false) const;
    QColor highlightColor(bool lightColored = false) const;
    QColor baseColor(bool lightColored = false) const;
    QColor shadowColor(bool lightColored = false) const;
    QColor mergedColors(const QColor &colorA, const QColor &colorB, int factor = 50)  const;
    QColor sidebarHighlight()  const { return QColor(255, 255, 255, 40); }
    QColor sidebarShadow()  const { return QColor(0, 0, 0, 40); }


    QPalette panelPalette(const QPalette &oldPalette, bool lightColored = false) const;
    bool lightColored(const QWidget *widget) const;
    bool panelWidget(const QWidget *widget) const;
    bool styleEnabled(const QWidget *widget) const;
    void drawArrow(QStyle::PrimitiveElement element, QPainter *painter,
                   const QStyleOption *option) const;
    void drawShadows(QPainter *painter,
                     const QStyleOption *option, bool lightColored) const;

    // Gradients used for panels
    void horizontalGradient(QPainter *painter, const QRect &spanRect,
                            const QRect &clipRect, bool lightColored = false) const;
    void verticalGradient(QPainter *painter, const QRect &spanRect,
                          const QRect &clipRect, bool lightColored = false) const;
    void menuGradient(QPainter *painter, const QRect &spanRect,
                      const QRect &clipRect) const;
    void horizontalGradientHelper(QPainter *painter, const QRect &spanRect,
                                  const QRect &clipRect, bool lightColored = false) const;
    void verticalGradientHelper(QPainter *painter, const QRect &spanRect,
                                const QRect &clipRect, bool lightColored = false) const;
    void menuGradientHelper(QPainter *painter, const QRect &spanRect,
                            const QRect &clipRect) const;
    bool usePixmapCache() const { return true; }

    void drawCornerImage(const QImage &img, QPainter *painter, const QRect &rect,
                int left = 0, int top = 0, int right = 0, int bottom = 0) const;
    QString dpiSpecificImageFile(const QString &fileName) const;
protected:
    NGTheme* m_theme;
    //StyleAnimator m_oAnimator;
protected:
    QColor m_baseColor;
    QColor m_requestedBaseColor;
    QImage m_lineeditImage;
    QImage m_lineeditImage_disabled;

};

#endif // QT_VERSION >= 0x050000

#endif // QNGSTYLE_H
