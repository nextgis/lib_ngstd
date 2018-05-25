/******************************************************************************
**  Project: NextGIS GIS libraries
**  Purpose: Framework library
**  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
**  Copyright (C) 2015 NextGIS, info@nextgis.ru
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

#include "framework/style.h"

#include "mainwindow.h"

#include <QStyleFactory>
#include <QWidget>
#include <QLineEdit>
#include <QPainter>
#include <QDockWidget>
#include <QLabel>
#include <QComboBox>
#include <QTabBar>
#include <QToolBar>
#include <QToolButton>
#include <QStatusBar>
#include <QMenuBar>
#include <QApplication>
#include <QPixmapCache>
#include <QTime>
#include <QFile>
#include <QFileInfo>
#include <QMetaEnum>

/**
 * @brief Clamps float color values within (0, 255)
 * @param x float value color
 * @return color value within (0, 255)
 */
static int clamp(float x)
{
    const int val = x > 255 ? 255 : static_cast<int>(x);
    return val < 0 ? 0 : val;
}

//------------------------------------------------------------------------------
// Style
//------------------------------------------------------------------------------

NGStyle::NGStyle(const QString &baseStyleName, NGTheme *theme) : 
    QProxyStyle(QStyleFactory::create(baseStyleName)), 
    m_theme(theme)
{
    m_baseColor = QColor(160, 160, 160);
    m_lineeditImage = QImage(dpiSpecificImageFile(
                                 QStringLiteral(":/images/inputfield.svg")));
    m_lineeditImage_disabled = QImage(dpiSpecificImageFile(
                                          QStringLiteral(":/images/inputfield_disabled.svg")));
}

NGStyle::~NGStyle()
{
    delete m_theme;
}

QPalette NGStyle::panelPalette(const QPalette &oldPalette, bool lightColored) const
{
    QPalette pal = oldPalette;
    if(nullptr == m_theme)
        return pal;

    QColor color = m_theme->color(lightColored ? NGTheme::PanelTextColorDark : 
                                                 NGTheme::PanelTextColorLight);
    pal.setBrush(QPalette::All, QPalette::WindowText, color);
    pal.setBrush(QPalette::All, QPalette::ButtonText, color);
    pal.setBrush(QPalette::All, QPalette::Foreground, color);
    if (lightColored)
        color.setAlpha(100);
    else
        color = m_theme->color(NGTheme::IconsDisabledColor);
    pal.setBrush(QPalette::Disabled, QPalette::WindowText, color);
    pal.setBrush(QPalette::Disabled, QPalette::ButtonText, color);
    pal.setBrush(QPalette::Disabled, QPalette::Foreground, color);
    return pal;
}

int NGStyle::pixelMetric(PixelMetric metric, const QStyleOption *option,
                       const QWidget *widget) const
{
    int retval = QProxyStyle::pixelMetric(metric, option, widget);
    switch (metric) {
    case PM_SplitterWidth:
        if (widget && widget->property("minisplitter").toBool())
            retval = 1;
        break;
    case PM_ToolBarIconSize:
        if (panelWidget(widget))
            retval = 48;
        break;
    case PM_ButtonIconSize:
        if (panelWidget(widget))
            retval = 16;
        break;
    case PM_SmallIconSize:
        retval = 16;
        break;
    case PM_DockWidgetHandleExtent:
    case PM_DockWidgetSeparatorExtent:
        return 1;
    case PM_MenuPanelWidth:
    case PM_MenuBarHMargin:
    case PM_MenuBarVMargin:
    case PM_ToolBarFrameWidth:
        if (panelWidget(widget))
            retval = 1;
        break;
    case PM_ButtonShiftVertical:
    case PM_ButtonShiftHorizontal:
        if (panelWidget(widget))
            retval = 0;
        break;
    case PM_MenuBarPanelWidth:
    case PM_ToolBarItemMargin:
    case PM_ToolBarItemSpacing:
        if (panelWidget(widget))
            retval = 0;
        break;
    case PM_DefaultFrameWidth:
        if (qobject_cast<const QLineEdit*>(widget) && panelWidget(widget))
            return 1;
        break;
    default:
        break;
    }
    return retval;
}


QPixmap NGStyle::standardPixmap(StandardPixmap standardPixmap,
                              const QStyleOption *opt,
                              const QWidget *widget) const
{
    if (widget && !panelWidget(widget))
        return QProxyStyle::standardPixmap(standardPixmap, opt, widget);

    QPixmap pixmap;
    switch (standardPixmap) {
        case QStyle::SP_TitleBarCloseButton:
            pixmap = QPixmap(":/images/multiply.svg");
            break;
        default:
            pixmap = QProxyStyle::standardPixmap(standardPixmap, opt, widget);
            break;
    }
    return pixmap;
}

void NGStyle::drawButtonSeparator(QPainter *painter, const QRect &rect,
                                bool reverse) const
{
    QLinearGradient grad(rect.topRight(), rect.bottomRight());
    grad.setColorAt(0, QColor(255, 255, 255, 20));
    grad.setColorAt(0.4, QColor(255, 255, 255, 60));
    grad.setColorAt(0.7, QColor(255, 255, 255, 50));
    grad.setColorAt(1, QColor(255, 255, 255, 40));
    painter->setPen(QPen(grad, 0));
    painter->drawLine(rect.topRight(), rect.bottomRight());
    grad.setColorAt(0, QColor(0, 0, 0, 30));
    grad.setColorAt(0.4, QColor(0, 0, 0, 70));
    grad.setColorAt(0.7, QColor(0, 0, 0, 70));
    grad.setColorAt(1, QColor(0, 0, 0, 40));
    painter->setPen(QPen(grad, 0));
    if (!reverse)
       painter->drawLine(rect.topRight() - QPoint(1,0),
                         rect.bottomRight() - QPoint(1,0));
    else
       painter->drawLine(rect.topLeft(), rect.bottomLeft());
}

void NGStyle::polish(QWidget *widget)
{
    QProxyStyle::polish(widget);

    // OxygenStyle forces a rounded widget mask on toolbars and dock widgets
    /*if (baseStyle()->inherits("OxygenStyle") || baseStyle()->inherits("Oxygen::Style")) {
        if (qobject_cast<QToolBar*>(widget) || qobject_cast<QDockWidget*>(widget)) {
            widget->removeEventFilter(baseStyle());
            widget->setContentsMargins(0, 0, 0, 0);
        }
    }*/
    if (panelWidget(widget)) {

        // Oxygen and possibly other styles override this
        if (qobject_cast<QDockWidget*>(widget))
            widget->setContentsMargins(0, 0, 0, 0);

        widget->setAttribute(Qt::WA_LayoutUsesWidgetRect, true);
        if (qobject_cast<QToolButton*>(widget)) {
            widget->setAttribute(Qt::WA_Hover);
            // Don't limit toolbar height
//            widget->setMaximumHeight(navigationWidgetHeight() - 2);
        } else if (qobject_cast<QLineEdit*>(widget)) {
            widget->setAttribute(Qt::WA_Hover);
            widget->setMaximumHeight(navigationWidgetHeight() - 2);
        } else if (qobject_cast<QLabel*>(widget)) {
//            Get problem with text color
//            widget->setPalette(panelPalette(widget->palette(),
//                                            lightColored(widget)));
        } else if (widget->property("panelwidget_singlerow").toBool()) {
            widget->setFixedHeight(navigationWidgetHeight());
        } else if (qobject_cast<QStatusBar*>(widget)) {
            widget->setFixedHeight(navigationWidgetHeight() + 2);
        } else if (qobject_cast<QComboBox*>(widget)) {
            widget->setMaximumHeight(navigationWidgetHeight() - 2);
            widget->setAttribute(Qt::WA_Hover);
        }
    }
}

