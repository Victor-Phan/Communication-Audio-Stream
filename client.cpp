/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: 	client.cpp - This file contains the logic portions neccessary for a client to connect
--                               and send data to the Server application.
--
--
-- PROGRAM: 		Communication Audio Program
--
-- FUNCTIONS:
--                  bool createSocket()
--                  DWORD connectTCPServer(LPVOID lpParameter)
--                  bool connectServer()
--                  bool disconnectClient()
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
--      The Client portion of the application allows the user to connect to a specified Server via
--      IP or Host name. After we entered connect mode, we are able to send UDP or TCP packets. The Client class is
--      a singleton since there can only be one client per application.
--      Inherits methods and class variables from ConnectionDevice.
--
--
--------------------------------------------------------------------------------------------------------------------*/
#include "client.h"

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	createSocket
--
-- DATE:		February 12, 2020
--
-- REVISIONS:   Add argument for socket type - Ellaine Chan
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	bool createSocket()
--
-- RETURNS:     Returns true if socket creation is successful, uses TCPUDPOptions for params for socket creation.
--
-- NOTES:
--      Creates a socket and saves it to the Client object. Saves values such as the address of the server and port
--      to the client object.
--
-------------------------------------------------------------------------------------------------------------------*/
bool Client::createSocket(protocol type)
{
    if (!startUpWSA())
    {
        qDebug() << "WSAStartup failed with error %d\n"
                 << WSAGetLastError();
        return FALSE;
    }
    struct hostent *hp;
    const char *ipAddress = this->ip.c_str();

    if ((hp = gethostbyname(ipAddress)) == NULL)
    {
        qDebug() << "Unknown server address\n";
        return false;
    }

    //IF TCP
    if (type == protocol::TCP)
    {
        if ((clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
        {
            qDebug() << "Can't create a socket\n"
                     << WSAGetLastError();
            return false;
        }
    }
    else
    {
        if ((clientSocket = WSASocket(PF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
        {
            qDebug() << "Can't create a socket\n"
                     << WSAGetLastError();
            return false;
        }
    }

    memset((char *)&serverAddressInfo, 0, sizeof(serverAddressInfo));
    serverAddressInfo.sin_family = AF_INET;
    serverAddressInfo.sin_port = htons((u_short)this->port);
    memcpy((char *)&serverAddressInfo.sin_addr, hp->h_addr, hp->h_length);

    return true;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	connectTCPServer
--
-- DATE:		February 12, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	DWORD connectTCPServer(LPVOID lpParameter)
--                                     lpParameter - unused: params can be passed to thread if needed
--
-- RETURNS:     Returns true if connected to TCP Server is succesful
--
-- NOTES:
--      Progress messages are printed to the screen during the process of connection with timestamps.
--      This function calls WSAConnect in order to connect to the TCP Server.
--
-------------------------------------------------------------------------------------------------------------------*/
DWORD Client::connectTCPServer(LPVOID lpParameter)
{
    if (!Client::getInstance()->createSocket(protocol::TCP))
    {
        qDebug() << "Failure creating socket." << WSAGetLastError();
        return false;
    }
    Client::getInstance()->buf = (char *)malloc(CLIENT_DATABUF_SIZE);
    if (WSAConnect(Client::getInstance()->clientSocket,
                   (struct sockaddr *)&Client::getInstance()->serverAddressInfo,
                   sizeof(Client::getInstance()->serverAddressInfo), NULL, NULL, NULL, NULL) == SOCKET_ERROR)
    {
        qDebug() << "Can't connect to server: " << WSAGetLastError() << "\n";
        emit Client::getInstance()->sendMessageToScreen("Unable to connect to server..");
        return TRUE;
    }
    //Probably can shorten this call
    Client::getInstance()->connected = true;
    TCPSendReceiveData *options = new TCPSendReceiveData();
    options->socket = &Client::getInstance()->clientSocket;
    options->device = Client::getInstance();
    options->expectResponse = !Client::getInstance()->upload;
    options->data = Client::getInstance()->fileName;
    options->fileName = Client::getInstance()->upload;
    emit Client::getInstance()->sendMessageToScreen("Connected to Server..");
    if ((Client::getInstance()->threadHandle = CreateThread(NULL, 0, &sendTCPPackets, options, 0, &Client::getInstance()->threadArray[Client::getInstance()->threadIndex++])) == NULL)
    {
        qDebug() << "CreateThread failed with error %d\n"
                 << GetLastError();
        return FALSE;
    }
    return TRUE;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	connectServer
--
-- DATE:		February 12, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	void connectServer()
--
-- RETURNS:     void
--
-- NOTES:
--      This function connects to a TCP or UDP server. Creates a thread to connect to TCP server.
--
-------------------------------------------------------------------------------------------------------------------*/
void Client::connectServer()
{
    //IF TCP
    if (true)
    {
        if ((threadHandle = CreateThread(NULL, 0, connectTCPServer, NULL, 0, &threadArray[threadIndex++])) == NULL)
        {
            qDebug() << "CreateThread failed with error %d\n"
                     << GetLastError();
        }
    }
    else
    {
        //return connectUDPServer();
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	disconnectClient
--
-- DATE:		February 12, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	bool disconnectClient()
--
-- RETURNS:     Returns true if successful.
--
-- NOTES:
--      Resets the Client member variables.
--
-------------------------------------------------------------------------------------------------------------------*/
bool Client::disconnectClient()
{
    emit Client::getInstance()->sendMessageToScreen("Disconnecting Client..");
    int i = 0;
    if (clientSocket != 0)
    {
        closesocket(clientSocket);
    }
    WSACleanup();
    while (i < MAX_THREADS)
    {
        if (threadArray[i] != 0)
        {
            TerminateThread(&threadArray[i], 0);
            threadArray[i] = 0;
        }
        i++;
    }
    threadHandle = nullptr;
    threadIndex = 0;
    connected = false;
    emit Client::getInstance()->sendMessageToScreen("Disconnected..");
    return true;
}

//! Stream ----------------------------------------------------------------

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	joinStream
--
-- DATE:		April 3, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Ellaine Chan
--
-- PROGRAMMER: 	Ellaine Chan
--
-- INTERFACE:	void joinStream()
--
-- RETURNS:     void
--
-- NOTES:
--      Calls create socket to set up the udp socket and creates a new thread to join a multicast stream.
--
-------------------------------------------------------------------------------------------------------------------*/
void Client::joinStream()
{
    if (!startUpWSA())
    {
        qDebug() << "WSAStartup failed with error %d\n"
                 << WSAGetLastError();
        return;
    }

    Client::clientSocket = createSocket(protocol::UDP);


    if ((Client::getInstance()->threadHandle = CreateThread(NULL, 0, &joinMulticastStream, clientAudioPlayer, 0, &Client::getInstance()->threadArray[Client::getInstance()->threadIndex++])) == NULL)
    {
        qDebug() << "CreateThread failed with error %d\n"
                 << GetLastError();
        return;
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	joinMulticastStream
--
-- DATE:		April 3, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Ellaine Chan
--
-- PROGRAMMER: 	Ellaine Chan
--
-- INTERFACE:	DWORD WINAPI joinMulticastStream(LPVOID lpParameter)
--
-- RETURNS:     void
--
-- NOTES:
--      Sets up socket to join a multicast stream. It loops recvFrom to read chunks of audio data
--      from the multicast socket continuously. RecFrom uses a completion routine to process received data.
--
-------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI Client::joinMulticastStream(LPVOID lpParameter)
{
    AudioDevice *audioPlayer = (AudioDevice *)lpParameter;
    qDebug() << "in joinMulticastStream!";
    SOCKADDR_IN stLclAddr;
    struct ip_mreq stMreq;
    SOCKET hSocket;
    bool fFlag;
    int nRet;

    hSocket = socket(AF_INET,
                     SOCK_DGRAM,
                     0);
    if (hSocket == INVALID_SOCKET)
    {
        printf("socket() failed, Err: %d\n", WSAGetLastError());
        WSACleanup();
        exit(1);
    }
    fFlag = TRUE;
    nRet = setsockopt(hSocket,
                      SOL_SOCKET,
                      SO_REUSEADDR,
                      (char *)&fFlag,
                      sizeof(fFlag));
    if (nRet == SOCKET_ERROR)
    {
        printf("setsockopt() SO_REUSEADDR failed, Err: %d\n",
               WSAGetLastError());
    }
    stLclAddr.sin_family = AF_INET;
    stLclAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    stLclAddr.sin_port = htons(Client::getInstance()->port);
    nRet = bind(hSocket,
                (struct sockaddr *)&stLclAddr,
                sizeof(stLclAddr));
    if (nRet == SOCKET_ERROR)
    {
        printf("bind() port: %d failed, Err: %d\n", Client::getInstance()->port,
               WSAGetLastError());
    }

    /* Join the multicast group so we can receive from it */
    stMreq.imr_multiaddr.s_addr = inet_addr(DEFAULT_MULTICAST_ADDR);
    stMreq.imr_interface.s_addr = INADDR_ANY;
    nRet = setsockopt(hSocket,
                      IPPROTO_IP,
                      IP_ADD_MEMBERSHIP,
                      (char *)&stMreq,
                      sizeof(stMreq));
    if (nRet == SOCKET_ERROR)
    {
        printf(
                    "setsockopt() IP_ADD_MEMBERSHIP address %s failed, Err: %d\n",
                    DEFAULT_MULTICAST_ADDR, WSAGetLastError());
    }

    //set up completion routine read
    DWORD Flags, Index, RecvBytes;
    WSAOVERLAPPED Overlapped;
    WSAEVENT readEvent;
    LPSOCKET_INFORMATION SocketInfo;

    if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
                                                        sizeof(SOCKET_INFORMATION))) == NULL)
    {
        qDebug() << "GlobalAlloc() failed with error \n"
                 << GetLastError();
        return 1;
    }
    ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
    SocketInfo->Socket = hSocket;
    SocketInfo->DataBuf.len = DATA_BUFSIZE;
    SocketInfo->DataBuf.buf = SocketInfo->Buffer;
    SocketInfo->audioPlayer = audioPlayer;

    if ((readEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        qDebug() << "WSACreateEvent() failed with error \n"
                 << WSAGetLastError();
        return 1;
    }

    if (WSAEventSelect(hSocket, readEvent, FD_READ))
    {
        qDebug() << "Faled to tie event to socket";
        return 1;
    }
    SocketInfo->audioPlayer->playFromBuffer();
    qDebug() << "right before recvb!";
    while (true)
    {
        Flags = 0;
        Index = WSAWaitForMultipleEvents(1, &readEvent, FALSE, WSA_INFINITE, TRUE);
        if (WSARecvFrom(hSocket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
                        NULL, NULL, &(SocketInfo->Overlapped), PlayStreamWorkerRoutine) == SOCKET_ERROR)
        {
            printf("failed to read\n");
            GlobalFree(SocketInfo);
            closeSocket(hSocket);
            return -1;
        }
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	PlayStreamWorkerRoutine
--
-- DATE:		April 3, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Ellaine Chan
--
-- PROGRAMMER: 	Ellaine Chan
--
-- INTERFACE:	void PlayStreamWorkerRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD InFlags)
--
-- RETURNS:     void
--
-- NOTES:
--      Processes received audio data from multicast socket. Writes the received message into the audio device
--      to play. If there is an error in receiving it will close the socket.
--
-------------------------------------------------------------------------------------------------------------------*/
void CALLBACK Client::PlayStreamWorkerRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD InFlags)
{
    // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
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


//! Two Way Mic ------------------------------------------------------------

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    connectCall
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
-- Used to call the start of receiving voice messages fro Two Way Mic
-------------------------------------------------------------------------------------------------------------------*/
void Client::connectCall()
{
    if (!startUpWSA()) {
        qDebug() << "WSAStartup failed with error %d\n" << WSAGetLastError();
        return;
    }

    Client::clientSocket = createSocket(protocol::UDP);

    if ((Client::getInstance()->threadHandle = CreateThread(NULL, 0, &joinCall, clientMicrophone, 0, &Client::getInstance()->threadArray[Client::getInstance()->threadIndex++])) == NULL) {
        qDebug() << "CreateThread failed with error %d\n" << GetLastError();
        return;
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:    joinCall
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
-- This function creates a socekt, starts recording mic. it then reads the file the audio is sent to and
-- sends it to the client its connected too.
-------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI Client::joinCall(LPVOID lpParameter)
{
    AudioDevice* mic = (AudioDevice*) lpParameter;
    bool fFlag = TRUE;
    struct hostent *hp;
    const char *ipAddress = Client::getInstance()->ip.c_str();
    Client::getInstance()->fileHandler = new FileHandler("recording.wav");

    qDebug() << "in call!";
    SOCKADDR_IN addr;
    SOCKET sock;
    int err = 0;

    mic->startRecording();

    sock = socket(PF_INET , SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        printf ("socket() failed, Err: %d\n", WSAGetLastError());
        WSACleanup();
        exit(1);
    }

    err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&fFlag, sizeof(fFlag));
    if (err == SOCKET_ERROR) {
        printf ("setsockopt() SO_REUSEADDR failed, Err: %d\n",
                WSAGetLastError());
    }

    if ((hp = gethostbyname(ipAddress)) == NULL)
    {
        qDebug() << "Unknown server address\n";
        return false;
    }

    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(Client::getInstance()->port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memcpy((char *)&addr.sin_addr, hp->h_addr, hp->h_length);

    //! Send from File Record --------------------------

    FileHandler fileHandler("recording.wav");
    char streamBuffer[MIC_BUFF];
    int read = fileHandler.readFile(MIC_BUFF,streamBuffer);

    while (Client::getInstance()->isConnected)
    {
        err = sendto(sock, (char*)streamBuffer, sizeof(streamBuffer), 0, (struct sockaddr*) &addr, sizeof(addr));

        if(err < 0)
        {
            perror("send to \n");
        }
        read = fileHandler.readFile(MIC_BUFF,streamBuffer);
        Sleep(100);
    }
    closesocket(sock);
}


