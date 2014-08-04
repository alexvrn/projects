#ifndef SERVICECLIENT_H
#define SERVICECLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QModelIndex>
#include "Service/credentials.h"
#include "Model/fileuploadinfomodel.h"
#include "Service/RemoteDialog/filesystemremotemodel.h"

#include "protocol/remoteclient.pb.h"

class ServiceClient : public QObject
{
    Q_OBJECT

private:
    enum WAIT_STATE {
        WAIT_ID,
        WAIT_SIZE,
        WAIT_MESSAGE
    };

public:
    explicit ServiceClient(QObject *parent = 0);
    ~ServiceClient();

    bool Connect(QString host, quint16 port);
    void Disconnect();

    bool isConnected() const;

private:
    void parseBuffer(const QByteArray& buff);

    void clearData();

    //записывает в начало сообщения(array) ID и размер сообщения(byteSize)
    void writeOnBeginningMessage(char *& array, int byteSize);
    void send(::uploadercontrol::ClientRequest &clientRequest);

    //конвертация типа
    FileShortInfoModel::FileTypeModel convertFileType(const uploadercontrol::FileType&) const;
    uploadercontrol::FileType convertFileType(const FileShortInfoModel::FileTypeModel&) const;

    uploadercontrol::TaskStatus convertStatus(const FileShortInfoModel::UploadStatusModel& ) const;
    FileShortInfoModel::UploadStatusModel convertStatus(const uploadercontrol::TaskStatus& ) const;

signals:
    void isLogon(const Credentials&, bool);
    void dataUpload(const FileUploadInfoModel&);
    //void dataCopying(const FileUploadInfoModel&);
    void dataInfoCopying(const FileUploadInfoModel&);
    void isPaused(bool);
    void questionsAboutDownloading(const QString&);
    void responseFolderContents(const QList<FileSystemRemoteItemModel>&, bool);

private slots:
    void sltConnected();
    void sltDisconnected();

    void sltQueryLogon(const Credentials&);
    void sltLogout();

    void sltDataForUpload(const QList<FileShortInfoModel*>, const QString&,bool,bool,bool);
    void sltPausedCommand(bool);
    void sltArchiveCommand(bool);
    void sltRepeatCommand();
    void sltSendInformationMail();
    void sltGetArchiveCommand();
    void sltCancelDownload(const QModelIndexList&);
    void sltChangeIsBackup(const QString&, bool);
    void sltRequestFolderContents(const QString&, bool);

    void sltResponseAboutErrorDownloading(::uploadercontrol::CommandType);

    void reportError(QAbstractSocket::SocketError ErrCode);
    void readFromSocket();

private:
    QTcpSocket *m_qTcpSocket;

    Credentials m_Credentials;
    int m_iPortService;
    int m_iPortServer;
    WAIT_STATE m_flagForWaitBytes;
    quint32 m_uiBlockSize;

    //flag for endian
    bool m_bIsLittleEndian;

private:
   static const quint16 ID_Message;
};

#endif // SERVICECLIENT_H
