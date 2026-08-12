/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Explicit adapters for existing Qt widgets
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_ADAPTERS_H
#define NGSTD_WIDGETS_ADAPTERS_H

#include <ngstd/widgets/widgets.h>

#include <QObject>
#include <QScopedPointer>

class QComboBox;
class QEvent;
class QWizard;

namespace ngstd {
namespace widgets {

class ComboBoxAdapterPrivate;
class WizardAdapterPrivate;

class NGSTD_WIDGETS_EXPORT ComboBoxAdapter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool attached READ isAttached NOTIFY attachedChanged)

public:
    static ComboBoxAdapter *attach(QComboBox *comboBox);
    ~ComboBoxAdapter() override;

    QComboBox *comboBox() const;
    bool isAttached() const;

    void apply();
    void detach();

signals:
    void attachedChanged(bool attached);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    NGSTD_WIDGETS_LOCAL explicit ComboBoxAdapter(QComboBox *comboBox);
    NGSTD_WIDGETS_LOCAL void refreshPopup(bool position = false);

    Q_DISABLE_COPY(ComboBoxAdapter)
    QScopedPointer<ComboBoxAdapterPrivate> d;
};

class NGSTD_WIDGETS_EXPORT WizardAdapter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(
        bool changeWizardStyle
        READ changeWizardStyle
        WRITE setChangeWizardStyle
        RESET resetChangeWizardStyle
        NOTIFY changeWizardStyleChanged)
    Q_PROPERTY(bool attached READ isAttached NOTIFY attachedChanged)

public:
    static WizardAdapter *attach(QWizard *wizard);
    ~WizardAdapter() override;

    QWizard *wizard() const;
    bool changeWizardStyle() const;
    void setChangeWizardStyle(bool enabled);
    void resetChangeWizardStyle();
    bool isAttached() const;

    void apply();
    void detach();

signals:
    void changeWizardStyleChanged(bool enabled);
    void attachedChanged(bool attached);

private:
    NGSTD_WIDGETS_LOCAL explicit WizardAdapter(QWizard *wizard);

    Q_DISABLE_COPY(WizardAdapter)
    QScopedPointer<WizardAdapterPrivate> d;
};

} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_ADAPTERS_H
