#ifndef FILESYSTEMREMOTECONTROLLER_H
#define FILESYSTEMREMOTECONTROLLER_H

#include <QObject>
#include "Service/RemoteDialog/filesystemremotemodel.h"

class FileSystemRemoteController : public QObject
{
    Q_OBJECT
public:
    explicit FileSystemRemoteController(
            //ITableView * view,
            QObject *parent = 0);

signals:

public slots:
    //void responseData(const QStringList&);

private:
    //FileSystemRemoteModel m_Model;
};

#endif // FILESYSTEMREMOTECONTROLLER_H
