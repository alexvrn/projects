#ifndef PASSWORDDIALOGPRESENTER_H
#define PASSWORDDIALOGPRESENTER_H

#include <QObject>
#include <QMutex>

#include "View/iview.h"
#include "View/iviewdialogpassword.h"
#include "Service/credentials.h"
#include "Model/passworddialogmodel.h"

class IView;
class IViewPasswordDialog;

class PasswordDialogPresenter : public QObject
{
    Q_OBJECT
public:
    explicit PasswordDialogPresenter(IView *view, QObject *parent = 0);
    ~PasswordDialogPresenter();

signals:
    void isLogon(const Credentials&, bool);
    void queryLogon(const Credentials&);

public slots:
    void sltIsLogon(const Credentials&, bool);
    void sltLogoutCommand();
    void sltQueryLogon();//const Credentials&);
    void sltCredentialsEdit(const Credentials&);

private:
    void refreshView();
    //data checking on correctly
    bool checkInputData(Credentials &, QString& );

    //terminateUser=true - if server sent request about repeated autorization  for the same User
    void sendRequest(bool terminatedUser);

private:
     IViewPasswordDialog* m_view;

     PasswordDialogModel m_model;

     QMutex mutex;
};

#endif // PASSWORDDIALOGPRESENTER_H
