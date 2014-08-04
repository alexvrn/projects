#include "passworddialogmodel.h"

PasswordDialogModel::PasswordDialogModel(QObject *parent) :
    QObject(parent)
{
}

Credentials PasswordDialogModel::credentials() const
{
    return m_Credentials;
}

void PasswordDialogModel::setCredentials(const Credentials& c)
{
    m_Credentials = c;
}

bool PasswordDialogModel::connect() const
{
    return m_bConnect;
}

void PasswordDialogModel::setConnect(bool _connect)
{
    m_bConnect = _connect;
}
