#include "splitbutton.h"

#include <QFrame>
#include <QMenu>
#include <QResizeEvent>

SplitButton::SplitButton(QWidget *parent) :
    QPushButton(parent)
{
}

void SplitButton::setUsers(const QStringList& _users)
{
    QMenu* _menu = menu();
    if (_menu)
        _menu->clear();
    else
    {
        _menu = new QMenu(this);
        setMenu(_menu);
    }

    for(QString _user: _users)
    {
        QAction * act = new QAction(_user, this);
        _menu->addAction(act);
    }
}

void SplitButton::setMenu( QMenu* menu )
{
    if (menu)
    {
        line=new QFrame(this);
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);
        line->setProperty("splitbutton", QVariant(true));
        line->show();
    }
    QPushButton::setMenu(menu);
}

void SplitButton::resizeEvent ( QResizeEvent * event )
{
    if (menu())
    {
        int width=event->size().width();
        int height=event->size().height();
        line->setGeometry(QRect(width-18, 2, 1, height-2));
        line->show();
    }
}

void SplitButton::keyPressEvent ( QKeyEvent * event )
{
    if (menu())
    {
        switch (event->key())
        {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                emit clicked();
                break;
            case Qt::Key_Up:
            case Qt::Key_Down:
                showMenu();
                break;
            default:
            QPushButton::keyPressEvent(event);
        }
    }
    else
        QPushButton::keyPressEvent(event);
}

void SplitButton::mousePressEvent( QMouseEvent * event )
{
    if (menu())
    {
        if((width()-event->x())<=15)
            showMenu();
        else
            setDown(true);
    }
    else
        QPushButton::mousePressEvent(event);
}
