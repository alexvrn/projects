#include "fileshortinfodelegate.h"
#include "Model/fileshortinfomodel.h"
#include "Model/fileshortinfotablemodel.h"

#include <QComboBox>
#include <QCheckBox>
#include <QPainter>
#include <QApplication>
#include <QStyledItemDelegate>

FileShortInfoDelegate::FileShortInfoDelegate(QObject * parent)
    : QStyledItemDelegate(parent)
{
    m_ListTypeName[0] = FileShortInfoModel::VideoTrack;
    m_ListTypeName[1] = FileShortInfoModel::RoadNotes;
    m_ListTypeName[2] = FileShortInfoModel::Filter;
    m_ListTypeName[3] = FileShortInfoModel::Other;
}

QWidget *FileShortInfoDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                      const QModelIndex &index) const
{
    switch(index.column())
    {
        case FileShortInfoTableModel::FIELD_FILETYPE:
        {
            QComboBox *cbType = new QComboBox(parent);
            cbType->clear();
            //foreach (int index, m_ListTypeName.keys())
            cbType->insertItem(0, FileShortInfoModel::typeName(m_ListTypeName[0]));
            cbType->insertItem(1, FileShortInfoModel::typeName(m_ListTypeName[1]));
            cbType->insertItem(2, FileShortInfoModel::typeName(m_ListTypeName[2]));
            cbType->insertItem(3, FileShortInfoModel::typeName(m_ListTypeName[3]));

            return cbType;
        }
        case FileShortInfoTableModel::FIELD_ISBACKUP:
        {
            QCheckBox * editor = new QCheckBox(parent);
            connect(editor, SIGNAL(editingFinished()),
                        this, SLOT(commitAndCloseEditor()));
            return editor;
        }
        default:
            return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void FileShortInfoDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    switch(index.column())
    {
        case FileShortInfoTableModel::FIELD_FILETYPE:
        {
            QString value = index.model()->data(index, Qt::DisplayRole).toString();
            QComboBox *cbType = static_cast<QComboBox*>(editor);
            cbType->setCurrentIndex(cbType->findText(value));
            break;
        }
        case FileShortInfoTableModel::FIELD_ISBACKUP:
        {
            bool value = index.model()->data(index, Qt::CheckStateRole).toBool();
            QCheckBox *chBackup = static_cast<QCheckBox*>(editor);
            chBackup->setChecked(value);
            break;
        }
        default:
            QStyledItemDelegate::setEditorData(editor, index);
    }
}

void FileShortInfoDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                  const QModelIndex &index) const
{
    switch(index.column())
    {
        case FileShortInfoTableModel::FIELD_FILETYPE:
        {
            QComboBox *cbType = static_cast<QComboBox*>(editor);
            int value = cbType->currentIndex();
            model->setData(index, value, Qt::DisplayRole);
            break;
        }
        case FileShortInfoTableModel::FIELD_ISBACKUP:
        {
            QCheckBox *chBackup = static_cast<QCheckBox*>(editor);
            model->setData(index, chBackup->isChecked(), Qt::DisplayRole);
            break;
        }
        default:
            QStyledItemDelegate::setModelData(editor, model, index);
    }
}

void FileShortInfoDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    switch(index.column())
    {
        case FileShortInfoTableModel::FIELD_FILETYPE:
        {
            QRect r = option.rect;
            r.setLeft( r.left() + 2 );
            r.setTop( r.top() + 2 );
            r.setRight( r.right() - 2 );
            r.setBottom( r.bottom() - 2 );
            editor->setGeometry( r );
            break;
        }
        case FileShortInfoTableModel::FIELD_ISBACKUP:
        {
            QRect r = option.rect;
            r.setX( r.x() + 3 ); // just for showing the overlay
            //r.setSize( editor->sizeHint() );
            editor->setGeometry( r );
            break;
        }
        default:
            editor->setGeometry(option.rect);
    }
}

// painting
void FileShortInfoDelegate::paint(QPainter *painter,
           const QStyleOptionViewItem &option,
           const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    //Отрисовка линий
    int px = option.rect.x();
    int py = option.rect.y();
    int ph = option.rect.height();
    int pw = option.rect.width();
    const int gridHeight = 2;
    const QColor gridColor = QColor(50, 50, 50);

    painter->save();
    painter->setPen(gridColor);//option.palette.color(QPalette::Dark));
    painter->setBrush(QBrush(gridColor));
    painter->drawRect(px, py + ph - gridHeight, pw, gridHeight);
    painter->restore();
}

void FileShortInfoDelegate::commitAndCloseEditor()
{
    QWidget *editor = qobject_cast<QWidget *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}
