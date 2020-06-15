#pragma once
#include "connectiondevice.h"
#include "filehandler.h"

#define MAX_CLIENT_CONNECTIONS 100
#define MAX_SERVER_THREADS 100

class Server : public ConnectionDevice {
    Q_OBJECT
private:
    Server() = default;
    static DWORD WINAPI createTCPServer(LPVOID lpParameter);
    static DWORD WINAPI createMulticastServer(LPVOID lpParameter);
    static DWORD WINAPI createCallReceiver(LPVOID lpParameter);

    static DWORD WINAPI acceptTCPConnectionThread(LPVOID lpParameter);

    DWORD getVoice();
    void resetServerObj();
    bool acceptTCPConnections();

public:
    SOCKET clientSocket[MAX_CLIENT_CONNECTIONS] = {};
    int clientConnectionIndex = 0;
    WSADATA wsaData;
    SOCKET serverSocket;
    SOCKADDR_IN sockAddress;
    SOCKADDR_IN multicastDestination;
    INT ret;
    HANDLE threadHandle;
    WSAEVENT acceptEvent;
    ip_mreq stMreq;
    int port;
    std::string streamFileName;

    AudioDevice *audioPlayer;
    AudioDevice *audioDevice;
    FileHandler *fileHandler;

    void startServer(protocol pSelection);
    bool isReceiving = false;

    static Server* getInstance() {
        static Server* server = new Server();
        return server;
    }
    void operator=(Server const&) = delete;
    ~Server() = default;

    void setAudioPlayer(AudioDevice * player) {
        audioPlayer = player;
    }
    static QString createPacketMessage(QString bytesReceived);

    bool shutDownServer();
    static void CALLBACK PlayVoiceWorkerRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD InFlags);

public slots:
    void sendNextChunk();
};
