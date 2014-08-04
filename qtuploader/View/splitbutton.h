#ifndef SPLITBUTTON_H
#define SPLITBUTTON_H

#include <QPushButton>

class QMenu;
class QFrame;
class SplitButton : public QPushButton
{
    Q_OBJECT
public:
    explicit SplitButton(QWidget *parent = 0);

    void setMenu(QMenu* menu);
    void setUsers(const QStringList& );

protected:
    void resizeEvent ( QResizeEvent * );
    void keyPressEvent ( QKeyEvent * );
    void mousePressEvent ( QMouseEvent * );

signals:

public slots:

private:
    QFrame* line;
};

#endif // SPLITBUTTON_H
