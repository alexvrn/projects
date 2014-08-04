#include "addfilespanelview.h"
#include "Model/fileshortinfomodel.h"
#include "Model/fileshortinfodelegate.h"
#include "Model/fileshortinfotablemodel.h"

#include <QMessageBox>
#include <QUiLoader>
#include <QFile>
#include <QVBoxLayout>
#include <QTableView>
#include <QPushButton>
#include <QToolButton>
#include <QProgressBar>
#include <QLabel>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QHeaderView>
#include <QDebug>

AddFilesPanelView::AddFilesPanelView(QWidget *parent) :
    QWidget(parent), IViewData("AddFilesPanel", parent)//, ui(new Ui::AddFilesPanel)
{
    form = Q_NULLPTR;
}

AddFilesPanelView::~AddFilesPanelView()
{
    if(!m_pModel)
        delete m_pModel;

    if(!menu)
        delete menu;//if (!menu->parent()) delete menu;

    if(!form)
        delete form;
}

bool AddFilesPanelView::initControls()
{
    if(form)
        return true;

    try
    {
        if((form = loadUiFile()) == Q_NULLPTR)
        {
            //throw QString("Error loading AddFilesPanel");
            return false;
        }

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(form);
        setLayout(layout);

        m_pDlgService = new DialogService();

        m_pModel = new FileShortInfoTableModel();

        tableViewFileInfo = form->findChild<QTableView*>("tableViewFileInfo");
        Q_CHECK_PTR(tableViewFileInfo);
        QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
        proxyModel->setSourceModel(m_pModel);
        tableViewFileInfo->setModel(proxyModel);

        FileShortInfoDelegate* infoDelegate = new FileShortInfoDelegate();
        Q_CHECK_PTR(infoDelegate);
        tableViewFileInfo->setItemDelegate(infoDelegate);
        tableViewFileInfo->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tableViewFileInfo, SIGNAL(customContextMenuRequested(const QPoint &)),
                    SLOT(customMenuRequested(const QPoint &)));

        labelTask = form->findChild<QLabel*>("labelTask");
        Q_CHECK_PTR(labelTask);
        labelTask->setText(tr("Task:"));

        labelFiles = form->findChild<QLabel*>("labelFiles");
        Q_CHECK_PTR(labelFiles);
        labelFiles->setText(tr("Files:"));

        pushButtonAddFiles = form->findChild<QPushButton*>("addFileButton");
        Q_CHECK_PTR(pushButtonAddFiles);
        connect(pushButtonAddFiles, SIGNAL(clicked(bool)), this, SLOT(on_addFileButton_clicked(bool)));
        pushButtonAddFiles->setText(tr("Add files"));

        pushButtonAddFolder = form->findChild<QPushButton*>("addFolderButton");
        Q_CHECK_PTR(pushButtonAddFolder);
        connect(pushButtonAddFolder, SIGNAL(clicked(bool)), this, SLOT(on_addFolderButton_clicked(bool)));
        pushButtonAddFolder->setText(tr("Add folder"));

        uploadButton = form->findChild<QPushButton*>("uploadButton");
        Q_CHECK_PTR(uploadButton);
        connect(uploadButton, SIGNAL(clicked(bool)), this, SLOT(on_uploadButton_clicked(bool)));
        uploadButton->setText(tr("Upload"));
        uploadButton->setEnabled(false);

        pushButtonLogout = form->findChild<QPushButton*>("pushButtonLogout");
        Q_CHECK_PTR(pushButtonLogout);
        connect(pushButtonLogout, SIGNAL(clicked(bool)), this, SLOT(on_logout_clicked(bool)));
        pushButtonLogout->hide();

        labelVersion = form->findChild<QLabel*>("labelVersion");
        Q_CHECK_PTR(labelVersion);

        lineEditTask = form->findChild<QLineEdit*>("lineEditTask");
        Q_CHECK_PTR(lineEditTask);
        connect(lineEditTask, SIGNAL(textEdited(const QString &)), this, SLOT(onTaskEdit(const QString &)));


        checkBoxIndexFiles = form->findChild<QCheckBox*>("checkBoxIndexFiles");
        Q_CHECK_PTR(checkBoxIndexFiles);
        connect(checkBoxIndexFiles, SIGNAL(clicked(bool)), this, SLOT(on_isindexed_clicked(bool)));
        checkBoxIndexFiles->setText(tr("Loading indexed files"));

        checkBoxIntermediateCopy = form->findChild<QCheckBox*>("checkBoxIntermediateCopy");
        Q_CHECK_PTR(checkBoxIntermediateCopy);
        connect(checkBoxIntermediateCopy, SIGNAL(clicked(bool)), this, SLOT(on_isintermediatecopy_clicked(bool)));
        checkBoxIntermediateCopy->setText(tr("Make intermediate copying"));

        checkBoxDeleteSource = form->findChild<QCheckBox*>("checkBoxDeleteSource");
        Q_CHECK_PTR(checkBoxDeleteSource);
        connect(checkBoxDeleteSource, SIGNAL(clicked(bool)), this, SLOT(on_isdeletesource_clicked(bool)));
        checkBoxDeleteSource->setText(tr("Remove the source"));

        tableViewFileInfo->setColumnWidth(FileShortInfoTableModel::FIELD_FILENAME, 300);
        tableViewFileInfo->setColumnWidth(FileShortInfoTableModel::FIELD_FILETYPE, 90);
        tableViewFileInfo->setColumnWidth(FileShortInfoTableModel::FIELD_ISBACKUP, 140);
        tableViewFileInfo->setEditTriggers(QAbstractItemView::CurrentChanged);
        tableViewFileInfo->setSelectionMode(QAbstractItemView::ExtendedSelection);
        tableViewFileInfo->setWordWrap(false);//отмена переноса строки на новую строку

        createMenu();

        return true;
    }
    catch(...)
    {
        return false;
    }
}

