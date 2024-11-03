#ifndef APP_H
#define APP_H

#include <QObject>
#include "Audio/AudioInput.h"

class App : public QObject
{
    Q_OBJECT
public:
    explicit App(QObject *parent = nullptr);
    Q_INVOKABLE void start();

Q_SIGNALS:

private:
    AudioInput* audioInput;
};

#endif