void NGStyle::unpolish(QWidget *widget)
{
    QProxyStyle::unpolish(widget);
    if (panelWidget(widget)) {
        widget->setAttribute(Qt::WA_LayoutUsesWidgetRect, false);
        if (qobject_cast<QTabBar*>(widget))
            widget->setAttribute(Qt::WA_Hover, false);
        else if (qobject_cast<QToolBar*>(widget))
            widget->setAttribute(Qt::WA_Hover, false);
        else if (qobject_cast<QComboBox*>(widget))
            widget->setAttribute(Qt::WA_Hover, false);
    }
}


int NGStyle::styleHint(StyleHint hint, const QStyleOption *option,
                     const QWidget *widget, QStyleHintReturn *returnData) const
{
    int ret = QProxyStyle::styleHint(hint, option, widget, returnData);
    switch (hint) {
    case QStyle::SH_EtchDisabledText:
        if (panelWidget(widget) || qobject_cast<const QMenu *> (widget) )
            ret = false;
        break;
    case QStyle::SH_ItemView_ArrowKeysNavigateIntoChildren:
        ret = true;
        break;
    case QStyle::SH_ItemView_ActivateItemOnSingleClick:
        // default depends on the style
        if (widget) {
            QVariant activationMode = widget->property("ActivationMode");
            if (activationMode.isValid())
                ret = activationMode.toBool();
        }
        break;
    //case QStyle::SH_FormLayoutFieldGrowthPolicy:
        // The default in QMacStyle, FieldsStayAtSizeHint, is just always the wrong thing
        // Use the same as on all other shipped styles
    //    if (HostOsInfo::isMacHost())
    //        ret = QFormLayout::AllNonFixedFieldsGrow;
    //    break;
    default:
        break;
    }
    return ret;
}
/*
QLinearGradient Style::statusBarGradient(const QRect &statusBarRect) const
{
    QLinearGradient grad(statusBarRect.topLeft(), QPoint(statusBarRect.center().x(), statusBarRect.bottom()));
    QColor startColor = shadowColor().darker(164);
    QColor endColor = baseColor().darker(130);
    grad.setColorAt(0, startColor);
    grad.setColorAt(1, endColor);
    return grad;
}
*/
// Because designer needs to disable this for widget previews
// we have a custom property that is inherited
bool NGStyle::styleEnabled(const QWidget *widget) const
{
    const QWidget *p = widget;
    while (p) {
        if (p->property("_q_custom_style_disabled").toBool()) {
            return false;
        }
        p = p->parentWidget();
    }
    return true;
}

// Consider making this a QStyle state
bool NGStyle::panelWidget(const QWidget *widget) const
{
    if (!widget)
        return false;

    // Do not style dialogs or explicitly ignored widgets
    if ((widget->window()->windowFlags() & Qt::WindowType_Mask) == Qt::Dialog)
        return false;

    if (qobject_cast<const NGMainWindow *>(widget))
        return true;

    if (qobject_cast<const QTabBar *>(widget))
        return styleEnabled(widget);

    const QWidget *p = widget;
    while (p) {
        if (qobject_cast<const QToolBar *>(p) ||
            qobject_cast<const QStatusBar *>(p) ||
            qobject_cast<const QMenuBar *>(p) ||
            p->property("panelwidget").toBool())
            return styleEnabled(widget);
        p = p->parentWidget();
    }
    return false;
}


// Consider making this a QStyle state
bool NGStyle::lightColored(const QWidget *widget) const
{
    if (!widget)
        return false;

    // Don't style dialogs or explicitly ignored widgets
    if ((widget->window()->windowFlags() & Qt::WindowType_Mask) == Qt::Dialog)
        return false;

    const QWidget *p = widget;
    while (p) {
        if (p->property("lightColored").toBool()) {
            return true;
        }
        p = p->parentWidget();
    }
    return false;
}

// Draws a CSS-like border image where the defined borders are not stretched
// Unit for rect, left, top, right and bottom is user pixels
void NGStyle::drawCornerImage(const QImage &img, QPainter *painter, const QRect &rect,
                                  int left, int top, int right, int bottom) const
{
    // source rect for drawImage() calls needs to be specified in DIP unit of the image
    const qreal imagePixelRatio = img.devicePixelRatio();
    const qreal leftDIP = left * imagePixelRatio;
    const qreal topDIP = top * imagePixelRatio;
    const qreal rightDIP = right * imagePixelRatio;
    const qreal bottomDIP = bottom * imagePixelRatio;

    const QSize size = img.size();
    if (top > 0) { //top
        painter->drawImage(QRectF(rect.left() + left, rect.top(),
                                  rect.width() -right - left,
                                  top), img,
                           QRectF(leftDIP, 0, size.width() - rightDIP - leftDIP,
                                  topDIP));
        if (left > 0) //top-left
            painter->drawImage(QRectF(rect.left(), rect.top(), left, top), img,
                               QRectF(0, 0, leftDIP, topDIP));
        if (right > 0) //top-right
            painter->drawImage(QRectF(rect.left() + rect.width() - right, rect.top(),
                                      right, top), img,
                               QRectF(size.width() - rightDIP, 0, rightDIP,
                                      topDIP));
    }
    //left
    if (left > 0)
        painter->drawImage(QRectF(rect.left(), rect.top()+top, left,
                                  rect.height() - top - bottom), img,
                           QRectF(0, topDIP, leftDIP,
                                  size.height() - bottomDIP - topDIP));
    //center
    painter->drawImage(QRectF(rect.left() + left, rect.top()+top,
                              rect.width() -right - left,
                              rect.height() - bottom - top), img,
                       QRectF(leftDIP, topDIP, size.width() - rightDIP - leftDIP,
                              size.height() - bottomDIP - topDIP));
    if (right > 0) //right
        painter->drawImage(QRectF(rect.left() +rect.width() - right,
                                  rect.top()+top, right,
                                  rect.height() - top - bottom), img,
                           QRectF(size.width() - rightDIP, topDIP, rightDIP,
                                  size.height() - bottomDIP - topDIP));
    if (bottom > 0) { //bottom
        painter->drawImage(QRectF(rect.left() +left,
                                  rect.top() + rect.height() - bottom,
                                  rect.width() - right - left, bottom), img,
                           QRectF(leftDIP, size.height() - bottomDIP,
                                  size.width() - rightDIP - leftDIP, bottomDIP));
        if (left > 0) //bottom-left
            painter->drawImage(QRectF(rect.left(),
                                      rect.top() + rect.height() - bottom,
                                      left, bottom), img,
                               QRectF(0, size.height() - bottomDIP, leftDIP,
                                      bottomDIP));
        if (right > 0) //bottom-right
            painter->drawImage(QRectF(rect.left() + rect.width() - right,
                                      rect.top() + rect.height() - bottom,
                                      right, bottom), img,
                               QRectF(size.width() - rightDIP,
                                      size.height() - bottomDIP, rightDIP,
                                      bottomDIP));
    }
}

