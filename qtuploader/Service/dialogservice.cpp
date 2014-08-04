#include "dialogservice.h"

#include <QDialog>
#include <QFileDialog>
#include <QDebug>
#include "Service/RemoteDialog/filesystemremotedialog.h"

DialogService::DialogService(QObject *parent) :
    QObject(parent)
{
}

QStringList DialogService::formats()
{
    return QStringList() << "*.dvr" << "*.mps" << "*.gpx" << "*.mp" << "*.ubx";
}

QStringList DialogService::GetFiles()
{
    try
    {
        return QFileDialog::getOpenFileNames(0,
            "Open files", "", QString("Files (%1)").arg(DialogService::formats().join(" ")));
    }
    catch(...)
    {
         qDebug() << "Error receipt files!(function GetFiles())";
         return QStringList();
    }
}

QStringList DialogService::GetFilesInFolder(const QString& folder)
{
    try
    {
        QDir dir(folder);

        //Files for current dir
        QStringList lstAllFiles;
        foreach (QFileInfo fileInfo, dir.entryInfoList(DialogService::formats(), QDir::Files)) {
            lstAllFiles.append(fileInfo.absoluteFilePath());
        }

        //Folders
        foreach (QFileInfo dirInfo, dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks)) {
           lstAllFiles.append(GetFilesInFolder(dirInfo.filePath()));
        }

        return lstAllFiles;
    }
    catch(...)
    {
        qDebug() << "Error receipt files!(function GetFilesInFolder(QString))";
        return QStringList();
    }
}

QStringList DialogService::GetFilesRemote()
{
    try
    {
        //return QFileDialog::getOpenFileNames(0,
        //    "Open files", "", QString("Files (%1)").arg(DialogService::formats().join(" ")));
        FileSystemRemoteDialog dialog(false);
        QObject::connect(&dialog, SIGNAL(requestFolderContents(const QString&, bool)), this, SIGNAL(requestFolderContents(const QString&, bool)));
        QObject::connect(this, SIGNAL(responseFolderContents(const QList<FileSystemRemoteItemModel>&)), &dialog, SLOT(sltResponseFolderContents(const QList<FileSystemRemoteItemModel>&)));

        //dialog.show();
        dialog.getContentFolder();
        if(dialog.exec() == QDialog::Rejected)
            return QStringList();

        //int result = dialog.result();
        return dialog.selectFiles();
    }
    catch(...)
    {
         qDebug() << "Error receipt files!(function GetFiles())";
         return QStringList();
    }
}

QString DialogService::GetFolderRemote()
{
    try
    {
        //return QFileDialog::getOpenFileNames(0,
        //    "Open files", "", QString("Files (%1)").arg(DialogService::formats().join(" ")));
        FileSystemRemoteDialog dialog(true);
        dialog.setStyleSheet("");
        QObject::connect(&dialog, SIGNAL(requestFolderContents(const QString&, bool)), this, SIGNAL(requestFolderContents(const QString&, bool)));
        QObject::connect(this, SIGNAL(responseFolderContents(const QList<FileSystemRemoteItemModel>&)), &dialog, SLOT(sltResponseFolderContents(const QList<FileSystemRemoteItemModel>&)));

        //dialog.show();
        dialog.getContentFolder();
        if(dialog.exec() == QDialog::Rejected)
            return QString();

        //int result = dialog.result();
        return dialog.selectFolder();//emit requestDir(currentPath(), false);
    }
    catch(...)
    {
         qDebug() << "Error receipt files!(function GetFiles())";
         return QString();
    }
}

QStringList DialogService::GetFilesInFolder()
{
    try
    {
        QString dirName = QFileDialog::getExistingDirectory(0,
             "Open folder", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        return GetFilesInFolder(dirName);
    }
    catch(...)
    {
        qDebug() << "Error receipt files!(function GetFilesInFolder())";
        return QStringList();
    }
}

