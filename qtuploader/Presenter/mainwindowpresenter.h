#ifndef MAINWINDOWPRESENTER_H
#define MAINWINDOWPRESENTER_H

#include <QObject>
#include <QTranslator>
#include "Service/credentials.h"
#include "Service/serviceclient.h"
#include "Model/fileuploadinfomodel.h"
#include "Model/mainwindowmodel.h"
#include "protocol/remoteclient.pb.h"

class IView;
class IViewMain;

class MainWindowPresenter : public QObject
{
    Q_OBJECT
public:
    explicit MainWindowPresenter(IView *view, QObject *parent = 0);
    ~MainWindowPresenter();

    void addPresenter(QObject * addPresenter, QObject * uploadPresenter);

signals:
    void isLogon(const Credentials&, bool);
    void logoutCommand();
    void dataUpload(const FileUploadInfoModel&);
    //void dataCopying(const FileUploadInfoModel&);
    void dataInfoCopying(const FileUploadInfoModel&);
    void responseAboutErrorDownloading(::uploadercontrol::CommandType);

public slots:
    void sltInformation(const QString&);
    void sltLogout();
    void sltQuestionsAboutDownloading(const QString&);
    void sltIsLogon(const Credentials&, bool);

    void sltChangeTranslator();

private:
    void refreshView();

    void setLanguage(MainWindowModel::Language);

private:


    IViewMain* m_view;
    MainWindowModel m_model;
    ServiceClient * m_client;

    QTranslator * qt_translator;
};

#endif // MAINWINDOWPRESENTER_H
