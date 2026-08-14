/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Private accessibility factories for semantic component states
 *****************************************************************************/
#include "accessibility_p.h"

#include <ngstd/widgets/button.h>
#include <ngstd/widgets/card.h>
#include <ngstd/widgets/combo_box.h>
#include <ngstd/widgets/disclosure.h>

#include <QAccessible>
#include <QAccessibleWidget>
#include <QButtonGroup>

namespace {

class ButtonAccessible final : public QAccessibleWidget
{
public:
    explicit ButtonAccessible(ngstd::widgets::Button *button)
        : QAccessibleWidget(button, QAccessible::Button)
    {}

    QAccessible::State state() const override
    {
        QAccessible::State result = QAccessibleWidget::state();
        const auto *button =
            qobject_cast<const ngstd::widgets::Button *>(object());
        if (!button) return result;
        result.focusable = true;
        result.checkable = button->isCheckable();
        result.checked = button->isChecked();
        result.busy = button->isLoading();
        return result;
    }

    QString text(QAccessible::Text textType) const override
    {
        QString baseText = QAccessibleWidget::text(textType);
        if (!baseText.isEmpty() || textType != QAccessible::Name)
            return baseText;
        const auto *button =
            qobject_cast<const ngstd::widgets::Button *>(object());
        return button ? button->text() : QString();
    }

    QStringList actionNames() const override
    {
        return {QAccessibleActionInterface::pressAction()};
    }

    void doAction(const QString &actionName) override
    {
        auto *button = qobject_cast<ngstd::widgets::Button *>(object());
        if (button && !button->isLoading() &&
            actionName == QAccessibleActionInterface::pressAction()) {
            button->click();
            return;
        }
        QAccessibleWidget::doAction(actionName);
    }
};

class CardButtonAccessible final : public QAccessibleWidget
{
public:
    explicit CardButtonAccessible(ngstd::widgets::CardButton *button)
        : QAccessibleWidget(button, QAccessible::CheckBox)
    {}

    QAccessible::Role role() const override
    {
        const auto *button =
            qobject_cast<const ngstd::widgets::CardButton *>(object());
        if (!button) return QAccessible::CheckBox;
        const QButtonGroup *group = button->group();
        return button->autoExclusive() || (group && group->exclusive())
                   ? QAccessible::RadioButton
                   : QAccessible::CheckBox;
    }

    QAccessible::State state() const override
    {
        QAccessible::State result = QAccessibleWidget::state();
        const auto *button =
            qobject_cast<const ngstd::widgets::CardButton *>(object());
        if (!button) return result;
        result.focusable = true;
        result.checkable = button->isCheckable();
        result.checked = button->isChecked();
        return result;
    }

    QStringList actionNames() const override
    {
        return {QAccessibleActionInterface::toggleAction()};
    }

    QString text(QAccessible::Text textType) const override
    {
        const auto *button =
            qobject_cast<const ngstd::widgets::CardButton *>(object());
        if (button && textType == QAccessible::Value)
            return button->isChecked() ? QStringLiteral("checked")
                                       : QStringLiteral("unchecked");
        return QAccessibleWidget::text(textType);
    }

    void doAction(const QString &actionName) override
    {
        auto *button =
            qobject_cast<ngstd::widgets::CardButton *>(object());
        if (button && button->isEnabled() && button->isCheckable() &&
            actionName == QAccessibleActionInterface::toggleAction()) {
            button->click();
            return;
        }
        QAccessibleWidget::doAction(actionName);
    }
};

class DisclosureAccessible final : public QAccessibleWidget
{
public:
    explicit DisclosureAccessible(ngstd::widgets::Disclosure *disclosure)
        : QAccessibleWidget(disclosure, QAccessible::Grouping)
    {}

    QAccessible::State state() const override
    {
        QAccessible::State result = QAccessibleWidget::state();
        const auto *disclosure =
            qobject_cast<const ngstd::widgets::Disclosure *>(object());
        if (!disclosure) return result;
        result.expandable = true;
        result.expanded = disclosure->isExpanded();
        result.collapsed = !disclosure->isExpanded();
        return result;
    }

