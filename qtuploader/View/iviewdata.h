#ifndef IVIEWDATA_H
#define IVIEWDATA_H

#include <QObject>
#include <QWidget>

class IViewData //: public QObject
{
    //Q_OBJECT
public:
    explicit IViewData(QString name, QWidget *parent = 0);

protected:
    QWidget* loadUiFile();

private:
     QString pathToUI(QString);

signals:

private:
    QString m_strNameData;
    QWidget* m_qParent;
};

#endif // IVIEWDATA_H
