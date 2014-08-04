#include "mainwindowview.h"

#include <QFile>
#include <QUiLoader>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QPushButton>
#include <QDesktopWidget>
#include <QStatusBar>
#include <QDebug>

QSharedPointer<MainWindowView> MainWindowView::m_Instance;

MainWindowView::MainWindowView(QWidget *parent) :
    QMainWindow(parent), IViewData("MainWindow", parent)
{
    form = Q_NULLPTR;
}

MainWindowView::MainWindowView(const MainWindowView& ) :
    QMainWindow(), IViewData("MainWindow")
{

}

MainWindowView::~MainWindowView()
{
    //delete layout;
    //delete m_pPasswordDialogService;
    //delete m_pPresenterPasswordDialog;

    //delete form;
    qDebug() << "oh del!";
}

bool MainWindowView::initControls()
{
    if(form)
        return true;

    try
    {
        if((form = loadUiFile()) == Q_NULLPTR)
        {
            //throw QString("Error loading MainWindow");
            return false;
        }

        layout = new QVBoxLayout;
        Q_CHECK_PTR(layout);
        layout->addWidget(form);

        setCentralWidget(form);

        splitter = form->findChild<QSplitter*>("splitter");
        Q_CHECK_PTR(splitter);

        m_pPasswordDialogService = new PasswordDialogView(this);
        if(!m_pPasswordDialogService->initControls())
        {
            qDebug() << "Don't create PasswordDialogView!";
            return false;
        }
        m_pPresenterPasswordDialog = new PasswordDialogPresenter(m_pPasswordDialogService);

        QObject::connect(m_pPasswordDialogService, SIGNAL(cancel()), this, SLOT(cancel()));
        QObject::connect(m_pPresenterPasswordDialog, SIGNAL(queryLogon(const Credentials&)), this, SIGNAL(queryLogon(const Credentials&)));

        QObject::connect(this, SIGNAL(isLogon(const Credentials&, bool)), m_pPresenterPasswordDialog, SIGNAL(isLogon(const Credentials&, bool)));
        QObject::connect(this, SIGNAL(logoutCommand()), m_pPresenterPasswordDialog, SLOT(sltLogoutCommand()));

        //status bar
        createStatusBar();

        //Position on Center Screen
        setGeometry(0, 0, 900, 600);
        QRect r = geometry();
        r.moveCenter(QApplication::desktop()->availableGeometry().center());
        setGeometry(r);

        m_pPasswordDialogService->dialogShow(true);

        return true;
    }
    catch(...)
    {
        return true;
    }
}

void MainWindowView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        toolButtonTranslator->setText(tr("EN"));
    }
    else
        QWidget::changeEvent(event);
}

void MainWindowView::createStatusBar()
{
    status = new QStatusBar(this);
    setStatusBar(status);

    labelInfo = new QLabel(QString(), this);
    Q_CHECK_PTR(labelInfo);
    labelInfo->setProperty("statusbar", QVariant(true));

    //status->addWidget(labelInfo, 0);

    toolButtonTranslator = new QToolButton(this);
    Q_CHECK_PTR(toolButtonTranslator);
    toolButtonTranslator->setText(tr("EN"));
    toolButtonTranslator->setVisible(false);
    connect(toolButtonTranslator, SIGNAL(clicked(bool)), this, SLOT(on_check_language(bool)));

    status->addWidget(toolButtonTranslator, 0);
    status->addWidget(labelInfo, 1);

    //QHBoxLayout * hLayout = new QHBoxLayout(status);
    //hLayout->addWidget(labelInfo, 0, Qt::AlignLeft);
    //hLayout->addWidget(toolButtonTranslator, 1, Qt::AlignRight);
    //hLayout->setSpacing(2);
    //status->setLayout(hLayout);
}

void MainWindowView::on_check_language(bool)
{
    emit checkLanguage();
}

void MainWindowView::setInformation(const QString& inf)
{
    labelInfo->setText(inf);
}

void MainWindowView::setTitleText(const QString& txt)
{
    setWindowTitle(txt);
}

void MainWindowView::setTextSwitchLanguage(const QString& txt)
{
    //toolButtonTranslator->setText(txt);
}

void MainWindowView::setPanel(IView *panelTop, IView *panelBottom)
{
    QWidget *view_obj_top = dynamic_cast<QWidget*>(panelTop);
    QWidget *view_obj_bottom = dynamic_cast<QWidget*>(panelBottom);

    splitter->addWidget(view_obj_top);
    splitter->addWidget(view_obj_bottom);
}

void MainWindowView::cancel()
{
    labelInfo->setText("");
    close();
}
