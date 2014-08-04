#include <QtWidgets/QApplication>
#include <QTextCodec>
#include <QFile>
#include <QDebug>
#include <QtMsgHandler>
#include <QLibraryInfo>
#include <QTranslator>
#include <QMessageBox>
#include <QSharedPointer>

#include "View/mainwindowview.h"
#include "View/addfilespanelview.h"
#include "View/uploadprogressview.h"
#include "Presenter/addfilespresenter.h"
#include "Presenter/uploadprogresspresenter.h"
#include "Presenter/mainwindowpresenter.h"
#include "Service/messageoutput.h"
#include "Service/setting.h"


QString loadStyleSheet(QString path)
{
    QFile styleFile(path);
    QString styleSheet = QString();
    if( styleFile.open(QFile::ReadOnly) )
    {
        styleSheet = QLatin1String(styleFile.readAll());
    }
    return styleSheet;
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);

    qInstallMessageHandler(messageOutput);
    QApplication app(argc, argv);
    app.setStyleSheet(loadStyleSheet(":/qss/main.qss"));

    // Вызываем поддержку перевода
    QTranslator * qt_translator = new QTranslator();
    QString file_translate = QApplication::applicationDirPath() + "/ru.qm";
    if ( !qt_translator->load(file_translate)) {
        delete qt_translator;
    }
    else
        qApp->installTranslator( qt_translator );


    //initialize file of settings
    if(!Setting::init())
    {
        QMessageBox::information(0, "Error", "Can not create file of setting: uploader.ini");
        qDebug() << "Can not create file of setting: uploader.ini";
        exit(0);
    }
    else qDebug() << "Create uploader.ini";

    qDebug() << "Project started";

    try
    {
        MainWindowView* viewMain = MainWindowView::instance();
        //MainWindowView viewMain;
        if(!viewMain->initControls())
            throw(QString("Don't create MainWindowView!"));

        AddFilesPanelView viewAddPanel(viewMain);
        if(!viewAddPanel.initControls())
            throw(QString("Don't create AddFilePanel!"));

        UploadProgressView viewUploadPanel(viewMain);
        if(!viewUploadPanel.initControls())
            throw(QString("Don't create UploadProgressView!"));

        AddFilesPresenter presenterAdd(&viewAddPanel);
        UploadProgressPresenter presenterUpload(&viewUploadPanel);

        viewMain->setPanel(&viewAddPanel, &viewUploadPanel);

        MainWindowPresenter presenterMain(viewMain);
        presenterMain.addPresenter(&presenterAdd, &presenterUpload);

        viewMain->show();
        return app.exec();
    }
    catch(QString ex_str)
    {
        QMessageBox::information(0, "Error!", ex_str);
        qDebug() << "Error: " << ex_str;
        exit(0);
    }
    catch(...)
    {
        QMessageBox::information(0, "Error!", "Unknown error!");
        qDebug() << "Unknown error!";
        exit(0);
    }

    return 0;//app.exec();
}
