/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: 	connectiondevice.cpp - This file contains the logic portions neccessary for a client or server
--                                         to read and send from the socket. The
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
--      ConnectionDevice is the base class for Server and Client. It contains functions related to sockets.
--      Such as reading from a socket, writing to a socket, and closing a socket.
--
--------------------------------------------------------------------------------------------------------------------*/
#include "connectiondevice.h"

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	closeSocket
--
-- DATE:		March 20, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	void closeSocket(SOCKET &socket)
--
-- RETURNS:     void
--
-- NOTES:
--
--              Shuts down communication for the socket. Then, closes down the specified socket.
--
-------------------------------------------------------------------------------------------------------------------*/
void ConnectionDevice::closeSocket(SOCKET &socket) {
    int iResult = shutdown(socket, SD_BOTH);
    qDebug() << "Closing Socket: " << socket;
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(socket);
    }
    closesocket(socket);
    socket = NULL;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	readTCPPacket
--
-- DATE:		March 20, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	bool readTCPPacket()
--
-- RETURNS:     Returns true if creation of read thread is successful
--
-- NOTES:
--
--              Creates a thread in order to read TCP stream data.
--
-------------------------------------------------------------------------------------------------------------------*/
bool ConnectionDevice::readTCPPacket(SOCKET* socket) {
    TCPSendReceiveData* options = new TCPSendReceiveData();
    options->device = this;
    options->socket = socket;
    if ((CreateThread(NULL, 0, readTCPPacketThread, options, 0,&threadArray[threadIndex++])) == NULL) {
        qDebug() << "CreateThread failed with error \n" << GetLastError();
        return false;
    }
    return true;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	ReadSocketWorkerRoutine
--
-- DATE:		March 20, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	void CALLBACK ReadSocketWorkerRoutine(DWORD Error, DWORD BytesTransferred,LPWSAOVERLAPPED Overlapped, DWORD InFlags)
--                                             Error - Error number
--                                             BytesTransferred - number of bytes read from socket
--                                             Overlapped - Contains the LPSOCKET_INFORMATION structure
--                                             InFlags - flags for completion routine
--
--
-- RETURNS:     void
--
-- NOTES:
--              Completion Routine for TCP and/or UDP that will be run when data is read from the socket buffer.
--              This function will print statistic data to the Console UI and write the data to a file.
--
-------------------------------------------------------------------------------------------------------------------*/
void CALLBACK ConnectionDevice::ReadSocketWorkerRoutine(DWORD error, DWORD bytesTransferred,LPWSAOVERLAPPED overlapped, DWORD InFlags) {
    // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
    LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) overlapped;
    if (error != 0) {
        qDebug() << "I/O operation failed with error: " << error;
    }

    if (error != 0 || bytesTransferred == 0) {
        //Close the socket
        QString message = QString("Read Complete on socket: ").append(QString::number(SI->Socket));
        emit SI->device->sendMessageToScreen(message);
        closeSocket(SI->Socket);
        GlobalFree(SI);
        return;
    }
    //Possibly can use thread pool here..
    char * buffer = new char[DATA_BUFSIZE];
    memcpy(buffer,SI->Buffer,bytesTransferred);
    //if file name..
    if(bytesTransferred <= MAX_FILENAME_SIZE && (strstr(buffer,FILE_EXT))) {
        //start sending thread..
        TCPSendReceiveData* options = new TCPSendReceiveData();
        char subbuff[bytesTransferred + 1];
        memcpy( subbuff, buffer,bytesTransferred);
        subbuff[bytesTransferred] = '\0';
        options->device = SI->device;
        options->socket = &(SI->Socket);
        options->expectResponse = false;
        options->data = std::string(subbuff);
        options->fileName = true;
        DWORD threadId;
        HANDLE threadHandle;
        QString fileName = QString(subbuff);
        emit SI->device->sendMessageToScreen(QString("Reading file name: ").append(fileName));
        if ((threadHandle = CreateThread(NULL, 0, &sendTCPPackets, options, 0, &threadId)) == NULL) {
            qDebug() << "CreateThread failed with error %d\n" << GetLastError();
        }

    } else if (bytesTransferred <= MAX_FILENAME_SIZE && strstr(buffer,FILE_NOT_EXIST)) {
        //alert file does not exist..
        qDebug() << "File does not exist on server";
        emit SI->device->sendMessageToScreen("File does not exist on server");
    } else {
        //TODO: Create a more unique name..
        emit SI->device->sendMessageToScreen("Saving data to file..");
        std::string filePath = FILE_PATH + std::to_string(SI->Socket) + FILE_SUFFIX + FILE_EXT;
        //Write to file
        FileHandler::saveDataToFile(filePath,buffer,bytesTransferred);
    }
    delete[] buffer;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	readTCPPacketThread
--
-- DATE:		March 20, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	DWORD readTCPPacketThread(LPVOID lpParameter)
--                                    lpParameter - unused
--
-- RETURNS:     Returns false if error occurs when creating event or reading socket
--
-- NOTES:
--              Calls WSARecv in order to read. Reading will occur in worker routine.
--              This thread is blocked by creating an event and waiting for the event
--              FD_READ to occur by using WSAWaitForMultipleEvents function.
--
-------------------------------------------------------------------------------------------------------------------*/
DWORD ConnectionDevice::readTCPPacketThread(LPVOID lpParameter) {
    //Make copy then delete
    TCPSendReceiveData options = *static_cast<TCPSendReceiveData*>(lpParameter);
    WSAEVENT readEvent;
    DWORD Index;
    if ((readEvent = WSACreateEvent()) == WSA_INVALID_EVENT) {
        qDebug() << "WSACreateEvent() failed with error \n" << WSAGetLastError();
        return false;
    }

    if(WSAEventSelect(*(options.socket), readEvent, FD_READ)) {
        qDebug() << "Faled to tie event to socket";
        return false;
    }

    while(TRUE) {
        DWORD Flags;
        LPSOCKET_INFORMATION SocketInfo;
        DWORD RecvBytes;

        if ((SocketInfo = (LPSOCKET_INFORMATION) GlobalAlloc(GPTR,
                          sizeof(SOCKET_INFORMATION))) == NULL) {
            qDebug() << "GlobalAlloc() failed with error \n" << GetLastError();
            return FALSE;
        }
        SocketInfo->Socket = *(options.socket);
        ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
        SocketInfo->DataBuf.len = DATA_BUFSIZE;
        SocketInfo->DataBuf.buf = SocketInfo->Buffer;
        SocketInfo->device = options.device;

        Flags = 0;
        Index = WSAWaitForMultipleEvents(1, &readEvent, FALSE, WSA_INFINITE, TRUE);
        if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
                    &(SocketInfo->Overlapped), ReadSocketWorkerRoutine) == SOCKET_ERROR) {
            int errCode = WSAGetLastError();
            if (errCode != WSA_IO_PENDING && errCode != 0) {
                qDebug() << "WSARecv() failed with error \n" << WSAGetLastError();
                if(!static_cast<TCPSendReceiveData*>(lpParameter)) {
                    delete static_cast<TCPSendReceiveData*>(lpParameter);
                }
                return FALSE;
            }
        }
        WSAResetEvent(readEvent);
    }
    if(!static_cast<TCPSendReceiveData*>(lpParameter)) {
        delete static_cast<TCPSendReceiveData*>(lpParameter);
    }
    return true;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	sendTCPPackets