void NGStyle::drawArrow(QStyle::PrimitiveElement element, QPainter *painter,
                        const QStyleOption *option) const
{
    if (option->rect.width() <= 1 || option->rect.height() <= 1)
        return;

    const qreal devicePixelRatio = painter->device()->devicePixelRatio();
    QRect r = option->rect;
    int size = qMin(r.height(), r.width());
    QPixmap pixmap;
    QString pixmapName;
    pixmapName.sprintf("arrow-%s-%d-%d-%d-%lld-%f",
                       "$qt_ia",
                       uint(option->state), element,
                       size, option->palette.cacheKey(),
                       devicePixelRatio);
    if (!QPixmapCache::find(pixmapName, pixmap)) {
        const QCommonStyle* const style =
                qobject_cast<QCommonStyle*>(QApplication::style());
        if (!style)
            return;

        QImage image(size * devicePixelRatio, size * devicePixelRatio,
                     QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter painter(&image);

        QStyleOption tweakedOption(*option);
        tweakedOption.state = QStyle::State_Enabled;

        if (!(option->state & QStyle::State_Enabled)) {
            tweakedOption.palette.setColor(QPalette::ButtonText,
                                           option->palette.mid().color());
            tweakedOption.rect = image.rect();
            style->QCommonStyle::drawPrimitive(element, &tweakedOption, &painter);
        } else {
            tweakedOption.palette.setColor(QPalette::ButtonText, Qt::black);
            painter.setOpacity(0.2);
            tweakedOption.rect = image.rect().adjusted(0, devicePixelRatio, 0,
                                                       devicePixelRatio);
            style->QCommonStyle::drawPrimitive(element, &tweakedOption, &painter);

            tweakedOption.palette.setColor(QPalette::ButtonText,
                                           QColor(220, 220, 220));
            painter.setOpacity(1);
            tweakedOption.rect = image.rect();
            style->QCommonStyle::drawPrimitive(element, &tweakedOption, &painter);
        }
        painter.end();
        pixmap = QPixmap::fromImage(image);
        pixmap.setDevicePixelRatio(devicePixelRatio);
        QPixmapCache::insert(pixmapName, pixmap);
    }
    int xOffset = r.x() + (r.width() - size) / 2;
    int yOffset = r.y() + (r.height() - size) / 2;
    painter->drawPixmap(xOffset, yOffset, pixmap);
}


QColor NGStyle::borderColor(bool lightColored) const
{
    QColor result = baseColor(lightColored);
    result.setHsv(result.hue(),
                  result.saturation(),
                  result.value() / 2);
    return result;
}

QColor NGStyle::baseColor(bool lightColored) const
{
    if (!lightColored)
        return m_baseColor;
    else
        return m_baseColor.lighter(230);
}

QColor NGStyle::highlightColor(bool lightColored) const
{
    QColor result = baseColor(lightColored);
    if (!lightColored)
        result.setHsv(result.hue(),
                  clamp(result.saturation()),
                  clamp(result.value() * 1.16f));
    else
        result.setHsv(result.hue(),
                  clamp(result.saturation()),
                  clamp(result.value() * 1.06f));
    return result;
}

QColor NGStyle::shadowColor(bool lightColored) const
{
    QColor result = baseColor(lightColored);
    result.setHsv(result.hue(),
                  clamp(result.saturation() * 1.1f),
                  clamp(result.value() * 0.70f));
    return result;
}

void NGStyle::drawControl(ControlElement element, const QStyleOption *option,
                                 QPainter *painter, const QWidget *widget) const
{
    if (!panelWidget(widget) && !qobject_cast<const QMenu *>(widget))
        return QProxyStyle::drawControl(element, option, painter, widget);

    switch (element) {
    case CE_Splitter:
        painter->fillRect(option->rect, m_theme->color(NGTheme::SplitterColor));
        break;

//    case CE_TabBarTabShape:
//        // Most styles draw a single dark outline. This looks rather ugly when
//        // combined with our single pixel dark separator so we adjust the first
//        // tab to compensate for this

//        if (const QStyleOptionTab *tab =
//                qstyleoption_cast<const QStyleOptionTab*>(option)) {
//            QStyleOptionTab adjustedTab = *tab;
//            if (tab->cornerWidgets == QStyleOptionTab::NoCornerWidgets && (
//                    tab->position == QStyleOptionTab::Beginning ||
//                    tab->position == QStyleOptionTab::OnlyOneTab))
//            {
//                if (option->direction == Qt::LeftToRight)
//                    adjustedTab.rect = adjustedTab.rect.adjusted(-1, 0, 0, 0);
//                else
//                    adjustedTab.rect = adjustedTab.rect.adjusted(0, 0, 1 ,0);
//            }
//            QProxyStyle::drawControl(element, &adjustedTab, painter, widget);
//            return;
//        }
//        break;

    case CE_MenuItem:
        painter->save();
        if (const QStyleOptionMenuItem *mbi =
                qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            const bool enabled = mbi->state & State_Enabled;
            QStyleOptionMenuItem item = *mbi;
            item.rect = mbi->rect;
            const QColor color = m_theme->color(enabled
               ? NGTheme::MenuItemTextColorNormal
               : NGTheme::MenuItemTextColorDisabled);
            if (color.isValid()) {
                QPalette pal = mbi->palette;
                pal.setBrush(QPalette::Text, color);
                item.palette = pal;
            }
            QProxyStyle::drawControl(element, &item, painter, widget);
        }
        painter->restore();
        break;

    case CE_MenuBarItem:
        painter->save();
        if (const QStyleOptionMenuItem *mbi =
                qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            QColor highlightOutline = borderColor().lighter(120);
            const bool act = mbi->state & (State_Sunken | State_Selected);
            const bool dis = !(mbi->state & State_Enabled);

            painter->fillRect(option->rect,
                              m_theme->color(NGTheme::MenuBarItemBackgroundColor));

            QStyleOptionMenuItem item = *mbi;
            item.rect = mbi->rect;
            QPalette pal = mbi->palette;
            pal.setBrush(QPalette::ButtonText, dis
                ? m_theme->color(NGTheme::MenuBarItemTextColorDisabled)
                : m_theme->color(NGTheme::MenuBarItemTextColorNormal));
            item.palette = pal;
            QCommonStyle::drawControl(element, &item, painter, widget);

            if (act) {
                // Fill|
                QColor basecolor = baseColor();
                QLinearGradient grad(option->rect.topLeft(),
                                     option->rect.bottomLeft());
                grad.setColorAt(0, basecolor.lighter(120));
                grad.setColorAt(1, basecolor.lighter(130));
                painter->fillRect(option->rect.adjusted(1, 1, -1, 0), grad);

                // Outline
                painter->setPen(QPen(highlightOutline, 0));
                const QRect r = option->rect;
                painter->drawLine(QPoint(r.left(), r.top() + 1),
                                  QPoint(r.left(), r.bottom()));
                painter->drawLine(QPoint(r.right(), r.top() + 1),
                                  QPoint(r.right(), r.bottom()));
                painter->drawLine(QPoint(r.left() + 1, r.top()),
                                  QPoint(r.right() - 1, r.top()));
                highlightOutline.setAlpha(60);
                painter->setPen(QPen(highlightOutline, 0));
                painter->drawPoint(r.topLeft());
                painter->drawPoint(r.topRight());

                QPalette pal = mbi->palette;
                int alignment = Qt::AlignCenter | Qt::TextShowMnemonic |
                        Qt::TextDontClip | Qt::TextSingleLine;
                if (!styleHint(SH_UnderlineShortcut, mbi, widget))
                    alignment |= Qt::TextHideMnemonic;
                pal.setBrush(QPalette::Text, dis ? Qt::gray : QColor(0, 0, 0, 60));
                drawItemText(painter, item.rect.translated(0, 1), alignment,
                             pal, mbi->state & State_Enabled,
                             mbi->text, QPalette::Text);
                pal.setBrush(QPalette::Text, dis ? Qt::gray : Qt::white);
                drawItemText(painter, item.rect, alignment, pal,
                             mbi->state & State_Enabled, mbi->text,
                             QPalette::Text);
            }
        }
        painter->restore();
        break;

    case CE_ComboBoxLabel:
        if (const QStyleOptionComboBox *cb =
                qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            if (panelWidget(widget)) {
                painter->save();
                QRect editRect = subControlRect(CC_ComboBox, cb,
                                                SC_ComboBoxEditField, widget);
                QPalette customPal = cb->palette;
                bool drawIcon = !(widget && widget->property("hideicon").toBool());

                if (!cb->currentIcon.isNull() && drawIcon) {
                    QIcon::Mode mode = cb->state & State_Enabled ? QIcon::Normal
                                                                 : QIcon::Disabled;
                    QPixmap pixmap = cb->currentIcon.pixmap(cb->iconSize, mode);
                    QRect iconRect(editRect);
                    iconRect.setWidth(cb->iconSize.width() + 4);
                    iconRect = alignedRect(cb->direction,
                                           Qt::AlignLeft | Qt::AlignVCenter,
                                           iconRect.size(), editRect);
                    if (cb->editable) {
                        painter->fillRect(iconRect, customPal.brush(QPalette::Base));
                    }
                    drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);

                    if (cb->direction == Qt::RightToLeft) {
                        editRect.translate(-4 - cb->iconSize.width(), 0);
                    }
                    else {
                        editRect.translate(cb->iconSize.width() + 4, 0);
                    }

                    // Reserve some space for the down-arrow
                    editRect.adjust(0, 0, -13, 0);
                }

                QLatin1Char asterisk('*');
                int elideWidth = editRect.width();

                bool notElideAsterisk = widget
                        && widget->property("notelideasterisk").toBool()
                        && cb->currentText.endsWith(asterisk)
                        && option->fontMetrics.width(cb->currentText) > elideWidth;

                QString text;
                if (notElideAsterisk) {
                    elideWidth -= option->fontMetrics.width(asterisk);
                    text = asterisk;
                }
                text.prepend(option->fontMetrics.elidedText(cb->currentText,
                                                    Qt::ElideRight, elideWidth));

                if (m_theme->flag(NGTheme::ComboBoxDrawTextShadow)
                    && (option->state & State_Enabled))
                {
                    painter->setPen(QColor(0, 0, 0, 70));
                    painter->drawText(editRect.adjusted(1, 0, -1, 0),
                                      Qt::AlignLeft | Qt::AlignVCenter, text);
                }
                if (!(option->state & State_Enabled))
                    painter->setOpacity(0.8);
                painter->setPen(m_theme->color(NGTheme::ComboBoxTextColor));
                painter->drawText(editRect.adjusted(1, 0, -1, 0),
                                  Qt::AlignLeft | Qt::AlignVCenter, text);

                painter->restore();
            } else {
                QProxyStyle::drawControl(element, option, painter, widget);
            }
        }
        break;

    case CE_MenuBarEmptyArea: {
            if (m_theme->widgetStyle() == NGTheme::StyleDefault) {
                menuGradient(painter, option->rect, option->rect);
                painter->save();
                painter->setPen(borderColor());
                painter->drawLine(option->rect.bottomLeft() + QPointF(0.5, 0.5),
                                  option->rect.bottomRight() + QPointF(0.5, 0.5));
                painter->restore();
            } else {
                painter->fillRect(option->rect,
                                  m_theme->color(NGTheme::MenuBarEmptyAreaBackgroundColor));
            }
        }
        break;

    case CE_ToolBar: {
            QRect rect = option->rect;
            bool drawLightColored = lightColored(widget);
            if (m_theme->widgetStyle() == NGTheme::StyleFlat) {
                painter->fillRect (rect,
                                   m_theme->color(NGTheme::ToolBarBackgroundColor));
            }
            else {
                if (option->state & State_Horizontal)
                {
                    // Map offset for global window gradient
                    QRect gradientSpan;
                    if (widget) {
                        QPoint offset = widget->window()->mapToGlobal(option->rect.topLeft()) -
                                        widget->mapToGlobal(option->rect.topLeft());
                        gradientSpan = QRect(offset, widget->window()->size());
                    }

                    horizontalGradient(painter, gradientSpan, rect, drawLightColored);
                } else {
                    // Map offset for global window gradient
                    QRect gradientSpan;
                    if (widget) {
                        QPoint offset = widget->window()->mapToGlobal(option->rect.topLeft()) -
                                        widget->mapToGlobal(option->rect.topLeft());
                        gradientSpan = QRect(offset, widget->window()->size());
                    }
                    verticalGradient(painter, gradientSpan, rect, drawLightColored);
                }
            }

            drawShadows(painter, option, drawLightColored);

            //painter->setPen(m_pTheme->color(Theme::SplitterColor));

            /*QColor lighter;//(sidebarHighlight());
            if (drawLightColored)
                lighter = QColor(255, 255, 255, 180);
            else
                lighter = QColor(255, 255, 255, 45);//lighter = QColor(0, 0, 0, 180);

            QColor daker;
            if (drawLightColored)
                daker = QColor(0, 0, 0, 180);
            else
                daker = QColor(0, 0, 0, 45);

            painter->setPen(lighter);
            painter->drawLine(rect.topLeft(), rect.bottomLeft());
            painter->drawLine(rect.topLeft(), rect.topRight());
            painter->setPen(daker);
            painter->drawLine(rect.topRight(), rect.bottomRight());
            painter->drawLine(rect.bottomLeft(), rect.bottomRight());

            if (horizontal) {
                // Note: This is a hack to determine if the
                // toolbar should draw the top or bottom outline
                // (needed for the find toolbar for instance)

                if (widget && widget->property("topBorder").toBool()) {
                    painter->drawLine(rect.topLeft(), rect.topRight());
                    painter->setPen(lighter);
                    painter->drawLine(rect.topLeft() + QPoint(0, 1), rect.topRight() + QPoint(0, 1));
                } else {
                    painter->drawLine(rect.bottomLeft(), rect.bottomRight());
                    painter->setPen(lighter);
                    painter->drawLine(rect.topLeft(), rect.topRight());
                }
            } else {
                painter->drawLine(rect.topLeft(), rect.bottomLeft());
                painter->drawLine(rect.topRight(), rect.bottomRight());
                painter->setPen(lighter);
                painter->drawLine(rect.topLeft() + QPoint(0, 1), rect.bottomLeft() + QPoint(0, 1));
                painter->drawLine(rect.topRight() + QPoint(0, 1), rect.bottomRight() + QPoint(0, 1));
            }*/
        }
        break;

    default:
        QProxyStyle::drawControl(element, option, painter, widget);
        break;
    }
}

