#ifndef FILESHORTINFOTABLEMODEL_H
#define FILESHORTINFOTABLEMODEL_H

#include <QAbstractTableModel>
#include <QString>
#include <QHash>

#include "Model/fileshortinfomodel.h"

class FileShortInfoTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:

    enum Fields {
        FIELD_FILENAME = 0,
        FIELD_FILETYPE,
        FIELD_ISBACKUP,
    };

    FileShortInfoTableModel(QObject *parent = 0);
    ~FileShortInfoTableModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void setCollection(QList<FileShortInfoModel*> files);
    QList<FileShortInfoModel*> collection() const;

private:
    bool isBooleanColumn( const QModelIndex &index ) const;

    void reset();

    void clearCollection();

private:
    QHash<int, QString> m_hashFieldName;

private:
    QList<FileShortInfoModel*> m_fileCollection;
    
};

#endif // FILESHORTINFOMODEL_H
