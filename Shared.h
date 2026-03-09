#pragma once
#include <stdint.h>

#pragma pack(push, 1)

enum PacketID {
    ID_C_LOGIN = 1,       // 클라이언트 -> 서버 (로그인 요청)
    ID_S_LOGIN_RESULT = 2, // 서버 -> 클라이언트 (로그인 결과)
    ID_C_MOVE = 3          // 이동 (기존 번호 수정 가능)
};

struct PacketHeader {
    uint16_t size;
    uint16_t id;
};

// 클라이언트가 서버로 보내는 데이터
struct C_LOGIN {
    PacketHeader header;
    char playerID[32]; // 아이디 (최대 32자)
};

// 서버가 클라이언트로 보내는 응답
struct S_LOGIN_RESULT {
    PacketHeader header;
    bool bSuccess;     // 로그인 성공 여부
};

#pragma pack(pop)
