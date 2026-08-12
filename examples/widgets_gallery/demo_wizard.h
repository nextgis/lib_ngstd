/******************************************************************************
 * Project: NextGIS widgets gallery
 * Purpose: QWizard and navigation style demonstration
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_GALLERY_DEMO_WIZARD_H
#define NGSTD_WIDGETS_GALLERY_DEMO_WIZARD_H

#include <QWizard>

class DemoWizard : public QWizard
{
    Q_OBJECT

public:
    explicit DemoWizard(QWidget *parent = nullptr);

private:
    QWizardPage *createWelcomePage();
    QWizardPage *createSettingsPage();
    QWizardPage *createCommitPage();
};

#endif // NGSTD_WIDGETS_GALLERY_DEMO_WIZARD_H
