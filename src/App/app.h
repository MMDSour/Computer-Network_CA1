#ifndef APP_H
#define APP_H

#include <QObject>
#include "Network/Client.h"

class App : public QObject
{
    Q_OBJECT
public:
    explicit App(QObject *parent = nullptr, bool isOfferer = false);
    Q_INVOKABLE void start();
    Q_INVOKABLE void end();
    QString verName() const { return m_verName; }

Q_SIGNALS:
    void verNameChanged();
private:
    QString m_verName;
    bool isOfferer;
    Client* client;

    Q_PROPERTY(QString verName READ verName NOTIFY verNameChanged)
};

#endif
