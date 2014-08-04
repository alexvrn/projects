#include "uploadprogresspresenter.h"

#include "View/iview.h"
#include "View/iviewupload.h"
#include "Model/fileshortinfomodel.h"
#include "Model/fileuploadinfomodel.h"
#include "Model/uploadprogressmodel.h"

#include <QMessageBox>
#include <QDebug>

UploadProgressPresenter::UploadProgressPresenter(IView *view, QObject *parent) :
    QObject(parent)//, m_model(new UploadProgressModel)
{
    m_view = (IViewUpload*)view;

    QObject *view_obj = dynamic_cast<QObject*>(view);
    QObject::connect(view_obj, SIGNAL(pausedCommand()),       this, SLOT(sltPausedCommand()));
    QObject::connect(view_obj, SIGNAL(repeatCommand()),       this, SLOT(sltRepeatCommand()));
    QObject::connect(view_obj, SIGNAL(archiveCommand(bool)),  this, SIGNAL(archiveCommand(bool)));
    QObject::connect(view_obj, SIGNAL(sendInformationMail()), this, SIGNAL(sendInformationMail()));

    QObject::connect(view_obj, SIGNAL(getArchiveCommand(bool)),    this, SLOT(sltGetArchiveCommand(bool)));
    QObject::connect(view_obj, SIGNAL(showUploadingCommand(bool)), this, SLOT(sltShowUploadingCommand(bool)));
    QObject::connect(view_obj, SIGNAL(showCopyingCommand(bool)),   this, SLOT(sltShowCopyingCommand(bool)));
    QObject::connect(view_obj, SIGNAL(showErrorCommand(bool)),     this, SLOT(sltShowErrorCommand(bool)));
    QObject::connect(view_obj, SIGNAL(showFinishedCommand(bool)),  this, SLOT(sltShowFinishedCommand(bool)));

    QObject::connect(view_obj, SIGNAL(cancelDownload(const QModelIndexList&)), this, SIGNAL(cancelDownload(const QModelIndexList&)));
    QObject::connect(view_obj, SIGNAL(changeIsBackup(const QString&, bool)), this, SLOT(sltChangeIsBackup(const QString&, bool)));

    m_view->setModel(&m_model);
}

UploadProgressPresenter::~UploadProgressPresenter()
{
    //qDebug() << "Delete UploadProgressPresenter";
}

void UploadProgressPresenter::refreshView(int newitem)
{
    m_view->setShowArchive(m_model.showArchive());
    m_view->setShowCopying(m_model.showCopying());
    m_view->setShowError(m_model.showError());
    m_view->setShowOK(m_model.showOK());
    m_view->setShowUploading(m_model.showUploading());
    m_view->resetItem(newitem);
    //m_view->setCollection(_newitem);//m_model.collection());
    m_view->setAdmin(m_model.admin());
    m_view->setVisiblePause(m_model.visiblePause());
    m_view->setNameOnPause(m_model.nameOnPause());
    m_view->setEnabledRepeat(m_model.isEnabledRepeat());
}

void UploadProgressPresenter::addFiles(const FileUploadInfoModel& item)
{
    int newitem;
    UploadProgressModel::ResetModel type_reset = m_model.addFile(item, newitem);
    switch(type_reset)
    {
        case UploadProgressModel::Item:
        {
            refreshView(newitem);
            break;
        }
        case UploadProgressModel::All:
        {
            refreshView(-1);
            break;
        }
        default: return;
    }
}

void UploadProgressPresenter::sltChangeIsBackup(const QString& checksum, bool isbackup)
{
    m_model.changeIsBackup(checksum, isbackup);
}

void UploadProgressPresenter::sltDataUpload(const FileUploadInfoModel& uf)
{
    addFiles(uf);

    //шлем информация о статистике загрузок(для отображения на statusBar)
    emit information(m_model.statistic());
}

void UploadProgressPresenter::sltDataInfoCopying(const FileUploadInfoModel& fu)
{
    //m_model.setTotalAndCopiedBytes(fu.progress(), fu.fileSize());
    refreshView();
}

void UploadProgressPresenter::sltIsPaused(bool pause)
{
    m_model.setIsPaused(pause);
    refreshView();
}

void UploadProgressPresenter::sltLogout()
{
    clear();
}

void UploadProgressPresenter::sltPausedCommand()
{
    emit pausedCommand(!m_model.isPaused());
}

void UploadProgressPresenter::sltGetArchiveCommand(bool arc)
{
    m_model.setShowArchive(arc);
    refreshView();

    if(arc)
        emit getArchiveCommand(arc);
}

void UploadProgressPresenter::sltShowUploadingCommand(bool show)
{
    m_model.setShowUploading(show);
    refreshView();
}

void UploadProgressPresenter::sltShowCopyingCommand(bool show)
{
    m_model.setShowCopying(show);
    refreshView();
}

void UploadProgressPresenter::sltShowErrorCommand(bool show)
{
    m_model.setShowError(show);
    refreshView();
}

void UploadProgressPresenter::sltShowFinishedCommand(bool show)
{
    m_model.setShowOK(show);
    refreshView();
}

void UploadProgressPresenter::sltRepeatCommand()
{
    emit repeatCommand();
}

void UploadProgressPresenter::sltIsLogon(const Credentials& c, bool connect)
{
    m_model.setConnect(connect, c);
    refreshView();
}

void UploadProgressPresenter::clear()
{
    m_model.clear();
    refreshView();
}
