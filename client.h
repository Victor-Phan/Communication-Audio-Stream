#pragma once
#include <QDebug>
#include <winsock2.h>
#include <windows.h>
#include <pthread.h>
#include "server.h"
#include "connectiondevice.h"
#include "filehandler.h"


#define CLIENT_DATABUF_SIZE 4096

class Client : public ConnectionDevice
{

private:
    Client() = default;
    bool connected = false;
    struct sockaddr_in serverAddressInfo, clientAddressInfo;

    bool createSocket(protocol type);

    static DWORD WINAPI connectTCPServer(LPVOID lpParameter);
    static DWORD WINAPI joinMulticastStream(LPVOID lpParameter);
    static DWORD WINAPI joinCall(LPVOID lpParameter);

    LPSOCKET_INFORMATION getSocketInfo( AudioDevice *audioPlayer);

public:
    SOCKET clientSocket, serverSocket;
    HANDLE threadHandle, voiceHandle;

    static Client *getInstance()
    {
        static Client *client = new Client();
        return client;
    }
    void operator=(Client const &) = delete;
    ~Client() = default;
    void connectServer();
    bool disconnectClient();
    bool getConnected() const
    {
        return connected;
    }

    // Two Way Functions
    void connectCall();
    static DWORD WINAPI sendVoice(LPVOID lpParameter);
    void endCall();

    AudioDevice *clientAudioPlayer;
    AudioDevice *clientMicrophone;
    FileHandler *fileHandler;

    char recvBuf[CLIENT_DATABUF_SIZE];
    char sendBuf[CLIENT_DATABUF_SIZE];

    std::string rcvMsg;

    char *buf, *bufPtr;

    int port;
    std::string ip;
    std::string fileName;
    bool upload = false;
    bool isConnected = false;
    void joinStream();
    static void CALLBACK PlayStreamWorkerRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD InFlags);

public slots:
    void sendVoice(SOCKET, sockaddr_in, AudioDevice*);
};
