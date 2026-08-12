/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Theme widget decorator strategies
 *****************************************************************************/
#include "widget_decorator_p.h"

#include "state_journal_p.h"

#include <ngstd/widgets/icons.h>
#include <ngstd/widgets/design_tokens.h>
#include <ngstd/widgets/widget_style.h>

#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QVariant>

namespace ngstd {
namespace widgets {
namespace internal {

namespace {

class DialogButtonBoxDecorator final : public WidgetDecorator
{
public:
    bool supports(QWidget *widget) const override
    {
        return qobject_cast<QDialogButtonBox *>(widget) != nullptr;
    }

    void decorate(QWidget *widget, ColorScheme scheme,
                  const ThemeOptions &options,
                  StateJournal *journal) const override
    {
        Q_UNUSED(scheme)
        if (!options.isFeatureEnabled(ThemeFeature::DialogButtonBoxes) ||
            !journal) {
            return;
        }
        QDialogButtonBox *buttonBox = qobject_cast<QDialogButtonBox *>(widget);
        if (!buttonBox) return;
        for (QAbstractButton *button : buttonBox->buttons()) {
            if (!button || button->dynamicPropertyNames().contains(
                               QByteArrayLiteral("ngstdButtonVariant"))) {
                continue;
            }
            const QDialogButtonBox::ButtonRole role =
                buttonBox->buttonRole(button);
            const bool primary = role == QDialogButtonBox::AcceptRole ||
                                 role == QDialogButtonBox::ApplyRole ||
                                 role == QDialogButtonBox::YesRole;
            journal->setProperty(button,
                                 QByteArrayLiteral("ngstdButtonVariant"),
                                 primary ? QStringLiteral("primary")
                                         : QStringLiteral("secondary"));
            const QDialogButtonBox::StandardButton standardButton =
                buttonBox->standardButton(button);
            const bool reject =
                standardButton == QDialogButtonBox::Cancel ||
                standardButton == QDialogButtonBox::Close ||
                standardButton == QDialogButtonBox::Abort ||
                standardButton == QDialogButtonBox::Discard ||
                standardButton == QDialogButtonBox::No ||
                standardButton == QDialogButtonBox::NoToAll;
            journal->setProperty(
                button, QByteArrayLiteral("_ngstdIconRole"),
                static_cast<int>(reject ? IconRole::CloseCircle
                                        : IconRole::Check));
            const IconRole iconRole = reject ? IconRole::CloseCircle
                                             : IconRole::Check;
            const int iconSize = DesignTokens::controlIconSize();
            const QColor idleColor = DesignTokens::color(
                primary ? ColorRole::White : ColorRole::Text, scheme);
            const QColor activeColor = DesignTokens::color(
                primary ? ColorRole::White : ColorRole::LinkHover, scheme);
            const QColor disabledColor =
                DesignTokens::color(ColorRole::TextDisabled, scheme);
            QIcon buttonIcon;
            buttonIcon.addPixmap(
                iconPixmap(iconRole, QSize(iconSize, iconSize),
                           button->devicePixelRatioF(), idleColor),
                QIcon::Normal, QIcon::Off);
            buttonIcon.addPixmap(
                iconPixmap(iconRole, QSize(iconSize, iconSize),
                           button->devicePixelRatioF(), activeColor),
                QIcon::Active, QIcon::Off);
            buttonIcon.addPixmap(
                iconPixmap(iconRole, QSize(iconSize, iconSize),
                           button->devicePixelRatioF(), disabledColor),
                QIcon::Disabled, QIcon::Off);
            buttonIcon.addPixmap(
                iconPixmap(iconRole, QSize(iconSize, iconSize),
                           button->devicePixelRatioF(),
                           DesignTokens::color(ColorRole::White, scheme)),
                QIcon::Normal, QIcon::On);
            buttonIcon.addPixmap(
                iconPixmap(iconRole, QSize(iconSize, iconSize),
                           button->devicePixelRatioF(),
                           DesignTokens::color(ColorRole::White, scheme)),
                QIcon::Active, QIcon::On);
            button->setIcon(buttonIcon);
            button->setIconSize(QSize(iconSize, iconSize));
            WidgetStyle::refresh(button);
        }
    }
};

} // namespace

std::vector<std::unique_ptr<WidgetDecorator>> createWidgetDecorators()
{
    std::vector<std::unique_ptr<WidgetDecorator>> decorators;
    decorators.emplace_back(new DialogButtonBoxDecorator);
    return decorators;
}

} // namespace internal
} // namespace widgets
} // namespace ngstd
