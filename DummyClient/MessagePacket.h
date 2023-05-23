#pragma once

enum class EMessageID : int
{
    UNDEFINED                   = 0,        //미확인
    C2S_REQ_SIGNUP              = 11001,    //클라이언트_서버 회원가입 요청 메시지
    C2S_REQ_LOGIN               = 11002,    //클라이언트_서버 로그인 요청 메시지

    S2C_RES_CLINET_CONNECT      = 20001,    //서버_클라이언트 연결 응답 메시지
    S2C_RES_CLINET_DISCONNET    = 20002,    //서버_클라이언트 연결해제 응답 메시지

    S2C_REQ_SIGNUP              = 21001,    //서버_클라이언트 회원가입 요청 메시지
    S2C_REQ_LOGIN               = 21002,    //서버_클라이언트 로그인 요청 메시지
};

enum class EProcessFlag : int
{
    UNDEFINED = 0,  //미정의
    PROCESS_OK = 1,  //처리완료
    PROCESS_FAIL = 2   //처리실패
};

#pragma pack(push,1)

struct MessageHeader
{
    int MessageID;
    int MessageSize;
    int SenderSocketID;
    int ReceiverSocketID;
};

struct MessageResInsertPlayer
{
    MessageHeader MsgHead;
    int PROCESS_FLAG;
};

struct MessageReqSignup : public MessageHeader
{
    char USER_ID[32];
    char USER_PASSWORD[32];
};

struct MessageReqLogin : public MessageHeader
{
    char USER_ID[32];
    char USER_PASSWORD[32];
};


#pragma pack(pop)