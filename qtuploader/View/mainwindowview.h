#ifndef MAINWINDOWVIEW_H
#define MAINWINDOWVIEW_H

#include <QMainWindow>
#include <QSplitter>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QLabel>
#include <QToolButton>
#include <QEvent>
#include <QSharedPointer>

#include "View/iviewmain.h"
#include "View/passworddialogview.h"
#include "View/iviewdata.h"
#include "Service/credentials.h"
#include "Presenter/passworddialogpresenter.h"

class MainWindowView : public QMainWindow, public IViewMain, public IViewData
{
    Q_OBJECT
    
public:
    //explicit MainWindowView(QWidget *parent = 0);
    ~MainWindowView();

public:
    static MainWindowView* instance()
    {
        if (m_Instance.isNull())
            m_Instance = QSharedPointer<MainWindowView>(new MainWindowView());

        return m_Instance.data();
    }

protected:
    //static MainWindowView* m_Instance;
    static QSharedPointer<MainWindowView> m_Instance;

private:
    MainWindowView& operator=(const MainWindowView&) { return *this; }
    MainWindowView(const MainWindowView&);

protected:
    MainWindowView(QWidget *parent = 0);

public:
    void setPanel(IView *panelTop, IView *panelBottom);//QWidget *panelTop, QWidget *panelBottom);

    void setInformation(const QString&);
    void setTitleText(const QString&);
    void setTextSwitchLanguage(const QString&);

    bool initControls();

protected:
    void changeEvent(QEvent *);

signals:
    void isLogon(const Credentials&, bool);
    void queryLogon(const Credentials&);
    void logoutCommand();
    void checkLanguage();

public slots:
    void cancel();
    void on_check_language(bool = false);

private:
    void createStatusBar();

private:
    QSplitter * splitter;
    //Ui::MainWindow *ui;

    QWidget * form;
    QVBoxLayout *layout;
    QToolButton* toolButtonTranslator;
    PasswordDialogView * m_pPasswordDialogService;
    PasswordDialogPresenter * m_pPresenterPasswordDialog;

    QStatusBar * status;
    QLabel * labelInfo;
};

#endif // MAINWINDOW_H
