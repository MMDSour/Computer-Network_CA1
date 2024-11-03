#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "Network/Client.h"
#include "App/App.h"
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    if (stoi(argv[1]) == 1)
    {
        Client* c = new Client(nullptr, "https://localhost:8080", "peer1", true, "peer2");
        c->startCall("peer2");
    }
    else if (stoi(argv[1]) == 0)
        Client* c = new Client(nullptr, "https://localhost:8080", "peer2", false, "peer1");

    QString qmlPath;

#ifdef Q_OS_MAC
    qmlPath = "../../../../../Main.qml";  // Mac-specific path
#elif defined(Q_OS_WIN)
    qmlPath = "../../UI/Main.qml";  // Windows-specific path
#else
    qmlPath = QCoreApplication::applicationDirPath() + "/Main.qml";  // Default path
#endif

    App application;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("app", &application);
    engine.load(QUrl::fromLocalFile(qmlPath));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
