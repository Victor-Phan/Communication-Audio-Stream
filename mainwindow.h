#pragma once

#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include "server.h"
#include "client.h"
#include "mediahandler.h"
#include "audiodevice.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    //! Page Change
    void on_menu_svr_files_triggered();
    void on_menu_clnt_files_triggered();
    void on_menu_svr_stream_triggered();
    void on_menu_clnt_stream_triggered();
    void on_menu_clnt_voice_triggered();

    //! Volume Control
    void on_media_slider_vol_sliderMoved(int position);
    void on_media_btn_vol_clicked();
    void on_volumeChange(qint64 position);

    //! Media Control
    void on_media_btn_play_clicked();
    void on_media_btn_stop_clicked();
    void on_media_slider_progress_sliderMoved(int position);
    void on_durationChange(qint64 position);
    void on_progressChange(qint64 position);

    //! File Transfer
    void on_svr_files_btn_start_clicked();
    void on_clnt_files_btn_send_clicked();
    void printTCPClientMessage(QString message);
    void printTCPServerMessage(QString message);

    //! Streaming
    void on_svr_stream_btn_audio_file_clicked();
    void on_svr_stream_btn_start_clicked();
    void on_clnt_stream_btn_start_clicked();

    //! Mic
    void on_clnt_voice_btn_call_clicked();
    void on_clnt_voice_btn_accept_call_clicked();

    void on_clnt_files_input_btn_browse_clicked();

private:
    Ui::MainWindow *ui;
    QMediaPlayer *player;
    bool isPlaying = false;
    bool isRecording = false;
    bool isConnected = false;
    QString audio_file;
    AudioDevice *audioDevice;
    QFile sourceFile;
};
