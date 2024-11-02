#include "app.h"

App::App(QObject *parent)
    : QObject{parent}
{
    this->audioInput = new AudioInput(new AudioOutput());
}

void App::start()
{
    audioInput->start();
}
