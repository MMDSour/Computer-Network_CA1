#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H
#include <QAudioSource>
#include <QIODevice>
#include <QDebug>
#include "opus.h"
#include "audioOutput.h"

class AudioInput : public QIODevice{
public:
    AudioInput(AudioOutput *output);
    qint64 writeData (const char* data, qint64 len);
    qint64 readData(char *data, qint64 maxlen){return 0;};
    void close(){return;};
    bool seek(qint64 pos){return 0;};
    void start();
    QByteArray encodeData(const char* data, qint64 len);
private:
    QAudioSource * qAudioSource;
    OpusEncoder *opusEncoder;
    int sampleRate;
    int channels;
    AudioOutput* audioOutput;
};

#endif // AUDIOINPUT_H
