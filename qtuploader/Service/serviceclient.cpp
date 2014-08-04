#include "serviceclient.h"
#include "Service/setting.h"

#include <QDebug>
#include <QMessageBox>
#include <QByteArray>
#include <QDataStream>
#include <QDir>

#include "protocol/remoteclient.pb.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/coded_stream.h>

#include <iostream>
#include <sstream>
#include <string>

#include <QtEndian>
#include <QSharedPointer>
#include <QSettings>

const quint16 ServiceClient::ID_Message = 0xCAFE;

ServiceClient::ServiceClient(QObject *parent) :
    QObject(parent)
{
    m_qTcpSocket = new QTcpSocket(this);
    connect(m_qTcpSocket, SIGNAL(connected()), this, SLOT(sltConnected()));
    connect(m_qTcpSocket, SIGNAL(disconnected()), this, SLOT(sltDisconnected()));

    connect(m_qTcpSocket,  SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(reportError(QAbstractSocket::SocketError)), Qt::DirectConnection);

    connect(m_qTcpSocket,  SIGNAL(readyRead()),
        this, SLOT(readFromSocket()), Qt::DirectConnection);


#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    m_bIsLittleEndian = true;
#endif

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    m_bIsLittleEndian = false;
#endif

}

ServiceClient::~ServiceClient()
{
    //qDebug() << "Delete ServiceClient";
    Disconnect();
    delete m_qTcpSocket;
}

void ServiceClient::clearData()
{
    m_flagForWaitBytes = WAIT_ID;
    m_uiBlockSize = 0;
}

bool ServiceClient::Connect(QString host, quint16 port)
{
    clearData();

    m_qTcpSocket->abort();
    m_qTcpSocket->connectToHost(host, port);
    if (m_qTcpSocket->waitForConnected(10000))
    {
        qDebug() << "Connected!";
        return true;
    }
    else
    {
        return false;
    }
}

void ServiceClient::Disconnect()
{
    m_qTcpSocket->disconnectFromHost();
    if (m_qTcpSocket->state() == QAbstractSocket::UnconnectedState || m_qTcpSocket->waitForDisconnected(1000))
        qDebug() << "Disconnected!";

    clearData();
}

void ServiceClient::writeOnBeginningMessage(char * &array, int byteSize)
{
    quint16 id_msg = m_bIsLittleEndian ? qToBigEndian<quint16>(ID_Message) : ID_Message;
    char c_id[sizeof(quint16)];
    memcpy(&c_id, &id_msg, sizeof(quint16));

    quint32 size_msg = m_bIsLittleEndian ? qToBigEndian<quint32>(byteSize) : byteSize;
    char c_size[sizeof(quint32)];
    memcpy(&c_size, &size_msg, sizeof(quint32));

    int size = byteSize + sizeof(quint16) + sizeof(quint32);
    array = new char[size];
    if(!array)
        throw QString("Error: new char[%1]").arg(size);

    memcpy(array, &c_id, sizeof(quint16));
    memcpy(array + sizeof(quint16), &c_size, sizeof(quint32));
}

void ServiceClient::send(::uploadercontrol::ClientRequest &clientRequest)
{
    try
    {
        char* array = NULL;
        writeOnBeginningMessage(array, clientRequest.ByteSize());

        clientRequest.SerializeToArray(array + sizeof(quint16) + sizeof(quint32), clientRequest.ByteSize());
        m_qTcpSocket->write(array, clientRequest.ByteSize() + sizeof(quint16) + sizeof(quint32));
        m_qTcpSocket->waitForBytesWritten(10000);
    }
    catch(const QString& ex_str)
    {
        qDebug() << ex_str;
        QMessageBox::information(0, "Error", ex_str);
    }
    catch(...)
    {
        qDebug() << "Error: send message! Function: send()";
        QMessageBox::information(0, "Error", "Error: send message!");
    }
}

