#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H
#include <QAudioSource>
#include <QMediaDevices>
#include <QIODevice>
#include <QDebug>
#include "opus.h"

class AudioInput : public QIODevice{
public:
    AudioInput();
    qint64 writeData (const char* data, qint64 len);
    qint64 readData(char *data, qint64 maxlen){return 0;};
    void close(){return;};
    bool seek(qint64 pos){return 0;};
    void start();
private:
    QAudioSource * qAudioSource;
    OpusEncoder *opusEncoder;
    int sampleRate;
    int channels;
};

#endif // AUDIOINPUT_H
