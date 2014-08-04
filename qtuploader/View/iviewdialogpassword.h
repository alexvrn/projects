#ifndef IVIEWDIALOGPASSWORD_H
#define IVIEWDIALOGPASSWORD_H

#include "iview.h"
#include "Service/credentials.h"

class IViewPasswordDialog: public IView
{
public:
    void data() {}

public :
    virtual void dialogShow(bool) = 0;

    virtual void setCredentials(const Credentials&) = 0;

public: // signals
    virtual void queryLogon() = 0;//const Credentials&) = 0;
    virtual void credentialsEdit(const Credentials&) = 0;

};

#endif // IVIEWDIALOGPASSWORD_H
