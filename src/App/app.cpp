#include "app.h"

App::App(QObject *parent, bool isOfferer)
    : QObject{parent}
{
    if (isOfferer)
        m_verName = "Offerer";
    else
    {
        m_verName = "Answerer";
        client = new Client(nullptr, "https://localhost:8080", "peer2", false, "peer1");
    }
    this->isOfferer = isOfferer;
}

void App::start()
{
    if (isOfferer)
    {
        client = new Client(nullptr, "https://localhost:8080", "peer1", true, "peer2");
        client->startCall("peer2");
    }
}

void App::end()
{
    delete client;
    client = nullptr;
}
