#ifndef SIGNDIALOG_H
#define SIGNDIALOG_H

#include "framework/framework.h"

#include <QDialog>

namespace Ui {
class NGSignDialog;
}

class NGFRAMEWORK_EXPORT NGSignDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NGSignDialog(QWidget *parent = nullptr);
    virtual ~NGSignDialog();
    void updateContent();

    QPushButton *getSignButton () const;

private slots:
    void onSignClicked();

private:
    Ui::NGSignDialog *ui;
};

#endif // SIGNDIALOG_H
