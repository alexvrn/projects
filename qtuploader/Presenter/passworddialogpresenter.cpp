#include "passworddialogpresenter.h"

#include "View/iview.h"
#include "View/iviewdialogpassword.h"
#include "Model/passworddialogmodel.h"

#include <QMessageBox>
#include <QDebug>
#include <QMutexLocker>
#include <QMutex>

PasswordDialogPresenter::PasswordDialogPresenter(IView *view, QObject *parent) :
    QObject(parent)
{
    m_view = (IViewPasswordDialog*)view;

    QObject *view_obj = dynamic_cast<QObject*>(view);
    QObject::connect(view_obj, SIGNAL(queryLogon()),                        this, SLOT(sltQueryLogon()));
    QObject::connect(this,     SIGNAL(isLogon(const Credentials&, bool)),   this, SLOT(sltIsLogon(const Credentials&, bool)));
    QObject::connect(view_obj, SIGNAL(credentialsEdit(const Credentials&)), this, SLOT(sltCredentialsEdit(const Credentials&)));

    //чтение логина-пароля
    Credentials c;
    Credentials::readCredentials(c);
    m_model.setConnect(false);
    m_model.setCredentials(c);
    refreshView();
}

PasswordDialogPresenter::~PasswordDialogPresenter()
{
}

void PasswordDialogPresenter::refreshView()
{
    m_view->setCredentials(m_model.credentials());
    m_view->dialogShow(!m_model.connect());
}

void PasswordDialogPresenter::sltCredentialsEdit(const Credentials &c)
{
    m_model.setCredentials(c);
}

bool PasswordDialogPresenter::checkInputData(Credentials &c, QString& error)
{
    try
    {
        c = m_model.credentials();
        c.setLogin(c.login().trimmed());
        c.setServer(c.server().toLower().replace(QRegExp(" "), ""));
        c.setService(c.service().toLower().replace(QRegExp(" "), ""));
        c.setAlreadyAuthorized(false);
        c.setTerminateUser(false);

        if(c.login().isEmpty()) throw QString("Input login!");
        if(c.password().isEmpty()) throw QString("Input password!");

        //Mask for IP_address
        QRegExp rx_ip("\\b((([01]?\\d?\\d|2[0-4]\\d|25[0-5])\\.){3}([01]?\\d?\\d|2[0-4]\\d|25[0-5])|localhost)\\b");
        if(!rx_ip.exactMatch(c.server())) throw QString("Input correctly server!");
        if(!rx_ip.exactMatch(c.service())) throw QString("Input correctly service!");

        return true;
    }
    catch(const QString& str_error)
    {
        error = str_error;
        return false;
    }
    catch(...)
    {
        error = "Error sltQueryLogon()";
        return false;
    }
}

void PasswordDialogPresenter::sendRequest(bool terminatedUser)
{
    Credentials c;
    QString str_error = "";
    if(checkInputData(c, str_error))
    {
        c.setTerminateUser(terminatedUser);
        emit queryLogon(c);
    }
    else
    {
        qDebug() << str_error;
        QMessageBox::information(0, "Error of input!", str_error);
    }
}

void PasswordDialogPresenter::sltQueryLogon()//const Credentials & c)
{
    sendRequest(false);
}

void PasswordDialogPresenter::sltIsLogon(const Credentials& c, bool connect)
{
    if(connect)
    {
        QMutexLocker locker(&mutex);
        Credentials::writeCredentials(c);
    }
    else
    {
        if(m_model.connect())//if Connect now
        {
            QMessageBox::information(0, tr("Authorization"), tr("Other user want authorization!"));
        }
        else
        {
            if(!c.alreadyAuthorized())
                QMessageBox::information(0, tr("Authorization"), tr("Incorrect login-password!"));
            else
            {   //if the user is already authorized
                bool terminatedUser = (QMessageBox::Yes == QMessageBox::question(0, tr("Authorization"), tr("Terminated user?")));
                if(terminatedUser)
                {
                    sendRequest(terminatedUser);
                }
            }
        }
    }
    m_model.setConnect(connect);
    refreshView();
}

void PasswordDialogPresenter::sltLogoutCommand()
{
    //Credentials c;
    //Credentials::readCredentials(c);
    m_model.setConnect(false);
    refreshView();
}
