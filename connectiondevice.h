#pragma once
#include <winsock2.h>
#include <windows.h>
#include <QString>
#include <QObject>
#include <QDateTime>
#include <QDebug>
#include <ws2tcpip.h>
#include "filehandler.h"
#include "audiodevice.h"

#define DATA_BUFSIZE 4000
#define PACKET_SIZE 64000
#define MAX_THREADS 100
#define MAX_FILENAME_SIZE 1024

#define FILE_PATH "./files/"
#define FILE_SUFFIX "Socket"
#define FILE_EXT ".wav"
#define FILE_NOT_EXIST "FILE_NOT_EXIST"
#define FILE_NOT_EXIST_LEN 14
#define DEFAULT_MULTICAST_ADDR "234.5.6.7"

class ConnectionDevice : public QObject
{
    Q_OBJECT
signals:
    void sendMessageToScreen(QString message);

public:
    enum protocol
    {
        UDP,
        TCP,
        UDP_CALL
    };
    DWORD threadArray[MAX_THREADS] = {};
    int threadIndex = 0;
    ConnectionDevice() = default;
    virtual ~ConnectionDevice() = default;
    static DWORD WINAPI readTCPPacketThread(LPVOID lpParameter);
    static void CALLBACK ReadSocketWorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
    static DWORD WINAPI sendTCPPackets(LPVOID lpParameter);
    bool readTCPPacket(SOCKET *socket);
    static void closeSocket(SOCKET &socket);
    bool startUpWSA()
    {
        WSADATA wsaData;
        return (WSAStartup(0x0202, &wsaData) == 0);
    }
    char *multicast_addr = DEFAULT_MULTICAST_ADDR;
    int multicast_ttl = 2;
};

/*The order of this struct DOES matter.*/
typedef struct _SOCKET_INFORMATION
{
    OVERLAPPED Overlapped;
    SOCKET Socket;
    CHAR Buffer[DATA_BUFSIZE];
    WSABUF DataBuf;
    ConnectionDevice *device;
    AudioDevice *audioPlayer;
    DWORD BytesRecv;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

struct TCPSendReceiveData
{
    ConnectionDevice *device;
    SOCKET *socket;
    bool expectResponse;
    std::string data;
    bool fileName;
    AudioDevice *audioPlayer;
};
