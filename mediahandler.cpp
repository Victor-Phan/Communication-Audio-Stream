#include "mediahandler.h"


/*-----------------------------------------------------------------------------------------------------------------
-- Function:    set_audio
--
-- DATE:		April 8, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Nicole Jingco
--
-- PROGRAMMER: 	Nicole Jingco
--
-- INTERFACE:   file - file path
--
-- RETURNS:     void
--
-- NOTES:
-- sets the auido with the file path from user
-------------------------------------------------------------------------------------------------------------------*/
void MediaHandler::set_audio(QString file)
{
    getPlayer()->setMedia(QUrl::fromLocalFile(file));
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    play_audio
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
--  This function plays the audio file
-------------------------------------------------------------------------------------------------------------------*/
void MediaHandler::play_audio()
{
     getPlayer()->play();
     qDebug() << getPlayer() -> errorString();
}


/*-----------------------------------------------------------------------------------------------------------------
-- Function:    pause_audio
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
--  This function pauses the audio file
-------------------------------------------------------------------------------------------------------------------*/
void MediaHandler::pause_audio()
{
     getPlayer()->pause();
     qDebug() << getPlayer() -> errorString();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    stop_audio
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
--  This function stops the audio file
-------------------------------------------------------------------------------------------------------------------*/
void MediaHandler::stop_audio()
{
     getPlayer()->stop();
     qDebug() << getPlayer() -> errorString();
}

