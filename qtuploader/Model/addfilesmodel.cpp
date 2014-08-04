#include "Model/addfilesmodel.h"
#include <QDebug>
#include <QStringList>
#include <QHash>
#include <QMessageBox>

AddFilesModel::AddFilesModel(QObject *parent) :
    QObject(parent)
{
    clear();
}

AddFilesModel::~AddFilesModel()
{
    //qDebug() << "Delete AddModel";
    clearCollection();
}

QString AddFilesModel::task() const
{
    return m_strTask;
}

void AddFilesModel::setTask(const QString& _task)
{
    m_strTask = _task;

    setEnabledUpload(isEnabledUpload());
}

QString AddFilesModel::version() const
{
    return m_strVersion;
}

void AddFilesModel::setVersion(const QString& v)
{
    m_strVersion = v;
}

void AddFilesModel::setTextLogout(QString txt_log)
{
    m_strTextLogout = txt_log;
}

QString AddFilesModel::textLogout() const
{
    return m_strTextLogout;
}

void AddFilesModel::setCollection(QList<FileShortInfoModel*> _collection)
{
    clearCollection();

    for(const auto &f: _collection)
        m_Collection[f] = qHash(f);

    setEnabledUpload(isEnabledUpload());
}

QList<FileShortInfoModel*> AddFilesModel::collection() const
{
    return m_Collection.keys();
}

void AddFilesModel::setVisibleCheckIndexFiles(bool vcif)
{
    m_bVisibleCheckIndexFiles = vcif;
}

bool AddFilesModel::visibleCheckIndexFiles() const
{
    return m_bVisibleCheckIndexFiles;
}

void AddFilesModel::sltDeleteSelectItem(const QModelIndexList& indexList)
{
    for(const QModelIndex& mIndex: indexList)
    {
        uint h = qHash(mIndex.data(Qt::DisplayRole).toString().toLower());
        if(!m_Collection.values().contains(h))
            continue;

        auto delKey = m_Collection.key(h);
        m_Collection.remove(delKey);
        delete delKey;
    }
    setEnabledUpload(isEnabledUpload());
}

void AddFilesModel::sltEstablishIsBackupSelectItem(const QModelIndexList& indexList, bool isBackup)
{
    for(const QModelIndex& mIndex: indexList)
    {
        uint h = qHash(mIndex.data(Qt::DisplayRole).toString().toLower());
        if(!m_Collection.values().contains(h))
            continue;

        m_Collection.key(h)->setIsBackup(isBackup);
    }
}

void AddFilesModel::sltEstablishIsBackupAll(bool isBackup)
{
    for(auto& key: m_Collection.keys())
        key->setIsBackup(isBackup);
}

void AddFilesModel::setEnabledUpload(bool eu)
{
    m_bEnabledUpload = eu;
}

bool AddFilesModel::enabledUpload() const
{
    return m_bEnabledUpload;
}

void AddFilesModel::setIntermediateCopy(bool ic)
{
    m_bIntermediateCopy = ic;

    setEnabledDeleteSource(ic);
}

bool AddFilesModel::intermediateCopy() const
{
    return m_bIntermediateCopy;
}

void AddFilesModel::setCheckDeleteSource(bool ds)
{
    m_bDeleteSource = ds;
}

bool AddFilesModel::isCheckDeleteSource() const
{
    return m_bDeleteSource;
}

void AddFilesModel::setEnabledDeleteSource(bool eds)
{
    m_bEnabledDeleteSource = eds;
}

bool AddFilesModel::isEnabledDeleteSource() const
{
    return m_bEnabledDeleteSource;
}

void AddFilesModel::setIsRemoteService(bool rs)
{
    m_bIsRemoteService = rs;
}

bool AddFilesModel::isRemoteService() const
{
    return m_bIsRemoteService;
}

void AddFilesModel::setIndexed(bool i)
{
    m_bIndexed = i;
}

bool AddFilesModel::indexed() const
{
    return m_bIndexed;
}

void AddFilesModel::setVisibleLogout(bool vl)
{
    m_bVisibleLogout = vl;
}

bool AddFilesModel::visibleLogout() const
{
    return m_bVisibleLogout;
}

bool AddFilesModel::isEnabledUpload() const
{
    return collection().size() > 0 && !task().trimmed().isEmpty();
}

void AddFilesModel::logout()
{
    clear();
}

void AddFilesModel::clear()
{
    setTask(QString());

    setTextLogout(QString());

    setIntermediateCopy(false);

    setIndexed(false);

    setVisibleLogout(false);

    setIntermediateCopy(false);

    clearCollection();

    setIsRemoteService(false);
}

void AddFilesModel::clearCollection()
{
    qDeleteAll(m_Collection.keys());
    m_Collection.clear();

    setEnabledUpload(isEnabledUpload());
}

void AddFilesModel::setConnect(bool _connect, const Credentials& _c)
{
    if(_connect)
    {
        setVisibleLogout(true);
        setIntermediateCopy(false);
        setVisibleCheckIndexFiles(_c.admin());
        setTextLogout(_c.login() + tr(": Exit"));
        setIsRemoteService(_c.isRemoteService());
    }
    else
    {
        logout();//clear()
    }
}

void AddFilesModel::addFiles(const QStringList& files)
{
    for(const QString &f: files)
    {
        uint h = qHash(f.toLower());
        if(!m_Collection.values().contains(h))
        {
            FileShortInfoModel * fi = FileShortInfoModel::LoadRelatedFields(f);
            m_Collection.insert(fi, h);
        }
    }
    setEnabledUpload(isEnabledUpload());
}
