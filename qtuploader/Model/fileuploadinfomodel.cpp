#include "Model/fileuploadinfomodel.h"

FileUploadInfoModel::FileUploadInfoModel()
{
    setLogin(QString());
    setDateTime(QDateTime());
}

FileUploadInfoModel::FileUploadInfoModel(const FileUploadInfoModel& obj)
    : FileShortInfoModel(obj)
{
    setLogin(obj.login());
    setFileName(obj.fileName());
    setCheckSum(obj.checkSum());
    setFileSize(obj.fileSize());
    setAttemptsRemaining(obj.attemptsRemaining());
    setFileType(obj.fileType());
    setIndexed(obj.indexed());
    setIsBackup(obj.isBackup());
    setProgress(obj.progress());
    setTask(obj.task());
    setUploadStatus(obj.uploadStatus());
    setResult(obj.result());
    setIsArchive(obj.isArchive());
    setDateTime(obj.dateTime());
}

QString FileUploadInfoModel::login() const
{
    return m_strLogin;
}

void FileUploadInfoModel::setLogin(QString login)
{
    m_strLogin = login;
}

QString FileUploadInfoModel::task() const
{
    return m_strTask;
}

void FileUploadInfoModel::setTask(QString task)
{
    m_strTask = task;
}

quint64 FileUploadInfoModel::progress() const
{
    return m_iProgress;
}

void FileUploadInfoModel::setProgress(quint64 progress)
{
    m_iProgress = progress;
}

bool FileUploadInfoModel::indexed() const
{
    return m_bIndexed;
}

void FileUploadInfoModel::setIndexed(bool index)
{
    m_bIndexed = index;
}

FileUploadInfoModel::UploadStatusModel FileUploadInfoModel::uploadStatus() const
{
    return m_Status;
}

void FileUploadInfoModel::setUploadStatus(const UploadStatusModel& status)
{
    m_Status = status;
}

QString FileUploadInfoModel::result() const
{
    return m_strResult;
}

void FileUploadInfoModel::setResult(QString result)
{
    m_strResult = result;
}

int FileUploadInfoModel::attemptsRemaining() const
{
    return m_iAttemptsRemaining;
}

void FileUploadInfoModel::setAttemptsRemaining(int attemptsRemaining)
{
    m_iAttemptsRemaining = attemptsRemaining;
}

QString FileUploadInfoModel::checkSum() const
{
    return m_strCheckSum;
}

void FileUploadInfoModel::setCheckSum(QString checkSum)
{
    m_strCheckSum = checkSum;
}

bool FileUploadInfoModel::isArchive() const
{
    return m_bArchive;
}

void FileUploadInfoModel::setIsArchive(bool _arc)
{
    m_bArchive = _arc;
}

QDateTime FileUploadInfoModel::dateTime() const
{
    return m_qDateTime;
}

void FileUploadInfoModel::setDateTime(const QDateTime& dt)
{
    m_qDateTime = dt;
}

