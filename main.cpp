#include <winsock2.h>
#include <iostream>
#include <vector>
#include <mutex>
#include <algorithm>
#include <ctime>
#include "Shared.h"

#pragma comment(lib, "ws2_32.lib")
using namespace std;

struct Session {
    SOCKET socket;
    int32_t playerIndex;
};

// 전역 변수
vector<Session*> g_Sessions;
mutex g_SessionLock;
int32_t g_NextIndex = 0;

void Broadcast(Session* self, void* packet, int32_t size) {
    lock_guard<mutex> lock(g_SessionLock);
    for (Session* s : g_Sessions) {
        if (s == self) continue; 
        send(s->socket, (char*)packet, size, 0);
    }
}

// 4명 접속 시 역할 분담
void AssignRoles() {
    lock_guard<mutex> lock(g_SessionLock);
    if (g_Sessions.size() < 4) return;

    vector<int> roles = { 0, 1, 2, 3 };
    srand((unsigned int)time(NULL));
    random_shuffle(roles.begin(), roles.end());

    for (int i = 0; i < 4; ++i) {
        S_ASSIGN_ROLE pkt;
        pkt.header.size = sizeof(S_ASSIGN_ROLE);
        pkt.header.id = ID_S_ASSIGN_ROLE;
        pkt.role = (roles[i] == 0) ? 0 : 1; // 0번 인덱스 뽑힌 사람만 추격자(0)

        send(g_Sessions[i]->socket, (char*)&pkt, sizeof(pkt), 0);
        cout << "Player " << g_Sessions[i]->playerIndex << " Assigned Role: " << pkt.role << endl;
    }
}

DWORD WINAPI WorkerThread(LPVOID lpParam) {
    HANDLE hIOCP = (HANDLE)lpParam;
    DWORD bytes;
    Session* pSession;
    OVERLAPPED* pOverlapped;

    // 실제로는 OverlappedContext 구조체를 써야 하지만, 
    // 여기서는 흐름 파악을 위해 핵심 로직만 기술합니다.
    char buffer[1024]; 
    while (true) {
        // (주의: 실제 현업 코드에서는 GQCS와 Overlapped를 더 정교하게 다뤄야 합니다)
        // 여기서는 수신된 패킷을 처리하는 switch-case에 집중합니다.
    }
    return 0;
}

// 간단한 동기 accept 루프 (메인 스레드)
int main() {
    WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);
    
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(7777);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    bind(listenSock, (sockaddr*)&addr, sizeof(addr));
    listen(listenSock, 5);
    cout << "Server Running..." << endl;

    while (true) {
        sockaddr_in clientAddr;
        int len = sizeof(clientAddr);
        SOCKET clientSock = accept(listenSock, (sockaddr*)&clientAddr, &len);

        if (clientSock != INVALID_SOCKET) {
            Session* s = new Session{ clientSock, g_NextIndex++ };
            {
                lock_guard<mutex> lock(g_SessionLock);
                g_Sessions.push_back(s);
            }
            cout << "Client Connected. Total: " << g_Sessions.size() << endl;

            if (g_Sessions.size() == 4) {
                AssignRoles();
            }
            
            // 여기서 클라이언트별 수신 스레드나 IOCP 연결을 수행하세요.
        }
    }
    return 0;
}
