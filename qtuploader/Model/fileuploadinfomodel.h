#ifndef FILEUPLOADINFOMODEL_H
#define FILEUPLOADINFOMODEL_H

#include "Model/fileshortinfomodel.h"
#include <QDateTime>

class FileUploadInfoModel : public FileShortInfoModel
{
public:
    FileUploadInfoModel();
    FileUploadInfoModel(const FileUploadInfoModel&);

    QString login() const;
    void setLogin(QString);

    QString task() const;
    void setTask(QString);

    quint64 progress() const;
    void setProgress(quint64);

    bool indexed() const;
    void setIndexed(bool);

    UploadStatusModel uploadStatus() const;
    void setUploadStatus(const UploadStatusModel&);

    QString result() const;
    void setResult(QString);

    int attemptsRemaining() const;
    void setAttemptsRemaining(int);

    QString checkSum() const;
    void setCheckSum(QString);

    bool isArchive() const;
    void setIsArchive(bool);

    QDateTime dateTime() const;
    void setDateTime(const QDateTime&);

private:
    UploadStatusModel m_Status;
    quint64 m_iProgress;
    QString m_strLogin;
    QString m_strTask;
    bool m_bIndexed;
    QString m_strResult;
    int m_iAttemptsRemaining;
    QString m_strCheckSum;
    bool m_bArchive;
    QDateTime m_qDateTime;
};

#endif // FILEUPLOADINFO_H
