#include "Model/fileshortinfotablemodel.h"

#include <QMessageBox>
#include <QStandardItem>

FileShortInfoTableModel::FileShortInfoTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{       
    m_hashFieldName[FIELD_FILENAME] = tr("File");
    m_hashFieldName[FIELD_FILETYPE] = tr("Type");
    m_hashFieldName[FIELD_ISBACKUP] = tr("Backup (B\\U)");
}

FileShortInfoTableModel::~FileShortInfoTableModel()
{
    clearCollection();
}

void FileShortInfoTableModel::reset()
{
    beginResetModel();
    endResetModel();
}

void FileShortInfoTableModel::clearCollection()
{
    //qDeleteAll(m_fileCollection); - тут delete не нужно!
    m_fileCollection.clear();
}

int FileShortInfoTableModel::rowCount(const QModelIndex &) const
{
     return m_fileCollection.count();
}

int FileShortInfoTableModel::columnCount(const QModelIndex &) const
{
     return m_hashFieldName.count();
}

bool FileShortInfoTableModel::isBooleanColumn( const QModelIndex &index ) const
{
    return index.column() == FIELD_ISBACKUP;
}

Qt::ItemFlags FileShortInfoTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags f = QAbstractTableModel::flags(index);
    if (index.column() == FIELD_ISBACKUP)
        f = f | Qt::ItemIsEditable;

     if (isBooleanColumn( index ))
        f = f | Qt::ItemIsUserCheckable;

    return f;
}

QVariant FileShortInfoTableModel::data(const QModelIndex &index, int role) const
{
     if (!index.isValid())
         return QVariant();

     if (index.row() >= m_fileCollection.size())
         return QVariant();

     if (role == Qt::CheckStateRole)
     {
         if (isBooleanColumn( index ))
         {
             switch(index.column())
             {
                 case FIELD_ISBACKUP: return m_fileCollection[index.row()]->isBackup() ? Qt::Checked : Qt::Unchecked;
                 default:{return QVariant();}
             }
         }
     }

     if (role == Qt::DisplayRole)
     {
         switch(index.column())
         {
            case FIELD_FILENAME: return m_fileCollection[index.row()]->fileName();
            case FIELD_FILETYPE: return FileShortInfoModel::typeName(m_fileCollection[index.row()]->fileType());
            default:{ return QVariant();}
         }
     }
     else
         return QVariant();
}

bool FileShortInfoTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    if (role == Qt::DisplayRole)
    {
        switch(index.column())
        {
            case FIELD_FILETYPE:
            {
                bool isVal = false;
                FileShortInfoModel::FileTypeModel file_type = (FileShortInfoModel::FileTypeModel)value.toInt(&isVal);
                if(!isVal)
                    file_type = FileShortInfoModel::Other;
                m_fileCollection[index.row()]->setFileType(file_type);
                emit dataChanged(index, index);
                return true;
            }
            case FIELD_ISBACKUP:
            {
                m_fileCollection[index.row()]->setIsBackup(value.toBool());
                emit dataChanged(index, index);
                return true;
            }
            default:
            {
                return QAbstractTableModel::setData(index, value, role);
            }
        }
    }
    else
        return QAbstractTableModel::setData(index, value, role);
}

QVariant FileShortInfoTableModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const
{
    switch(role)
    {
        case Qt::DisplayRole:
        {
            if (orientation == Qt::Horizontal)
                return m_hashFieldName[section];
            else return QVariant();
            break;
        }
        default: return QVariant();
    }
}

void FileShortInfoTableModel::setCollection(QList<FileShortInfoModel *> files)
{
    clearCollection();

    QList<FileShortInfoModel*>::const_iterator it = files.constBegin();
    for(; it != files.constEnd(); ++it)
        m_fileCollection.append(*it);

    reset();
}

QList<FileShortInfoModel*> FileShortInfoTableModel::collection() const
{
    return m_fileCollection;
}
