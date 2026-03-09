#include <winsock2.h>
#include <iostream>
#include <vector>
#include "IOCPServer.h"
#include "Shared.h"

// 1. 네트워크 라이브러리 연결 (이게 없으면 LNK 에러 발생)
#pragma comment(lib, "ws2_32.lib")

using namespace std;

// 2. Worker Thread의 실제 몸체 (구현부)
// 제 이전 답변에서 이 부분이 누락되어 에러가 발생한 것이 맞습니다.
DWORD WINAPI WorkerThread(LPVOID lpParam) {
    HANDLE hIOCP = (HANDLE)lpParam;
    DWORD bytesTransferred = 0;
    Session* pSession = nullptr;
    OverlappedContext* pContext = nullptr;

    while (true) {
        // IOCP 큐에서 완료된 작업 꺼내기
        BOOL result = GetQueuedCompletionStatus(
            hIOCP,
            &bytesTransferred,
            (PULONG_PTR)&pSession,
            (LPOVERLAPPED*)&pContext,
            INFINITE
        );

        if (result == FALSE || bytesTransferred == 0) {
            cout << "클라이언트 접속 종료" << endl;
            // 여기서 세션 정리 로직이 필요합니다.
            continue;
        }

        // 데이터 수신 성공 시 처리
        // WorkerThread 내부의 switch-case 예시
        if (pContext->ioType == IO_TYPE::READ) {
            PacketHeader* header = (PacketHeader*)pContext->buffer;

            switch (header->id) {
            case ID_C_LOGIN: {
                C_LOGIN* loginPkt = (C_LOGIN*)pContext->buffer;
                cout << "로그인 시도: " << loginPkt->playerID << endl;

                // 응답 패킷 만들기
                S_LOGIN_RESULT resPkt;
                resPkt.header.size = sizeof(S_LOGIN_RESULT);
                resPkt.header.id = ID_S_LOGIN_RESULT;
                resPkt.bSuccess = true; // 지금은 무조건 성공

                // 클라이언트에게 전송
                // (실제로는 WSASend를 쓰는 것이 좋으나 우선 간단히 전송)
                send(pSession->socket, (char*)&resPkt, sizeof(resPkt), 0);
                break;
            }
            }
        }
    }
    return 0;
}

int main() {
    // 윈소켓 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 0;

    // IOCP 핸들 생성
    HANDLE hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    // 스레드 생성
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    int threadCount = sysInfo.dwNumberOfProcessors * 2;
    for (int i = 0; i < threadCount; ++i) {
        CreateThread(NULL, 0, WorkerThread, hIOCP, 0, NULL);
    }

    // 리슨 소켓 설정
    SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    SOCKADDR_IN serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(7777);

    bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    listen(listenSocket, SOMAXCONN);

    cout << "Server Started on Port 7777..." << endl;

    while (true) {
        SOCKADDR_IN clientAddr;
        int addrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);

        if (clientSocket != INVALID_SOCKET) {
            cout << "New Client Connected!" << endl;

            Session* newSession = new Session{ clientSocket };
            OverlappedContext* recvContext = new OverlappedContext();
            memset(recvContext, 0, sizeof(OverlappedContext));
            recvContext->ioType = IO_TYPE::READ;
            recvContext->wsaBuf.buf = recvContext->buffer;
            recvContext->wsaBuf.len = sizeof(recvContext->buffer);

            // IOCP 등록
            CreateIoCompletionPort((HANDLE)clientSocket, hIOCP, (ULONG_PTR)newSession, 0);

            // 최초 수신 예약
            DWORD flags = 0;
            DWORD recvBytes = 0;
            WSARecv(clientSocket, &recvContext->wsaBuf, 1, &recvBytes, &flags, &recvContext->overlapped, NULL);
        }
    }

    WSACleanup();
    return 0;
}
