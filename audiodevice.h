#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H
#include <QAudioOutput>
#include <QAudioInput>
#include <QAudioRecorder>
#include <QBuffer>
#include <QFile>
#include <QDebug>
#include <QObject>
#include <QUrl>
#include <string>
#include <iostream>
#include <QTimer>
#define REC_MAX_SIZE 5
#define BUFF_SIZE 4096
#include <QIODevice>
#include <QFile>
#include <QDebug>
#include <QObject>
#include <QBuffer>
#define DATA_BUFSIZE 4000
#define MIC_BUFF 1000

class AudioDevice: public QObject {
    Q_OBJECT
public:
    AudioDevice(QObject *parent = nullptr);
    QFile source_file_v2;
    QFile recording;
    void playFile(QString filename);
    void playFromBuffer();
    void playFromBufferSilent();
    void addToPlayBuffer(QByteArray buffer);

    QByteArray byteArray;
    int streaming;
     
    QAudioInput *mic;
    QAudioOutput *player;
    QIODevice *device;

    void pauseFile();
    void stopFile();

    void startRecording();
    void stopRecording();
  
    void startRecordingBuff();
    void stopRecordingBuff();
  
    QBuffer * getMicBuff();

    QBuffer *buf;
    QBuffer micBuf[MIC_BUFF];

    bool serverFirstPass = false;
    bool serverStreaming = false;
    bool playingFromFile = false;
  
private:
    QBuffer *qbuffer;

public slots:
    void handleStateChanged(QAudio::State newState);
    void handleMicStateChanged(QAudio::State newState);
  

signals:
    void finishedPlaying();
};

#endif // AUDIODEVICE_H
