#include "View/uploadprogressview.h"
#include "Model/fileuploadinfodelegate.h"
#include "Model/fileuploadinfotablemodel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QUiLoader>
#include <QFile>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QAction>
#include <QMenu>

#include "View/splitbutton.h"

UploadProgressView::UploadProgressView(QWidget *parent) :
    QWidget(parent), IViewData("UploadProgress", parent)
{
    form = Q_NULLPTR;
}

UploadProgressView::~UploadProgressView()
{
    //if(!form)
    //  delete form;
}

bool UploadProgressView::initControls()
{
    if(form)
        return true;

    try
    {
        if((form = loadUiFile()) == Q_NULLPTR)
        {
            //throw QString("Error loading UploadProgress");
            return false;
        }

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(form);
        setLayout(layout);

        model = new FileUploadInfoTableModel();
        Q_CHECK_PTR(model);

        tableViewUpload = form->findChild<QTableView*>("tableViewUpload");
        Q_CHECK_PTR(tableViewUpload);
        QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
        proxyModel->setDynamicSortFilter(false);
        proxyModel->setSourceModel(model);
        tableViewUpload->setModel(proxyModel);

        FileUploadInfoDelegate * delegateUpload = new FileUploadInfoDelegate();
        Q_CHECK_PTR(delegateUpload);
        tableViewUpload->setItemDelegate(delegateUpload);
        tableViewUpload->setEditTriggers(QAbstractItemView::CurrentChanged);
        tableViewUpload->setWordWrap(false);//отмена переноса строки на новую строку
        tableViewUpload->setColumnWidth(FileUploadInfoTableModel::FIELD_RESULT, 300);
        tableViewUpload->setContextMenuPolicy(Qt::CustomContextMenu);
        tableViewUpload->setColumnHidden(FileUploadInfoTableModel::FIELD_CHECKSUM, true);
        connect(tableViewUpload, SIGNAL(customContextMenuRequested(const QPoint &)),
                    SLOT(customMenuRequested(const QPoint &)));


        pushButtonStatistic = form->findChild<QPushButton*>("pushButtonStatistic");
        Q_CHECK_PTR(pushButtonStatistic);
        connect(pushButtonStatistic, SIGNAL(clicked(bool)), this, SLOT(on_statistic_clicked(bool)));
        pushButtonStatistic->setText(tr("Stats"));

        pushButtonPaused = form->findChild<QPushButton*>("pushButtonPaused");
        Q_CHECK_PTR(pushButtonPaused);
        connect(pushButtonPaused, SIGNAL(clicked(bool)), this, SLOT(on_paused_clicked(bool)));
        pushButtonPaused->setText(tr("Pause"));

        pushButtonArchive = form->findChild<QPushButton*>("pushButtonArchive");
        Q_CHECK_PTR(pushButtonArchive);
        connect(pushButtonArchive, SIGNAL(clicked(bool)), this, SLOT(on_archive_clicked(bool)));
        pushButtonArchive->setText(tr("Archive"));

        pushButtonRepeat = form->findChild<QPushButton*>("pushButtonRepeat");
        Q_CHECK_PTR(pushButtonRepeat);
        connect(pushButtonRepeat, SIGNAL(clicked(bool)), this, SLOT(on_repeat_clicked(bool)));
        pushButtonRepeat->setText(tr("Repeat"));

        checkBoxArchive = form->findChild<QCheckBox*>("checkBoxArchive");
        Q_CHECK_PTR(checkBoxArchive);
        connect(checkBoxArchive, SIGNAL(clicked(bool)), this, SLOT(on_getarchive_clicked(bool)));
        checkBoxArchive->setText(tr("Archive"));

        checkBoxUploading = form->findChild<QCheckBox*>("checkBoxUploading");
        Q_CHECK_PTR(checkBoxUploading);
        connect(checkBoxUploading, SIGNAL(clicked(bool)), this, SLOT(on_isuploading_clicked(bool)));
        checkBoxUploading->setText(tr("Pending/Uploading"));

        checkBoxError = form->findChild<QCheckBox*>("checkBoxError");
        Q_CHECK_PTR(checkBoxError);
        connect(checkBoxError, SIGNAL(clicked(bool)), this, SLOT(on_iserror_clicked(bool)));
        checkBoxError->setText(tr("Error"));

        checkBoxFinished = form->findChild<QCheckBox*>("checkBoxFinished");
        Q_CHECK_PTR(checkBoxFinished);
        connect(checkBoxFinished, SIGNAL(clicked(bool)), this, SLOT(on_isfinished_clicked(bool)));
        checkBoxFinished->setText(tr("Finished"));

        checkBoxCopying = form->findChild<QCheckBox*>("checkBoxCopying");
        Q_CHECK_PTR(checkBoxCopying);
        connect(checkBoxCopying, SIGNAL(clicked(bool)), this, SLOT(on_iscopying_clicked(bool)));
        checkBoxCopying->setText(tr("Copying"));

        createMenu();

        return true;
    }
    catch(...)
    {
        return false;
    }
}

void UploadProgressView::initTitleControls()
{
    pushButtonStatistic->setText(tr("Stats"));
}

void UploadProgressView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        initTitleControls();
    }
    else
        QWidget::changeEvent(event);
}

void UploadProgressView::createMenu()
{
    menu = new QMenu(this);
    Q_CHECK_PTR(menu);

    actCancel = new QAction(tr("Cancel the download"), this);
    Q_CHECK_PTR(actCancel);
    menu->addAction(actCancel);
    connect(actCancel, SIGNAL(triggered()), this, SLOT(sltCancelDownload()));
}

