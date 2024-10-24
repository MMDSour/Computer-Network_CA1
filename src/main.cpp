#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "AudioInput.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    std::unique_ptr<AudioInput> AI = std::make_unique<AudioInput>();
    AI->start();

    QQmlApplicationEngine engine;
    engine.load(QUrl::fromLocalFile("../../Main.qml"));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