    QString text(QAccessible::Text textType) const override
    {
        QString baseText = QAccessibleWidget::text(textType);
        if (!baseText.isEmpty() || textType != QAccessible::Name)
            return baseText;
        const auto *disclosure =
            qobject_cast<const ngstd::widgets::Disclosure *>(object());
        return disclosure ? disclosure->title() : QString();
    }

    QStringList actionNames() const override
    {
        return {QAccessibleActionInterface::toggleAction()};
    }

    void doAction(const QString &actionName) override
    {
        auto *disclosure =
            qobject_cast<ngstd::widgets::Disclosure *>(object());
        if (disclosure &&
            actionName == QAccessibleActionInterface::toggleAction()) {
            disclosure->setExpanded(!disclosure->isExpanded());
            return;
        }
        QAccessibleWidget::doAction(actionName);
    }
};

class ComboBoxAccessible final : public QAccessibleWidget
{
public:
    explicit ComboBoxAccessible(ngstd::widgets::ComboBox *comboBox)
        : QAccessibleWidget(comboBox, QAccessible::ComboBox)
    {}

    QAccessible::State state() const override
    {
        QAccessible::State result = QAccessibleWidget::state();
        const auto *comboBox =
            qobject_cast<const ngstd::widgets::ComboBox *>(object());
        if (!comboBox) return result;
        const QFrame *popup = comboBox->findChild<QFrame *>(
            QStringLiteral("_ngstdComboBoxPopup"));
        result.focusable = true;
        result.hasPopup = true;
        result.expanded =
            popup && comboBox->property("_ngstdPopupOpen").toBool();
        result.collapsed = !result.expanded;
        return result;
    }

    QString text(QAccessible::Text textType) const override
    {
        const auto *comboBox =
            qobject_cast<const ngstd::widgets::ComboBox *>(object());
        if (!comboBox) return QString();
        if (textType == QAccessible::Name &&
            !comboBox->accessibleName().isEmpty()) {
            return comboBox->accessibleName();
        }
        if (textType == QAccessible::Name || textType == QAccessible::Value)
            return comboBox->currentText();
        return QAccessibleWidget::text(textType);
    }

    QStringList actionNames() const override
    {
        return {QAccessibleActionInterface::pressAction()};
    }

    void doAction(const QString &actionName) override
    {
        auto *comboBox = qobject_cast<ngstd::widgets::ComboBox *>(object());
        if (comboBox &&
            actionName == QAccessibleActionInterface::pressAction()) {
            const QFrame *popup = comboBox->findChild<QFrame *>(
                QStringLiteral("_ngstdComboBoxPopup"));
            if (popup && popup->isVisible())
                comboBox->hidePopup();
            else
                comboBox->showPopup();
            return;
        }
        QAccessibleWidget::doAction(actionName);
    }
};

} // namespace

namespace ngstd {
namespace widgets {
namespace internal {

QAccessible::Id registerButtonAccessibility(Button *button)
{
    // QAccessible owns the interface until deleteAccessibleInterface().
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    return QAccessible::registerAccessibleInterface(
        new ButtonAccessible(button));
}

QAccessible::Id registerCardButtonAccessibility(CardButton *button)
{
    // QAccessible owns the interface until deleteAccessibleInterface().
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    return QAccessible::registerAccessibleInterface(
        new CardButtonAccessible(button));
}

QAccessible::Id registerDisclosureAccessibility(Disclosure *disclosure)
{
    // QAccessible owns the interface until deleteAccessibleInterface().
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    return QAccessible::registerAccessibleInterface(
        new DisclosureAccessible(disclosure));
}

QAccessible::Id registerComboBoxAccessibility(ComboBox *comboBox)
{
    // QAccessible owns the interface until deleteAccessibleInterface().
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    return QAccessible::registerAccessibleInterface(
        new ComboBoxAccessible(comboBox));
}

void unregisterAccessibility(QAccessible::Id identifier)
{
    if (identifier) QAccessible::deleteAccessibleInterface(identifier);
}

} // namespace internal
} // namespace widgets
} // namespace ngstd
