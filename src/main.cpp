#include "mainwindow.h"

#include <QApplication>
#include <QThread>
#include <QThreadPool>
#include <QSettings>
#include <QtGlobal>
#include <QFile>
#include <QDir>
#include <QDateTime>

static void createDefaultSettings()
{
    QSettings config;
    auto keys = config.allKeys();

    if (!keys.contains("editorFont")) {
        config.setValue("editorFont", "monospace");
    }

    if (!keys.contains("editorFontSize")) {
        config.setValue("editorFontSize", 12);
    }

    if (!keys.contains("caseSensitive")) {
        config.setValue("caseSensitive", true);
    }

    if (!keys.contains("checkUpdate")) {
        config.setValue("checkUpdate", true);
    }

    if (!keys.contains("tryFrom")) {
        config.setValue("tryFrom", QDateTime::currentDateTime());
    }

    if (!keys.contains("recentFile")) {
        config.setValue("recentFile", QStringList{});
    }

    if (!keys.contains("recentPrj")) {
        config.setValue("recentPrj", QStringList{});
    }

    if (!keys.contains("wrap")) {
        config.setValue("wrap", false);
    }

    if (!keys.contains("register")) {
        config.setValue("register", "");
    }

    config.sync();
}

QFile gLogFile;
static const QString gLevels[] = {"Debg", "Warn", "Err-", "Err+", "Info", "Syst"};
static void myMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    auto level = gLevels[(int)type];
    auto time = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.z");
    QString filePath(context.file);
    auto lastSeperartor=filePath.lastIndexOf(QDir::separator());
    if (lastSeperartor >= 0) {
        filePath = filePath.mid(lastSeperartor+1);
    }

    auto line = QString("[%1][%2][%3:%4] %5\n").arg(level, time, filePath).arg(context.line).arg(msg.toLocal8Bit().data());

    fprintf(stdout, "%s", line.toLocal8Bit().data());
    fflush(stdout);
    gLogFile.write(line.toLocal8Bit());
    gLogFile.flush();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("compilelife");
    QCoreApplication::setOrganizationDomain("compilelife.com");
    QCoreApplication::setApplicationName("loginsight");
    a.setWindowIcon(QIcon(":/res/img/logo.png"));

    auto logPath = QDir::tempPath()+QDir::separator()+"loginsight.log";
    gLogFile.setFileName(logPath);
    if (gLogFile.open(QIODevice::WriteOnly)) {
        qInfo()<<"logfile at: "<<logPath;
        qInstallMessageHandler(myMessageHandler);
    } else {
        qWarning()<<"can't not open "<<logPath;
    }

    auto idealCpuCount = QThread::idealThreadCount();
    qDebug()<<"cpu count: "<<idealCpuCount;
    if (idealCpuCount >= 3) {
        QThreadPool::globalInstance()->setMaxThreadCount(2);
    }

    createDefaultSettings();


    MainWindow w;
    w.setWindowTitle("Loginsight");
    w.show();
    if (argc > 1) {
        w.doOpenFile(argv[1]);
    }

    auto ret =  a.exec();

    gLogFile.close();
    return ret;
}
