#ifndef MEDIAHANDLER_H
#define MEDIAHANDLER_H

#include <QObject>
#include <QMediaPlayer>
#include <QDebug>
#include <string.h>

using namespace std;

class MediaHandler: public QObject
{
private:
    MediaHandler() = default;
public:
    static QMediaPlayer* getPlayer(){
        static QMediaPlayer* player = new QMediaPlayer();
        return player;
    }

    void set_audio(QString file);
    void play_audio();
    void pause_audio();
    void stop_audio();

    void position_changed(qint64 position);
    void duration_changed(qint64 position);

};

#endif // MEDIAHANDLER_H