void ServiceClient::sltConnected()
{
    //Запрос на авторизацию
    qDebug() << "Query to autorization: " << m_Credentials.login();

    ::uploadercontrol::ClientRequest clientRequest;
    clientRequest.set_messagetype(::uploadercontrol::ClientRequest::AUTHORIZATION);

    clientRequest.mutable_authorization()->set_login(m_Credentials.login().toStdString());
    clientRequest.mutable_authorization()->set_passwordhash(m_Credentials.passwordHash().toStdString());
    clientRequest.mutable_authorization()->set_host(m_Credentials.server().toStdString());
    clientRequest.mutable_authorization()->set_port(m_iPortServer);

    if(m_Credentials.terminateUser())
        clientRequest.mutable_authorization()->set_terminateuser(true);

    send(clientRequest);
}

void ServiceClient::sltDisconnected()
{
    //emit isLogon(c, false, false);??
    //QMessageBox::information(0, "Disconnect", "");
    clearData();
}

bool ServiceClient::isConnected() const
{
    return m_qTcpSocket->state() == QAbstractSocket::ConnectedState;
}

void ServiceClient::reportError(QAbstractSocket::SocketError ErrCode)
{
    qDebug() << "SOCKET ERROR=" << ErrCode;
}

void ServiceClient::parseBuffer(const QByteArray& buff)
{
    //qDebug() << "Recieved! size=" << m_uiBlockSize;
    //QByteArray buff = m_qTcpSocket->read(m_uiBlockSize);

    ::uploadercontrol::ServiceResponse serverResponse;
    bool bParse = serverResponse.ParseFromArray(buff, m_uiBlockSize);//bytes.data(), bytes.size());
    if(!bParse)
    {
        //QMessageBox::information(0, "", "Ошибка чтения данных!");
        qDebug() << "Error of read!";
        m_flagForWaitBytes = WAIT_ID;
        m_uiBlockSize = 0;

        //
        /*QString rn = QString::number(200 * ((double)qrand()/RAND_MAX));
        FileUploadInfoModel fu1;
        fu1.setProgress(50);
        fu1.setFileName(rn);
        fu1.setIsBackup(false);
        fu1.setUploadStatus(FileUploadInfoModel::Pending);
        fu1.setCheckSum(rn);
        fu1.setIsArchive(false);
        fu1.setTask("fgfg");
        fu1.setDateTime(QDateTime::currentDateTime());
        fu1.setAttemptsRemaining(22);
        fu1.setResult("");
        fu1.setIndexed(false);
        emit dataUpload(fu1);

        fu1.setCheckSum("dfdfdfdf");
        fu1.setFileName("111111111111");
        fu1.setUploadStatus(FileUploadInfoModel::Pending);
        fu1.setIsBackup(true);
        emit dataUpload(fu1);*/
        //

        return;
    }
    //разбор байтов и определение типа

    switch(serverResponse.messagetype())
    {
        case ::uploadercontrol::ServiceResponse::AUTHORIZATION_STATUS:
        {
            //если пользователь еще не авторизован, то принимаем класс AutorizationResponce
            m_Credentials.setAdmin(serverResponse.authorization().isadmin());
            m_Credentials.setAlreadyAuthorized(serverResponse.authorization().alreadyauthorized());
            if(!serverResponse.authorization().isok())
                Disconnect();

            emit isLogon(m_Credentials, serverResponse.authorization().isok());

            break;
        }
        case ::uploadercontrol::ServiceResponse::FILE_STATUS:
        {
            //если пользователь авторизован, то другой тип сообщения
            FileUploadInfoModel fu;
            if(serverResponse.filestatus().has_login())
                fu.setLogin(QString::fromStdString(serverResponse.filestatus().login()));
            for(const auto& item : serverResponse.filestatus().infos())
            {
                int progress = 0;
                if(item.size() != 0)
                    progress = (100 * item.bytesprocessed()) / ((double)item.size());

                fu.setProgress(progress);
                fu.setFileName(QString::fromStdString(item.pathname()));
                fu.setIsBackup(item.isbackup());
                fu.setUploadStatus(convertStatus(item.status()));
                fu.setCheckSum(QString::fromStdString(item.checksum()));
                fu.setIsArchive(serverResponse.filestatus().responsetype() == ::uploadercontrol::Archive);
                fu.setTask(QString::fromStdString(item.task()));
                fu.setDateTime(QDateTime::fromTime_t(item.uploaddate()));
                fu.setAttemptsRemaining(item.retriescount());
                fu.setResult(QString::fromStdString(item.additionalmessage()));
                fu.setIndexed(item.has_fileid());

                //qDebug() << fu.fileName();
                emit dataUpload(fu);
            }
            break;
        }
        case ::uploadercontrol::ServiceResponse::COPYING_STATUS:
        {
            //информация о том сколько байтов скопировано при промежуточном копировании
            FileUploadInfoModel fu;
            fu.setFileSize(serverResponse.copyingstatus().filestotal());
            fu.setProgress(serverResponse.copyingstatus().filescopied());
            emit dataInfoCopying(fu);
            break;
        }
        case ::uploadercontrol::ServiceResponse::MAINTENANCE_STATUS:
        {
            //
            emit isPaused(serverResponse.maintenancestatus().ispaused());
            break;
        }
        case ::uploadercontrol::ServiceResponse::FILELIST:
        {
            //emit isPaused(serverResponse.maintenancestatus().ispaused());
            QList<FileSystemRemoteItemModel> fileList;
            for(int i = 0; i < serverResponse.filelist().list_size(); i++)
            {
                FileSystemRemoteItemModel item;
                item.setIsFolder(serverResponse.filelist().list(i).isfolder());
                item.setName(QString::fromStdString(serverResponse.filelist().list(i).name()));
                fileList.append(item);
            }
            responseFolderContents(fileList, serverResponse.filelist().foldercontentrecursively());
            break;
        }
        case ::uploadercontrol::ServiceResponse::ERRORMESSAGE:
        {
            //
            emit questionsAboutDownloading(QString::fromStdString(serverResponse.error().message()));
            break;
        }
        default:
        {
            qDebug() << "default messagetype = " << serverResponse.messagetype();
        }
    }
    //Обнуляем переменную
    m_flagForWaitBytes = WAIT_ID;
    m_uiBlockSize = 0;
}

