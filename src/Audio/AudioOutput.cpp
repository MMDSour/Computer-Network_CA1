#include "AudioOutput.h"
#include <QDebug>

AudioOutput::AudioOutput(QObject *parent)
    : QObject(parent), opusDecoder(nullptr) {


    audioFormat.setSampleRate(48000);
    audioFormat.setChannelCount(1);
    audioFormat.setSampleFormat(QAudioFormat::Int16);
    audioSink = new QAudioSink(QMediaDevices::defaultAudioOutput(), audioFormat, this);
    audioDevice = audioSink->start();

    int error;
    opusDecoder = opus_decoder_create(audioFormat.sampleRate(), audioFormat.channelCount(), &error);
    if (error != OPUS_OK) {
        qDebug() << "Failed to create Opus decoder:" << opus_strerror(error);
    }

    connect(this, &AudioOutput::newPacket, this, &AudioOutput::play);
}

void AudioOutput::addData(const QByteArray &data) {
    QMutexLocker locker(&mutex);
    qDebug() << "Adding data of size:" << data;
    audioQueue.enqueue(data);
    Q_EMIT newPacket();
}

void AudioOutput::play() {

    // QMutexLocker locker(mutex);
    while (!audioQueue.isEmpty()) {
        QByteArray audioData = audioQueue.dequeue();

        opus_int16 pcmData[960];
        int decodedSamples = opus_decode(opusDecoder, reinterpret_cast<const unsigned char *>(audioData.data()),
                                         audioData.size(), pcmData, 960, 0);

        if (decodedSamples < 0) {
            qDebug() << "Opus decoding error:" << opus_strerror(decodedSamples);
            continue;
        }

        if (audioDevice) {
            audioDevice->write(reinterpret_cast<const char *>(pcmData), decodedSamples * sizeof(opus_int16));
        }
    }
}
