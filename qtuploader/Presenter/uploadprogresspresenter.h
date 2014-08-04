#ifndef UPLOADPROGRESSPRESENTER_H
#define UPLOADPROGRESSPRESENTER_H

#include "Model/fileshortinfomodel.h"
#include "Model/fileuploadinfomodel.h"
#include "Service/credentials.h"
#include "Model/uploadprogressmodel.h"

#include <QObject>
#include <QModelIndex>

class IView;
class IViewUpload;

class UploadProgressPresenter : public QObject
{
    Q_OBJECT
public:
    explicit UploadProgressPresenter(IView *view, QObject *parent = 0);
    ~UploadProgressPresenter();

signals:

public slots:
    void sltDataUpload(const FileUploadInfoModel&);

    void sltLogout();

private slots:
    void sltPausedCommand();
    void sltRepeatCommand();
    void sltGetArchiveCommand(bool);

    void sltShowUploadingCommand(bool);
    void sltShowErrorCommand(bool);
    void sltShowFinishedCommand(bool);
    void sltShowCopyingCommand(bool);

    void sltIsLogon(const Credentials& c, bool connect);

    void sltDataInfoCopying(const FileUploadInfoModel&);
    void sltIsPaused(bool);

    void sltChangeIsBackup(const QString&, bool);

signals:
    void pausedCommand(bool);
    void repeatCommand();
    void archiveCommand(bool);
    void sendInformationMail();

    void getArchiveCommand(bool);

    void cancelDownload(const QModelIndexList&);
    void changeIsBackup(const QString&, bool);

    void information(const QString&);

private:
    void addFiles(const FileUploadInfoModel&);
    void clear();

    void refreshView(int = -1);

private:
     IViewUpload* m_view;

     UploadProgressModel m_model;
};

#endif // UPLOADPROGRESSPRESENTER_H
