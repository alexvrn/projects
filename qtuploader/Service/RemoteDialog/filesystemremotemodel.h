#ifndef FILESYSTEMREMOTEMODEL_H
#define FILESYSTEMREMOTEMODEL_H

#include <QObject>
#include <QString>
#include <QAbstractListModel>
#include <QAbstractTableModel>

class FileSystemRemoteItemModel
{
public:
    void setName(const QString&s){m_strName = s;}
    QString name() const{return m_strName;}

    void setIsFolder(bool folder){m_bIsFolder = folder;}
    int isFolder() const{return m_bIsFolder;}

private:
    QString m_strName;
    bool m_bIsFolder;
};

class FileSystemRemote : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Fields {
        FIELD_FILENAME = 0,
        FIELD_TYPE,
    };

    explicit FileSystemRemote(bool isfolder = false, QObject *parent = 0);

    bool isFolder() const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const;
    //int rowCount(const QModelIndex &parent = QModelIndex()) const;
    //int columnCount(const QModelIndex &parent = QModelIndex()) const;
    void getData(const QList<FileSystemRemoteItemModel>&);

private:
    void setIsFolder(bool);

private:
    QHash<int, QString> m_hashFieldName;

    QList<FileSystemRemoteItemModel*> m_Collection;
    QString currentPath;
    //FileSystemRemoteModel m_Model;
    bool m_bIsFolder;
};

#endif // FILESYSTEMREMOTEMODEL_H