void ServiceClient::readFromSocket()
{
    try
    {
        //qDebug() << "Recieved! new data! size=" << m_qTcpSocket->bytesAvailable();
        if (m_flagForWaitBytes == WAIT_ID)
        {
            //ждем ID-сообщения
            if (m_qTcpSocket->bytesAvailable() < (qint64)sizeof(quint16))
                return;

            quint16 _id;
            memcpy(&_id, m_qTcpSocket->read(sizeof(quint16)).data(), sizeof(quint16));
            _id = m_bIsLittleEndian ? qFromBigEndian(_id) : _id;
            if(_id != ID_Message)
                return;

            m_flagForWaitBytes = WAIT_SIZE;
        }
        if (m_flagForWaitBytes == WAIT_SIZE)
        {
            //ждем размер сообщения
            if (m_qTcpSocket->bytesAvailable() < (qint64)sizeof(quint32))
                return;

            memcpy(&m_uiBlockSize, m_qTcpSocket->read(sizeof(quint32)).data(), sizeof(quint32));
            m_uiBlockSize = m_bIsLittleEndian ? qFromBigEndian(m_uiBlockSize) : m_uiBlockSize;

            m_flagForWaitBytes = WAIT_MESSAGE;
        }

        //ждем пока блок прийдет полностью
        //m_flagForWaitBytes = WAIT_MESSAGE
        if (m_qTcpSocket->bytesAvailable() < m_uiBlockSize)
            return;
        else
        {
            //можно принимать новый блок
            qint64 bytesAvailable = m_qTcpSocket->bytesAvailable();
            qint64 bytesCurrent = 0;//количество прочитанный байт

            //полная обработка сообщения
            while(true)
            {
                if(m_flagForWaitBytes != WAIT_MESSAGE)
                {
                    if(bytesAvailable - bytesCurrent < sizeof(quint16) + sizeof(quint32))
                    {
                        m_flagForWaitBytes = WAIT_ID;
                        m_uiBlockSize = 0;
                        return;
                    }
                    //id
                    quint16 _id;
                    memcpy(&_id, m_qTcpSocket->read(sizeof(quint16)).data(), sizeof(quint16));
                    _id = m_bIsLittleEndian ? qFromBigEndian(_id) : _id;
                    if(_id != ID_Message)
                    {
                        bytesCurrent += sizeof(quint16);
                        continue;
                    }

                    memcpy(&m_uiBlockSize, m_qTcpSocket->read(sizeof(quint32)).data(), sizeof(quint32));
                    m_uiBlockSize = m_bIsLittleEndian ? qFromBigEndian(m_uiBlockSize) : m_uiBlockSize;

                    bytesCurrent += sizeof(quint16) + sizeof(quint32);
                    m_flagForWaitBytes = WAIT_MESSAGE;
                    //если недостаточная длина буфера
                    if(bytesAvailable - bytesCurrent < m_uiBlockSize)
                        return;
                    else
                        continue;
                }
                if(m_flagForWaitBytes == WAIT_MESSAGE)
                {
                    QByteArray buff = m_qTcpSocket->read(m_uiBlockSize);
                    bytesCurrent += m_uiBlockSize;
                    m_flagForWaitBytes = WAIT_ID;
                    parseBuffer(buff);
                }
            }//while
        }
    }
    catch(...)
    {
        qDebug() << "Error: read socket! Function: readFromSocket()";
        QMessageBox::information(0, "Error", "Error: read socket!");
    }
}

