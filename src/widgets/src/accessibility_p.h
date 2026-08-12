/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Private accessibility factories for semantic component states
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_ACCESSIBILITY_P_H
#define NGSTD_WIDGETS_ACCESSIBILITY_P_H

#include <QAccessible>

namespace ngstd {
namespace widgets {

class Button;
class ComboBox;
class Disclosure;

namespace internal {

QAccessible::Id registerButtonAccessibility(Button *button);
QAccessible::Id registerComboBoxAccessibility(ComboBox *comboBox);
QAccessible::Id registerDisclosureAccessibility(Disclosure *disclosure);
void unregisterAccessibility(QAccessible::Id identifier);

} // namespace internal
} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_ACCESSIBILITY_P_H
