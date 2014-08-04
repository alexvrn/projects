#include "uploadprogressmodel.h"
#include <QDebug>
#include <QHash>
#include <QModelIndex>

UploadProgressModel::UploadProgressModel(QObject *parent) :
    QObject(parent)
{
    clear();
}

UploadProgressModel::~UploadProgressModel()
{
    //qDebug() << "Delete UploadProgressModel";
    clear();
}

bool UploadProgressModel::isVisibleStatus(const FileUploadInfoModel * item)
{
    if(item->isArchive() && showArchive())
        return true;

    if(item->uploadStatus() == FileUploadInfoModel::OK && showOK())
        return true;

    if(item->uploadStatus() == FileUploadInfoModel::Error && showError())
        return true;

    if(item->uploadStatus() == FileUploadInfoModel::Copying && showCopying())
        return true;

    if((item->uploadStatus() == FileUploadInfoModel::Pending ||
        item->uploadStatus() == FileUploadInfoModel::Processing ||
        item->uploadStatus() == FileUploadInfoModel::Restarting ||
        item->uploadStatus() == FileUploadInfoModel::Uploading ||
        item->uploadStatus() == FileUploadInfoModel::Paused) && showUploading())
        return true;

    return false;
}

UploadProgressModel::ResetModel UploadProgressModel::addFile(const FileUploadInfoModel& item, int &indexUpdate)
{
    QString h = item.checkSum();
    if(!m_Collection.values().contains(h))
    {
        FileUploadInfoModel * newItem = new FileUploadInfoModel(item);
        Q_CHECK_PTR(newItem);
        m_Collection[newItem] = h;
        if(isVisibleStatus(newItem))
        {
            update();
            return ResetModel::All;
        }
        else
            return ResetModel::No;
    }
    else
    {
        FileShortInfoModel * ref = m_Collection.key(h);
        FileUploadInfoModel * obj = (FileUploadInfoModel*)ref;//m_Collection.key(h));
        Q_CHECK_PTR(obj);
        obj->setProgress(item.progress());
        obj->setUploadStatus(item.uploadStatus());
        obj->setIsArchive(item.isArchive());
        obj->setResult(item.result());
        obj->setFileName(item.fileName());
        obj->setTask(item.task());
        obj->setDateTime(item.dateTime());
        obj->setAttemptsRemaining(item.attemptsRemaining());
        obj->setIsBackup(item.isBackup());
        if(isVisibleStatus(obj))
        {
            //return m_Collection.keys().indexOf(ref);
            if(m_conditionCollection.contains(ref))
            {
                indexUpdate = m_conditionCollection.indexOf(ref);
                return ResetModel::Item;
            }
            else
                return ResetModel::All;
        }
        else
        {
            if(m_conditionCollection.contains(ref))
                return ResetModel::All;
            else
                return ResetModel::No;
        }
    }
}

void UploadProgressModel::changeIsBackup(const QString& checksum, bool isbackup)
{
    //uint h = qHash(checksum);
    if(!m_Collection.values().contains(checksum))
        return;

    ((FileUploadInfoModel*)m_Collection.key(checksum))->setIsBackup(isbackup);
}

FileShortInfoModel* UploadProgressModel::keyForIndex(const QModelIndex& index) const
{
    //uint h = qHash( index.sibling(index.row(), FIELD_CHECKSUM).data(Qt::DisplayRole).toString());
    //return m_Collection.key(h);
    if(index.row() >= m_conditionCollection.size() )
        return Q_NULLPTR;

    return (FileUploadInfoModel *)(m_conditionCollection[index.row()]);
    //return (FileUploadInfoModel *)m_Collection.keys()[index.row()];
}

void UploadProgressModel::setCollection(QList<FileShortInfoModel*> _collection)
{
    clear();

    for(const auto &f: _collection)
        m_Collection[f] = ((FileUploadInfoModel*)f)->checkSum();
}

void UploadProgressModel::update()
{
    m_conditionCollection.clear();
    m_conditionCollection = collection();
}

int UploadProgressModel::size() const
{
    return m_conditionCollection.size();
}

QList<FileShortInfoModel*> UploadProgressModel::collection() const
{
    QList<FileShortInfoModel*> conditionCollection;
    for(const auto &item: m_Collection.keys())
    {
        FileUploadInfoModel* uploadItem = (FileUploadInfoModel*)item;

        if (showArchive())
        {
            if(uploadItem->isArchive())
            {
                conditionCollection.append(item);
                continue;
            }
        }
        else
            if(uploadItem->isArchive())
                continue;

        if (showOK())
        {
            if(uploadItem->uploadStatus() == FileUploadInfoModel::OK)
            {
                conditionCollection.append(item);
                continue;
            }
        }
        if (showError())
        {
            if(uploadItem->uploadStatus() == FileUploadInfoModel::Error ||
               uploadItem->uploadStatus() == FileUploadInfoModel::Canceled)
            {
                conditionCollection.append(item);
                continue;
            }
        }

        if (showCopying())
        {
            if(uploadItem->uploadStatus() == FileUploadInfoModel::Copying)
            {
                conditionCollection.append(item);
                continue;
            }
        }
        if (showUploading())
        {
            if(uploadItem->uploadStatus() == FileUploadInfoModel::Pending ||
               uploadItem->uploadStatus() == FileUploadInfoModel::Uploading ||
               uploadItem->uploadStatus() == FileUploadInfoModel::Processing ||
               uploadItem->uploadStatus() == FileUploadInfoModel::Restarting ||
               uploadItem->uploadStatus() == FileUploadInfoModel::Paused)
            {
                conditionCollection.append(item);
                continue;
            }
        }
    }
    return conditionCollection;//m_Collection.keys();
}

