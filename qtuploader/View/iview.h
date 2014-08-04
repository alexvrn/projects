#ifndef IVIEW_H
#define IVIEW_H

#include <QList>
#include "Model/fileshortinfomodel.h"

/*!
    \interface IWiringView
    Интерфейс
*/

class IView
{
public:
    virtual void data() = 0;
};

#endif // IVIEW_H