void NGStyle::drawShadows(QPainter *painter, const QStyleOption *option, bool lightColored) const
{
    QRect rect = option->rect;
    QColor lighter;
    if (lightColored)
        lighter = QColor(255, 255, 255, 180);
    else
        lighter = QColor(255, 255, 255, 25);//lighter = QColor(0, 0, 0, 180);

    QColor daker;
    if (lightColored)
        daker = QColor(0, 0, 0, 180);
    else
        daker = QColor(0, 0, 0, 25);

    //QColor split = QColor(0, 0, 0, 100);

    //painter->setPen(split);
    //painter->drawLine(rect.topLeft(), rect.topRight());

    painter->setPen(lighter);
    painter->drawLine(rect.topLeft(), rect.topRight());
    painter->drawLine(rect.topLeft(), rect.bottomLeft());
    painter->setPen(daker);
    painter->drawLine(rect.topRight(), rect.bottomRight());
    painter->drawLine(rect.bottomLeft(), rect.bottomRight());
}

void NGStyle::drawComplexControl(ComplexControl control,
                               const QStyleOptionComplex *option,
                               QPainter *painter, const QWidget *widget) const
{
    if (!panelWidget(widget))
         return QProxyStyle::drawComplexControl(control, option, painter, widget);

    QRect rect = option->rect;
    switch (control) {
    case CC_ToolButton:
        if (const QStyleOptionToolButton *toolbutton =
                qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            bool reverse = option->direction == Qt::RightToLeft;
            bool drawborder = (widget && widget->property("showborder").toBool());

            if (drawborder)
                drawButtonSeparator(painter, rect, reverse);

            QRect button, menuarea;
            button = subControlRect(control, toolbutton, SC_ToolButton, widget);
            menuarea = subControlRect(control, toolbutton, SC_ToolButtonMenu, widget);

            State bflags = toolbutton->state;
            if (bflags & State_AutoRaise) {
                if (!(bflags & State_MouseOver))
                    bflags &= ~State_Raised;
            }

            State mflags = bflags;
            if (toolbutton->state & State_Sunken) {
                if (toolbutton->activeSubControls & SC_ToolButton)
                    bflags |= State_Sunken;
                if (toolbutton->activeSubControls & SC_ToolButtonMenu)
                    mflags |= State_Sunken;
            }

            QStyleOption tool(0);
            tool.palette = toolbutton->palette;
            if (toolbutton->subControls & SC_ToolButton) {
                tool.rect = button;
                tool.state = bflags;
                drawPrimitive(PE_PanelButtonTool, &tool, painter, widget);
            }

            QStyleOptionToolButton label = *toolbutton;            
            if (widget && widget->property("highlightWidget").toBool())
                label.palette.setColor(QPalette::ButtonText, Qt::red);
            int fw = pixelMetric(PM_DefaultFrameWidth, option, widget);
            label.rect = button.adjusted(fw, fw, -fw, -fw);

            drawControl(CE_ToolButtonLabel, &label, painter, widget);

            if (toolbutton->subControls & SC_ToolButtonMenu) {
                tool.state = mflags;
                tool.rect = menuarea.adjusted(1, 1, -1, -1);
                if (mflags & (State_Sunken | State_On | State_Raised)) {
                    painter->setPen(Qt::gray);
                    painter->drawLine(tool.rect.topLeft(), tool.rect.bottomLeft());
                    if (mflags & (State_Sunken)) {
                        QColor shade(0, 0, 0, 50);
                        painter->fillRect(tool.rect.adjusted(0, -1, 1, 1), shade);
                    }
                }
                tool.rect = tool.rect.adjusted(2, 2, -2, -2);
                drawPrimitive(PE_IndicatorArrowDown, &tool, painter, widget);
            } else if (toolbutton->features & QStyleOptionToolButton::HasMenu
                       && widget && !widget->property("noArrow").toBool()) {
                int arrowSize = 6;
                QRect ir = toolbutton->rect.adjusted(1, 1, -1, -1);
                QStyleOptionToolButton newBtn = *toolbutton;
                newBtn.palette = panelPalette(option->palette);
                newBtn.rect = QRect(ir.right() - arrowSize - 1,
                                    ir.height() - arrowSize - 2, arrowSize, arrowSize);
                drawPrimitive(PE_IndicatorArrowDown, &newBtn, painter, widget);
            }
        }
        break;

    case CC_ComboBox:
        if (const QStyleOptionComboBox *cb =
                qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            painter->save();
            bool isEmpty = cb->currentText.isEmpty() && cb->currentIcon.isNull();
            bool reverse = option->direction == Qt::RightToLeft;
            bool drawborder = !(widget && widget->property("hideborder").toBool());
            bool drawleftborder = (widget && widget->property("drawleftborder").toBool());
            bool alignarrow = !(widget && widget->property("alignarrow").toBool());

            if (drawborder) {
                drawButtonSeparator(painter, rect, reverse);
                if (drawleftborder)
                    drawButtonSeparator(painter, rect.adjusted(0, 0, -rect.width() + 2, 0), reverse);
            }

            QStyleOption toolbutton = *option;
            if (isEmpty)
                toolbutton.state &= ~(State_Enabled | State_Sunken);
            painter->save();
            if (drawborder) {
                int leftClipAdjust = 0;
                if (drawleftborder)
                    leftClipAdjust = 2;
                painter->setClipRect(toolbutton.rect.adjusted(leftClipAdjust, 0, -2, 0));
            }
            drawPrimitive(PE_PanelButtonTool, &toolbutton, painter, widget);
            painter->restore();
            // Draw arrow
            const int menuButtonWidth = 12;
            int left = !reverse ? rect.right() - menuButtonWidth : rect.left();
            int right = !reverse ? rect.right() : rect.left() + menuButtonWidth;
            QRect arrowRect((left + right) / 2 + (reverse ? 6 : -6), rect.center().y() - 3, 9, 9);

            if (!alignarrow) {
                int labelwidth = option->fontMetrics.width(cb->currentText);
                if (reverse)
                    arrowRect.moveLeft(qMax(rect.width() - labelwidth - menuButtonWidth - 2, 4));
                else
                    arrowRect.moveLeft(qMin(labelwidth + menuButtonWidth - 2, rect.width() - menuButtonWidth - 4));
            }
            if (option->state & State_On)
                arrowRect.translate(QProxyStyle::pixelMetric(PM_ButtonShiftHorizontal, option, widget),
                                    QProxyStyle::pixelMetric(PM_ButtonShiftVertical, option, widget));

            QStyleOption arrowOpt = *option;
            arrowOpt.rect = arrowRect;
            if (isEmpty)
                arrowOpt.state &= ~(State_Enabled | State_Sunken);

            if (styleHint(SH_ComboBox_Popup, option, widget)) {
                arrowOpt.rect.translate(0, -3);
                drawPrimitive(PE_IndicatorArrowUp, &arrowOpt, painter, widget);
                arrowOpt.rect.translate(0, 6);
                drawPrimitive(PE_IndicatorArrowDown, &arrowOpt, painter, widget);
            } else {
                drawPrimitive(PE_IndicatorArrowDown, &arrowOpt, painter, widget);
            }

            painter->restore();
        }
        break;

    default:
        QProxyStyle::drawComplexControl(control, option, painter, widget);
        break;
    }
}

void NGStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                                   QPainter *painter, const QWidget *widget) const
{
    if (!panelWidget(widget))
        return QProxyStyle::drawPrimitive(element, option, painter, widget);

//    bool animating = (option->state & State_Animating);
//    int state = option->state;
    QRect rect = option->rect;

    switch (element) {
    case PE_IndicatorDockWidgetResizeHandle:
        painter->fillRect(option->rect, m_theme->color(NGTheme::DockWidgetResizeHandleColor));
        break;
    case PE_FrameDockWidget:
        QCommonStyle::drawPrimitive(element, option, painter, widget);
        break;
    case PE_PanelLineEdit:
        {
            painter->save();

            // Fill the line edit background
            QRect filledRect = option->rect.adjusted(1, 1, -1, -1);
            painter->setBrushOrigin(filledRect.topLeft());
            painter->fillRect(filledRect, option->palette.base());

            if (option->state & State_Enabled)
                drawCornerImage(m_lineeditImage, painter, option->rect, 5, 5, 5, 5);
            else
                drawCornerImage(m_lineeditImage_disabled, painter, option->rect, 5, 5, 5, 5);

            if (option->state & State_HasFocus || option->state & State_MouseOver) {
                QColor hover = baseColor();
                if (option->state & State_HasFocus)
                    hover.setAlpha(100);
                else
                    hover.setAlpha(50);

                painter->setPen(QPen(hover, 1));
                painter->drawRect(QRectF(option->rect).adjusted(1.5, 1.5, -1.5, -1.5));
            }
            painter->restore();
        }
        break;

    case PE_FrameStatusBarItem:
//        return QCommonStyle::drawPrimitive(element, option, painter, widget);
        break;

    case PE_PanelButtonTool:
        if (option->state & State_Sunken || option->state & State_On) {
            QProxyStyle::drawPrimitive(element, option, painter, widget);
        } else if (option->state & State_Enabled && option->state & State_MouseOver) {
            QProxyStyle::drawPrimitive(element, option, painter, widget);
        } else if (widget && widget->property("highlightWidget").toBool()) {
            QProxyStyle::drawPrimitive(element, option, painter, widget);
        }

        if (option->state & State_HasFocus && (option->state & State_KeyboardFocusChange)) {
            const QRect rect = option->rect;
            QColor highlight = option->palette.highlight().color();
            highlight.setAlphaF(0.4);
            painter->setPen(QPen(highlight.lighter(), 1));
            highlight.setAlphaF(0.3);
            painter->setBrush(highlight);
            painter->setRenderHint(QPainter::Antialiasing);
            painter->drawRoundedRect(rect.adjusted(2, 2, -2, -2), 2, 2);
        }

        break;

    case PE_PanelStatusBar:
        painter->save();
        painter->fillRect(option->rect, m_theme->color(NGTheme::PanelStatusBarBackgroundColor));
        painter->restore();
        drawShadows(painter, option, lightColored(widget));
//        QProxyStyle::drawPrimitive(element, option, painter, widget);
        break;

    case PE_IndicatorToolBarSeparator:
        {
            QRect separatorRect = rect;
            separatorRect.setLeft(rect.width() / 2);
            separatorRect.setWidth(1);
            drawButtonSeparator(painter, separatorRect, false);
        }
        break;

    case PE_IndicatorArrowUp:
    case PE_IndicatorArrowDown:
    case PE_IndicatorArrowRight:
    case PE_IndicatorArrowLeft:
        drawArrow(element, painter, option);
        break;

    default:
        QProxyStyle::drawPrimitive(element, option, painter, widget);
        break;
    }
}

