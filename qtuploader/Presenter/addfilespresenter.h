#ifndef ADDFILESPRESENTER_H
#define ADDFILESPRESENTER_H

#include <QObject>
#include "Model/fileshortinfomodel.h"
#include "Model/addfilesmodel.h"
#include "Model/fileuploadinfomodel.h"
#include "Service/credentials.h"
#include "Service/RemoteDialog/filesystemremotemodel.h"
#include <QStringList>
#include <QModelIndex>
#include <QSharedPointer>
#include <QTranslator>

class IView;
class IViewAdd;

class AddFilesPresenter : public QObject
{
    Q_OBJECT
public:
    explicit AddFilesPresenter(IView *view, QObject *parent = 0);
    ~AddFilesPresenter();

    void clear();

signals:
    void logoutCommand();

    void dataForUpload(const QList<FileShortInfoModel*>, const QString&, bool, bool, bool);

    void requestFolderContents(const QString&, bool);
    void responseFolderContents(const QList<FileSystemRemoteItemModel>&, bool);
    void responseFolderContents_SelectedFiles(const QList<FileSystemRemoteItemModel>&);

public slots:
    void sltIsLogon(const Credentials&, bool);

private slots:
    void sltAddFilesCommand();
    void sltAddFolderCommand();
    void sltUploadCommand();
    void sltLogoutCommand();
    void sltEditTaskCommand(const QString&);
    void sltDeleteSelectItem(const QModelIndexList&);
    void sltEstablishIsBackupSelectItem(const QModelIndexList&, bool);
    void sltEstablishIsBackupAll(bool);
    void sltIntermediateCopy(bool);
    void sltDeleteSource(bool);
    void sltIndexed(bool);

    void sltResponseFolderContents(const QList<FileSystemRemoteItemModel>&, bool);

private:
    void addFiles(const QStringList&);

    void refreshView(bool = false);

private:
     IViewAdd* m_view;

     AddFilesModel m_model;

     bool m_bIsAdmin;
};

#endif // ADDFILESPRESENTER_H
