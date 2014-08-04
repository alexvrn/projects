#include "passworddialogview.h"
#include "Service/serialization.h"

#include <QVBoxLayout>
#include <QUiLoader>
#include <QFile>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QApplication>
#include <QDesktopWidget>
#include <QPolygon>
#include <QRegion>
#include <QLabel>

const quint16 PasswordDialogView::BORDER_RADUIS = 7;

PasswordDialogView::PasswordDialogView(QWidget *parent) :
    QDialog(parent, Qt::MSWindowsFixedSizeDialogHint/*|Qt::CustomizeWindowHint*/ ), IViewData("PasswordDialog", parent)
{
    form = Q_NULLPTR;
}

PasswordDialogView::~PasswordDialogView()
{
    //delete form;
}

bool PasswordDialogView::initControls()
{
    if(form)
        return true;

    try
    {
        if((form = loadUiFile()) == Q_NULLPTR)
        {
            //throw QString("Error loading PasswordDialog");
            return false;
        }

        QVBoxLayout *layout = new QVBoxLayout(this);
        Q_CHECK_PTR(layout);
        layout->addWidget(form);
        layout->setContentsMargins(1, 1, 1, 1);
        setLayout(layout);

        lineEditUser = form->findChild<QLineEdit*>("lineEditUser");
        Q_CHECK_PTR(lineEditUser);
        connect(lineEditUser, SIGNAL(textEdited(const QString &)), this, SLOT(onCredentialsEdit(const QString &)));

        QLabel * labelLogin = form->findChild<QLabel*>("labelLogin");
        Q_CHECK_PTR(labelLogin);
        labelLogin->setText(tr("Login:"));

        lineEditPassword = form->findChild<QLineEdit*>("lineEditPassword");
        Q_CHECK_PTR(lineEditPassword);
        connect(lineEditPassword, SIGNAL(textEdited(const QString &)), this, SLOT(onCredentialsEdit(const QString &)));

        QLabel * labelPassword = form->findChild<QLabel*>("labelPassword");
        Q_CHECK_PTR(labelPassword);
        labelPassword->setText(tr("Password:"));

        lineEditServer = form->findChild<QLineEdit*>("lineEditServer");
        Q_CHECK_PTR(lineEditServer);
        connect(lineEditServer, SIGNAL(textEdited(const QString &)), this, SLOT(onCredentialsEdit(const QString &)));

        QLabel * labelServer = form->findChild<QLabel*>("labelServer");
        Q_CHECK_PTR(labelServer);
        labelServer->setText(tr("Server:"));

        lineEditService = form->findChild<QLineEdit*>("lineEditService");
        Q_CHECK_PTR(lineEditService);
        connect(lineEditService, SIGNAL(textEdited(const QString &)), this, SLOT(onCredentialsEdit(const QString &)));

        QLabel * labelService = form->findChild<QLabel*>("labelService");
        Q_CHECK_PTR(labelService);
        labelService->setText(tr("Service:"));

        QDialogButtonBox * buttonBox = form->findChild<QDialogButtonBox*>("buttonBox");
        Q_CHECK_PTR(buttonBox);
        connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
        connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

        setMinimumWidth(190);
        setWindowOpacity(0.9);

        //setModal(true);

        setMouseTracking(true);

        return true;
    }
    catch(...)
    {
        return false;
    }
}

void PasswordDialogView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_ptPosition.x() >= 0 && event->buttons() && Qt::LeftButton)
    {
        QPoint diff = event->pos() - m_ptPosition;
        QPoint newpos = this->pos() + diff;

        this->move(newpos);
    }
}

void PasswordDialogView::mousePressEvent(QMouseEvent *ev)
{
    m_ptPosition = ev->pos();
    //QMessageBox::information(0, "", "");
}

void PasswordDialogView::mouseReleaseEvent(QMouseEvent *)
{
    //clear old value position of press mouse
    m_ptPosition = QPoint(-1, -1);
}


void PasswordDialogView::setCredentials(const Credentials &c)
{
    lineEditUser->setText(c.login());
    lineEditPassword->setText(c.password());
    lineEditServer->setText(c.server());
    lineEditService->setText(c.service());
}

void PasswordDialogView::onCredentialsEdit(const QString & )
{
    Credentials credentials(
        lineEditUser->text(),
        lineEditPassword->text(),
        lineEditServer->text(),
        lineEditService->text());
    emit credentialsEdit(credentials);
}

void PasswordDialogView::accept()
{
    emit queryLogon();
}

void PasswordDialogView::reject()
{
    emit cancel();
}

QRegion PasswordDialogView::maskBorder(int radius) const
{
    int w = width();
    int h = height();
#ifdef Q_OS_LINUX
    h += 22;
#endif
    QPolygon polygon;
    polygon << QPoint(0, radius) << QPoint(radius, radius) << QPoint(radius, 0) << QPoint(w - radius, 0) << QPoint(w - radius, radius)
            << QPoint(w, radius) << QPoint(w, h - radius) << QPoint(w - radius, h - radius) << QPoint(w - radius, h)
            << QPoint(radius, h) << QPoint(radius, h - radius) << QPoint(0, h - radius) << QPoint(0, radius);
    QRegion regionMask(polygon);
    int d = 2 * radius;
    regionMask = regionMask.united(QRegion(    0,     0, d, d, QRegion::Ellipse));
    regionMask = regionMask.united(QRegion(w - d,     0, d, d, QRegion::Ellipse));
    regionMask = regionMask.united(QRegion(w - d, h - d, d, d, QRegion::Ellipse));
    regionMask = regionMask.united(QRegion(0    , h - d, d, d, QRegion::Ellipse));
    return regionMask;
}

void PasswordDialogView::dialogShow(bool isShow)
{
    //isShow ? show() : hide();
    if(isShow)
    {
        QRect rg = geometry();
        rg.moveCenter(QApplication::desktop()->availableGeometry().center());
        setGeometry(rg);

        setMask(maskBorder(BORDER_RADUIS));

        show();
        raise();
        activateWindow();
    }
    else
        hide();
}
