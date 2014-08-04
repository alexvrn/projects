#include "Model/fileshortinfomodel.h"

#include <QDir>

FileShortInfoModel::FileShortInfoModel(const QString &path)
{
    //this = LoadRelatedFields(path);
}

FileShortInfoModel::FileShortInfoModel(const FileShortInfoModel &obj)
{
    setFileName(obj.fileName());
    setFileSize(obj.fileSize());
    setFileType(obj.fileType());
    setIsBackup(obj.isBackup());
}

QString FileShortInfoModel::fileName() const
{
    return m_strFileName;
}

void FileShortInfoModel::setFileName(const QString &fileName)
{
    m_strFileName = fileName;
}

quint64 FileShortInfoModel::fileSize() const
{
    return m_iFileSize;
}

void FileShortInfoModel::setFileSize(quint64 size)
{
    m_iFileSize = size;
}

bool FileShortInfoModel::isBackup() const
{
    return m_bIsBackup;
}

void FileShortInfoModel::setIsBackup(bool backup)
{
    m_bIsBackup = backup;
}

FileShortInfoModel::FileTypeModel FileShortInfoModel::fileType() const
{
    return m_FileType;
}

void FileShortInfoModel::setFileType(FileTypeModel type)
{
    m_FileType = type;
}


QString FileShortInfoModel::typeName(FileTypeModel type)
{
    switch(type)
    {
        case VideoTrack: return "VideoTrack";
        case RoadNotes:  return "RoadNotes";
        case Filter:     return "Filter";
        case Other:      return "Other";
        default: {return "Other";}
    }
}

QString FileShortInfoModel::statusName(UploadStatusModel status)
{
    switch(status)
    {
        case None:       return "None";
        case Pending:    return "Pending";
        case Processing: return "Processing";
        case Canceling:  return "Canceling";
        case Canceled:   return "Canceled";
        case Restarting: return "Restarting";
        case Uploading:  return "Uploading";
        case Paused:     return "Paused";
        case Copying:    return "Copying";
        case Error:      return "Error";
        case OK:         return "OK";
        default: {return "";}
    }
}

FileShortInfoModel* FileShortInfoModel::LoadRelatedFields(const QString &path)
{
    FileShortInfoModel* fi = new FileShortInfoModel();
    QFileInfo fileInfo(path);
    QString extension = fileInfo.completeSuffix();
    if (extension.isNull())
        fi->setFileType( Other );
    else
    {
        extension = extension.toLower();
        if (extension == "dvr")
            fi->setFileType( VideoTrack );
        else if (extension == "mp")
            fi->setFileType( RoadNotes );
        else
            fi->setFileType( Other );
    }
    fi->setIsBackup( FileShortInfoModel::DetectIsBackupFlag(path) );
    fi->setFileSize( fileInfo.size() );
    fi->setFileName( path );

    return fi;
}

bool FileShortInfoModel::DetectIsBackupFlag(const QString &path)
{
    return (path.toLower().contains("backup") ||
              path.toLower().contains("бэкап") ||
              path.toLower().contains("запасные"));
}