void NGStyle::setTheme(NGTheme *theme)
{
    if (theme == m_theme)
        return;
    delete m_theme;
    m_theme = theme;

    m_baseColor = theme->color(NGTheme::ToolBarBackgroundColor);
}

const NGTheme *NGStyle::getTheme() const
{
    return m_theme;
}

QString NGStyle::dpiSpecificImageFile(const QString &fileName) const
{
    // See QIcon::addFile()
    if (qApp->devicePixelRatio() > 1.0) {
        const QFileInfo fi(fileName);
        const QString at2xfileName = fi.path() + QLatin1Char('/')
                + fi.completeBaseName() + QStringLiteral("@2x.") + fi.suffix();
        if (QFile::exists(at2xfileName))
            return at2xfileName;
    }
    return fileName;
}


void NGStyle::verticalGradientHelper(QPainter *p, const QRect &spanRect,
                                   const QRect &rect, bool lightColored) const
{
    if (lightColored) {
        QLinearGradient shadowGradient(rect.topRight(), rect.topLeft());
        shadowGradient.setColorAt(0, 0xf0f0f0);
        shadowGradient.setColorAt(1, 0xcfcfcf);
        p->fillRect(rect, shadowGradient);
        return;
    }

    QColor base = baseColor(lightColored);
    QColor highlight = highlightColor(lightColored);
    QColor shadow = shadowColor(lightColored);
    QLinearGradient grad(rect.topRight(), rect.topLeft());
    grad.setColorAt(0, highlight.lighter(120));
    if (rect.height() == navigationWidgetHeight()) {
        grad.setColorAt(0.4, highlight);
        grad.setColorAt(0.401, base);
    }
    grad.setColorAt(1, shadow);
    p->fillRect(rect, grad);

    QLinearGradient shadowGradient(spanRect.topRight(), spanRect.bottomRight());
        shadowGradient.setColorAt(0, QColor(0, 0, 0, 30));
    QColor lighterHighlight;
    lighterHighlight = highlight.lighter(130);
    lighterHighlight.setAlpha(100);
    shadowGradient.setColorAt(0.7, lighterHighlight);
        shadowGradient.setColorAt(1, QColor(0, 0, 0, 40));
    p->fillRect(rect, shadowGradient);
}

void NGStyle::verticalGradient(QPainter *painter, const QRect &spanRect,
                             const QRect &clipRect, bool lightColored) const
{
    if (usePixmapCache()) {
        QString key;
        QColor keyColor = baseColor(lightColored);
        key.sprintf("mh_vertical %d %d %d %d %d",
            spanRect.width(), spanRect.height(), clipRect.width(),
            clipRect.height(), keyColor.rgb());

        QPixmap pixmap;
        if (!QPixmapCache::find(key, pixmap)) {
            pixmap = QPixmap(clipRect.size());
            QPainter p(&pixmap);
            QRect rect(0, 0, clipRect.width(), clipRect.height());
            verticalGradientHelper(&p, spanRect, rect, lightColored);
            p.end();
            QPixmapCache::insert(key, pixmap);
        }

        painter->drawPixmap(clipRect.topLeft(), pixmap);
    } else {
        verticalGradientHelper(painter, spanRect, clipRect, lightColored);
    }
}

void NGStyle::horizontalGradientHelper(QPainter *p, const QRect &spanRect,
                                     const QRect &rect, bool lightColored) const
{
    if (lightColored) {
        QLinearGradient shadowGradient(rect.topLeft(), rect.bottomLeft());
        shadowGradient.setColorAt(0, 0xf0f0f0);
        shadowGradient.setColorAt(1, 0xcfcfcf);
        p->fillRect(rect, shadowGradient);
        return;
    }

    QColor base = baseColor(lightColored);
    QColor highlight = highlightColor(lightColored);
    QColor shadow = shadowColor(lightColored);
    QLinearGradient grad(rect.topLeft(), rect.bottomLeft());
    grad.setColorAt(0, highlight.lighter(120));
    if (rect.height() == navigationWidgetHeight()) {
        grad.setColorAt(0.4, highlight);
        grad.setColorAt(0.401, base);
    }
    grad.setColorAt(1, shadow);
    p->fillRect(rect, grad);

    QLinearGradient shadowGradient(spanRect.topLeft(), spanRect.topRight());
        shadowGradient.setColorAt(0, QColor(0, 0, 0, 30));
    QColor lighterHighlight;
    lighterHighlight = highlight.lighter(130);
    lighterHighlight.setAlpha(100);
    shadowGradient.setColorAt(0.7, lighterHighlight);
        shadowGradient.setColorAt(1, QColor(0, 0, 0, 40));
    p->fillRect(rect, shadowGradient);
}

