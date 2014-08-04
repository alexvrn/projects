#ifndef UPLOADPROGRESSMODEL_H
#define UPLOADPROGRESSMODEL_H

#include <QObject>
#include "Model/fileshortinfomodel.h"
#include "Model/fileuploadinfomodel.h"
#include "Service/credentials.h"
#include <QHash>

class UploadProgressModel : public QObject
{
    Q_OBJECT
public:
    enum ResetModel {
        No = 0,
        All,
        Item
    };

    explicit UploadProgressModel(QObject *parent = 0);
    ~UploadProgressModel();

    void setCollection(QList<FileShortInfoModel*>);
    QList<FileShortInfoModel*> collection() const;

    bool admin() const;
    void setAdmin(bool);

    bool isVisibleStatus(const FileUploadInfoModel*);

    void setShowUploading(bool);
    bool showUploading() const;

    void setShowCopying(bool);
    bool showCopying() const;

    void setShowArchive(bool);
    bool showArchive() const;

    void setShowError(bool);
    bool showError() const;

    void setShowOK(bool);
    bool showOK() const;

    void setVisiblePause(bool);
    bool visiblePause() const;

    bool isEnabledRepeat() const;

    ResetModel addFile(const FileUploadInfoModel&, int &);

    void clear();

    void setConnect(bool _connect, const Credentials& );

    void setIsPaused(bool);
    bool isPaused() const;

    QString statistic() const;

    QStringList users() const;

    QString nameOnPause() const;

    void changeIsBackup(const QString&, bool);

    FileShortInfoModel* keyForIndex(const QModelIndex& index) const;

    void update();

    int size() const;

private:
    void setNameOnPause(const QString&);

signals:

public slots:

private:
    bool m_bShowUploadong;
    bool m_bShowCopying;
    bool m_bShowArchive;
    bool m_bShowError;
    bool m_bShowOK;

    bool m_bVisiblePause;

    //QHash<FileShortInfoModel*, uint> m_Collection;
    QHash<FileShortInfoModel*, QString> m_Collection;
    //QList<FileShortInfoModel*> m_Collection;
    bool m_bIsAdmin;
    bool m_bIsPaused;
    QString m_strNameOnPause;

    QList<FileShortInfoModel*> m_conditionCollection;
};

//inline uint qHash(const FileUploadInfoModel& object, uint seed = 0)
//{
//    return qHash(object.checkSum(), seed);
//}

#endif // UPLOADPROGRESSMODEL_H