void AddFilesPanelView::initTitleControls()
{
    labelTask->setText(tr("Task:"));

    labelFiles->setText(tr("Files:"));

    pushButtonAddFiles->setText(tr("Add files"));

    pushButtonAddFolder->setText(tr("Add folder"));

    uploadButton->setText(tr("Upload"));
}

void AddFilesPanelView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        initTitleControls();
    }
    else
        QWidget::changeEvent(event);
}

void AddFilesPanelView::createMenu()
{
    menu = new QMenu(this);
    Q_CHECK_PTR(menu);

    QAction * actDelete = new QAction(tr("Delete"), this);
    Q_CHECK_PTR(actDelete);
    menu->addAction(actDelete);
    connect(actDelete, SIGNAL(triggered()), this, SLOT(deleteFromList()));

    menu->addSeparator();

    QAction * actIsBackup = new QAction(tr("Set as backups"), this);
    Q_CHECK_PTR(actIsBackup);
    menu->addAction(actIsBackup);
    connect(actIsBackup, SIGNAL(triggered()), this, SLOT(establishIsBackup()));

    QAction * actIsBackupCancel = new QAction(tr("Reset mark backup"), this);
    Q_CHECK_PTR(actIsBackupCancel);
    menu->addAction(actIsBackupCancel);
    connect(actIsBackupCancel, SIGNAL(triggered()), this, SLOT(establishIsBackupCancel()));

    menu->addSeparator();

    QAction * actIsBackupAll = new QAction(tr("Set all as backups"), this);
    Q_CHECK_PTR(actIsBackupAll);
    menu->addAction(actIsBackupAll);
    connect(actIsBackupAll, SIGNAL(triggered()), this, SLOT(establishIsBackupAll()));

    QAction * actIsBackupCancelAll = new QAction(tr("Reset all mark backup"), this);
    Q_CHECK_PTR(actIsBackupCancelAll);
    menu->addAction(actIsBackupCancelAll);
    connect(actIsBackupCancelAll, SIGNAL(triggered()), this, SLOT(establishIsBackupCancelAll()));
}

QString AddFilesPanelView::task() const
{
    return lineEditTask->text();
}

void AddFilesPanelView::setTask(QString _task)
{
    //if(!_task.trimmed().isEmpty())
    {
        lineEditTask->setText(_task.trimmed());
    }
}

QString AddFilesPanelView::version() const
{
    return labelVersion->text();
}

void AddFilesPanelView::setVersion(QString version)
{
    labelVersion->setText(version);
}

void AddFilesPanelView::setVisibleLogout(bool v)
{
    pushButtonLogout->setVisible(v);
}

bool AddFilesPanelView::visibleLogout() const
{
    return pushButtonLogout->isVisible();
}

void AddFilesPanelView::setTextLogout(QString _text)
{
    //labelLogout->setText(_text);
    pushButtonLogout->show();
    pushButtonLogout->setText(_text);
}

