#ifndef FILEINFOUPLOADTABLEMODEL_H
#define FILEINFOUPLOADTABLEMODEL_H

#include <QAbstractTableModel>
#include <QString>
#include <QHash>

#include "Model/fileshortinfomodel.h"
#include "Model/fileuploadinfomodel.h"
#include "Model/uploadprogressmodel.h"

class FileUploadInfoTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Fields {
        FIELD_LOGIN = 0,
        FIELD_CHECKSUM,
        FIELD_TASK,
        FIELD_ISBACKUP,
        FIELD_INDEXED,
        FIELD_FILENAME,
        FIELD_ATTEMPTSREMAINING,
        FIELD_PROGRESS,
        FIELD_STATUS,
        FIELD_RESULT,
        FIELD_END_UPLOADING,

        FIELD_ISARCHIVE
    };

    explicit FileUploadInfoTableModel(QObject *parent = 0);
    ~FileUploadInfoTableModel();

    void setModel(UploadProgressModel *);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void setCollection(const QList<FileShortInfoModel*> &files);
    QList<FileShortInfoModel*> collection() const;

    void resetItem(int);

private:
    bool isBooleanColumn( const QModelIndex &index ) const;

    void clearCollection();

    void reset();

    //FileUploadInfoModel* keyForIndex(const QModelIndex& index) const;

private:
    QHash<int, QString> m_hashFieldName;

signals:

public slots:

private:
    //QList<FileUploadInfoModel*> m_fileCollection;
    //QList<FileShortInfoModel*> m_fileCollection;
    //mutable QHash<FileShortInfoModel*, uint> m_Collection;
    mutable UploadProgressModel * m_Model;
};

#endif // FILEINFOUPLOADMODEL_H
