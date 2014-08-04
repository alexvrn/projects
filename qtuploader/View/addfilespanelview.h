#ifndef ADDFILESPANELVIEW_H
#define ADDFILESPANELVIEW_H

#include <QLineEdit>
#include <QTableView>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QCheckBox>
#include <QMenu>
#include <QEvent>

#include "Service/dialogservice.h"
#include "View/iviewadd.h"
#include "Model/fileshortinfomodel.h"
#include "Model/fileshortinfotablemodel.h"
#include "View/iviewdata.h"

/*namespace Ui {
    class AddFilesPanel;
}*/

class AddFilesPanelView : public QWidget, public IViewAdd, public IViewData
{
    Q_OBJECT
    
public:
    explicit AddFilesPanelView(QWidget *parent = 0);
    ~AddFilesPanelView();

    void setCollection(QList<FileShortInfoModel*>);
    QList<FileShortInfoModel*> collection() const;

    QString task() const;
    void setTask(QString);

    QString version() const;
    void setVersion(QString);

    void setVisibleLogout(bool);
    bool visibleLogout() const;

    void setTextLogout(QString);

    void setVisibleCheckIndexFiles(bool);
    bool visibleCheckIndexFiles() const;

    void setIntermediateCopy(bool);
    bool intermediateCopy() const;

    void setCheckDeleteSource(bool);
    bool isCheckDeleteSource() const;

    void setEnabledDeleteSource(bool);
    bool isEnabledDeleteSource() const;

    void setIndexed(bool);
    bool indexed() const;

    void setEnabledUpload(bool);

    void clear();

    bool initControls();

protected:
    void changeEvent(QEvent *event);

signals:
    void addFilesCommand();
    void addDirCommand();
    void uploadCommand();
    void logoutCommand();
    void editTaskCommand(const QString&);
    void deleteSelectItem(const QModelIndexList&);
    void establishIsBackupSelectItem(const QModelIndexList&, bool);
    void establishIsBackupAll(bool);
    void isIntermediateCopy(bool);
    void isIndexed(bool);
    void isDeleteSource(bool);
    
private slots:
    void on_addFileButton_clicked(bool = false);

    void on_addFolderButton_clicked(bool = false);

    void on_uploadButton_clicked(bool = false);

    void on_logout_clicked(bool = false);

    void on_isintermediatecopy_clicked(bool = false);

    void on_isindexed_clicked(bool = false);

    void on_isdeletesource_clicked(bool = false);

    void onTaskEdit(const QString &);

    void customMenuRequested(const QPoint &);

    void deleteFromList();
    void establishIsBackup();
    void establishIsBackupCancel();
    void establishIsBackupAll();
    void establishIsBackupCancelAll();

private:
    void createMenu();

    void initTitleControls();

private:
    //Ui::AddFilesPanel *form;
    QWidget* form;

    QLabel * labelTask;
    QLabel * labelFiles;
    QTableView * tableViewFileInfo;
    QLineEdit * lineEditTask;
    QLabel * labelVersion;
    QCheckBox* checkBoxDeleteSource;
    QPushButton* pushButtonLogout;
    QPushButton* uploadButton;
    QPushButton * pushButtonAddFiles;
    QPushButton * pushButtonAddFolder;

    DialogService *m_pDlgService;
    FileShortInfoTableModel * m_pModel;

    QCheckBox* checkBoxIndexFiles;
    QCheckBox* checkBoxIntermediateCopy;

    QMenu * menu;
};

#endif // ADDFILESPANELVIEW_H
