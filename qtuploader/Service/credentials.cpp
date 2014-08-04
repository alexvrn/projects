#include "credentials.h"
#include "Service/serialization.h"
#include "Service/setting.h"

#include <QDomDocument>
#include <QFile>
#include <QMetaProperty>
#include <QVariant>
#include <QTextStream>
#include <typeinfo>
#include <QDebug>
#include <QSettings>
#include <QCryptographicHash>
#include <QNetworkInterface>
#include <QHostInfo>

Credentials::Credentials(QObject *parent)
    : QObject(parent),
      m_strLogin(QString()),
      m_strPassword(QString()),
      m_strPasswordHash(QString()),
      m_strServer(QString()),
      m_strService(QString()),
      m_bAdmin(false),
      m_bIsRemoteService(false),
      m_bAlreadyAuthorized(false),
      m_bTerminateUser(false)
{

}

Credentials::Credentials(
            const QString& login,
            const QString& password,
            const QString& server,
            const QString& service,
            bool _admin)
{
    setLogin(login);
    setPassword(password);
    setServer(server);
    setService(service);
    setAdmin(_admin);
}

Credentials::Credentials(const Credentials& obj)
    : QObject()
{
    setLogin(obj.login());
    setPassword(obj.password());
    setPasswordHash(obj.passwordHash());
    setServer(obj.server());
    setService(obj.service());
    setAdmin(obj.admin());
    setAlreadyAuthorized(obj.alreadyAuthorized());
    setTerminateUser(obj.terminateUser());
    setRemoteService(obj.isRemoteService());
}

Credentials& Credentials::operator=(const Credentials& obj)
{
    //проверка на самоприсваивание
    if (this == &obj)
    {
        return *this;
    }

    m_bAdmin             = obj.admin();
    m_strLogin           = obj.login();
    m_strPassword        = obj.password();
    m_strPasswordHash    = obj.passwordHash();
    m_strServer          = obj.server();
    m_strService         = obj.service();
    m_bIsOfRefServer     = obj.IsOfRefServer();
    m_bIsRemoteService   = obj.isRemoteService();
    m_bAlreadyAuthorized = obj.alreadyAuthorized();
    m_bTerminateUser     = obj.terminateUser();
    return *this;
}

QString Credentials::login() const
{
    return m_strLogin;
}

void Credentials::setLogin(const QString &_login)
{
    m_strLogin = _login;
}

QString Credentials::password() const
{
    return m_strPassword;
}

void Credentials::setPassword(const QString &_password)
{
    m_strPassword = _password;

    //
    if(m_strPasswordHash != _password && !_password.isEmpty())
        m_strPasswordHash = QString(QCryptographicHash::hash(_password.toUtf8(),  QCryptographicHash::Sha1).toHex());
}

QString Credentials::passwordHash() const
{
    return m_strPasswordHash;
}

void Credentials::setPasswordHash(const QString& pswhash)
{
    m_strPasswordHash = pswhash;
}

QString Credentials::server() const
{
    return m_strServer;
}

void Credentials::setServer(const QString& srv)
{
    m_strServer = srv;
}

QString Credentials::service() const
{
    return m_strService;
}

void Credentials::setService(const QString& srvc)
{
    m_strService = srvc;

    //to specify the remote service
    setRemoteService(true);
    for(const QHostAddress &address: QNetworkInterface::allAddresses())
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol &&
           address.toString() == (m_strService.toLower() == "localhost" ?
                                    QHostAddress(QHostAddress::LocalHost).toString() : m_strService))
        {
            setRemoteService(false);
            break;
        }
    }
}

bool Credentials::isRemoteService() const
{
    return m_bIsRemoteService;
}

void Credentials::setRemoteService(bool remote)
{
    m_bIsRemoteService = remote;
}

bool Credentials::IsOfRefServer() const
{
    return m_bIsOfRefServer;
}

bool Credentials::admin() const
{
    return m_bAdmin;
}

void Credentials::setAdmin(bool _admin)
{
    m_bAdmin = _admin;
}

bool Credentials::alreadyAuthorized() const
{
    return m_bAlreadyAuthorized;
}

void Credentials::setAlreadyAuthorized(bool aa)
{
    m_bAlreadyAuthorized = aa;
}

bool Credentials::terminateUser() const
{
    return m_bTerminateUser;
}

void Credentials::setTerminateUser(bool tu)
{
    m_bTerminateUser = tu;
}

QString Credentials::pathToConfig()
{
    return Setting::value(Setting::KEY_CREDENTIAL_PATH).toString();
}

void Credentials::writeCredentials(const Credentials & c)
{
    if(!Serialization::SerializeToXML(c, pathToConfig()))
    {
        qDebug() << "Error of serialization!";
        return;
    }
}

void Credentials::readCredentials(Credentials & c)
{
    if(!Serialization::DeserializeFromXML(&c, pathToConfig()))
    {
        qDebug() << "Error of Deserialization!";
        return;
    }

    //
    c.setPassword(c.passwordHash());
}
