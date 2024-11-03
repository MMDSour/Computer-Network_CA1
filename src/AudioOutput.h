#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <QObject>
#include <QAudioFormat>
#include <QAudioSink>
#include <QMediaDevices>
#include <QMutex>
#include <QQueue>
#include <QByteArray>
#include <opus.h>

class AudioOutput : public QObject {
    Q_OBJECT

public:
    explicit AudioOutput(QObject *parent = nullptr);
    void addData(const QByteArray &data);

Q_SIGNALS:
    void newPacket();

private Q_SLOTS:
    void play();

private:
    QAudioSink *audioSink;
    QIODevice *audioDevice;
    QAudioFormat audioFormat;
    QMutex mutex;
    QQueue<QByteArray> audioQueue;
    OpusDecoder *opusDecoder;
};

#endif
