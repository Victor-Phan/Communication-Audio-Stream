/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: 	server.cpp - This file contains the logic portions neccessary for a TCP or UDP Multicast Server
--                               to be created. Also contains logic to accept connections from a Client.
--
--
-- PROGRAM: 		Communication Audio Program
--
-- FUNCTIONS:
--                  DWORD createTCPServer(LPVOID lpParameter)
--                  void resetServerObj()
--                  void startServer()
--                  bool startUpWSA()
--                  bool acceptTCPConnections()
--                  bool shutDownServer()
--
-- DATE: 			March 20, 2020
--
-- REVISIONS:
--
-- DESIGNER: 		Victor Phan
--
-- PROGRAMMER: 		Victor Phan
--
-- NOTES:
--      The Client class is a singleton since there can only be one client per application.
--      The Server portion of the application allows the user to create a TCP or UDP Multicast Server.
--      A TCP server can accept connections and read packets.
--      The TCP Server can also send over a requested media file.
--      The TCP Server will send an error message to the client if the file cannot be found.
--      The Server will print out progress messages to the application.
--      Inherits methods and class variables from ConnectionDevice.
--
--------------------------------------------------------------------------------------------------------------------*/

#include "server.h"

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	createTCPServer
--
-- DATE:		March 20, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	DWORD createUDPServer(LPVOID lpParameter)
--                                    lpParameter - unused
--
-- RETURNS:     Returns true when reading is complete
--
-- NOTES:
--              Creates a socket to accept connections and read in TCP stream data.
--              Socket contains overlapped structure in order to read within a worker routine.
--
-------------------------------------------------------------------------------------------------------------------*/
DWORD Server::createTCPServer(LPVOID lpParameter) {
    if (!Server::getInstance()->startUpWSA()) {
        qDebug() << "WSAStartup failed with error \n" << Server::getInstance()->ret;
        return FALSE;
    }

    if ((Server::getInstance()->serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
                                                         WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
        qDebug() << "Failed to get a socket \n" << WSAGetLastError();
        WSACleanup();
        return FALSE;
    }

    Server::getInstance()->sockAddress.sin_family = AF_INET;
    Server::getInstance()->sockAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    Server::getInstance()->sockAddress.sin_port = htons((u_short)Server::getInstance()->port);

    if (bind(Server::getInstance()->serverSocket, (PSOCKADDR) &Server::getInstance()->sockAddress,
             sizeof(sockAddress)) == SOCKET_ERROR) {
        qDebug() << "bind() failed with error \n" << WSAGetLastError();
        return FALSE;
    }

    if (listen(Server::getInstance()->serverSocket, 5)) {
        qDebug() << "listen() failed with error \n" << WSAGetLastError();
        return FALSE;
    }
    emit Server::getInstance()->sendMessageToScreen("Started Server..");
    Server::getInstance()->acceptTCPConnections();
    return TRUE;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	createMulticastServer
--
-- DATE:		March 23, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Ellaine Chan
--
-- PROGRAMMER: 	Ellaine Chan
--
-- INTERFACE:	DWORD createUDPServer(LPVOID lpParameter)
--                                    lpParameter - unused
--
-- RETURNS:     Returns true when reading is complete
--
-- NOTES:
--              Creates a udp socket to multicast datagrams to all clients that are joined.
--              The audio file must already be selected or else this function will return false.
-------------------------------------------------------------------------------------------------------------------*/
DWORD Server::createMulticastServer(LPVOID lpParameter) {
    if(Server::getInstance()->streamFileName.size() < 1 ||
            !FileHandler::fileExists(Server::getInstance()->streamFileName)) {
        qDebug() << "file not selected \n";
        return FALSE;
    }
    if (!Server::getInstance()->startUpWSA()) {
        qDebug() << "WSAStartup failed with error \n" << Server::getInstance()->ret;
        return FALSE;
    }

    if ((Server::getInstance()->serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        qDebug() << "Failed to get a socket \n" << WSAGetLastError();
        WSACleanup();
        return FALSE;
    }
    memset(&Server::getInstance()->sockAddress, 0, sizeof(Server::getInstance()->sockAddress));
    Server::getInstance()->sockAddress.sin_family = AF_INET;
    Server::getInstance()->sockAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    Server::getInstance()->sockAddress.sin_port = 0;

    if (bind(Server::getInstance()->serverSocket, (PSOCKADDR) &Server::getInstance()->sockAddress,
             sizeof(sockAddress)) == SOCKET_ERROR) {
        qDebug() << "bind() failed with error \n" << WSAGetLastError();
        return FALSE;
    }

    Server::getInstance()->stMreq.imr_multiaddr.s_addr = inet_addr(Server::getInstance()->multicast_addr);
    Server::getInstance()->stMreq.imr_interface.s_addr = INADDR_ANY;
    if (setsockopt(Server::getInstance()->serverSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                   (char *)&Server::getInstance()->stMreq, sizeof(stMreq)) == SOCKET_ERROR) {
        qDebug() << "Failed to setsockopt to add membership \n" << WSAGetLastError();
    }

    if (setsockopt(Server::getInstance()->serverSocket, IPPROTO_IP, IP_MULTICAST_TTL,
                   (char *)&Server::getInstance()->multicast_ttl, sizeof(Server::getInstance()->multicast_ttl)) == SOCKET_ERROR) {
        qDebug() << "Failed to setsockopt to set ttl \n" << WSAGetLastError();
    };

    bool fFlag = FALSE;
    if (setsockopt(Server::getInstance()->serverSocket, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&fFlag, sizeof(fFlag)) == SOCKET_ERROR) {
        qDebug() << "Failed to setsockopt to disable loopback \n" << WSAGetLastError();
    }

    Server::getInstance()->multicastDestination.sin_family = AF_INET;
    Server::getInstance()->multicastDestination.sin_addr.s_addr = inet_addr(Server::getInstance()->multicast_addr);
    Server::getInstance()->multicastDestination.sin_port = htons((u_short)Server::getInstance()->port);

    Server::getInstance()->fileHandler = new FileHandler(Server::getInstance()->streamFileName);

    char streamBuffer[DATA_BUFSIZE];
    int read = Server::getInstance()->fileHandler->readFile(DATA_BUFSIZE,streamBuffer);
    int sent;
    if (read > 0) {
        Server::getInstance()->audioPlayer->playFromBufferSilent();
        Server::getInstance()->audioPlayer->addToPlayBuffer(QByteArray::fromRawData(streamBuffer, DATA_BUFSIZE));
        if((sent = sendto(Server::getInstance()->serverSocket, (char*)streamBuffer, sizeof(streamBuffer), 0, (struct sockaddr*) &Server::getInstance()->multicastDestination, sizeof(multicastDestination))) < 0) {
            perror("send to \n");
        }
    }
    return TRUE;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	sendNextChunk
--
-- DATE:		March 23, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Ellaine Chan
--
-- PROGRAMMER: 	Ellaine Chan
--
-- INTERFACE:	void sendNextChunk()
--
-- RETURNS:     void
--
-- NOTES:
--              This is the method that is called when the audio device has no more data to play.
--              This method will read the next chunk of data that the audio device should play, and write it to
--              the audio device to play.
-------------------------------------------------------------------------------------------------------------------*/
void Server::sendNextChunk(){
    qDebug() << "sending next chunk";
    int sent;
    char streamBuffer[DATA_BUFSIZE];
    if (Server::getInstance()->fileHandler->readFile(DATA_BUFSIZE,streamBuffer) > 0) {
        Server::getInstance()->audioPlayer->addToPlayBuffer(QByteArray::fromRawData(streamBuffer, DATA_BUFSIZE));
        if((sent = sendto(Server::getInstance()->serverSocket, (char*)streamBuffer, sizeof(streamBuffer), 0, (struct sockaddr*) &Server::getInstance()->multicastDestination, sizeof(multicastDestination))) < 0) {
            perror("send to \n");
        }
    } else {
        Server::getInstance()->audioPlayer->player->stop();
    }
};

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	accpetTCPConnectionThread
--
-- DATE:		March 20, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	DWORD accpetTCPConnectionThread(LPVOID lpParameter)
--                                    lpParameter - WSAEvent accpet event
--
-- RETURNS:     Returns true when reading is complete
--
-- NOTES:
--              Creates a socket to accept connections and read in TCP stream data.
--              Socket contains overlapped structure in order to read within a worker routine.
--
-------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI Server::acceptTCPConnectionThread(LPVOID lpParameter) {
    WSAEVENT EventArray[1];
    DWORD Index;
    EventArray[0] = (WSAEVENT) lpParameter;


    while(TRUE) {
        // Wait for accept() to signal an event and also process WorkerRoutine() returns.
        while(TRUE) {
            Index = WSAWaitForMultipleEvents(1, EventArray, FALSE, WSA_INFINITE, TRUE);
            if (Index == WSA_WAIT_FAILED) {
                qDebug() << "WSAWaitForMultipleEvents failed with error \n" << WSAGetLastError();
                return FALSE;
            }

            if (Index != WAIT_IO_COMPLETION) {
                // An accept() call event is ready - break the wait loop
                break;
            }
        }
        struct sockaddr saClient;
        int iClientSize = sizeof(saClient);
        Server::getInstance()->clientSocket[Server::getInstance()->clientConnectionIndex] = WSAAccept(Server::getInstance()->serverSocket, &saClient, &iClientSize, NULL,NULL);
        if(Server::getInstance()->clientSocket[Server::getInstance()->clientConnectionIndex] == INVALID_SOCKET) {
            qDebug() << "Socket Accept failure" << WSAGetLastError();
            return false;
        }
        //Intentionally setting index to the CURRENT socket
        int index = Server::getInstance()->clientConnectionIndex++;
        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss,zzz").append("");
        QString acceptMessage = QString("Client Connected on Socket: ")
                .append(QString::number(Server::getInstance()->clientSocket[index]))
                .append("\nTime: ")
                .append(currentTime);
        // Create a thread in to read in data grams for this client
        emit Server::getInstance()->sendMessageToScreen(acceptMessage);
        Server::getInstance()->readTCPPacket(&Server::getInstance()->clientSocket[Server::getInstance()->clientConnectionIndex - 1]);
        WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);
    }
    return TRUE;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	resetServerObj
--
-- DATE:		March 20, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	void resetServerObj()
--
-- RETURNS:     void
--
-- NOTES:
--
--              Resets the Server's member variables and threads.
--
-------------------------------------------------------------------------------------------------------------------*/
void Server::resetServerObj() {
    WSADATA resetWsaData;
    SOCKADDR_IN resetAddress;
    for(int i = 0; i < MAX_SERVER_THREADS; i++) {
        threadArray[i] = 0;
    }
    threadIndex = 0;
    wsaData = resetWsaData;
    sockAddress =  resetAddress;
    for(int i = 0; i < MAX_CLIENT_CONNECTIONS; i++) {
        clientSocket[i] = NULL;
    }
    serverSocket = NULL;
    ret = NULL;
    threadHandle= nullptr;
    acceptEvent = nullptr;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	startServer
--
-- DATE:		March 20, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	void startServer()
--
-- RETURNS:     void
--
-- NOTES:
--
--              Starts the Server on a new thread
--
-------------------------------------------------------------------------------------------------------------------*/
void Server::startServer(protocol pSelection) {
    //TODO: If TCPServer
    if(pSelection == protocol::TCP) {
        if ((threadHandle = CreateThread(NULL, 0, createTCPServer, NULL, 0, &threadArray[threadIndex++])) == NULL) {
            qDebug() << "CreateThread failed with error \n" << GetLastError();
            return;
        }
    } else if (pSelection == protocol::UDP) {
        if ((threadHandle = CreateThread(NULL, 0, createMulticastServer, NULL, 0, &threadArray[threadIndex++])) == NULL) {
            qDebug() << "CreateThread failed with error \n" << GetLastError();
            return;
        }
    } else {
        if ((threadHandle = CreateThread(NULL, 0, createCallReceiver, NULL, 0, &threadArray[threadIndex++])) == NULL) {
            qDebug() << "CreateThread failed with error \n" << GetLastError();
            return;
        }
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	acceptTCPConnections
--
-- DATE:		March 20, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	bool acceptTCPConnections()
--
-- RETURNS:     Return false when error occurs
--
-- NOTES:
--              This function is used in order to accept TCP connections.
--              When a client connections to the Server the server will begin to
--              read TCP stream data within another thread.
--              A message will be printed to the application when a client has connected.
--
-------------------------------------------------------------------------------------------------------------------*/
bool Server::acceptTCPConnections() {

    if ((acceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT) {
        qDebug() << "WSACreateEvent() failed with error \n" << WSAGetLastError();
        return false;
    }
    if(WSAEventSelect(serverSocket, acceptEvent, FD_ACCEPT) == SOCKET_ERROR) {
        qDebug() << "WSAEventSelect for accept failed: " << WSAGetLastError();
        return false;
    }
    // Create a worker thread to service completed I/O requests.

    if ((threadHandle = CreateThread(NULL, 0, acceptTCPConnectionThread, (LPVOID) acceptEvent, 0,&threadArray[threadIndex++])) == NULL) {
        qDebug() << "CreateThread failed with error \n" << GetLastError();
        return false;
    }
    return true;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	shutDownServer
--
-- DATE:		March 20, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	bool shutDownServer()
--
-- RETURNS:     Return true if shutdown is successful
--
-- NOTES:
--              This function will reset the Server's member variables, close open sockets, call WSACleanup, and
--              terminate any Server related threads.
--
-------------------------------------------------------------------------------------------------------------------*/
bool Server::shutDownServer() {
    int i = 0;
    if(serverSocket != 0) {
        closesocket(serverSocket);
    }
    for(int j = 0; j < MAX_CLIENT_CONNECTIONS; j++) {
        if(clientSocket[j] != 0) {
            closesocket(clientSocket[j]);
        }
    }
    WSACleanup();
    while(i < MAX_SERVER_THREADS) {
        if(threadArray[i] != 0) {
            TerminateThread(&threadArray[i],0);
        }
        i++;
    }
    resetServerObj();
    emit Server::getInstance()->sendMessageToScreen("Shut Down Server..");
    return true;
}


//! Mic --------------------------------------------------------------

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    createCallReceiver
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
--  Creates users socket and binds for two way audio
-------------------------------------------------------------------------------------------------------------------*/
DWORD Server::createCallReceiver(LPVOID lpParameter)
{
    int err;
    char recvBuff = MIC_BUFF;

    if (!Server::getInstance()->startUpWSA()) {
        qDebug() << "WSAStartup failed with error \n" << Server::getInstance()->ret;
        return FALSE;
    }

    if ((Server::getInstance()->serverSocket =  WSASocket(PF_INET, SOCK_DGRAM, 0, NULL,0,WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        qDebug() << "Failed to get a socket \n" << WSAGetLastError();
        WSACleanup();
        return FALSE;
    }

    Server::getInstance()->sockAddress.sin_family = PF_INET;
    Server::getInstance()->sockAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    Server::getInstance()->sockAddress.sin_port = htons((u_short)getInstance()->port);

    err = setsockopt(Server::getInstance()->serverSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&recvBuff, sizeof(recvBuff));

    if (bind(Server::getInstance()->serverSocket, (PSOCKADDR) &Server::getInstance()->sockAddress,
             sizeof(sockAddress)) == SOCKET_ERROR) {
        qDebug() << "bind() failed with error \n" << WSAGetLastError();
        return FALSE;
    }

    getInstance()->getVoice();


    return TRUE;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    getVoice
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
-- This function receives udp datagram from the client
-------------------------------------------------------------------------------------------------------------------*/
DWORD Server::getVoice()
{
    LPSOCKET_INFORMATION SocketInfo;
    AudioDevice *audioPlayer = getInstance()->audioDevice;
    DWORD Flags, Index, RecvBytes;
    WSAEVENT readEvent;
    struct sockaddr_in SenderAddr;
    int SenderAddrSize = sizeof (SenderAddr);

    Flags = 0;

    if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
                                                        sizeof(SOCKET_INFORMATION))) == NULL)
    {
        qDebug() << "GlobalAlloc() failed with error \n"
                 << GetLastError();
    }

    SocketInfo->Socket = Server::getInstance()->serverSocket;
    ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
    SocketInfo->DataBuf.len = MIC_BUFF;
    SocketInfo->DataBuf.buf = SocketInfo->Buffer;
    SocketInfo->audioPlayer = audioPlayer;
    SocketInfo->audioPlayer->playFromBuffer();

    if ((readEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        qDebug() << "WSACreateEvent() failed with error \n"
                 << WSAGetLastError();
        return 1;
    }

    if (WSAEventSelect(getInstance()->serverSocket, readEvent, FD_READ))
    {
        qDebug() << "Faled to tie event to socket";
        return 1;
    }

    SocketInfo->audioPlayer->playFromBuffer();

    while (Server::getInstance()->isReceiving)
    {
        Index = WSAWaitForMultipleEvents(1, &readEvent, FALSE, WSA_INFINITE, TRUE);

        if (WSARecvFrom(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
                        (SOCKADDR *) &SenderAddr, &SenderAddrSize, &(SocketInfo->Overlapped), PlayVoiceWorkerRoutine) < 0)
        {

        }
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    PlayVoiceWorkerRoutine
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
-- Worker function used to play audio received from getVoice
-------------------------------------------------------------------------------------------------------------------*/
void CALLBACK Server::PlayVoiceWorkerRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD InFlags)
{
    LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION)overlapped;
    if (error != 0)
    {
        qDebug() << "I/O operation failed with error: " << error;
    }

    if (error != 0 || bytesTransferred == 0)
    {
        //Close the socket
        QString message = QString("Read Complete on socket: ").append(QString::number(SI->Socket));
        emit SI->device->sendMessageToScreen(message);
        closeSocket(SI->Socket);
        GlobalFree(SI);
        return;
    }
    qDebug() << "received! Playing!";
    SI->audioPlayer->addToPlayBuffer(QByteArray::fromRawData(SI->Buffer, bytesTransferred));
}