void UploadProgressView::resetItem(int index)
{
    model->resetItem(index);
}

void UploadProgressView::setCollection(const QList<FileShortInfoModel*> &files)
{
    model->setCollection(files);

    //QStringList d;
    //d.append("user1");
    //d.append("user2");
    //d.append("user3");
    //((SplitButton*)pushButtonStatistic)->setUsers(d);
}

void UploadProgressView::setModel(UploadProgressModel *_model)
{
    model->setModel(_model);
}

QList<FileShortInfoModel*> UploadProgressView::collection() const
{
    return QList<FileShortInfoModel*>();// return model->collection();
}

void UploadProgressView::setShowUploading(bool check)
{
    checkBoxUploading->setChecked(check);
}

bool UploadProgressView::showUploading() const
{
    return checkBoxUploading->isChecked();
}

void UploadProgressView::setShowCopying(bool check)
{
    checkBoxCopying->setChecked(check);
}

bool UploadProgressView::showCopying() const
{
    return checkBoxCopying->isChecked();
}

void UploadProgressView::setShowArchive(bool check)
{
    checkBoxArchive->setChecked(check);
}

bool UploadProgressView::showArchive() const
{
    return checkBoxArchive->isChecked();
}

void UploadProgressView::setShowError(bool check)
{
    checkBoxError->setChecked(check);
}

bool UploadProgressView::showError() const
{
    return checkBoxError->isChecked();
}

void UploadProgressView::setShowOK(bool check)
{
    checkBoxFinished->setChecked(check);
}

bool UploadProgressView::showOK() const
{
    return checkBoxFinished->isChecked();
}

void UploadProgressView::setVisiblePause(bool v)
{
    pushButtonPaused->setVisible(v);
}

bool UploadProgressView::visiblePause() const
{
    return pushButtonPaused->isVisible();
}

void UploadProgressView::setEnabledRepeat(bool e)
{
    pushButtonRepeat->setEnabled(e);
}

void UploadProgressView::clear()
{
    //setCollection(QList<FileShortInfoModel*>());
}

void UploadProgressView::sltChangeIsBackup(const QString& checksum, bool isbackup)
{
    //QMessageBox::information(0, "", checksum);
    emit changeIsBackup(checksum, isbackup);
}

void UploadProgressView::on_paused_clicked(bool)
{
    emit pausedCommand();
}

void UploadProgressView::on_statistic_clicked(bool)
{
    //QMessageBox::information(0, "", "stat");
    emit sendInformationMail();
}

void UploadProgressView::on_archive_clicked(bool)
{
    QMessageBox msgBox;
    msgBox.setText("Архив");
    msgBox.setInformativeText("Удалить исходные файлы?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    bool bDeleteSourse = false;
    switch(msgBox.exec())
    {
        case QMessageBox::Yes:
            bDeleteSourse = true;
            break;
        case QMessageBox::No:
            bDeleteSourse = false;
            break;
        default: return;
    }
    emit archiveCommand(bDeleteSourse);
}

void UploadProgressView::on_repeat_clicked(bool)
{
    //for(int i = 0; i < tableViewUpload->model()->rowCount(); i++)
    //    tableViewUpload->model()->it
    emit repeatCommand();
}

void UploadProgressView::on_getarchive_clicked(bool check)
{
    emit getArchiveCommand(check);
}

void UploadProgressView::on_iserror_clicked(bool check)
{
    emit showErrorCommand(check);
}

void UploadProgressView::on_isfinished_clicked(bool check)
{
    emit showFinishedCommand(check);
}

void UploadProgressView::on_iscopying_clicked(bool check)
{
    emit showCopyingCommand(check);
}

void UploadProgressView::on_isuploading_clicked(bool check)
{
    emit showUploadingCommand(check);
}

void UploadProgressView::setNameOnPause(const QString& name)
{
    pushButtonPaused->setText(name);
}

void UploadProgressView::setAdmin(bool admin)
{
    //эти поля ВСЕГДА скрыты
    tableViewUpload->setColumnHidden(FileUploadInfoTableModel::FIELD_CHECKSUM, true);
    tableViewUpload->setColumnHidden(FileUploadInfoTableModel::FIELD_ISARCHIVE, true);

    tableViewUpload->setColumnHidden(FileUploadInfoTableModel::FIELD_LOGIN, !admin);
    tableViewUpload->setColumnHidden(FileUploadInfoTableModel::FIELD_INDEXED, !admin);
}

void UploadProgressView::customMenuRequested(const QPoint& pos)
{
    QModelIndex index = tableViewUpload->indexAt(pos);
    if(index.row() < 0)
        return;

    //костыль
    //m_selectModelIndexList = tableViewUpload->selectionModel()->selectedRows(FileUploadInfoTableModel::FIELD_CHECKSUM);

    QString status = index.sibling(index.row(), FileUploadInfoTableModel::FIELD_STATUS).data(Qt::DisplayRole).toString();

    //в зависимости от статуса меняется доступность пунктов меню
    actCancel->setDisabled(
                status != FileShortInfoModel::statusName(FileShortInfoModel::Pending));

    menu->popup(tableViewUpload->viewport()->mapToGlobal(pos));
}

void UploadProgressView::sltCancelDownload()
{
    //emit cancelDownload(m_selectModelIndexList);//tableViewUpload->selectionModel()->selectedRows(FileUploadInfoTableModel::FIELD_CHECKSUM));
    emit cancelDownload(tableViewUpload->selectionModel()->selectedRows(FileUploadInfoTableModel::FIELD_CHECKSUM));
}
