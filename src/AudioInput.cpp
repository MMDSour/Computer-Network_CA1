#include "AudioInput.h"

AudioInput::AudioInput()
    : opusEncoder(nullptr), sampleRate(48000), channels(1) {

    QAudioFormat format;
    format.setSampleRate(sampleRate);
    format.setChannelCount(channels);
    format.setSampleFormat(QAudioFormat::Int16);
    qAudioSource = new QAudioSource(format, this);

    int error;
    opusEncoder = opus_encoder_create(sampleRate, channels, OPUS_APPLICATION_AUDIO, &error);
    if (error != OPUS_OK) {
        qDebug() << "Failed to create Opus encoder:" << opus_strerror(error);
    }

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
