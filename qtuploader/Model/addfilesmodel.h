#ifndef ADDMODEL_H
#define ADDMODEL_H

#include <QObject>
#include <QString>
#include "Model/fileshortinfomodel.h"
#include "Service/credentials.h"
#include <QModelIndexList>
#include <QHash>

class AddFilesModel : public QObject
{
    Q_OBJECT
public:
    explicit AddFilesModel(QObject *parent = 0);
    ~AddFilesModel();

    QString task() const;
    void setTask(const QString&);

    QString version() const;
    void setVersion(const QString&);

    virtual void setTextLogout(QString);
    QString textLogout() const;

    void setCollection(QList<FileShortInfoModel*>);
    QList<FileShortInfoModel*> collection() const;

    void setVisibleCheckIndexFiles(bool);
    bool visibleCheckIndexFiles() const;

    void sltDeleteSelectItem(const QModelIndexList&);
    void sltEstablishIsBackupSelectItem(const QModelIndexList&, bool);
    void sltEstablishIsBackupAll(bool);

    bool enabledUpload() const;

    void setIntermediateCopy(bool);
    bool intermediateCopy() const;

    void setIndexed(bool);
    bool indexed() const;

    void setVisibleLogout(bool);
    bool visibleLogout() const;

    void setCheckDeleteSource(bool);
    bool isCheckDeleteSource() const;

    void setEnabledDeleteSource(bool);
    bool isEnabledDeleteSource() const;

    void setIsRemoteService(bool);
    bool isRemoteService() const;

    void clear();

    void clearCollection();

    void setConnect(bool, const Credentials&);

    void logout();

    void addFiles(const QStringList&);

private:
    bool isEnabledUpload() const;

    void setEnabledUpload(bool);

signals:

public slots:

private:
    QString m_strTask;
    QString m_strVersion;
    bool m_bVisibleCheckIndexFiles;
    QString m_strTextLogout;
    //QList<FileShortInfoModel*> m_Collection;
    QHash<FileShortInfoModel*, uint> m_Collection;
    bool m_bEnabledUpload;
    bool m_bIntermediateCopy;
    bool m_bIndexed;
    bool m_bVisibleLogout;
    bool m_bDeleteSource;
    bool m_bEnabledDeleteSource;
    bool m_bIsRemoteService;
};

#endif // ADDMODEL_H
