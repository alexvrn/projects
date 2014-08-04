#ifndef DIALOGSERVICE_H
#define DIALOGSERVICE_H

#include <QObject>
#include <QStringList>
#include <QString>
#include "Service/RemoteDialog/filesystemremotemodel.h"

class DialogService : public QObject
{
    Q_OBJECT
public:
    explicit DialogService(QObject *parent = 0);
    
    QStringList GetFiles();
    QStringList GetFilesInFolder();

    QStringList GetFilesRemote();
    QString GetFolderRemote();

private:
    QStringList GetFilesInFolder(const QString&);

    static QStringList formats();//support formats

signals:
    void requestFolderContents(const QString&, bool);
    void responseFolderContents(const QList<FileSystemRemoteItemModel>&);
    
public slots:
    
};

#endif // DIALOGSERVICE_H
