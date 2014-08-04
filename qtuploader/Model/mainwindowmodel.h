#ifndef MAINWINDOWMODEL_H
#define MAINWINDOWMODEL_H

#include <QString>

class MainWindowModel
{
public:
    enum Language {
        EN_Language = 0,
        RU_Language
    };

    MainWindowModel();

    QString statusInfo() const;
    void setStatusInfo(const QString&);

    QString titleText() const;

    void setRemoteService(bool, const QString& = QString());
    bool isRemoteService() const;

    QString service() const;
    void setService(const QString&);

    void setLanguage(Language);
    Language language() const;

    QString textSwitchLanguage() const;

private:
    static const QString MainWindowTitle;

private:
    QString m_strStatusInfo;
    bool m_bRemoteService;
    QString m_strService;
    Language m_Language;
};

#endif // MAINWINDOWMODEL_H
