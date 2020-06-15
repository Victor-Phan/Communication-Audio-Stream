#include "audiodevice.h"

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    AudioDevice
--
-- DATE:		March  23, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Ellaine Chan
--
-- PROGRAMMER: 	Ellaine Chan
--
-- INTERFACE:
--
-- RETURNS:     NA
--
-- NOTES:
--
-- Constructor for the AudioDevice class. Sets up the format of which the player should sample the audio file and
-- connects the signal and slots for when the device changes states (e.g., from active to idle).
-------------------------------------------------------------------------------------------------------------------*/
AudioDevice::AudioDevice(QObject *parent)
{
    QAudioFormat format;
    // Set up the format, eg.
    format.setSampleRate(8000);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format)) {
        qWarning() << "Raw audio format not supported by backend, cannot play audio.";
    }

    player = new QAudioOutput(format);
    mic = new QAudioInput(format, this);

    connect(player, &QAudioOutput::stateChanged, this, &AudioDevice::handleStateChanged);
    connect(mic, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleMicStateChanged(QAudio::State)));
}

//! Device Play  -------------------------------------------------------------------

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    playfile
--
-- DATE:		March  23, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Ellaine Chan
--
-- PROGRAMMER: 	Ellaine Chan
--
-- INTERFACE:   void playFile(QString filename)
--
-- RETURNS:     NA
--
-- NOTES:
--
-- Opens the user selected file and starts the audio device to play from the file. If file
-- does not exist or there is an error in opening it, returns an error message.
-------------------------------------------------------------------------------------------------------------------*/
void AudioDevice::playFile(QString filename) {
    playingFromFile = true;
    source_file_v2.setFileName(filename);
    if (!source_file_v2.open(QIODevice::ReadOnly)) {
        std::cout << "not opened properly";
    }
    player->start(&source_file_v2);
};

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    playFromBuffer
--
-- DATE:		March  23, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Ellaine Chan
--
-- PROGRAMMER: 	Ellaine Chan
--
-- INTERFACE:   void playFile(QString filename)
--
-- RETURNS:     NA
--
-- NOTES:
--
-- Sets up the audio device to start playing for a buffer instead of a file.
-------------------------------------------------------------------------------------------------------------------*/
void AudioDevice::playFromBuffer() {
    player->setBufferSize(DATA_BUFSIZE);
    device = player->start();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    startRecording
--
-- DATE:		March  30, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:
--
-- RETURNS:     NA
--
-- NOTES:
--
-- This function starts the recording by writing the mic input to an wav file
-------------------------------------------------------------------------------------------------------------------*/
void AudioDevice::playFromBufferSilent() {
    serverStreaming = true;
    serverFirstPass = true;
    player->setVolume(0);
    player->setBufferSize(DATA_BUFSIZE);
    device = player->start();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    addToPlayBuffer
--
-- DATE:		March  23, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Ellaine Chan
--
-- PROGRAMMER: 	Ellaine Chan
--
-- INTERFACE:   void addToPlayBuffer(QByteArray buffer)
--
-- RETURNS:     NA
--
-- NOTES:
--
-- Writes new audio data for the device to play.
-------------------------------------------------------------------------------------------------------------------*/
void AudioDevice::addToPlayBuffer(QByteArray buffer) {
        streaming = 1;
        byteArray = buffer;
        device->write(byteArray);
        device->seek(0);
}

//! Microphone Record -------------------------------------------------------------------

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    startRecording
--
-- DATE:		March  30, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:
--
-- RETURNS:     NA
--
-- NOTES:
--
-- This function starts the recording by writing the mic input to an wav file
-------------------------------------------------------------------------------------------------------------------*/
void AudioDevice::startRecording()
{
    recording.setFileName("recording.wav");
    recording.open(QIODevice::WriteOnly | QIODevice::Truncate);

    mic->start(&recording);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    stopRecording
--
-- DATE:		March  30, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:
--
-- RETURNS:     NA
--
-- NOTES:
--
-- This function stops the recording
-------------------------------------------------------------------------------------------------------------------*/
void AudioDevice::stopRecording()
{
    mic->stop();
    recording.close();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    getMicBuff
--
-- DATE:		March  30, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:   NA
--
-- RETURNS:     QBuffer * to mic buffer
--
-- NOTES:
--
-- this function retiurns the pointer to the mic buffer
-------------------------------------------------------------------------------------------------------------------*/
QBuffer * AudioDevice::getMicBuff()
{
    return micBuf;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    startRecordingBuff
--
-- DATE:		March  30, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:
--
-- RETURNS:     NA
--
-- NOTES:
--
-- This function records from mic to Qbuffer
-------------------------------------------------------------------------------------------------------------------*/
void AudioDevice::startRecordingBuff()
{
    micBuf->open(QIODevice::WriteOnly| QIODevice::Truncate);
    mic->start(micBuf);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    stopRecordingBuff
--
-- DATE:		March  30, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:   NA
--
-- RETURNS:     NA
--
-- NOTES:
--
-- This function stops mic from recording
-------------------------------------------------------------------------------------------------------------------*/
void AudioDevice::stopRecordingBuff()
{
    mic->stop();
}

//! Slots States -------------------------------------------------------------------

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    handleStateChanged
--
-- DATE:		March  27, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Ellaine Chan
--
-- PROGRAMMER: 	Ellaine Chan
--
-- INTERFACE:   void handleStateChanged(QAudio::State newState)
--
-- RETURNS:     NA
--
-- NOTES:
--
-- This function
-------------------------------------------------------------------------------------------------------------------*/
void AudioDevice::handleStateChanged(QAudio::State newState)
{
    switch (newState) {
        case QAudio::IdleState:
            // Finished playing (no more data)
            if(playingFromFile == true) {
                qDebug() << "stoping player";
                player->stop();
                source_file_v2.close();
                break;
            }

            //if streaming, signal to server that current data chunk has finished playing
            //AudioDevice goes to idle between starting and the server is able to send the first
            //chunk so only send signal after first state update
            if(playingFromFile != true && serverStreaming == true) {
                if (serverFirstPass == false) {
                    emit finishedPlaying();
                } else {
                    serverFirstPass = false;
                }
            }
        break;

        //if streaming, signal to server that current data chunk has finished playing
        //AudioDevice goes to idle between starting and the server is able to send the first
        //chunk so only send signal after first state update
        if(playingFromFile != true && serverStreaming == true) {
            if (serverFirstPass == false) {
                //emit finishedPlaying();
            } else {
                serverFirstPass = false;
            }
        }
        break;

    case QAudio::StoppedState:
        // Stopped for other reasons
        if (player->error() != QAudio::NoError) {
            // Error handling
            qDebug() << player->error();
        }
        qDebug() << "player stopped";

        break;
    default:
        // ... other cases as appropriate
        break;
    }
}

void AudioDevice::handleMicStateChanged(QAudio::State newState)
{
    switch (newState)
    {
    case QAudio::StoppedState:
        if(mic->error() != QAudio::NoError)
        {
            //Error handling
        } else
        {
            //Finished recordin
        }
        break;
    case QAudio::ActiveState:
        //Started recording, read from IO Device
        break;

    default:
        break;
    }
}

