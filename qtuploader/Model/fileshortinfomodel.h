#ifndef FILESHORTINFOMODEL_H
#define FILESHORTINFOMODEL_H
#include <QString>

class FileShortInfoModel
{
public:
    enum UploadStatusModel {
        None = 0,
        Pending,
        Copying,
        Canceling,
        Canceled,
        Processing,
        Restarting,
        Uploading,
        Error,
        Paused,
        OK
    };

    enum FileTypeModel {
        /// <remarks/>
        VideoTrack = 0,

        /// <remarks/>
        RoadNotes,

        /// <remarks/>
        Filter,

        /// <remarks/>
        Other
    };

    QString fileName() const;
    void setFileName(const QString&);

    quint64 fileSize() const;
    void setFileSize(quint64);

    bool isBackup() const;
    void setIsBackup(bool);

    FileTypeModel fileType() const;
    void setFileType(FileTypeModel);

public:
    FileShortInfoModel(){ }
    FileShortInfoModel(const FileShortInfoModel&);
    FileShortInfoModel(const QString &path);

    static QString typeName(FileTypeModel);
    static QString statusName(UploadStatusModel);
    static FileShortInfoModel* LoadRelatedFields(const QString &path);
    static bool DetectIsBackupFlag(const QString &path);

private:
    QString m_strFileName;
    bool m_bIsBackup;
    quint64 m_iFileSize;//FileShortInfo
    FileTypeModel m_FileType;
};

#endif // FILESHORTINFO_H
