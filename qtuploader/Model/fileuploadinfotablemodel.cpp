#include "Model/fileuploadinfotablemodel.h"
#include "Model/fileuploadinfomodel.h"

#include <QColor>
#include <QBrush>
#include <QFont>

FileUploadInfoTableModel::FileUploadInfoTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    m_hashFieldName[FIELD_LOGIN]    = tr("Login");
    m_hashFieldName[FIELD_CHECKSUM] = "";
    m_hashFieldName[FIELD_TASK]     = tr("Task");
    m_hashFieldName[FIELD_ISBACKUP] = tr("Backup(B\\U)");
    m_hashFieldName[FIELD_INDEXED]  = tr("Indexed");
    m_hashFieldName[FIELD_FILENAME] = tr("Filename");
    m_hashFieldName[FIELD_ATTEMPTSREMAINING] = tr("Attempts");
    m_hashFieldName[FIELD_PROGRESS] = tr("Uploaded");
    m_hashFieldName[FIELD_STATUS]   = tr("Status");
    m_hashFieldName[FIELD_RESULT]   = tr("More");
    m_hashFieldName[FIELD_END_UPLOADING] = tr("Date of last status change");

    m_hashFieldName[FIELD_ISARCHIVE] = "";

    m_Model = NULL;
}

FileUploadInfoTableModel::~FileUploadInfoTableModel()
{
    clearCollection();
}

void FileUploadInfoTableModel::setModel(UploadProgressModel * model)
{
    m_Model = model;
}

void FileUploadInfoTableModel::reset()
{
    beginResetModel();
    endResetModel();
}

void FileUploadInfoTableModel::clearCollection()
{
    //qDeleteAll(m_fileCollection); - тут delete не нужно!
    //m_Collection.clear();
    if(m_Model == NULL)
        return;

    m_Model->clear();
}

int FileUploadInfoTableModel::rowCount(const QModelIndex &) const
{
    if(m_Model == NULL)
        return 0;
     //return m_Collection.keys().count();
    return m_Model->size();
}

int FileUploadInfoTableModel::columnCount(const QModelIndex &) const
{
     return m_hashFieldName.count();
}

bool FileUploadInfoTableModel::isBooleanColumn( const QModelIndex &index ) const
{
    return index.column() == FIELD_ISBACKUP || index.column() == FIELD_INDEXED;
}

Qt::ItemFlags FileUploadInfoTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags f = QAbstractTableModel::flags(index);

    if (index.column() == FIELD_ISBACKUP && m_Model != NULL)
    {
        auto keyForIndex = m_Model->keyForIndex(index);
        Q_CHECK_PTR(keyForIndex);
        if(keyForIndex)
        {
            if(((FileUploadInfoModel*)keyForIndex)->uploadStatus() == FileUploadInfoModel::Pending)
                f = f | Qt::ItemIsEditable;
        }
    }

    if (isBooleanColumn( index ))
        f = f | Qt::ItemIsUserCheckable;

    return f;
    /*if ( isBooleanColumn( index ) )
    {
        return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable |Qt::ItemIsEditable;
    }
    else
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;//QAbstractListModel::flags(index);
    }*/
}

QVariant FileUploadInfoTableModel::data(const QModelIndex &index, int role) const
{
     if (!index.isValid())
         return QVariant();

     if(m_Model == NULL)
         return QVariant();

     if (index.row() >= m_Model->size())
         return QVariant();

     auto keyForIndex = m_Model->keyForIndex(index);
     Q_CHECK_PTR(keyForIndex);
     if(!keyForIndex)
         return QVariant();

     FileUploadInfoModel * itemFileUpload = (FileUploadInfoModel*)keyForIndex;
     Q_CHECK_PTR(itemFileUpload);

     if (role == Qt::CheckStateRole)
     {
         if (isBooleanColumn( index ))
         {
             switch(index.column())
             {
                 case FIELD_ISBACKUP: return itemFileUpload->isBackup() ? Qt::Checked : Qt::Unchecked;
                 case FIELD_INDEXED:  return itemFileUpload->indexed() ? Qt::Checked : Qt::Unchecked;
                 default: return QVariant();
             }
         }
     }

     if (role == Qt::DisplayRole)
     {
         switch(index.column())
         {
            case FIELD_LOGIN:    return itemFileUpload->login();
            case FIELD_CHECKSUM: return itemFileUpload->checkSum();
            case FIELD_TASK:     return itemFileUpload->task();
            case FIELD_FILENAME: return itemFileUpload->fileName();
            case FIELD_ATTEMPTSREMAINING: return itemFileUpload->attemptsRemaining();
            case FIELD_STATUS:   return FileShortInfoModel::statusName(itemFileUpload->uploadStatus());
            case FIELD_RESULT:   return itemFileUpload->result();
            case FIELD_END_UPLOADING: return itemFileUpload->dateTime().toString("yyyy.MM.dd hh.mm");
            case FIELD_ISARCHIVE: return itemFileUpload->isArchive();
            default:
                return QVariant();
         }
     }
     if (role == Qt::UserRole)
     {
         switch(index.column())
         {
             case FIELD_PROGRESS: return itemFileUpload->progress();
             default:
                return QVariant();
         }
     }
     /*if (role == Qt::FontRole)
     {
         return QFont("Times New Roman", 11);
     }
     if (role == Qt::TextColorRole)
     {
         return QBrush(QColor(255, 0 ,0));
     }
     if (role == Qt::ForegroundRole)
     {
         return QColor(255, 0 ,0);
     }*/
     else
         return QVariant();
}

