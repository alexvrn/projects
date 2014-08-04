#include "filesystemremotemodel.h"
#include <QList>

FileSystemRemote::FileSystemRemote(bool isfolder, QObject *parent) :
    QAbstractTableModel(parent)
{

    setIsFolder(isfolder);

    m_hashFieldName[FIELD_FILENAME] = tr("Name");
    m_hashFieldName[FIELD_TYPE] = tr("Type");

    currentPath = QString();
}

bool FileSystemRemote::isFolder() const
{
    return m_bIsFolder;
}

void FileSystemRemote::setIsFolder(bool isfolder)
{
    m_bIsFolder = isfolder;
}

void FileSystemRemote::getData(const QList<FileSystemRemoteItemModel> &list)
{
    qDeleteAll(m_Collection);
    m_Collection.clear();
    for(int i = 0; i < list.size(); i++)
    {
        if(!list[i].isFolder() && isFolder())
            continue;

        FileSystemRemoteItemModel * item = new FileSystemRemoteItemModel;
        item->setIsFolder(list[i].isFolder());
        item->setName(list[i].name());
        m_Collection.append(item);
    }
    beginResetModel();
    endResetModel();
}

int FileSystemRemote::rowCount(const QModelIndex &parent) const
{
    return m_Collection.size();
}

int FileSystemRemote::columnCount(const QModelIndex &parent) const
{
    return m_hashFieldName.count();
}

QVariant FileSystemRemote::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_Collection.size())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        switch(index.column())
        {
           case FIELD_FILENAME: return m_Collection[index.row()]->name();
           case FIELD_TYPE: {
                return m_Collection[index.row()]->isFolder() ? tr("folder") : tr("file");
           }
           default:{ return QVariant();}
        }
    }
    if (role == Qt::UserRole)
    {
        switch(index.column())
        {
           case FIELD_FILENAME: return m_Collection[index.row()]->isFolder();
           default:{ return QVariant();}
        }
    }
    else
        return QVariant();
}

QVariant FileSystemRemote::headerData(int section, Qt::Orientation orientation,
                                      int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return m_hashFieldName[section];
    else
        return QVariant();
}
