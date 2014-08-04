#include "filesystemremotedialog.h"
#include <QLayout>
#include <QMessageBox>
#include <QModelIndex>
#include <QDir>
#include <QKeyEvent>

FileSystemRemoteDialog::FileSystemRemoteDialog(bool isfolder, QWidget *parent) :
    QDialog(parent)
{
    setIsFolder(isfolder);

    labelPath = new QLabel(this);

    pbBack = new QPushButton(tr("Back"), this);
    connect(pbBack, SIGNAL(clicked(bool)), this, SLOT(sltClickedBack(bool)));
    pbBack->setEnabled(false);
    pbBack->setMaximumWidth(40);

    QLabel * l_hLayout = new QLabel("", this);
    l_hLayout->setMinimumHeight(50);
    l_hLayout->setMaximumHeight(60);
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->addWidget(labelPath, Qt::AlignLeft);
    hLayout->addWidget(pbBack, Qt::AlignRight);
    l_hLayout->setLayout(hLayout);

    table = new QTableView(this);
    table->setEditTriggers(QAbstractItemView::DoubleClicked);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    if(isFolder())
        table->setSelectionMode(QAbstractItemView::SingleSelection);
    else
        table->setSelectionMode(QAbstractItemView::MultiSelection);
    table->setShowGrid(false);
    table->setWordWrap(false);
    connect(table, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(stlDoubleClicked(const QModelIndex&)));

    model = new FileSystemRemote(isFolder(), this);
    table->setModel(model);
    table->setColumnWidth(FileSystemRemote::FIELD_FILENAME, 150);
    table->setColumnWidth(FileSystemRemote::FIELD_TYPE, 70);

    dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Cancel, this);
    connect(dialogButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialogButtonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout * l = new QVBoxLayout(this);
    l->addWidget(l_hLayout, Qt::AlignJustify);
    l->addWidget(table);
    l->addWidget(dialogButtonBox);
    l->setSpacing(5);
    setLayout(l);

    fsrController = new FileSystemRemoteController(parent);

    if(!isFolder())
        setWindowTitle("Open files...");
    else
        setWindowTitle("Open folder...");

    setModal(true);

    installEventFilter(this);

    m_bWaitResponse = true;
}

bool FileSystemRemoteDialog::eventFilter(QObject * o, QEvent * e)
{
    if (e->type() == QEvent::KeyPress)
    {
        // key pressed
        // transforms QEvent into QKeyEvent
        QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(e);
        switch (pKeyEvent->key())
        {
            case Qt::Key_Backspace:
            case Qt::Key_Back:
            {
                sltClickedBack();
                break;
            }
        }
        return true;
    }
    else
    {
        return QDialog::eventFilter(o, e);
    }
}

void FileSystemRemoteDialog::setIsFolder(bool isfolder)
{
    m_bIsFolder = isfolder;
}

bool FileSystemRemoteDialog::isFolder() const
{
    return m_bIsFolder;
}

void FileSystemRemoteDialog::stlDoubleClicked(const QModelIndex& index)
{
    if(!index.data(Qt::UserRole).toBool())
        return;

    moveToFolder(index.data(Qt::DisplayRole).toString());
}

QStringList FileSystemRemoteDialog::selectFiles() const
{
    QStringList list;
    for(QModelIndex model: table->selectionModel()->selectedRows(0))
    {
        if(!model.data(Qt::UserRole).toBool())
            list.append(currentPath() + model.data(Qt::DisplayRole).toString());
    }
    return list;
}

QString FileSystemRemoteDialog::selectFolder() const
{
    if(table->selectionModel()->selectedRows(0).size() != 1)
        return QString();

    return currentPath() + table->selectionModel()->selectedRows(0)[0].data(Qt::DisplayRole).toString();
}

void FileSystemRemoteDialog::controlsChangeState(bool enabled)
{
    table->setEnabled(enabled);
    labelPath->setText(currentPath());
    pbBack->setEnabled(m_CurrentPath.size() > 0 && enabled);
}

void FileSystemRemoteDialog::getContentFolder()
{
    m_bWaitResponse = false;
    controlsChangeState(m_bWaitResponse);

    emit requestFolderContents(currentPath(), false);
}

void FileSystemRemoteDialog::sltResponseFolderContents(const QList<FileSystemRemoteItemModel>& list)
{
    model->getData(list);

    m_bWaitResponse = true;
    controlsChangeState(m_bWaitResponse);
}

QString FileSystemRemoteDialog::currentPath() const
{
    QString str = "";
    QChar sep = '\\';
    for(auto s: m_CurrentPath)
    {
        if(s[s.length() - 1] == sep)//for Windows
            s.remove(s.length() - 1, 1);
        str += s + sep;
    }
    return str;
}

void FileSystemRemoteDialog::sltClickedBack(bool)
{
    if(m_CurrentPath.size() == 0)
        return;

    m_CurrentPath.removeLast();
    getContentFolder();

    controlsChangeState(m_bWaitResponse);
}

void FileSystemRemoteDialog::moveToFolder(const QString& folder)
{
    m_CurrentPath.append(folder);
    getContentFolder();

    controlsChangeState(m_bWaitResponse);
}


