#include "mainwindowpresenter.h"
#include "View/iview.h"
#include "View/iviewmain.h"

#include "Presenter/addfilespresenter.h"
#include "Presenter/uploadprogresspresenter.h"

#include "protocol/remoteclient.pb.h"

#include <QMessageBox>
#include <QDebug>
#include <QSettings>
#include <QFile>
#include <QTranslator>
#include <QApplication>

MainWindowPresenter::MainWindowPresenter(IView *view, QObject *parent) :
    QObject(parent)
{

    m_view = (IViewMain*)view;

    QObject *view_obj = dynamic_cast<QObject*>(view);

    QObject::connect(this, SIGNAL(logoutCommand()), view_obj, SIGNAL(logoutCommand()));
    QObject::connect(this, SIGNAL(isLogon(const Credentials&, bool)), view_obj, SIGNAL(isLogon(const Credentials&, bool)));

    m_client = new ServiceClient(this);
    QObject::connect(view_obj, SIGNAL(queryLogon(const Credentials&)), m_client, SLOT(sltQueryLogon(const Credentials&)));
    QObject::connect(view_obj, SIGNAL(checkLanguage()), this, SLOT(sltChangeTranslator()));
    QObject::connect(m_client, SIGNAL(isLogon(const Credentials&, bool)), this, SIGNAL(isLogon(const Credentials&, bool)));
    QObject::connect(m_client, SIGNAL(isLogon(const Credentials&, bool)), this, SLOT(sltIsLogon(const Credentials&, bool)));
    QObject::connect(m_client, SIGNAL(dataUpload(const FileUploadInfoModel&)), this, SIGNAL(dataUpload(const FileUploadInfoModel&)));
    QObject::connect(m_client, SIGNAL(dataInfoCopying(const FileUploadInfoModel&)), this, SIGNAL(dataInfoCopying(const FileUploadInfoModel&)));
    QObject::connect(this,     SIGNAL(logoutCommand()), m_client, SLOT(sltLogout()));

    QObject::connect(m_client, SIGNAL(questionsAboutDownloading(const QString&)), this, SLOT(sltQuestionsAboutDownloading(const QString&)));
    QObject::connect(this, SIGNAL(responseAboutErrorDownloading(::uploadercontrol::CommandType)), m_client, SLOT(sltResponseAboutErrorDownloading(::uploadercontrol::CommandType)));

    qt_translator = NULL;
    qt_translator = new QTranslator();
    Q_CHECK_PTR(qt_translator);

    m_model.setLanguage(MainWindowModel::EN_Language);
    setLanguage(m_model.language());

    refreshView();
}

MainWindowPresenter::~MainWindowPresenter()
{
}

void MainWindowPresenter::sltChangeTranslator()
{
    if(m_model.language() == MainWindowModel::EN_Language)
        m_model.setLanguage(MainWindowModel::RU_Language);
    else
        m_model.setLanguage(MainWindowModel::EN_Language);

    setLanguage(m_model.language());

    refreshView();
}

void MainWindowPresenter::setLanguage(MainWindowModel::Language lan)
{
    QString file_translate = QApplication::applicationDirPath() + "/ru.qm";
    switch (lan)
    {
        case MainWindowModel::EN_Language:
            file_translate = QApplication::applicationDirPath() + "/en.qm";
            break;
        case MainWindowModel::RU_Language:
            file_translate = QApplication::applicationDirPath() + "/ru.qm";
            break;
        default:
            break;
    }

    if ( !qt_translator->load(file_translate, ".")) {
        bool k = qApp->removeTranslator(qt_translator);
    }
    else {
        qApp->installTranslator( qt_translator );
    }
}

void MainWindowPresenter::refreshView()//bool isRefreshCollection)
{
    m_view->setInformation(m_model.statusInfo());
    m_view->setTitleText(m_model.titleText());
    m_view->setTextSwitchLanguage(m_model.textSwitchLanguage());
}

void MainWindowPresenter::sltIsLogon(const Credentials& c, bool connect)
{
    m_model.setRemoteService(c.isRemoteService(), c.service());
    refreshView();
}

