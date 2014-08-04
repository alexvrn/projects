#ifndef IVIEWADD_H
#define IVIEWADD_H
#include "View/iview.h"

#include "Model/fileshortinfomodel.h"
#include <QList>
#include <QModelIndex>

class IViewAdd: public IView
{
public:
    void data() {}

    virtual QString task() const = 0;
    virtual void setTask(QString) = 0;

    virtual QString version() const = 0;
    virtual void setVersion(QString) = 0;

    virtual void setVisibleLogout(bool) = 0;
    virtual bool visibleLogout() const = 0;

    virtual void setTextLogout(QString) = 0;

    virtual void setCollection(QList<FileShortInfoModel*>) = 0;
    virtual QList<FileShortInfoModel*> collection() const = 0;

    virtual void setVisibleCheckIndexFiles(bool) = 0;
    virtual bool visibleCheckIndexFiles() const = 0;

    virtual void setIntermediateCopy(bool) = 0;
    virtual bool intermediateCopy() const = 0;

    virtual void setCheckDeleteSource(bool) = 0;
    virtual bool isCheckDeleteSource() const = 0;

    virtual void setEnabledDeleteSource(bool) = 0;
    virtual bool isEnabledDeleteSource() const = 0;

    virtual void setEnabledUpload(bool) = 0;

    virtual void setIndexed(bool) = 0;
    virtual bool indexed() const = 0;

    virtual void clear() = 0;

public: // signals
    virtual void addFilesCommand() = 0;
    virtual void logoutCommand() = 0;
    virtual void addDirCommand() = 0;
    virtual void uploadCommand() = 0;
    virtual void editTaskCommand(const QString&) = 0;
    virtual void deleteSelectItem(const QModelIndexList&) = 0;
    virtual void establishIsBackupSelectItem(const QModelIndexList&, bool) = 0;
    virtual void establishIsBackupAll(bool) = 0;
    virtual void isIntermediateCopy(bool) = 0;
    virtual void isDeleteSource(bool) = 0;
    virtual void isIndexed(bool) = 0;
};

#endif // IVIEWADD_H
