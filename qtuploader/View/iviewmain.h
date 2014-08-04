#ifndef IVIEWMAIN_H
#define IVIEWMAIN_H

#include "iview.h"

#include <QList>
#include "Model/fileshortinfomodel.h"
#include "Service/credentials.h"
#include "Model/fileuploadinfomodel.h"

class IViewMain: public IView
{
public:
    void data() {}
    void setCollection(QList<FileShortInfoModel*>) {}

    virtual void setInformation(const QString&) = 0;
    virtual void setTitleText(const QString&) = 0;
    virtual void setTextSwitchLanguage(const QString&) = 0;

public: // signals
    virtual void isLogon(const Credentials&, bool) = 0;
    virtual void queryLogon(const Credentials&) = 0;
    virtual void logoutCommand() = 0;
    virtual void checkLanguage() = 0;

public: //slots
    //virtual void sltLogout() = 0;

};

#endif // IVIEWMAIN_H
