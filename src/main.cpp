#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "AudioInput.h"
#include "AudioOutput.h"
#include "app.h"
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    std::unique_ptr<AudioInput> AI = std::make_unique<AudioInput>(new AudioOutput());

    QString qmlPath;

#ifdef Q_OS_MAC
    qmlPath = "../../../../../Main.qml";  // Mac-specific path
#elif defined(Q_OS_WIN)
    qmlPath = "../../Main.qml";  // Windows-specific path
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
