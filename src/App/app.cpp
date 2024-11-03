#include "app.h"

App::App(QObject *parent)
    : QObject{parent}
{
    this->audioInput = new AudioInput();
}

void App::start()
{
    audioInput->start();
}
