#pragma once
#include <stdint.h>

#pragma pack(push, 1)

enum PacketID {
    ID_C_LOGIN = 1,
    ID_S_LOGIN_RESULT = 2,
    ID_C_MOVE = 3,           // 클라이언트 -> 서버 (이동 보고)
    ID_S_MOVE_BROADCAST = 4, // 서버 -> 클라이언트 (다른 유저 이동 알림)
    ID_S_ASSIGN_ROLE = 5     // 서버 -> 클라이언트 (역할 부여: 0추격자, 1도망자)
};

struct PacketHeader {
    uint16_t size;
    uint16_t id;
};

struct C_LOGIN {
    PacketHeader header;
    char playerID[32];
};

struct S_LOGIN_RESULT {
    PacketHeader header;
    bool bSuccess;
    int32_t playerIndex; // 서버가 부여한 고유 번호
};

struct P_MOVE_DATA {
    PacketHeader header;
    int32_t playerIndex;
    float x, y, z;
    float pitch, yaw, roll;
};

struct S_ASSIGN_ROLE {
    PacketHeader header;
    int32_t role; // 0: 추격자, 1: 도망자
};

#pragma pack(pop)
