#include "fileuploadinfodelegate.h"

#include <QApplication>
#include <QProgressBar>
#include <QCheckBox>
#include <QPainter>
#include <QColor>

#include "Model/fileuploadinfotablemodel.h"

FileUploadInfoDelegate::FileUploadInfoDelegate(QObject * parent)
    : QStyledItemDelegate(parent)//QItemDelegate(parent)
{
}

QWidget *FileUploadInfoDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                      const QModelIndex &index) const
{
    switch(index.column())
    {
        case FileUploadInfoTableModel::FIELD_ISBACKUP:
            return new QCheckBox(parent);
        default:
            return QStyledItemDelegate::createEditor(parent, option, index);

    }
}

void FileUploadInfoDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    switch(index.column())
    {
        case FileUploadInfoTableModel::FIELD_ISBACKUP:
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

void FileUploadInfoDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                  const QModelIndex &index) const
{
    switch(index.column())
    {
        case FileUploadInfoTableModel::FIELD_ISBACKUP:
        {
            QCheckBox *chBackup = static_cast<QCheckBox*>(editor);
            model->setData(index, chBackup->isChecked(), Qt::DisplayRole);
            QString checksum = index.sibling(index.row(), FileUploadInfoTableModel::FIELD_CHECKSUM).data(Qt::DisplayRole).toString();
            emit changeIsBackup(checksum, chBackup->isChecked());
            break;
        }
        default:
            QStyledItemDelegate::setModelData(editor, model, index);
    }
}

void FileUploadInfoDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    switch(index.column())
    {
        case FileUploadInfoTableModel::FIELD_ISBACKUP:
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

void FileUploadInfoDelegate::paintBase(QPainter *painter,
           const QStyleOptionViewItem &option,
           const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    QStyledItemDelegate::paint(painter, option, index);
    painter->restore();
}

// painting
void FileUploadInfoDelegate::paint(QPainter *painter,
           const QStyleOptionViewItem &option,
           const QModelIndex &index) const
{
    const int gridHeight = 2;

    //painter->save();
    //painter->setRenderHint(QPainter::Antialiasing);
    //QStyledItemDelegate::paint(painter, option, index);
    //painter->restore();
    // QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    bool isArchive = index.sibling(index.row(), FileUploadInfoTableModel::FIELD_ISARCHIVE).data(Qt::DisplayRole).toBool();

    if( isArchive && //если данные в архиве(строка)
       index.column() != FileUploadInfoTableModel::FIELD_PROGRESS &&
       index.column() != FileUploadInfoTableModel::FIELD_INDEXED &&
       index.column() != FileUploadInfoTableModel::FIELD_ISBACKUP)
    {
        //QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);
        //const QStyleOptionViewItemV4 * optionItem = qstyleoption_cast<const QStyleOptionViewItemV4 *>(&option);

        //if( !optionItem )
        //{
            //Если по каким-то причинам не получилось преобразовать, рисуем элемент по умолчанию
            //QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);
            //return;
        //}

        //Получаем размер области, на которой будет отображаться текст
        QRect textRect = option.rect;//subElementRect(SE_ItemViewItemText, option, w);
        //Сдвигаем немного, чтобы текст не "прилипал" к краям ячейки
        textRect.setX( textRect.x() + 5 );
        textRect.setY( textRect.y() + 5 );
        //if( !optionItem->text.isEmpty() )
        {
            //Рисуем текст.
            QPen pen;
            QBrush brush;
            QLinearGradient linearGrad(option.rect.x(), option.rect.y(),
                                       option.rect.x(), option.rect.y() + option.rect.height());
            QColor colorCopyDark(110, 110, 110);
            QColor colorCopyLight(250, 250, 250);
            linearGrad.setColorAt(0.0, colorCopyLight);
            linearGrad.setColorAt(1.0, colorCopyDark);
            QBrush selectionBrush(linearGrad);
            if (option.state & QStyle::State_Selected)//if row is select
            {
                brush = selectionBrush;
                pen.setColor(QColor(40, 40, 40));
            }
            else
            {
                brush.setStyle(Qt::SolidPattern);
                brush.setColor(QColor(70, 70, 70));
                pen.setColor(QColor(140, 140, 140));
            }

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            pen.setStyle(Qt::NoPen);
            painter->setPen(pen);
            painter->setBrush(brush);
            //Background
            painter->drawRoundedRect(option.rect, 1, 1);
            pen.setStyle(Qt::SolidLine);
            painter->setPen(pen);
            //Text
            painter->drawText(textRect, Qt::TextSingleLine, index.data(Qt::DisplayRole).toString());//option.text);
            painter->restore();
        }
    }
    else
    if(index.column() == FileUploadInfoTableModel::FIELD_PROGRESS)
    {
        paintBase(painter, option, index);

        bool isVal = false;
        int progress = index.data(Qt::UserRole).toInt(&isVal);
        if(!isVal)
            progress = 0;

        QStyleOptionProgressBar progressBarOption;
        progressBarOption.state = option.state;

        const int shiftProgress = 4;
        progressBarOption.rect = option.rect;
        progressBarOption.rect.setLeft(option.rect.left() + shiftProgress);
        progressBarOption.rect.setTop(option.rect.top() + shiftProgress);
        progressBarOption.rect.setRight(option.rect.right() - shiftProgress);
        progressBarOption.rect.setBottom(option.rect.bottom() - shiftProgress - gridHeight);

        progressBarOption.direction = option.direction;
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.progress = progress;
        progressBarOption.textVisible = true;
        progressBarOption.textAlignment = Qt::AlignHCenter;

        QProgressBar bar;
        painter->save();
        QApplication::style()->drawControl(QStyle::CE_ProgressBar,
                                  &progressBarOption, painter, &bar);
        painter->restore();
        //Для отображения другим цветом при промежуточном копировании
        if(FileShortInfoModel::statusName(FileShortInfoModel::Copying) ==
                index.sibling(index.row(), FileUploadInfoTableModel::FIELD_STATUS).data(Qt::DisplayRole).toString())
        {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);

            QRect rt = progressBarOption.rect;
            rt.setWidth(rt.width() * (progress / 100.0));

            QLinearGradient linearGrad(
                        progressBarOption.rect.x(), progressBarOption.rect.y(),
                        progressBarOption.rect.x() + progressBarOption.rect.width(), progressBarOption.rect.y());//rt.x(), rt.y(), rt.x() + rt.width(), rt.y());
            QColor colorCopyDark(170, 170, 170);
            QColor colorCopyLight(255, 255, 255);
            linearGrad.setColorAt(0.0, colorCopyDark);
            linearGrad.setColorAt(0.5, colorCopyDark);
            linearGrad.setColorAt(0.8, colorCopyLight);
            linearGrad.setColorAt(1.0, colorCopyLight);

            QBrush selectionBrush(linearGrad);
            painter->setBrush(selectionBrush);
            painter->drawRoundedRect(rt, 1, 1);
            painter->restore();
        }
    }
    else
    {
        paintBase(painter, option, index);
    }

    //Отрисовка линий между строками
    int px = option.rect.x();
    int py = option.rect.y();
    int ph = option.rect.height();
    int pw = option.rect.width();
    const QColor gridColor = QColor(50, 50, 50);

    painter->save();
    painter->setPen(gridColor);//option.palette.color(QPalette::Dark));
    painter->setBrush(QBrush(gridColor));
    painter->drawRect(px, py + ph - gridHeight, pw, gridHeight);
    painter->restore();
}