void AddFilesPanelView::setVisibleCheckIndexFiles(bool visible)
{
    checkBoxIndexFiles->setVisible(visible);
}

bool AddFilesPanelView::visibleCheckIndexFiles() const
{
    return checkBoxIndexFiles->isVisible();
}

void AddFilesPanelView::setCheckDeleteSource(bool ds)
{
    checkBoxDeleteSource->setChecked(ds);
}

bool AddFilesPanelView::isCheckDeleteSource() const
{
    return checkBoxDeleteSource->isChecked();
}

void AddFilesPanelView::setEnabledDeleteSource(bool eds)
{
    checkBoxDeleteSource->setEnabled(eds);
}

bool AddFilesPanelView::isEnabledDeleteSource() const
{
    return checkBoxDeleteSource->isEnabled();
}

void AddFilesPanelView::setIntermediateCopy(bool check)
{
    checkBoxIntermediateCopy->setChecked(check);
}

bool AddFilesPanelView::intermediateCopy() const
{
    return checkBoxIntermediateCopy->isChecked();
}

void AddFilesPanelView::setIndexed(bool check)
{
    checkBoxIndexFiles->setChecked(check);
}

bool AddFilesPanelView::indexed() const
{
    return checkBoxIndexFiles->isChecked();
}

void AddFilesPanelView::setEnabledUpload(bool enabled)
{
    uploadButton->setEnabled(enabled);
}

void AddFilesPanelView::clear()
{
    setTask(QString());

    setCollection(QList<FileShortInfoModel*>());

    setTextLogout(QString());
    pushButtonLogout->hide();
}

void AddFilesPanelView::on_addFileButton_clicked(bool)
{
    emit addFilesCommand();
}

void AddFilesPanelView::setCollection(QList<FileShortInfoModel*> files)
{
    m_pModel->setCollection(files);

    if(m_pModel)
    {
        for( int i = 0; i < m_pModel->rowCount(); ++i )
            tableViewFileInfo->setIndexWidget(m_pModel->index(i, 2), new QCheckBox(this));
    }
}

QList<FileShortInfoModel*> AddFilesPanelView::collection() const
{
    return m_pModel->collection();
}

void AddFilesPanelView::on_addFolderButton_clicked(bool)
{
    emit addDirCommand();
}

void AddFilesPanelView::on_uploadButton_clicked(bool)
{
    emit uploadCommand();
}

void AddFilesPanelView::on_logout_clicked(bool)
{
    emit logoutCommand();
}

void AddFilesPanelView::onTaskEdit(const QString & txt)
{
    emit editTaskCommand(txt);
}

void AddFilesPanelView::on_isintermediatecopy_clicked(bool check)
{
    emit isIntermediateCopy(check);
}

void AddFilesPanelView::on_isindexed_clicked(bool check)
{
    emit isIndexed(check);
}

void AddFilesPanelView::on_isdeletesource_clicked(bool check)
{
    emit isDeleteSource(check);
}

void AddFilesPanelView::customMenuRequested(const QPoint & pos)
{
    QModelIndex index = tableViewFileInfo->indexAt(pos);
    if(index.row() < 0)
        return;

    menu->popup(tableViewFileInfo->viewport()->mapToGlobal(pos));
}

void AddFilesPanelView::deleteFromList()
{
    //model->deleteSelectItem(tableViewFileInfo->selectionModel()->selectedRows());
    emit deleteSelectItem(tableViewFileInfo->selectionModel()->selectedRows(FileShortInfoTableModel::FIELD_FILENAME));
}

void AddFilesPanelView::establishIsBackup()
{
    //model->establishIsBackupSelectItem(tableViewFileInfo->selectionModel()->selectedRows(), true);
    emit establishIsBackupSelectItem(tableViewFileInfo->selectionModel()->selectedRows(FileShortInfoTableModel::FIELD_FILENAME), true);
}

void AddFilesPanelView::establishIsBackupCancel()
{
    //model->establishIsBackupSelectItem(tableViewFileInfo->selectionModel()->selectedRows(), false);
    emit establishIsBackupSelectItem(tableViewFileInfo->selectionModel()->selectedRows(FileShortInfoTableModel::FIELD_FILENAME), false);
}

void AddFilesPanelView::establishIsBackupAll()
{
    emit establishIsBackupAll(true);
}

void AddFilesPanelView::establishIsBackupCancelAll()
{
    emit establishIsBackupAll(false);
}
