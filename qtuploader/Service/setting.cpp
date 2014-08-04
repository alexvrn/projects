#include <QFile>
#include <QSettings>
#include <QDir>

#include "setting.h"

const QString Setting::InitFileName = "uploader.ini";

bool Setting::init()
{
    try
    {
        QString pathToInit = QString("%1%2%3").arg(QDir::currentPath()).arg(QDir::separator()).arg(InitFileName);
        if(QFile::exists(pathToInit))
            return true;

        //create file with default settings
        QSettings sett(pathToInit, QSettings::IniFormat, 0);
        sett.beginGroup("ports");
        sett.setValue("port_server", 1080);
        sett.setValue("port_service", 9995);
        sett.endGroup();

        sett.beginGroup("path");
        sett.setValue("credentials",
                      QDir::toNativeSeparators(QString("%1%2%3").arg(QDir::homePath()).arg(QDir::separator()).arg("credentials.xml")));
        sett.endGroup();
        return true;
    }
    catch(...)
    {
        return false;
    }
}

QVariant Setting::value(KEYS key)
{
    try
    {
        if(!init())
            return QVariant();

        QString pathToInit = QString("%1%2%3").arg(QDir::currentPath()).arg(QDir::separator()).arg(InitFileName);
        QSettings sett(pathToInit, QSettings::IniFormat, 0);
        switch (key)
        {
            case KEY_PORT_SERVER:
                return sett.value("ports/port_server", 0);
            case KEY_PORT_SERVICE:
                return sett.value("ports/port_service", 0);
            case KEY_CREDENTIAL_PATH:
                return sett.value("path/credentials", "");
            default: return QVariant();
        }
    }
    catch(...)
    {
        return QVariant();
    }
}
