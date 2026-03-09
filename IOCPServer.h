#pragma once
#include <winsock2.h>
#include <vector>

enum class IO_TYPE { READ, WRITE };

struct OverlappedContext {
    WSAOVERLAPPED overlapped;
    WSABUF wsaBuf;
    char buffer[1024];
    IO_TYPE ioType;
};

struct Session {
    SOCKET socket;
};

// 함수 선언들
DWORD WINAPI WorkerThread(LPVOID lpParam);
void StartWorkerThreads(HANDLE hIOCP);
