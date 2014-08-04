#ifndef FILESHORTINFODELEGATE_H
#define FILESHORTINFODELEGATE_H

#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QHash>

#include "Model/fileshortinfomodel.h"

class FileShortInfoDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    FileShortInfoDelegate(QObject *parent = 0);

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
private slots:
    void commitAndCloseEditor();

signals:
    void dataChange(const QModelIndex &index) const;

private:
    QHash<int, FileShortInfoModel::FileTypeModel> m_ListTypeName;
};

#endif // FILESHORTINFODELEGATE_H