void ServiceClient::sltQueryLogon(const Credentials& c)
{
    m_Credentials = c;

    bool isVal = false;
    m_iPortService = Setting::value(Setting::KEY_PORT_SERVICE).toInt(&isVal);
    if(!isVal)
        m_iPortService = 0;
    m_iPortServer  = Setting::value(Setting::KEY_PORT_SERVER).toInt(&isVal);
    if(!isVal)
        m_iPortServer = 0;

    //Попытка установки соединения
    if(!Connect(c.service(), m_iPortService))
    {
        //
        //emit isLogon(c, true);
        qDebug() << QString("Unable to connect to remote server! port_server=%1, por_service=%2").arg(m_iPortServer).arg(m_iPortService);
        QMessageBox::information(0, "", "Не удается подключиться к удаленному серверу!");
        return;
    }
}

FileShortInfoModel::FileTypeModel ServiceClient::convertFileType(const uploadercontrol::FileType& type) const
{
    switch (type) {
        case uploadercontrol::VideoTrack:
            return FileShortInfoModel::VideoTrack;
        case uploadercontrol::RoadNotes:
            return FileShortInfoModel::RoadNotes;
        case uploadercontrol::Filter:
            return FileShortInfoModel::Filter;
        case uploadercontrol::Other:
            return FileShortInfoModel::Other;
        default:
            return FileShortInfoModel::Other;
    }
}

uploadercontrol::FileType ServiceClient::convertFileType(const FileShortInfoModel::FileTypeModel& type) const
{
    switch (type) {
        case FileShortInfoModel::VideoTrack:
            return uploadercontrol::VideoTrack;
        case FileShortInfoModel::RoadNotes:
            return uploadercontrol::RoadNotes;
        case FileShortInfoModel::Filter:
            return uploadercontrol::Filter;
        case FileShortInfoModel::Other:
            return uploadercontrol::Other;
        default:
            return uploadercontrol::Other;
    }
}

uploadercontrol::TaskStatus ServiceClient::convertStatus(const FileShortInfoModel::UploadStatusModel& status) const
{
    switch (status) {
        case FileShortInfoModel::Restarting:
            return uploadercontrol::Retrying;
        case FileShortInfoModel::Canceled:
            return uploadercontrol::Canceled;
        case FileShortInfoModel::Processing:
            return uploadercontrol::Processing;
        case FileShortInfoModel::Pending:
            return uploadercontrol::Pending;
        case FileShortInfoModel::Uploading:
            return uploadercontrol::Uploading;
        case FileShortInfoModel::Paused:
            return uploadercontrol::Paused;
        case FileShortInfoModel::Error:
            return uploadercontrol::Error;
        case FileShortInfoModel::OK:
            return uploadercontrol::Ok;
        default:
            return uploadercontrol::Pending;
    }
}

