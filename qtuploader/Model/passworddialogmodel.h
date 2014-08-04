#ifndef PASSWORDDIALOGMODEL_H
#define PASSWORDDIALOGMODEL_H

#include <QObject>
#include "Service/credentials.h"

class PasswordDialogModel : public QObject
{
    Q_OBJECT
public:
    explicit PasswordDialogModel(QObject *parent = 0);

    Credentials credentials() const;
    void setCredentials(const Credentials&);

    bool connect() const;
    void setConnect(bool);

signals:

public slots:

private:
    Credentials m_Credentials;

    bool m_bConnect;

};

#endif // PASSWORDDIALOGMODEL_H
