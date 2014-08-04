#ifndef CREDENTIALS_H
#define CREDENTIALS_H

#include <QString>
#include <QObject>

class Credentials: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString login        READ login        WRITE setLogin)
    Q_PROPERTY(QString passwordHash READ passwordHash WRITE setPasswordHash)
    Q_PROPERTY(QString server       READ server       WRITE setServer)
    Q_PROPERTY(QString service      READ service      WRITE setService)
private:
    enum RegistryKeyIndices
    {
        SERVER = 0,
        LOGIN = 1,
        PASSWORD = 2,
        NO_ENCRYPT = 3
    };

    enum CredentialsType
    {
        CLIENT = 0,
        REF_SERVER = 1,
        CROSS_SERVER = 2
    };

public:
   explicit Credentials(QObject *parent = 0);
   Credentials(const QString& login,
               const QString& password,
               const QString& server,
               const QString& service,
               bool _admin = false);

   Credentials(const Credentials&);

   Credentials& operator=(const Credentials& obj);


   QString login() const;
   void setLogin(const QString&);

   QString password() const;
   void setPassword(const QString&);

   QString passwordHash() const;
   void setPasswordHash(const QString&);

   QString server() const;
   void setServer(const QString&);

   QString service() const;
   void setService(const QString&);

   bool alreadyAuthorized() const;
   void setAlreadyAuthorized(bool);

   bool terminateUser() const;
   void setTerminateUser(bool);

   bool admin() const;
   void setAdmin(bool);

   bool isRemoteService() const;

   bool IsOfRefServer() const;

   static void writeCredentials(const Credentials & c);
   static void readCredentials(Credentials & c);

private:
   static QString pathToConfig();

   void setRemoteService(bool);
   //static string[] RegistryKeys            = { "Server", "Login", "Password", "NoEncrypt" };
   //static string[] RefServerRegistryKeys   = { "RefServer", "LoginRefServer", "PasswordRefServer", "NoEncryptRefServer" };
   //static string[] CrossServerRegistryKeys = { "CrossServer", "LoginCrossServer", "PasswordCrossServer", "NoEncryptCrossServer" };
   //bool UseClearTextPassword { get; set; } // true for early versions of server which don't support password encryption

private:
   QString m_strLogin;
   QString m_strPassword;
   QString m_strPasswordHash;
   QString m_strServer;
   QString m_strService;
   bool    m_bIsOfRefServer;
   bool    m_bAdmin;
   bool    m_bIsRemoteService;
   bool    m_bAlreadyAuthorized;
   bool    m_bTerminateUser;
};

#endif // CREDENTIALS_H
