#include "signdialog.h"
#include "ui_signdialog.h"

#include "access.h"

NGSignDialog::NGSignDialog(QWidget *parent) :
    QDialog(parent, Qt::FramelessWindowHint | Qt::Popup),
    ui(new Ui::NGSignDialog)
{
    ui->setupUi(this);
    update();
}

NGSignDialog::~NGSignDialog()
{
    delete ui;
}

void NGSignDialog::update()
{
    if(NGAccess::instance().isUserAuthorized()) {
        // Show user info in widget
        // Name, big avatar, plan
        ui->signButton->setText(tr("Exit"));
        ui->learnMore->setText(tr("<a href=\"http://my.nextgis.com\">Account</a>"));
        ui->userInfo->show();
        ui->userInfo->setText(QString("<html><head/><body><p>%1<br>%2</p><p><b>%3</b></p></body></html>")
                              .arg(NGAccess::instance().firstName())
                              .arg(NGAccess::instance().lastName())
                              .arg(NGAccess::instance().isUserSupported() ?
                                       tr("Supported") : tr("Unsupported")));
        ui->avatar->show();
        ui->avatar->setText(QString("<html><head/><body><p><img src=\"%1\" height=\"64\"/></p></body></html>")
                            .arg(NGAccess::instance().avatarFilePath()));

        ui->ngLogo->hide();
        ui->descriptionText->hide();
    }
    else {
        // Show sign in button and explaration
        ui->signButton->setText(tr("Sign in"));
        ui->learnMore->setText(tr("<a href=\"http://nextgis.com/pricing/\">Learn more ...</a>"));
        ui->avatar->hide();
        ui->userInfo->hide();
        ui->ngLogo->show();
        ui->descriptionText->show();
    }
}

void NGSignDialog::onSignClicked()
{
    if(NGAccess::instance().isUserAuthorized()) {
        NGAccess::instance().exit();
    }
    else {
        NGAccess::instance().authorize();
        hide();
    }
}
