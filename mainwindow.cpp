#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow),
    audioDevice(new AudioDevice(this)) {
    ui->setupUi(this);
    ui->widgets->setCurrentIndex(0);

    //Connects the statistical messages signals to the MainWindow
    connect(Server::getInstance(), &Server::sendMessageToScreen, this, &MainWindow::printTCPServerMessage);
    connect(Client::getInstance(), &Client::sendMessageToScreen, this, &MainWindow::printTCPClientMessage);

    connect(MediaHandler::getPlayer(), &QMediaPlayer::positionChanged, this, &MainWindow::on_progressChange);
    connect(MediaHandler::getPlayer(), &QMediaPlayer::durationChanged, this, &MainWindow::on_durationChange);
    connect(audioDevice, &AudioDevice::finishedPlaying, Server::getInstance(), &Server::sendNextChunk);

}

MainWindow::~MainWindow() {
    delete ui;
}

//! Page Changer ----------------------------------------------------------------------------------------------------------

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    on_menu_svr_stream_triggered
--
-- DATE:		April 8, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:   NA
--
-- RETURNS:     void
--
-- NOTES:
--
--  Changes the UI to the Server Stream UI
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_menu_svr_stream_triggered() {
    ui->widgets->setCurrentIndex(1);
    ui->frame->setVisible(false);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    on_menu_clnt_stream_triggered
--
-- DATE:		April 8, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:   NA
--
-- RETURNS:     void
--
-- NOTES:
--
-- Changes the UI to the Client Stream
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_menu_clnt_stream_triggered() {
    ui->widgets->setCurrentIndex(2);
    ui->media_slider_vol->setVisible(false);
    ui->frame->setVisible(false);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    on_menu_svr_files_triggered
--
-- DATE:		April 8, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:   NA
--
-- RETURNS:     void
--
-- NOTES:
--
-- Changes the UI to the Server File Transfer UI
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_menu_svr_files_triggered() {
    ui->widgets->setCurrentIndex(3);
    ui->frame->setVisible(true);

}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    on_menu_clnt_files_triggered
--
-- DATE:		April 8, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:   NA
--
-- RETURNS:     void
--
-- NOTES:
--
-- Changes the UI to the Client File Transfer UI
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_menu_clnt_files_triggered() {
    ui->widgets->setCurrentIndex(4);
    ui->frame->setVisible(true);

}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    on_menu_clnt_voice_triggered
--
-- DATE:		April 8, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:   NA
--
-- RETURNS:     void
--
-- NOTES:
--
-- Changes the UI to the  Voice Call UI
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_menu_clnt_voice_triggered() {
    ui->widgets->setCurrentIndex(5);
    ui->frame->setVisible(false);
}

//! Volume Changer ---------------------------------------------------------------------------------------------------------

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    on_media_slider_vol_sliderMoved
--
-- DATE:		April 8, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:   int position - position of the volume slider
--
-- RETURNS:     void
--
-- NOTES:
--
--  Adjust the volume icon
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_media_slider_vol_sliderMoved(int position) {
    MediaHandler::getPlayer()->setVolume(position);

    // change volume on speaker
    if (position <= 0) {
        // chnage icon to mute
        ui->media_btn_vol->setIcon(QIcon(":/icons/mute.png"));
    } else if (position > 0 && position < 33) {
        ui->media_btn_vol->setIcon(QIcon(":/icons/low-volume.png"));
    } else if (position > 33 && position < 66) {
        ui->media_btn_vol->setIcon(QIcon(":/icons/medium-volume.png"));
    } else {
        ui->media_btn_vol->setIcon(QIcon(":/icons/high-volume.png"));
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    on_volumeChange
--
-- DATE:		April 8, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:   qint64 position - position of the volume sider
--
-- RETURNS:     void
--
-- NOTES:
--
-- changes the volume
-------------------------------------------------------------------------------------------------------------------*/
//
void MainWindow::on_volumeChange(qint64 position) {
    MediaHandler::getPlayer()->setVolume(position);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    on_media_btn_vol_clicked
--
-- DATE:		April 8, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:
--
-- RETURNS:     void
--
-- NOTES:
--
--  Volume Slider Toggle
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_media_btn_vol_clicked() {
    if (ui->media_slider_vol->isVisible()) {
        ui->media_slider_vol->setVisible(false);
    } else {
        ui->media_slider_vol->setVisible(true);
    }
}

//! Media Control ----------------------------------------------------------------------------------------------------------
/*-----------------------------------------------------------------------------------------------------------------
-- Function:    on_media_btn_play_clicked
--
-- DATE:		April 8, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:   NA
--
-- RETURNS:     void
--
-- NOTES:
--
-- Plays the file
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_media_btn_play_clicked() {
    if (audio_file != NULL) {
        if (isPlaying) {
            isPlaying = false;
            MediaHandler::getPlayer()->pause();
            ui->media_btn_play->setIcon(QIcon(":/icons/play.png"));
            qDebug() << MediaHandler::getPlayer()->errorString();
        } else {
            isPlaying = true;
            audioDevice->playFile(audio_file); //testing - can remove later
            ui->media_btn_play->setIcon(QIcon(":/icons/pause.png"));
            qDebug() << MediaHandler::getPlayer()->errorString();
        }
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    on_media_btn_stop_clicked
--
-- DATE:		April 8, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:   NA
--
-- RETURNS:     void
--
-- NOTES:
--
-- Stops the audio player
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_media_btn_stop_clicked() {
    isPlaying = false;
    MediaHandler::getPlayer()->stop();
    ui->media_btn_play->setIcon(QIcon(":/icons/play.png"));
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    on_media_slider_progress_sliderMoved
--
-- DATE:		April 8, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:   int position - current time position
--
-- RETURNS:     void
--
-- NOTES:
--
-- change the position by
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_media_slider_progress_sliderMoved(int position) {
    MediaHandler::getPlayer()->setPosition(position);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    on_durationChange
--
-- DATE:		April 8, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:   NA
--
-- RETURNS:     void
--
-- NOTES:
--
--  Audio time
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_durationChange(qint64 position) {
    ui->media_slider_progress->setMaximum(position);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    on_progressChange
--
-- DATE:		April 8, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:   NA
--
-- RETURNS:     void
--
-- NOTES:
--
-- Moves player slider to the audio file position
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_progressChange(qint64 position) {
    ui->media_slider_progress->setValue(position);
}

//! File Transfer ----------------------------------------------------------------------------------------------------------

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	printTCPClientMessage
--
-- DATE:		March 20, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	void printTCPClientMessage(QString message)
--                  message - message to be printed to the screen
--
-- RETURNS:     void
--
-- NOTES:
--              Prints the message to the screen for the user to see.
--
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::printTCPClientMessage(QString message) {
    ui->clnt_files_box_activity->append(message);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	printTCPServerMessage
--
-- DATE:		March 20, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	void printTCPServerMessage(QString message)
--                  message - message to be printed to the screen
--
-- RETURNS:     void
--
-- NOTES:
--              Prints the message to the screen for the user to see.
--
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::printTCPServerMessage(QString message) {
    ui->svr_files_box_activity->append(message);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	on_svr_files_btn_start_clicked
--
-- DATE:		March 20, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	void on_svr_files_btn_start_clicked()
--
-- RETURNS:     void
--
-- NOTES:
--              Starts the server on the specified port when the button is clicked
--
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_svr_files_btn_start_clicked() {
    int port = ui->svr_files_txt_port->toPlainText().toInt();
    if (port == 0) {
        qDebug() << "Unable to parse port";
    } else {
        Server::getInstance()->port = port;
        Server::getInstance()->startServer(ConnectionDevice::protocol::TCP);
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	on_svr_stream_btn_start_clicked
--
-- DATE:		March 22, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Ellaine Chan
--
-- PROGRAMMER:  Ellaine Chan
--
-- INTERFACE:	void on_svr_stream_btn_start_clicked()
--
-- RETURNS:     void
--
-- NOTES:
--              Starts the server to stream the selected audio file through multicast
--
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_svr_stream_btn_start_clicked() {
    int port = ui->svr_stream_input_port->text().toInt();
    if (port == 0) {
        qDebug() << "Unable to parse port";
    } else {
        Server::getInstance()->port = port;
        Server::getInstance()->setAudioPlayer(audioDevice);
        Server::getInstance()->startServer(ConnectionDevice::protocol::UDP);
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	on_clnt_files_btn_send_clicked
--
-- DATE:		March 20, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	void on_clnt_files_btn_send_clicked()
--
-- RETURNS:     void
--
-- NOTES:
--              When the Client sends a request to the server, this function will grab the
--              ip, port, filename, and client request. It will hen try to connect
--              to the server and do the requested action (upload/download).
--
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_clnt_files_btn_send_clicked() {
    int port = ui->clnt_files_input_port->text().toInt();
    std::string ipAddress = ui->clnt_files_input_ip->text().toStdString();
    std::string fileName = ui->clnt_files_input_file->text().toStdString();
    bool upload = ui->clnt_files_input_rb_upload->isChecked();
    if (fileName.find(".wav") == std::string::npos) {
        //Invalid file name
        qDebug() << "Enter a valid file.";
        Client::getInstance()->sendMessageToScreen("Enter a valid file.");
        return;
    }
    if (port == 0) {
        qDebug() << "Unable to parse port";
    } else {
        Client::getInstance()->port = port;
        Client::getInstance()->ip = ipAddress;
        Client::getInstance()->fileName = fileName;
        Client::getInstance()->upload = upload;
        Client::getInstance()->connectServer();
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	on_clnt_files_input_btn_browse_clicked
--
-- DATE:		April 8, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	void on_clnt_files_input_btn_browse_clicked()
--
-- RETURNS:     void
--
-- NOTES:
--              Opens file explorer for the client to select a file. Appends the path to the input box.
--
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_clnt_files_input_btn_browse_clicked() {
    audio_file = QFileDialog::getOpenFileName(this, "Open a file", "directoryToOpen",
                 "Audio Files (*.mp3 *.wav)");
    ui->clnt_files_input_file->setText(audio_file);
}

//! Streaming ----------------------------------------------------------------------------------------------------------

void MainWindow::on_svr_stream_btn_audio_file_clicked() {
    audio_file = QFileDialog::getOpenFileName(this, "Open a file", "directoryToOpen",
                 "Audio Files (*.mp3 *.wav)");
    ui->svr_stream_input_audio_file->setText(audio_file);
    ui->media_lab_txt_curr_song->setText(audio_file);
    Server::getInstance()->streamFileName = audio_file.toLocal8Bit().constData();
    MediaHandler::getPlayer()->setMedia(QUrl::fromLocalFile(audio_file));
    //audioDevice->playFile(audio_file); //testing - can remove later
}

void MainWindow::on_clnt_stream_btn_start_clicked() {
    int port = ui->clnt_stream_input_port->text().toInt();
    std::string ipAddress = ui->clnt_stream_input_ip->text().toStdString();
    Client::getInstance()->clientAudioPlayer = audioDevice;
    Client::getInstance()->port = port;
    Client::getInstance()->ip = ipAddress;
    Client::getInstance()->joinStream();
}

//! Two Way Mic ---------------------------------------------------------------------------------------------------------

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    on_clnt_voice_btn_accept_call_clicked
--
-- DATE:		April 8, 2020
--
-- REVISIONS:   NA
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:   NA
--
-- RETURNS:     void
--
-- NOTES:
--
-- Allows the user to receive calls with the port specified by the user
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_clnt_voice_btn_accept_call_clicked() {
    if (!Server::getInstance()->isReceiving)
    {
        Server::getInstance()->audioDevice = audioDevice;
        Server::getInstance()->port = ui->clnt_voice_txt_your_port->text().toInt();
        Server::getInstance()->isReceiving= true;
        Server::getInstance()->startServer(ConnectionDevice::protocol::UDP_CALL);
        ui->clnt_voice_btn_accept_call->setText("Block Calls");

    }
    else
    {
        Server::getInstance()->isReceiving= false;
        ui->clnt_voice_btn_accept_call->setText("Accept Calls");

    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    on_clnt_voice_btn_call_clicked
--
-- DATE:		April 8, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:   NA
--
-- RETURNS:     void
--
-- NOTES:
--  Connects user to the other client using the port and ip the user inputs
-------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_clnt_voice_btn_call_clicked() {
    if (isConnected) {
        isConnected = false;
        Client::getInstance()->isConnected = false;
        ui->clnt_voice_btn_call->setText("Call");

    } else {
        ui->clnt_voice_btn_call->setText("Hangup");

        string ip = ui->clnt_voice_txt_ip->text().toStdString();
        int port = ui->clnt_voice_txt_port->text().toInt();

        Client::getInstance()->clientMicrophone = audioDevice;
        Client::getInstance()->ip = ip;
        Client::getInstance()->port = port;
        Client::getInstance()->isConnected = true;
        isConnected = true;
        Client::getInstance()->connectCall();
    }
}
