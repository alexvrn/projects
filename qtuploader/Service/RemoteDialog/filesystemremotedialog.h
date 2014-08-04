#ifndef FILESYSTEMREMOTEDIALOG_H
#define FILESYSTEMREMOTEDIALOG_H

#include <QDialog>
#include <QTableView>
#include <QLabel>
#include <QPushButton>
#include <QEvent>
#include <QDialogButtonBox>
#include "Service/RemoteDialog/filesystemremotecontroller.h"
#include "Service/RemoteDialog/filesystemremotemodel.h"

class FileSystemRemoteDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FileSystemRemoteDialog(bool isfolder = false, QWidget *parent = 0);

    void getContentFolder();

    QStringList selectFiles() const;
    QString selectFolder() const;

    bool isFolder() const;

signals:
    void requestFolderContents(const QString&, bool);

public slots:
    void sltResponseFolderContents(const QList<FileSystemRemoteItemModel>&);
    void stlDoubleClicked(const QModelIndex&);
    void sltClickedBack(bool = false);

protected:
    bool eventFilter(QObject * o, QEvent * e);

private:
    QString currentPath() const;
    void moveToFolder(const QString&);

    void setIsFolder(bool);

    void controlsChangeState(bool);

private:
    QTableView * table;
    QLabel * labelPath;
    QDialogButtonBox * dialogButtonBox;
    QPushButton * pbBack;

    FileSystemRemoteController * fsrController;
    FileSystemRemote * model;
    QList<QString> m_CurrentPath;

    bool m_bIsFolder;

    bool m_bWaitResponse;
};

#endif // FILESYSTEMREMOTEDIALOG_H
