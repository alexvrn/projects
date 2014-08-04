#ifndef UPLOADPROGRESSVIEW_H
#define UPLOADPROGRESSVIEW_H

#include <QWidget>
#include <QTableView>
#include <QList>
#include <QMenu>
#include <QPushButton>
#include <QCheckBox>
#include <QEvent>

#include "iviewupload.h"
#include "Model/fileuploadinfomodel.h"
#include "Model/fileuploadinfotablemodel.h"
#include "View/iviewdata.h"

class UploadProgressView : public QWidget, public IViewUpload, public IViewData
{
    Q_OBJECT
    
public:
    explicit UploadProgressView(QWidget *parent = 0);
    ~UploadProgressView();

    void resetItem(int);
    
    void setCollection(const QList<FileShortInfoModel*>&);
    QList<FileShortInfoModel*> collection() const;

    void setShowUploading(bool);
    bool showUploading() const;

    void setShowCopying(bool);
    bool showCopying() const;

    void setShowArchive(bool);
    bool showArchive() const;

    void setShowError(bool);
    bool showError() const;

    void setShowOK(bool);
    bool showOK() const;

    void setVisiblePause(bool);
    bool visiblePause() const;

    void setEnabledRepeat(bool);

    void clear();

    void setAdmin(bool);

    void setNameOnPause(const QString&);

    void setModel(UploadProgressModel *);

    bool initControls();

private:
    void createMenu();

    void initTitleControls();

protected:
    void changeEvent(QEvent *event);

private slots:
    void on_paused_clicked(bool = false);
    void on_archive_clicked(bool = false);
    void on_repeat_clicked(bool = false);
    void on_statistic_clicked(bool = false);

    void on_getarchive_clicked(bool = false);
    void on_isfinished_clicked(bool = false);
    void on_isuploading_clicked(bool = false);
    void on_iserror_clicked(bool = false);
    void on_iscopying_clicked(bool = false);

    void customMenuRequested(const QPoint&);

    void sltCancelDownload();

    void sltChangeIsBackup(const QString&, bool);

signals:
    void pausedCommand();
    void archiveCommand(bool);
    void repeatCommand();

    void sendInformationMail();

    void getArchiveCommand(bool);
    void showUploadingCommand(bool);
    void showErrorCommand(bool);
    void showFinishedCommand(bool);
    void showCopyingCommand(bool);

    void cancelDownload(const QModelIndexList&);

    void changeIsBackup(const QString&, bool);

private:
    QWidget* form;
    QTableView * tableViewUpload;

    FileUploadInfoTableModel * model;

    QPushButton* pushButtonStatistic;
    QPushButton* pushButtonPaused;
    QPushButton* pushButtonArchive;
    QPushButton* pushButtonRepeat;

    QCheckBox* checkBoxArchive;
    QCheckBox* checkBoxUploading;
    QCheckBox* checkBoxError;
    QCheckBox* checkBoxFinished;
    QCheckBox* checkBoxCopying;

    QMenu * menu;
    QAction * actCancel;

    //QModelIndexList m_selectModelIndexList;
    //QL
};

#endif // UPLOADPROGRESSCONTROLLER_H
