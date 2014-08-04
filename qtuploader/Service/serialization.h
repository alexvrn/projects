#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <QObject>
#include "Service/credentials.h"

class Serialization
{
public:
    Serialization();

    static bool SerializeToXML(const QObject &obj, QString name);

    static bool DeserializeFromXML(QObject* object, const QString& input);//QObject* object);
};

#endif // SERIALIZATION_H
