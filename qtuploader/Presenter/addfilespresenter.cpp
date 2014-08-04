#include "addfilespresenter.h"
#include "Service/dialogservice.h"
#include "View/iview.h"
#include "View/iviewadd.h"
#include "Model/addfilesmodel.h"

#include <QMessageBox>
#include <QDebug>

AddFilesPresenter::AddFilesPresenter(IView *view, QObject *parent) :
   QObject(parent)//, m_model(new AddFilesModel)
{
    m_view = (IViewAdd*)view;

    QObject *view_obj = dynamic_cast<QObject*>(view);
    QObject::connect(view_obj, SIGNAL(addFilesCommand()), this, SLOT(sltAddFilesCommand()));
    QObject::connect(view_obj, SIGNAL(addDirCommand()),   this, SLOT(sltAddFolderCommand()));
    QObject::connect(view_obj, SIGNAL(uploadCommand()),   this, SLOT(sltUploadCommand()));
    QObject::connect(view_obj, SIGNAL(logoutCommand()),   this, SLOT(sltLogoutCommand()));
    QObject::connect(view_obj, SIGNAL(editTaskCommand(const QString&)), this, SLOT(sltEditTaskCommand(const QString&)));
    QObject::connect(view_obj, SIGNAL(deleteSelectItem(const QModelIndexList&)), this, SLOT(sltDeleteSelectItem(const QModelIndexList&)));
    QObject::connect(view_obj, SIGNAL(establishIsBackupSelectItem(const QModelIndexList&, bool)), this, SLOT(sltEstablishIsBackupSelectItem(const QModelIndexList&, bool)));
    QObject::connect(view_obj, SIGNAL(establishIsBackupAll(bool)), this, SLOT(sltEstablishIsBackupAll(bool)));
    QObject::connect(view_obj, SIGNAL(isIntermediateCopy(bool)), this, SLOT(sltIntermediateCopy(bool)));
    QObject::connect(view_obj, SIGNAL(isDeleteSource(bool)), this, SLOT(sltDeleteSource(bool)));
    QObject::connect(view_obj, SIGNAL(isIndexed(bool)), this, SLOT(sltIndexed(bool)));

    QObject::connect(this, SIGNAL(responseFolderContents(const QList<FileSystemRemoteItemModel>&, bool)), this, SLOT(sltResponseFolderContents(const QList<FileSystemRemoteItemModel>&, bool)));
}

AddFilesPresenter::~AddFilesPresenter()
{
}

void AddFilesPresenter::refreshView(bool isRefreshCollection)
{      
    if(isRefreshCollection)
        m_view->setCollection(m_model.collection());
    m_view->setTask(m_model.task());
    m_view->setEnabledUpload(m_model.enabledUpload());
    m_view->setTask(m_model.task());
    m_view->setTextLogout(m_model.textLogout());
    m_view->setVersion(m_model.version());
    m_view->setVisibleCheckIndexFiles(m_model.visibleCheckIndexFiles());
    m_view->setVisibleLogout(m_model.visibleLogout());

    m_view->setIndexed(m_model.indexed());
    m_view->setIntermediateCopy(m_model.intermediateCopy());
    m_view->setCheckDeleteSource(m_model.isCheckDeleteSource());
    m_view->setEnabledDeleteSource(m_model.isEnabledDeleteSource());
}

void AddFilesPresenter::sltResponseFolderContents(const QList<FileSystemRemoteItemModel>& list, bool folderContentRecursively)
{
    if(!folderContentRecursively)
        emit responseFolderContents_SelectedFiles(list);
    else
    {
        QStringList s;
        for(auto l: list)
            s.append(l.name());
        addFiles(s);
    }
}

void AddFilesPresenter::addFiles(const QStringList& files)
{
    m_model.addFiles(files);
    refreshView(true);
}

void AddFilesPresenter::sltAddFilesCommand()
{
    try
    {
        DialogService dlg;
        if(!m_model.isRemoteService())
        {
            addFiles(dlg.GetFiles());
        }
        else
        {
            QObject::connect(&dlg, SIGNAL(requestFolderContents(const QString&, bool)), this, SIGNAL(requestFolderContents(const QString&, bool)));
            QObject::connect(this, SIGNAL(responseFolderContents_SelectedFiles(const QList<FileSystemRemoteItemModel>&)), &dlg, SIGNAL(responseFolderContents(const QList<FileSystemRemoteItemModel>&)));
            addFiles(dlg.GetFilesRemote());
        }
    }
    catch(...)
    {
        qDebug() << "Error of add files(function sltAddFilesCommand())";
        QMessageBox::information(0, "Error!", "Error of add files!");
    }
}

void AddFilesPresenter::sltAddFolderCommand()
{
    try
    {
        DialogService dlg;
        if(!m_model.isRemoteService())
        {
            addFiles(dlg.GetFilesInFolder());
        }
        else
        {
            QObject::connect(&dlg, SIGNAL(requestFolderContents(const QString&, bool)), this, SIGNAL(requestFolderContents(const QString&, bool)));
            QObject::connect(this, SIGNAL(responseFolderContents_SelectedFiles(const QList<FileSystemRemoteItemModel>&)), &dlg, SIGNAL(responseFolderContents(const QList<FileSystemRemoteItemModel>&)));

            QString selectFolder = dlg.GetFolderRemote();
            if(selectFolder == QString())
                return;

            requestFolderContents(selectFolder, true);
        }
    }
    catch(...)
    {
        qDebug() << "Error of add folder(function sltAddDirCommand())";
        QMessageBox::information(0, "Error!", "Error of add folder!");
    }
}

void AddFilesPresenter::sltUploadCommand()
{
    emit dataForUpload(m_model.collection(), m_model.task(), m_model.indexed(), m_model.isCheckDeleteSource(), m_model.intermediateCopy());
    clear();
}

void AddFilesPresenter::sltEditTaskCommand(const QString& txt)
{
    m_model.setTask(txt);
    refreshView();
}

void AddFilesPresenter::sltLogoutCommand()
{
    m_model.logout();
    refreshView(true);

    emit logoutCommand();
}

void AddFilesPresenter::clear()
{
    m_model.clearCollection();
    refreshView(true);
}

void AddFilesPresenter::sltDeleteSelectItem(const QModelIndexList& indexList)
{
    m_model.sltDeleteSelectItem(indexList);
    refreshView(true);
}

void AddFilesPresenter::sltEstablishIsBackupSelectItem(const QModelIndexList& indexList, bool isBackup)
{
    m_model.sltEstablishIsBackupSelectItem(indexList, isBackup);
    refreshView(true);
}

void AddFilesPresenter::sltEstablishIsBackupAll(bool isBackup)
{
    m_model.sltEstablishIsBackupAll(isBackup);
    refreshView(true);
}

void AddFilesPresenter::sltIntermediateCopy(bool check)
{
    m_model.setIntermediateCopy(check);
    refreshView();
}

void AddFilesPresenter::sltDeleteSource(bool ds)
{
    m_model.setCheckDeleteSource(ds);
    refreshView();
}

void AddFilesPresenter::sltIndexed(bool check)
{
    m_model.setIndexed(check);
    refreshView();
}

void AddFilesPresenter::sltIsLogon(const Credentials& c, bool connect)
{
    //QMessageBox::information(0, "add", c.Login());
    //bool admin = false;
    m_bIsAdmin = c.admin();

    m_model.setConnect(connect, c);
    refreshView(true);
}
