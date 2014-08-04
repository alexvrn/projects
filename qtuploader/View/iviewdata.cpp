#include "iviewdata.h"
#include <QUiLoader>
#include <QFile>
#include <QDir>

IViewData::IViewData(QString name, QWidget *parent) //: QObject(parent)
{
    m_strNameData = name;
    m_qParent = parent;
}

QWidget* IViewData::loadUiFile()
{
    try
    {
        QUiLoader loader;
        QFile file(pathToUI(m_strNameData));
        if(!file.open(QFile::ReadOnly))
            return Q_NULLPTR;

        loader.setLanguageChangeEnabled(true);
        QWidget *formWidget = loader.load(&file, m_qParent);
        file.close();

        return formWidget;
    }
    catch(...)
    {
        return Q_NULLPTR;
    }
}

QString IViewData::pathToUI(QString name)
{
    return QString(":/UI/%1.ui").arg(name.toLower());
}
