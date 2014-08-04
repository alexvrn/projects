#ifndef PASSWORDDIALOGVIEW_H
#define PASSWORDDIALOGVIEW_H

#include <QDialog>
#include <QWidget>
#include <QLineEdit>
#include <QMouseEvent>

#include "View/iviewdata.h"
#include "Service/credentials.h"
#include "View/iviewdialogpassword.h"

class PasswordDialogView : public QDialog, public IViewPasswordDialog, public IViewData
{
    Q_OBJECT
public:
    explicit PasswordDialogView(QWidget *parent = 0);
    ~PasswordDialogView();

    void dialogShow(bool);

    void setCredentials(const Credentials&);

    bool initControls();

protected:
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

signals:
    void cancel();
    void queryLogon();//const Credentials&);
    void isLogon(const Credentials&, bool);

    void credentialsEdit(const Credentials&);

private:
    QRegion maskBorder(int) const;

public slots:
    void accept();
    void reject();

private slots:
    void onCredentialsEdit(const QString &);

private:
    QWidget* form;

    QLineEdit *lineEditUser;
    QLineEdit *lineEditPassword;
    QLineEdit *lineEditServer;
    QLineEdit *lineEditService;

    QPoint m_ptPosition;//for move window

    static const quint16 BORDER_RADUIS;
};

#endif // PASSWORDDIALOGVIEW_H
