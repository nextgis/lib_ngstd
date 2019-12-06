/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: Framework library
*  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
*  Copyright (C) 2012-2019 NextGIS, info@nextgis.ru
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 2 of the License, or
*   (at your option) any later version.
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
#include "signdialog.h"
#include "ui_signdialog.h"

#include "access.h"

NGSignDialog::NGSignDialog(QWidget *parent) :
    QDialog(parent, Qt::FramelessWindowHint | Qt::Popup),
    ui(new Ui::NGSignDialog)
{
    ui->setupUi(this);
    updateContent();
}

NGSignDialog::~NGSignDialog()
{
    delete ui;
}

void NGSignDialog::updateContent()
{
    if(NGAccess::instance().isEnterprise()) {
        ui->signButton->hide();
        ui->learnMore->hide();
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
    else if(NGAccess::instance().isUserAuthorized()) {
        // Show user info in widget
        // Name, big avatar, plan
        ui->signButton->setText(tr("Sign out"));
        ui->learnMore->setText(QString("<a href=\"%1\">%2</a>").arg(NGAccess::instance().endPoint()).arg(tr("Account")));
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

QPushButton *NGSignDialog::getSignButton () const
{
    return ui->signButton;
}

void NGSignDialog::onSignClicked()
{
    if(NGAccess::instance().isUserAuthorized()) {
        NGAccess::instance().exit();
    }
    else {
        NGAccess::instance().authorize();
    }
    hide();
}
