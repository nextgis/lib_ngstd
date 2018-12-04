#ifndef SIGNDIALOG_H
#define SIGNDIALOG_H

#include <QDialog>

namespace Ui {
class NGSignDialog;
}

class Q_DECL_HIDDEN NGSignDialog : public QDialog
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
