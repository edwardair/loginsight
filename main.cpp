#include <QApplication>
#include <QQmlApplicationEngine>
#include "coreboot.h"
#include <QtGlobal>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include "linehighlighter.h"
#include <QQmlContext>
#include "nativehelper.h"
#include "textcodec.h"
#include <QIcon>
#include <QTranslator>

QFile gLogFile;
static const QString gLevels[] = {"Debg", "Warn", "Err-", "Err+", "Info", "Syst"};
static void myMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    auto level = gLevels[(int)type];
    auto time = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz");
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

static void prepareMyDir() {
    QDir::home().mkdir(".loginsight");
}

static void backupLog(NativeHelper& native) {
    QFile f(native.logPath());
    if (!f.exists())
        return;

    QFile(native.logBackupPath()).remove();

    f.copy(native.logBackupPath());
}


int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    qputenv("QT_LOGGING_RULES", "");
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("compilelife");
    QCoreApplication::setOrganizationDomain("compilelife.com");
    QCoreApplication::setApplicationName("loginsight");

    app.setWindowIcon(QIcon(":/images/logo.png"));

//    QTranslator trans;
//    trans.load(":/qtbase_zh_CN.qm");
//    app.installTranslator(&trans);

    prepareMyDir();
    NativeHelper native;

    backupLog(native);
    auto logPath = native.logPath();
    gLogFile.setFileName(logPath);
    if (gLogFile.open(QIODevice::WriteOnly)) {
        qInfo()<<"logfile at: "<<logPath;
        qInstallMessageHandler(myMessageHandler);
    } else {
        qWarning()<<"can't not open "<<logPath;
    }

    QQmlApplicationEngine engine;
    qmlRegisterType<CoreBoot>("com.cy.CoreBoot", 1, 0, "CoreBoot");
    qmlRegisterType<LineHighlighter>("com.cy.LineHighlighter", 1, 0, "LineHighlighter");
    qmlRegisterType<TextCodec>("com.cy.TextCodec", 1, 0, "TextCodec");
    engine.rootContext()->setContextProperty("NativeHelper", new NativeHelper);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