void MainWindowPresenter::sltQuestionsAboutDownloading(const QString& inf)
{
    QMessageBox msgBox;
    msgBox.setText(tr("Uploading"));
    msgBox.setDetailedText(inf);
    msgBox.setInformativeText(tr("Error in the folder structure! Please select an action."));
    msgBox.setStandardButtons(QMessageBox::Ignore | QMessageBox::Retry | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Retry);
    int ret = msgBox.exec();
    ::uploadercontrol::CommandType _commandType;
    switch (ret)
    {
      case QMessageBox::Ignore:
          _commandType = ::uploadercontrol::FolderStructureErrorIgnore;
          break;
      case QMessageBox::Retry:
          _commandType = ::uploadercontrol::FolderStructureErrorRetry;
          break;
      case QMessageBox::Cancel:
          _commandType = ::uploadercontrol::FolderStructureErrorCancel;
          break;
      default:
          _commandType = ::uploadercontrol::FolderStructureErrorCancel;
          break;
    }
    emit responseAboutErrorDownloading(_commandType);
}

void MainWindowPresenter::sltInformation(const QString& inf)
{
    m_model.setStatusInfo(inf);
    refreshView();
}

void MainWindowPresenter::sltLogout()
{
    m_model.setStatusInfo(QString());
    m_model.setRemoteService(false);
    refreshView();
}

void MainWindowPresenter::addPresenter(QObject * addPresenter, QObject * uploadPresenter)
{
     AddFilesPresenter *add_obj = (AddFilesPresenter*)addPresenter;//dynamic_cast<AddFilesPresenter*>(view);
     QObject::connect(this,    SIGNAL(isLogon(const Credentials&, bool)), add_obj, SLOT(sltIsLogon(const Credentials&, bool)));
     QObject::connect(add_obj, SIGNAL(logoutCommand()), this, SLOT(sltLogout()));
     QObject::connect(add_obj, SIGNAL(logoutCommand()), this, SIGNAL(logoutCommand()));
     QObject::connect(add_obj, SIGNAL(dataForUpload(const QList<FileShortInfoModel*>, const QString&,bool,bool,bool)),
             m_client, SLOT(sltDataForUpload(const QList<FileShortInfoModel*>, const QString&,bool,bool,bool)));
     //QObject::connect(this, SIGNAL(dataCopying(const FileUploadInfoModel&)), add_obj, SLOT(sltDataCopying(const FileUploadInfoModel&)));
     QObject::connect(add_obj, SIGNAL(requestFolderContents(const QString&, bool)), m_client, SLOT(sltRequestFolderContents(const QString&, bool)));
     QObject::connect(m_client, SIGNAL(responseFolderContents(const QList<FileSystemRemoteItemModel>&, bool)), add_obj, SIGNAL(responseFolderContents(const QList<FileSystemRemoteItemModel>&, bool)));

     UploadProgressPresenter *upload_obj = (UploadProgressPresenter*)uploadPresenter;
     QObject::connect(this,       SIGNAL(dataUpload(const FileUploadInfoModel&)), upload_obj, SLOT(sltDataUpload(const FileUploadInfoModel&)));
     QObject::connect(this,       SIGNAL(logoutCommand()), upload_obj,                        SLOT(sltLogout()));
     QObject::connect(this,       SIGNAL(isLogon(const Credentials&, bool)), upload_obj,      SLOT(sltIsLogon(const Credentials&, bool)));
     QObject::connect(upload_obj, SIGNAL(pausedCommand(bool)), m_client,                      SLOT(sltPausedCommand(bool)));
     QObject::connect(upload_obj, SIGNAL(archiveCommand(bool)), m_client,                     SLOT(sltArchiveCommand(bool)));
     QObject::connect(upload_obj, SIGNAL(repeatCommand()), m_client,                          SLOT(sltRepeatCommand()));
     QObject::connect(upload_obj, SIGNAL(sendInformationMail()), m_client,                    SLOT(sltSendInformationMail()));
     QObject::connect(upload_obj, SIGNAL(getArchiveCommand(bool)), m_client,                  SLOT(sltGetArchiveCommand()));
     QObject::connect(upload_obj, SIGNAL(cancelDownload(const QModelIndexList&)), m_client,   SLOT(sltCancelDownload(const QModelIndexList&)));
     QObject::connect(this,       SIGNAL(dataInfoCopying(const FileUploadInfoModel&)), upload_obj, SLOT(sltDataInfoCopying(const FileUploadInfoModel&)));
     QObject::connect(upload_obj, SIGNAL(information(const QString&)), this, SLOT(sltInformation(const QString&)));
     QObject::connect(m_client,   SIGNAL(isPaused(bool)), upload_obj, SLOT(sltIsPaused(bool)));
     QObject::connect(upload_obj, SIGNAL(changeIsBackup(const QString&, bool)), m_client, SLOT(sltChangeIsBackup(const QString&, bool)));
}
