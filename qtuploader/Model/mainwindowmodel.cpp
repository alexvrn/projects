#include "mainwindowmodel.h"

#include <QObject>

const QString MainWindowModel::MainWindowTitle = QObject::tr("Uploader");

MainWindowModel::MainWindowModel()
{
    setStatusInfo(QString());
    setRemoteService(false);
    setService(QString());
}

QString MainWindowModel::statusInfo() const
{
    return m_strStatusInfo;
}

void MainWindowModel::setStatusInfo(const QString& inf)
{
    m_strStatusInfo = inf;
}

QString MainWindowModel::titleText() const
{
    return isRemoteService() ? QString("%1 (%2 [%3])").arg(MainWindowTitle).arg(QObject::tr("Remote service")).arg(service()) : QString(MainWindowTitle);
}

void MainWindowModel::setRemoteService(bool rs, const QString& srv)
{
    m_bRemoteService = rs;

    setService(srv);
}

bool MainWindowModel::isRemoteService() const
{
    return m_bRemoteService;
}

QString MainWindowModel::service() const
{
    return m_strService;
}

void MainWindowModel::setService(const QString& s)
{
    m_strService = s;
}

void MainWindowModel::setLanguage(Language lan)
{
    m_Language = lan;
}

MainWindowModel::Language MainWindowModel::language() const
{
    return m_Language;
}

QString MainWindowModel::textSwitchLanguage() const
{
    switch (language())
    {
        case EN_Language: return "EN";
        case RU_Language: return "RU";
        default:
            return QString("?");
    }
}
