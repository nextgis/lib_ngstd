/******************************************************************************
 * Project: NextGIS widgets gallery
 * Purpose: QWizard and navigation style demonstration
 *****************************************************************************/
#include "demo_wizard.h"

#include <ngstd/widgets/combo_box.h>
#include <ngstd/widgets/components.h>
#include <ngstd/widgets/design_tokens.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWizardPage>

using ngstd::widgets::CardButton;
using ngstd::widgets::ComboBox;
using ngstd::widgets::ComponentMetric;
using ngstd::widgets::DesignTokens;

DemoWizard::DemoWizard(QWidget *parent) : QWizard(parent)
{
    setWindowTitle(tr("NextGIS Wizard"));
    setOption(QWizard::NoBackButtonOnStartPage, true);
    setMinimumSize(720, 480);

    addPage(createWelcomePage());
    addPage(createSettingsPage());
    addPage(createCommitPage());
    setButtonText(QWizard::BackButton, tr("Back"));
    setButtonText(QWizard::NextButton, tr("Next"));
    setButtonText(QWizard::CommitButton, tr("Apply"));
    setButtonText(QWizard::FinishButton, tr("Finish"));
    setButtonText(QWizard::CancelButton, tr("Cancel"));
}

QWizardPage *DemoWizard::createWelcomePage()
{
    QWizardPage *page = new QWizardPage;
    page->setTitle(tr("Connect a data source"));
    page->setSubTitle(
        tr("QWizard navigation and buttons are created by Qt and styled "
           "through ThemeController."));

    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(
        DesignTokens::componentMetric(ComponentMetric::WizardPageMargin),
        DesignTokens::componentMetric(ComponentMetric::WizardPageMargin),
        DesignTokens::componentMetric(ComponentMetric::WizardPageMargin),
        DesignTokens::componentMetric(ComponentMetric::WizardPageMargin));
    layout->setSpacing(
        DesignTokens::componentMetric(ComponentMetric::WizardPageSpacing));

    const auto createChoice = [page](const QString &title,
                                     const QString &description) {
        CardButton *card = new CardButton(page);
        card->setAutoExclusive(true);
        QLabel *titleLabel = new QLabel(title, card);
        titleLabel->setProperty("ngstdTypographyRole",
                                QStringLiteral("heading3"));
        QLabel *descriptionLabel = new QLabel(description, card);
        descriptionLabel->setProperty("_ngstdRole",
                                      QStringLiteral("secondary"));
        descriptionLabel->setWordWrap(true);
        card->contentLayout()->addWidget(titleLabel);
        card->contentLayout()->addWidget(descriptionLabel);
        card->setMinimumHeight(104);
        return card;
    };
    CardButton *recommended = createChoice(
        tr("Recommended installation"),
        tr("Standard component set for most users."));
    CardButton *custom = createChoice(
        tr("Custom installation"),
        tr("Choose plugins, libraries, and integrations manually."));
    QButtonGroup *choiceGroup = new QButtonGroup(page);
    choiceGroup->setExclusive(true);
    choiceGroup->addButton(recommended);
    choiceGroup->addButton(custom);
    recommended->setChecked(true);
    layout->addWidget(recommended);
    layout->addWidget(custom);
    layout->addStretch();
    return page;
}

QWizardPage *DemoWizard::createSettingsPage()
{
    QWizardPage *page = new QWizardPage;
    page->setTitle(tr("Connection settings"));
    page->setSubTitle(tr("Fields use shared corporate tokens."));

    QFormLayout *layout = new QFormLayout(page);
    layout->setContentsMargins(
        DesignTokens::componentMetric(ComponentMetric::WizardPageMargin),
        DesignTokens::componentMetric(ComponentMetric::WizardPageMargin),
        DesignTokens::componentMetric(ComponentMetric::WizardPageMargin),
        DesignTokens::componentMetric(ComponentMetric::WizardPageMargin));
    layout->setSpacing(
        DesignTokens::componentMetric(ComponentMetric::WizardPageSpacing));

    QLineEdit *nameEdit = new QLineEdit(tr("Primary connection"));
    ComboBox *sourceCombo = new ComboBox;
    sourceCombo->addItems(
        {tr("NextGIS Web"), tr("Local file"), tr("WFS service")});
    QCheckBox *cacheCheckBox = new QCheckBox(tr("Use local cache"));
    cacheCheckBox->setChecked(true);

    layout->addRow(tr("Name"), nameEdit);
    layout->addRow(tr("Source"), sourceCombo);
    layout->addRow(QString(), cacheCheckBox);
    return page;
}

QWizardPage *DemoWizard::createCommitPage()
{
    QWizardPage *page = new QWizardPage;
    page->setTitle(tr("Apply settings"));
    page->setSubTitle(
        tr("Commit and Finish use the primary variant; Back and Cancel use "
           "the secondary variant."));
    page->setCommitPage(true);

    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(
        DesignTokens::componentMetric(ComponentMetric::WizardPageMargin),
        DesignTokens::componentMetric(ComponentMetric::WizardPageMargin),
        DesignTokens::componentMetric(ComponentMetric::WizardPageMargin),
        DesignTokens::componentMetric(ComponentMetric::WizardPageMargin));
    QLabel *description = new QLabel(
        tr("Select Apply to complete the wizard."));
    description->setWordWrap(true);
    layout->addWidget(description);
    layout->addStretch();
    return page;
}
