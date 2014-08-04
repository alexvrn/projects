#ifndef IVIEWUPLOAD_H
#define IVIEWUPLOAD_H

#include "iview.h"
#include "Model/fileshortinfomodel.h"
#include "Model/uploadprogressmodel.h"
#include <QModelIndex>

class IViewUpload: public IView
{
public:
    void data() {}

    virtual void resetItem(int) = 0;

    virtual void setCollection(const QList<FileShortInfoModel*>&) = 0;
    virtual QList<FileShortInfoModel*> collection() const = 0;

    virtual void setShowUploading(bool) = 0;
    virtual bool showUploading() const = 0;

    virtual void setShowCopying(bool) = 0;
    virtual bool showCopying() const = 0;

    virtual void setShowArchive(bool) = 0;
    virtual bool showArchive() const = 0;

    virtual void setShowError(bool) = 0;
    virtual bool showError() const = 0;

    virtual void setShowOK(bool) = 0;
    virtual bool showOK() const = 0;

    virtual void setVisiblePause(bool) = 0;
    virtual bool visiblePause() const = 0;

    virtual void setEnabledRepeat(bool) = 0;

    virtual void clear() = 0;

    virtual void setAdmin(bool) = 0;

    virtual void setNameOnPause(const QString&) = 0;

    virtual void setModel(UploadProgressModel *) = 0;

public: // signals
    virtual void pausedCommand() = 0;
    virtual void archiveCommand(bool) = 0;
    virtual void repeatCommand() = 0;

    virtual void sendInformationMail() = 0;

    virtual void getArchiveCommand(bool) = 0;
    virtual void showUploadingCommand(bool) = 0;
    virtual void showErrorCommand(bool) = 0;
    virtual void showFinishedCommand(bool) = 0;
    virtual void showCopyingCommand(bool) = 0;

    virtual void cancelDownload(const QModelIndexList&) = 0;

    virtual void changeIsBackup(const QString&, bool) = 0;
};

#endif // IVIEWUPLOAD_H