FileShortInfoModel::UploadStatusModel ServiceClient::convertStatus(const uploadercontrol::TaskStatus& status) const
{
    switch (status) {
        case uploadercontrol::Processing:
            return FileShortInfoModel::Processing;
        case uploadercontrol::Retrying:
            return FileShortInfoModel::Restarting;
        case uploadercontrol::Pending:
            return FileShortInfoModel::Pending;
        case uploadercontrol::Canceled:
            return FileShortInfoModel::Canceled;
        case uploadercontrol::Uploading:
            return FileShortInfoModel::Uploading;
        case uploadercontrol::Paused:
            return FileShortInfoModel::Paused;
        case uploadercontrol::Error:
            return FileShortInfoModel::Error;
        case uploadercontrol::Copying:
            return FileShortInfoModel::Copying;
        case uploadercontrol::Ok:
            return FileShortInfoModel::OK;
        default:
            return FileShortInfoModel::Pending;
    }
}

void ServiceClient::sltDataForUpload(const QList<FileShortInfoModel*> collection,
                                     const QString& task, bool indexed, bool deleteSource, bool intermediateCopy)
{
    /////FOR TESTING
    //QList<FileShortInfoModel*> collection;
    //QStringList slist;
    //slist.append("/media/BOOT/123/2010-12-14_09-03-29.mps");
    /*slist.append("/media/BOOT/123/DVR.DVR");
    slist.append("/media/BOOT/123/DVR_0002.DVR");
    slist.append("/media/BOOT/123/DVR_0003.DVR");
    slist.append("/media/BOOT/123/DVR_0004.DVR");
    slist.append("/media/BOOT/123/DVR_0005.DVR");
    slist.append("/media/BOOT/123/DVR_0006.DVR");
    slist.append("/media/BOOT/123/DVR_0007.DVR");
    slist.append("/media/BOOT/123/DVR_0008.DVR");*/
    //slist.append("F:/123/DVR_0004.DVR");
    //slist.append("F:/123/DVR_0005.DVR");
    //FileShortInfoModel item1;
    /*for(int i = 0; i < slist.size(); i++)
    {
        FileShortInfoModel* item1 = new FileShortInfoModel();
        item1->setFileName(slist[i]);
        item1->setIsBackup(false);
        collection.append(item1);
    }*/
    /////FOR TESTING

    ::uploadercontrol::ClientRequest clientRequest;
    clientRequest.set_messagetype(::uploadercontrol::ClientRequest::ADDTASK);

    ::uploadercontrol::ClientRequest_AddTask* _addTask = clientRequest.mutable_addtask();
    _addTask->set_taskname(task.toStdString());
    _addTask->set_index(indexed);
    _addTask->set_removesourcefiles(deleteSource);
    _addTask->set_docopy(intermediateCopy);

    for(const auto* fi: collection)
    {
        uploadercontrol::ClientRequest_AddTask_File* _file = _addTask->add_files();
        _file->set_pathname(fi->fileName().toStdString());
        _file->set_isbackup(fi->isBackup());
        _file->set_filetype(convertFileType(fi->fileType()));
    }

    send(clientRequest);

    qDebug() << "Command: task";
}

void ServiceClient::sltPausedCommand(bool pause)
{
    ::uploadercontrol::ClientRequest clientRequest;
    clientRequest.set_messagetype(::uploadercontrol::ClientRequest::COMMAND);

    clientRequest.mutable_command()->set_commandtype(
        pause ? ::uploadercontrol::CommandType::Pause :
                ::uploadercontrol::CommandType::StartUploading);

    send(clientRequest);

    qDebug() << "Command: pause: " << pause;
}

