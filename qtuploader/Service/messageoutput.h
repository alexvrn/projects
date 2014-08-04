#ifndef MESSAGEOUTPUT_H
#define MESSAGEOUTPUT_H

#include <QFile>
#include <QDateTime>
#include <QtMsgHandler>
#include <QTextStream>

void messageOutput(QtMsgType type, const QMessageLogContext& /*context*/, const QString& msg)
{
    QFile fMessFile(qApp->applicationDirPath() + "//Log.log");
    if(!fMessFile.open(QIODevice::Append | QIODevice::Text)){
        return;
    }
    QString sCurrDateTime = "[" + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") + "]";
    QTextStream tsTextStream(&fMessFile);
    switch (type)
    {
        case QtDebugMsg:
        {
            //fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            tsTextStream << QString("ProjectLogDebug%1: %2\n").arg(sCurrDateTime).arg(msg);
            break;
        }
        case QtWarningMsg:
        {
            //fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            tsTextStream << QString("ProjectLogWarning%1: %2\n").arg(sCurrDateTime).arg(msg);
            break;
        }
        case QtCriticalMsg:
        {
            //fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            tsTextStream << QString("ProjectLogCritical%1: %2\n").arg(sCurrDateTime).arg(msg);
            break;
        }
        case QtFatalMsg:
        {
            //fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            tsTextStream << QString("ProjectLogFatal%1: %2\n").arg(sCurrDateTime).arg(msg);
            abort();
        }
    }
    tsTextStream.flush();
    fMessFile.flush();
    fMessFile.close();
}

#endif // MESSAGEOUTPUT_H
