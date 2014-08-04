#ifndef FILEUPLOADINFODELEGATE_H
#define FILEUPLOADINFODELEGATE_H

//#include <QItemDelegate>
#include <QStyledItemDelegate>

class FileUploadInfoDelegate : public QStyledItemDelegate//QItemDelegate
{
    Q_OBJECT

public:
    FileUploadInfoDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;

    // painting
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

signals:
    void changeIsBackup(const QString&, bool) const;

private:
    void paintBase(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
};

#endif // FILEUPLOADINFODELEGATE_H
