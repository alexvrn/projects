#ifndef SETTING_H
#define SETTING_H

#include <QString>
#include <QVariant>

class Setting
{
public:
    enum KEYS
    {
        KEY_PORT_SERVICE = 0,
        KEY_PORT_SERVER = 1,
        KEY_CREDENTIAL_PATH = 2
    };
private:
    Setting() {}

public:
    static bool init();
    static QVariant value(KEYS);

private:
   static const QString InitFileName;
};

#endif // SETTING_H