--
-- DATE:		March 20, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	DWORD sendTCPPackets(LPVOID lpParameter)
--                      lpParameter - TCPSendReceiveData object that contains
--                                    relevent information about the packet(s) to be sent
--                                    This structure MUST be filled in.
--
-- RETURNS:     Returns TRUE if successful
--
-- NOTES:
--      This function sends TCP packet(s) containing data which can be a filename, file, or error message.
--
-------------------------------------------------------------------------------------------------------------------*/
DWORD ConnectionDevice::sendTCPPackets(LPVOID lpParameter) {
    TCPSendReceiveData options = *static_cast<TCPSendReceiveData*>(lpParameter);
    WSAOVERLAPPED overlapped;
    DWORD bytesSent;
    WSABUF buffer;
    buffer.buf = new char[PACKET_SIZE];
    buffer.len = PACKET_SIZE;
    ZeroMemory(&(overlapped), sizeof(WSAOVERLAPPED));
    if(options.fileName && FileHandler::fileExists(options.data)) {
        emit options.device->sendMessageToScreen("Sending file contents..");
        FileHandler fileHandler(options.data);
        bool sending = true;
        while(sending) {
            buffer.len = fileHandler.readFile(PACKET_SIZE,buffer.buf);
            if(buffer.len <= 0) {
                sending = false;
            } else {
                WSASend(*options.socket, &buffer,1, &bytesSent, NULL, &overlapped, NULL);
            }
        }
        //Close the connection
        options.device->closeSocket(*options.socket);

    } else if (options.expectResponse) {
        //Send the fileName
        emit options.device->sendMessageToScreen("Sending file name..");
        const char * tmpBuf = options.data.c_str();
        int len = options.data.length();
        memcpy(buffer.buf,tmpBuf,len);
        buffer.len = len;
        WSASend(*options.socket, &buffer,1, &bytesSent, NULL, &overlapped, NULL);
        //start receive..
        readTCPPacketThread(&options);
        emit options.device->sendMessageToScreen("Completed Reading..");

    } else if(options.fileName) {
        emit options.device->sendMessageToScreen("Sending file does not exist..");
        memcpy(buffer.buf,FILE_NOT_EXIST,FILE_NOT_EXIST_LEN);
        buffer.len = FILE_NOT_EXIST_LEN;
        WSASend(*options.socket, &buffer,1, &bytesSent, NULL, &overlapped, NULL);
        options.device->closeSocket(*(options.socket));

    } else {
        emit options.device->sendMessageToScreen("Sending data..");
        const char * tmpBuf = options.data.c_str();
        int len = options.data.length();
        memcpy(buffer.buf,tmpBuf,len);
        buffer.len = len;
        WSASend(*options.socket, &buffer,1, &bytesSent, NULL, &overlapped, NULL);
    }
    delete[] buffer.buf;
    if(!static_cast<TCPSendReceiveData*>(lpParameter)) {
        delete static_cast<TCPSendReceiveData*>(lpParameter);
    }
    return TRUE;
}