void NGStyle::horizontalGradient(QPainter *painter, const QRect &spanRect,
                               const QRect &clipRect, bool lightColored) const
{
    if (usePixmapCache()) {
        QString key;
        QColor keyColor = baseColor(lightColored);
        key.sprintf("mh_horizontal %d %d %d %d %d %d",
            spanRect.width(), spanRect.height(), clipRect.width(),
            clipRect.height(), keyColor.rgb(), spanRect.x());

        QPixmap pixmap;
        if (!QPixmapCache::find(key, pixmap)) {
            pixmap = QPixmap(clipRect.size());
            QPainter p(&pixmap);
            QRect rect = QRect(0, 0, clipRect.width(), clipRect.height());
            horizontalGradientHelper(&p, spanRect, rect, lightColored);
            p.end();
            QPixmapCache::insert(key, pixmap);
        }

        painter->drawPixmap(clipRect.topLeft(), pixmap);

    } else {
        horizontalGradientHelper(painter, spanRect, clipRect, lightColored);
    }
}

void NGStyle::menuGradientHelper(QPainter *p, const QRect &spanRect,
                               const QRect &rect) const
{
    QLinearGradient grad(spanRect.topLeft(), spanRect.bottomLeft());
    QColor menuColor = mergedColors(baseColor(), QColor(244, 244, 244), 25);
    grad.setColorAt(0, menuColor.lighter(112));
    grad.setColorAt(1, menuColor);
    p->fillRect(rect, grad);
}

void NGStyle::menuGradient(QPainter *painter, const QRect &spanRect,
                         const QRect &clipRect) const
{
    if (usePixmapCache()) {
        QString key;
        key.sprintf("mh_menu %d %d %d %d %d",
            spanRect.width(), spanRect.height(), clipRect.width(),
            clipRect.height(), baseColor().rgb());

        QPixmap pixmap;
        if (!QPixmapCache::find(key, pixmap)) {
            pixmap = QPixmap(clipRect.size());
            QPainter p(&pixmap);
            QRect rect = QRect(0, 0, clipRect.width(), clipRect.height());
            menuGradientHelper(&p, spanRect, rect);
            p.end();
            QPixmapCache::insert(key, pixmap);
        }

        painter->drawPixmap(clipRect.topLeft(), pixmap);
    } else {
        menuGradientHelper(painter, spanRect, clipRect);
    }
}

QColor NGStyle::mergedColors(const QColor &colorA, const QColor &colorB,
                           int factor) const
{
    const int maxFactor = 100;
    QColor tmp = colorA;
    tmp.setRed((tmp.red() * factor) / maxFactor +
               (colorB.red() * (maxFactor - factor)) / maxFactor);
    tmp.setGreen((tmp.green() * factor) / maxFactor +
                 (colorB.green() * (maxFactor - factor)) / maxFactor);
    tmp.setBlue((tmp.blue() * factor) / maxFactor +
                (colorB.blue() * (maxFactor - factor)) / maxFactor);
    return tmp;
}

//------------------------------------------------------------------------------
// Theme
//------------------------------------------------------------------------------

NGTheme::NGTheme(const QString &name, QObject *parent) : QObject(parent)
{
    m_sName = name;
    m_eWidgetStyle = StyleFlat;

    const QMetaObject &m = NGTheme::staticMetaObject;
    m_aoColors.resize(m.enumerator(m.indexOfEnumerator("Color")).keyCount());
    m_aoImageFiles.resize(m.enumerator(m.indexOfEnumerator("ImageFile")).keyCount());
    m_aoGradients.resize(m.enumerator(m.indexOfEnumerator("Gradient")).keyCount());
    m_abFlags.resize(m.enumerator(m.indexOfEnumerator("Flag")).keyCount());
}

NGTheme::~NGTheme()
{
}

NGTheme::WidgetStyle NGTheme::widgetStyle() const
{
    return m_eWidgetStyle;
}

QStringList NGTheme::preferredStyles() const
{
    return m_lsPreferredStyles;
}

bool NGTheme::flag(NGTheme::Flag f) const
{
    return m_abFlags[f];
}

QColor NGTheme::color(NGTheme::Color role) const
{
    return m_aoColors[role].first;
}

QString NGTheme::imageFile(NGTheme::ImageFile imageFile,
                         const QString &fallBack) const
{
    const QString &file = m_aoImageFiles.at(imageFile);
    return file.isEmpty() ? fallBack : file;
}

QGradientStops NGTheme::gradient(NGTheme::Gradient role) const
{
    return m_aoGradients[role];
}

QPair<QColor, QString> NGTheme::readNamedColor(const QString &color) const
{
    if (m_moPalette.contains(color))
        return qMakePair(m_moPalette[color], color);
    if (color == QLatin1String("style"))
        return qMakePair(QColor(), QString());

    bool ok = true;
    const QRgb rgba = static_cast<QRgb>(color.toLongLong(&ok, 16));
    if (!ok) {
        qWarning("Color \"%s\" is neither a named color nor a valid color",
                 qPrintable(color));
        return qMakePair(Qt::black, QString());
    }
    return qMakePair(QColor::fromRgba(rgba), QString());
}

QString NGTheme::filePath() const
{
    return m_sFileName;
}

QString NGTheme::name() const
{
    return m_sName;
}

void NGTheme::setName(const QString &name)
{
    m_sName = name;
}

QVariantHash NGTheme::values() const
{
    QVariantHash result;
    const QMetaObject &m = *metaObject();
    {
        const QMetaEnum e = m.enumerator(m.indexOfEnumerator("Color"));
        for (int i = 0, total = e.keyCount(); i < total; ++i) {
            const QString key = QLatin1String(e.key(i));
            const QPair<QColor, QString> &var = m_aoColors.at(i);
            result.insert(key, var.first);
        }
    }

    {
        const QMetaEnum e = m.enumerator(m.indexOfEnumerator("Flag"));
        for (int i = 0, total = e.keyCount(); i < total; ++i) {
            const QString key = QLatin1String(e.key(i));
            result.insert(key, flag(static_cast<NGTheme::Flag>(i)));
        }
    }

    {
        const QMetaEnum e = m.enumerator(m.indexOfEnumerator("WidgetStyle"));
        result.insert(QLatin1String("WidgetStyle"),
                      QLatin1String(e.valueToKey(widgetStyle())));
    }

    return result;
}

QColor NGTheme::readColor(const QString &color)
{
    bool ok = true;
    const QRgb rgba = static_cast<QRgb>(color.toLongLong(&ok, 16));
    return QColor::fromRgba(rgba);
}

QString NGTheme::writeColor(const QColor &color)
{
    return QString::number(color.rgba(), 16);
}

