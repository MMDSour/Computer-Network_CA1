#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "Network/Client.h"
#include "App/App.h"
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    bool isOfferer = stoi(argv[1]);

    QString qmlPath;

#ifdef Q_OS_MAC
    qmlPath = "../../../../../UI/Main.qml";  // Mac-specific path
#elif defined(Q_OS_WIN)
    qmlPath = "../../UI/Main.qml";  // Windows-specific path
#else
    qmlPath = QCoreApplication::applicationDirPath() + "/Main.qml";  // Default path
#endif

    App* application = new App(nullptr, isOfferer);
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("app", application);
    engine.load(QUrl::fromLocalFile(qmlPath));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
