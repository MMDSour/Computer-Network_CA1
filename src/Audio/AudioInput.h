#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H
#include <QAudioSource>
#include <QIODevice>
#include <QDebug>
#include "opus.h"

class AudioInput : public QIODevice{
    Q_OBJECT
public:
    explicit AudioInput(QObject *parent = nullptr);
    qint64 writeData (const char* data, qint64 len);
    qint64 readData(char *data, qint64 maxlen){return 0;};
    void close(){return;};
    bool seek(qint64 pos){return 0;};
    void start();
    QByteArray encodeData(const char* data, qint64 len);
Q_SIGNALS:
    void dataReady(QByteArray &data);
private:
    QAudioSource * qAudioSource;
    OpusEncoder *opusEncoder;
    int sampleRate;
    int channels;
};

#endif
