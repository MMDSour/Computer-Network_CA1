#include "AudioInput.h"

AudioInput::AudioInput()
{
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Int16);
    qAudioSource = new QAudioSource(format, this);
    this->open(QIODevice::WriteOnly);
    if (!this->isOpen()) {
        qDebug() << "QIODevice is not open for writing.";
    }

}

void AudioInput::start(){
    qAudioSource->start(this);
    if (qAudioSource->state() != QAudio::ActiveState) {
        qDebug() << "Audio source not active. State:" << qAudioSource->state();
    }
}

qint64 AudioInput::writeData(const char *data, qint64 len){
    qDebug() << "Received audio data of size:" << len;
    return len;
}