bool FileUploadInfoTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    if(m_Model == NULL)
        return false;

    if (role == Qt::DisplayRole)
    {
        switch(index.column())
        {
            case FIELD_ISBACKUP:
            {
                auto keyForIndex = m_Model->keyForIndex(index);
                Q_CHECK_PTR(keyForIndex);
                if(!keyForIndex)
                    return true;
                ((FileUploadInfoModel*)keyForIndex)->setIsBackup(value.toBool());
                emit dataChanged(index, index);
                return true;
            }
            default:
                return QAbstractTableModel::setData(index, value, role);
        }
    }
    else
        return QAbstractTableModel::setData(index, value, role);
}

QVariant FileUploadInfoTableModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return m_hashFieldName[section];
    else
        return QVariant();
}

void FileUploadInfoTableModel::resetItem(int indexNewItem)//const QList<FileShortInfoModel *> &files)
{
    if(indexNewItem == -1)
    {
        reset();//beginInsertRows();
    }
    else
    {
        //bool update_isbackup = (elem->isBackup() != itemUpload->isBackup());//if new value for - IsBackup
        for(int col = 0; col < m_hashFieldName.count(); col++) {
            if(indexNewItem < 0) break;
            //if cell is editing and value not change, then Editor not closing
            //if(col == FIELD_ISBACKUP && !update_isbackup) continue;

            QModelIndex ModelIndex = index(indexNewItem, col, QModelIndex());
            emit dataChanged(ModelIndex, ModelIndex);
        }
    }
}

void FileUploadInfoTableModel::setCollection(const QList<FileShortInfoModel *> &files)
{
    /*Старый вариант clearCollection();??

    QList<FileShortInfoModel*>::const_iterator it = files.constBegin();
    for(; it != files.constEnd(); ++it)
        m_fileCollection.append(*it);
        */

    //удаление элементов, которые не вошли в files
    /*QVector<uint> vHash(files.count());
    for(int i = 0; i < files.count(); i++)
        vHash[i] = qHash(((FileUploadInfoModel*)files[i])->checkSum());

    bool isRemove = false;
    for(int i = 0; i < m_Collection.keys().count(); i++)
    {
        auto item = m_Collection.keys()[i];
        if(!vHash.contains(m_Collection[item]))
        {
            m_Collection.remove(item);
            i--;
            isRemove = true;
        }
    }
    //если произошли удаления элементов
    if(isRemove) reset();


    //Изменение существующих элементов или добавление!
    for(const auto& item: files)
    {
        uint h = qHash(((FileUploadInfoModel*)item)->checkSum());
        if(!m_Collection.values().contains(h))
        {
            //m_Collection[new FileUploadInfoModel((const FileUploadInfoModel&)*item)] = h;
            m_Collection[item] = h;
            reset();
        }
        else
        {
            FileUploadInfoModel* elem = (FileUploadInfoModel*)m_Collection.key(h);
            int row = m_Collection.keys().indexOf(m_Collection.key(h));//current row
            FileUploadInfoModel* itemUpload = (FileUploadInfoModel*)item;
            elem->setProgress(itemUpload->progress());
            elem->setUploadStatus(itemUpload->uploadStatus());
            elem->setResult(itemUpload->result());
            elem->setFileName(itemUpload->fileName());
            elem->setTask(itemUpload->task());
            elem->setDateTime(itemUpload->dateTime());
            elem->setAttemptsRemaining(itemUpload->attemptsRemaining());
            elem->setIsArchive(itemUpload->isArchive());
            bool update_isbackup = (elem->isBackup() != itemUpload->isBackup());//if new value for - IsBackup
            elem->setIsBackup(itemUpload->isBackup());
            //update all column(for current row)
            for(int col = 0; col < m_hashFieldName.count(); col++) {
                if(row < 0) break;
                //if cell is editing and value not change, then Editor not closing
                if(col == FIELD_ISBACKUP && !update_isbackup) continue;

                QModelIndex ModelIndex = index(row, col, QModelIndex());
                emit dataChanged(ModelIndex, ModelIndex);
            }
        }
    }*/
}

QList<FileShortInfoModel*> FileUploadInfoTableModel::collection() const
{
    return QList<FileShortInfoModel*>();
    //return m_Collection.keys();
}



