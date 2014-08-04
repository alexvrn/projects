#include "serialization.h"

#include <QString>
#include <QTextStream>
#include <QMetaProperty>
#include <QDomDocument>
#include <QFile>
#include <QDomNodeList>
#include <QMutexLocker>
#include "Service/credentials.h"

Serialization::Serialization()
{
}

bool Serialization::SerializeToXML(const QObject &obj, QString name)
{
    try
    {
        //?????
        QFile::remove(name);
        //?????

        QDomDocument doc;
        QDomElement root = doc.createElement(obj.metaObject()->className());
        doc.appendChild(root);

        for(int i = 0; i < obj.metaObject()->propertyCount(); i++)
        {
            const QMetaProperty prop = obj.metaObject()->property(i);
            QString propName = prop.name();
            if(propName == "objectName")
                continue;
            QDomElement el = doc.createElement(propName);
            QVariant value = obj.property(propName.toLocal8Bit().data());
            QDomText txt = doc.createTextNode( value.toString() );
            el.appendChild(txt);
            root.appendChild(el);
        }
        QFile output(name);
        if (output.open(QFile::ReadWrite))
        {
            QTextStream stream(&output);
            doc.save(stream, 2);
            return true;
        }
        return false;
    }
    catch(...)
    {
        return false;
    }
}

bool Serialization::DeserializeFromXML(QObject* object, const QString& input)//QObject* object)
{
    try
    {
        QDomDocument doc("credentials");
        QFile file(input);
        if (!file.open(QIODevice::ReadOnly))
            return false;
        if (!doc.setContent(&file))
        {
            file.close();
            return false;
        }
        file.close();

        QDomElement root = doc.documentElement();
        for(int i = 0; i < object->metaObject()->propertyCount(); i++)
        {
            QMetaProperty prop = object->metaObject()->property(i);
            QString propName = prop.name();
            if(propName == "objectName")
                continue;
            QDomNodeList nodeList = root.elementsByTagName(propName);
            if(nodeList.length() < 1)
                continue;
            QDomNode node = nodeList.at(0);
            //QVariant value = object->property(propName.toLocal8Bit().data());
            QString v = node.toElement().text();
            object->setProperty(propName.toLocal8Bit().data(), QVariant(v));
        }
        return true;
    }
    catch(...)
    {
        return false;
    }
}