void NGTheme::writeSettings(const QString &filename) const
{
    QSettings settings(filename, QSettings::IniFormat);

    const QMetaObject &m = *metaObject();
    settings.setValue(QLatin1String("ThemeName"), m_sName);
    settings.setValue(QLatin1String("PreferredStyles"), m_lsPreferredStyles);
    settings.beginGroup(QLatin1String("Palette"));
    for (int i = 0, total = m_aoColors.size(); i < total; ++i) {
        const QPair<QColor, QString> var = m_aoColors[i];
        if (var.second.isEmpty())
            continue;
        settings.setValue(var.second, writeColor(var.first));
    }
    settings.endGroup();

    {
        settings.beginGroup(QLatin1String("Colors"));
        const QMetaEnum e = m.enumerator(m.indexOfEnumerator("Color"));
        for (int i = 0, total = e.keyCount(); i < total; ++i) {
            const QString key = QLatin1String(e.key(i));
            const QPair<QColor, QString> var = m_aoColors[i];
            if (!var.second.isEmpty())
                settings.setValue(key, var.second); // named color
            else
                settings.setValue(key, writeColor(var.first));
        }
        settings.endGroup();
    }

    {
        settings.beginGroup(QLatin1String("ImageFiles"));
        const QMetaEnum e = m.enumerator(m.indexOfEnumerator("ImageFile"));
        for (int i = 0, total = e.keyCount(); i < total; ++i) {
            const QString key = QLatin1String(e.key(i));
            const QString &var = m_aoImageFiles.at(i);
            if (!var.isEmpty())
                settings.setValue(key, var);
        }
        settings.endGroup();
    }

    {
        settings.beginGroup(QLatin1String("Gradients"));
        const QMetaEnum e = m.enumerator(m.indexOfEnumerator("Gradient"));
        for (int i = 0, total = e.keyCount(); i < total; ++i) {
            const QString key = QLatin1String(e.key(i));
            QGradientStops stops = gradient(static_cast<NGTheme::Gradient>(i));
            settings.beginWriteArray(key);
            int k = 0;
            foreach (const QGradientStop stop, stops) {
                settings.setArrayIndex(k);
                settings.setValue(QLatin1String("pos"), stop.first);
                settings.setValue(QLatin1String("color"),
                                  writeColor(stop.second));
                ++k;
            }
            settings.endArray();
        }
        settings.endGroup();
    }

    {
        settings.beginGroup(QLatin1String("Flags"));
        const QMetaEnum e = m.enumerator(m.indexOfEnumerator("Flag"));
        for (int i = 0, total = e.keyCount(); i < total; ++i) {
            const QString key = QLatin1String(e.key(i));
            settings.setValue(key, flag(static_cast<NGTheme::Flag>(i)));
        }
        settings.endGroup();
    }

    {
        settings.beginGroup(QLatin1String("Style"));
        const QMetaEnum e = m.enumerator(m.indexOfEnumerator("WidgetStyle"));
        settings.setValue(QLatin1String("WidgetStyle"),
                          QLatin1String(e.valueToKey(widgetStyle ())));
        settings.endGroup();
    }
}

void NGTheme::readSettings(QSettings &settings)
{
    m_sFileName = settings.fileName();
    const QMetaObject &m = *metaObject();
    m_sName = settings.value(QLatin1String("ThemeName"),
                             QLatin1String("unnamed")).toString();
    m_lsPreferredStyles = settings.value(
                QLatin1String("PreferredStyles")).toStringList();

    settings.beginGroup(QLatin1String("Palette"));
    foreach (const QString &key, settings.allKeys()) {
        QColor c = readColor(settings.value(key).toString());
        m_moPalette[key] = c;
    }
    settings.endGroup();

    {
        settings.beginGroup(QLatin1String("Style"));
        QMetaEnum e = m.enumerator(m.indexOfEnumerator("WidgetStyle"));
        QString val = settings.value(QLatin1String("WidgetStyle")).toString();
        m_eWidgetStyle = static_cast<NGTheme::WidgetStyle>(
                    e.keysToValue (val.toLatin1().data()));
        settings.endGroup();
    }

    {
        settings.beginGroup(QLatin1String("Colors"));
        QMetaEnum e = m.enumerator(m.indexOfEnumerator("Color"));
        for (int i = 0, total = e.keyCount(); i < total; ++i) {
            const QString key = QLatin1String(e.key(i));
            if (!settings.contains(key)) {
                qWarning("Theme \"%s\" misses color setting for key \"%s\".",
                         qPrintable(m_sFileName), qPrintable(key));
                continue;
            }
            m_aoColors[i] = readNamedColor(settings.value(key).toString());
        }
        settings.endGroup();
    }

    {
        settings.beginGroup(QLatin1String("ImageFiles"));
        QMetaEnum e = m.enumerator(m.indexOfEnumerator("ImageFile"));
        for (int i = 0, total = e.keyCount(); i < total; ++i) {
            const QString key = QLatin1String(e.key(i));
            m_aoImageFiles[i] = settings.value(key).toString();
        }
        settings.endGroup();
    }

    {
        settings.beginGroup(QLatin1String("Gradients"));
        QMetaEnum e = m.enumerator(m.indexOfEnumerator("Gradient"));
        for (int i = 0, total = e.keyCount(); i < total; ++i) {
            const QString key = QLatin1String(e.key(i));
            QGradientStops stops;
            int size = settings.beginReadArray(key);
            for (int j = 0; j < size; ++j) {
                settings.setArrayIndex(j);
                double pos = settings.value(QLatin1String("pos")).toDouble();
                QColor c = readColor(settings.value(
                                         QLatin1String("color")).toString());
                stops.append(qMakePair(pos, c));
            }
            settings.endArray();
            m_aoGradients[i] = stops;
        }
        settings.endGroup();
    }

    {
        settings.beginGroup(QLatin1String("Flags"));
        QMetaEnum e = m.enumerator(m.indexOfEnumerator("Flag"));
        for (int i = 0, total = e.keyCount(); i < total; ++i) {
            const QString key = QLatin1String(e.key(i));
            m_abFlags[i] = settings.value(key).toBool();
        }
        settings.endGroup();
    }
}

QPalette NGTheme::initialPalette()
{
    static QPalette palette = QApplication::palette();
    return palette;
}

QPalette NGTheme::palette() const
{
    QPalette pal = initialPalette();
    if (!flag(DerivePaletteFromTheme))
        return pal;

    // FIXME: introduce some more color roles for this

    pal.setColor(QPalette::Window,          color(BackgroundColorNormal));
    pal.setBrush(QPalette::WindowText,      color(TextColorNormal));
    pal.setColor(QPalette::Base,            color(BackgroundColorNormal));
    pal.setColor(QPalette::AlternateBase,   color(BackgroundColorAlternate));
    pal.setColor(QPalette::Button,          color(BackgroundColorDark));
    pal.setColor(QPalette::BrightText,      Qt::red);
    pal.setBrush(QPalette::Text,            color(TextColorNormal));
    pal.setBrush(QPalette::ButtonText,      color(TextColorNormal));
    pal.setBrush(QPalette::ToolTipBase,     color(ToolTipBase));
    pal.setColor(QPalette::Highlight,       color(BackgroundColorSelected));
    pal.setColor(QPalette::Dark,            color(BackgroundColorDark));
    pal.setColor(QPalette::HighlightedText, Qt::white);
    pal.setColor(QPalette::ToolTipText,     color(ToolTipText));
    pal.setColor(QPalette::Link,            color(TextColorLink));
    pal.setColor(QPalette::LinkVisited,     color(TextColorLinkVisited));
    return pal;
}


QSize NGStyle::sizeFromContents(QStyle::ContentsType ct, const QStyleOption *opt,
                                const QSize &contentsSize, const QWidget *w) const
{
    QSize newSize = QProxyStyle::sizeFromContents(ct, opt, contentsSize, w);

    if (ct == CT_Splitter && w && w->property("minisplitter").toBool())
        return QSize(1, 1);
    else if (ct == CT_ComboBox && panelWidget(w))
        newSize += QSize(14, 0);
    return newSize;
}