void ServiceClient::sltArchiveCommand(bool bDeleteSource)
{
    ::uploadercontrol::ClientRequest clientRequest;
    clientRequest.set_messagetype(::uploadercontrol::ClientRequest::MAINTAINCE);
    clientRequest.mutable_maintaince()->set_requesttype(::uploadercontrol::RequestType::MoveToArchive);
    clientRequest.mutable_maintaince()->set_removesourcefiles(bDeleteSource);

    send(clientRequest);

    qDebug() << "Command: to archive";
}

void ServiceClient::sltGetArchiveCommand()
{
    ::uploadercontrol::ClientRequest clientRequest;
    clientRequest.set_messagetype(::uploadercontrol::ClientRequest::MAINTAINCE);
    clientRequest.mutable_maintaince()->set_requesttype(::uploadercontrol::RequestType::GiveMeArchive);

    send(clientRequest);

    qDebug() << "Command: get archive";
}

void ServiceClient::sltRepeatCommand()
{
    ::uploadercontrol::ClientRequest clientRequest;
    clientRequest.set_messagetype(::uploadercontrol::ClientRequest::COMMAND);
    clientRequest.mutable_command()->set_commandtype(::uploadercontrol::CommandType::Retry);

    send(clientRequest);

    qDebug() << "Command: repeat";
}

void ServiceClient::sltSendInformationMail()
{
    ::uploadercontrol::ClientRequest clientRequest;
    clientRequest.set_messagetype(::uploadercontrol::ClientRequest::MAINTAINCE);
    clientRequest.mutable_maintaince()->set_requesttype(::uploadercontrol::RequestType::SendEmail);
    //clientRequest.mutable_maintaince()->set_login("login");

    send(clientRequest);

    qDebug() << "Command: send mail";
}

void ServiceClient::sltCancelDownload(const QModelIndexList& indexList)
{
    for(const QModelIndex &index: indexList)
    {
        ::uploadercontrol::ClientRequest clientRequest;
        clientRequest.set_messagetype(::uploadercontrol::ClientRequest::COMMAND);
        clientRequest.mutable_command()->set_commandtype(::uploadercontrol::CommandType::Cancel);
        clientRequest.mutable_command()->set_checksum(index.data(Qt::DisplayRole).toString().toStdString());

        send(clientRequest);

        qDebug() << "Command: on cancel: checksum=" << index.data(Qt::DisplayRole).toString();
    }
}

void ServiceClient::sltChangeIsBackup(const QString& checksum, bool isbackup)
{
    ::uploadercontrol::ClientRequest clientRequest;
    clientRequest.set_messagetype(::uploadercontrol::ClientRequest::COMMAND);
    clientRequest.mutable_command()->set_checksum(checksum.toStdString());
    clientRequest.mutable_command()->set_isbackup(isbackup);
    clientRequest.mutable_command()->set_commandtype(::uploadercontrol::CommandType::ChangeOption);

    send(clientRequest);

    qDebug() << "Command: change Backup: checksum=" << checksum;
}

void ServiceClient::sltResponseAboutErrorDownloading(::uploadercontrol::CommandType type)
{
    ::uploadercontrol::ClientRequest clientRequest;
    clientRequest.set_messagetype(::uploadercontrol::ClientRequest::COMMAND);
    clientRequest.mutable_command()->set_commandtype(type);

    send(clientRequest);

    qDebug() << "Command: response from client about error of struct folder! type=" << type;
}

void ServiceClient::sltRequestFolderContents(const QString& dir, bool is_listrecursively)
{
    ::uploadercontrol::ClientRequest clientRequest;
    clientRequest.set_messagetype(::uploadercontrol::ClientRequest::OPEN_ITEM);
    clientRequest.mutable_openitem()->set_item(dir.toStdString());
    clientRequest.mutable_openitem()->set_listrecursively(is_listrecursively);

    send(clientRequest);

    qDebug() << "Command: get dir " << dir;
}

void ServiceClient::sltLogout()
{
    Disconnect();
}