bool UploadProgressModel::admin() const
{
    return m_bIsAdmin;
}

void UploadProgressModel::setAdmin(bool _admin)
{
    m_bIsAdmin = _admin;
}

void UploadProgressModel::setShowUploading(bool show)
{
    m_bShowUploadong = show;
    update();
}

bool UploadProgressModel::showUploading() const
{
    return m_bShowUploadong;
}

void UploadProgressModel::setShowCopying(bool show)
{
    m_bShowCopying = show;
    update();
}

bool UploadProgressModel::showCopying() const
{
    return m_bShowCopying;
}

void UploadProgressModel::setShowArchive(bool show)
{
    m_bShowArchive = show;
    update();
}

bool UploadProgressModel::showArchive() const
{
    return m_bShowArchive;
}

void UploadProgressModel::setShowError(bool show)
{
    m_bShowError = show;
    update();
}

bool UploadProgressModel::showError() const
{
    return m_bShowError;
}

void UploadProgressModel::setShowOK(bool show)
{
    m_bShowOK = show;
    update();
}

bool UploadProgressModel::showOK() const
{
    return m_bShowOK;
}

void UploadProgressModel::setVisiblePause(bool v)
{
    m_bVisiblePause = v;
}

bool UploadProgressModel::visiblePause() const
{
    return m_bVisiblePause;
}

bool UploadProgressModel::isEnabledRepeat() const
{
    for(const auto &item: m_Collection.keys())
    {
        FileUploadInfoModel* uploadItem = (FileUploadInfoModel*)item;
        if(uploadItem->uploadStatus() == FileUploadInfoModel::Restarting)
            return true;
    }
    return false;
}

void UploadProgressModel::setIsPaused(bool p)
{
    m_bIsPaused = p;

    setNameOnPause(isPaused() ? tr("Start") : tr("Pause"));
}

bool UploadProgressModel::isPaused() const
{
    return m_bIsPaused;
}

QString UploadProgressModel::nameOnPause() const
{
    return m_strNameOnPause;
}

void UploadProgressModel::setNameOnPause(const QString& name)
{
    m_strNameOnPause = name;
}

void UploadProgressModel::clear()
{
    setAdmin(false);
    setShowOK(false);
    setShowError(false);
    setShowArchive(false);
    setShowUploading(false);
    setShowCopying(false);
    setIsPaused(false);

    qDeleteAll(m_Collection.keys());
    m_Collection.clear();
}

void UploadProgressModel::setConnect(bool _connect, const Credentials& _c)
{
    if(_connect)
    {
        setAdmin(_c.admin());
        setShowOK(true);
        setShowError(true);
        setShowArchive(false);
        setShowUploading(true);
        setShowCopying(true);
        setIsPaused(false);
        setVisiblePause(_c.admin());
    }
    else
    {
        clear();
    }
}

QString UploadProgressModel::statistic() const
{
    int cntOK = 0;
    int cntUpload = 0;
    int cntError = 0;
    int cntCopying = 0;
    int cntArchive = 0;
    for(const auto &item: m_Collection.keys())
    {
        FileUploadInfoModel* uploadItem = (FileUploadInfoModel*)item;

        if(uploadItem->isArchive()) {
            cntArchive++;
            continue;
        }
        if(uploadItem->uploadStatus() == FileUploadInfoModel::OK) {
            cntOK++;
            continue;
        }
        if(uploadItem->uploadStatus() == FileUploadInfoModel::Uploading ||
           uploadItem->uploadStatus() == FileUploadInfoModel::Pending ||
           uploadItem->uploadStatus() == FileUploadInfoModel::Processing) {
            cntUpload++;
            continue;
        }
        if(uploadItem->uploadStatus() == FileUploadInfoModel::Copying) {
            cntCopying++;
            continue;
        }
        if(uploadItem->uploadStatus() == FileUploadInfoModel::Error ||
           uploadItem->uploadStatus() == FileUploadInfoModel::Canceled) {
            cntError++;
            continue;
        }
    }

    return QString("OK: %1, Upload: %2, Error: %3, Copying: %4, Archive: %5")
               .arg(cntOK)
               .arg(cntUpload)
               .arg(cntError)
               .arg(cntCopying)
               .arg(cntArchive);
}

QStringList UploadProgressModel::users() const
{
    QStringList _users;
    for(const auto &item: m_Collection.keys())
    {
        QString _login = ((FileUploadInfoModel*)item)->login();
        if(!_users.contains(_login))
            _users.append(_login);
    }
    return _users;
}
