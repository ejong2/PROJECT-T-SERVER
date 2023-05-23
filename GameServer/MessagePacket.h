#pragma once

enum class EMessageID : int
{
    UNDEFINED = 0,        //미확인
    C2S_REQ_SIGNUP = 11001,    //클라이언트_서버 회원가입 요청 메시지
    C2S_REQ_LOGIN = 11002,    //클라이언트_서버 로그인 요청 메시지

    S2C_RES_CLINET_CONNECT = 20001,    //서버_클라이언트 연결 응답 메시지
    S2C_RES_CLINET_DISCONNET = 20002,    //서버_클라이언트 연결해제 응답 메시지
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

struct MessageReqInsertPlayer
{
    MessageHeader MsgHead;
    char PLAYER_ID[30];
    char PLAYER_PWD[30];
    char PLAYER_NAME[30];
};

struct MessageResInsertPlayer
{
    MessageHeader MsgHead;
    int PROCESS_FLAG;
};

struct MessageReqLoginPlayer
{
    MessageHeader MsgHeader;
    char PLAYER_ID[30];
    char PLAYER_PWD[30];
};


struct MessageResLoginPlayer
{
    MessageHeader MsgHead;
    char PLAYER_ID[30];
    char PLAYER_PWD[30];
    char PLAYER_NAME[30];
    int PLAYER_GOLD;
    int PLAYER_EXP;
    int PLAYER_LEVEL;
    int PLAYERCHAR_TYPE;
    int PLAYERCHAR_BODY_SLOT;
    int PLAYERCHAR_HEAD_SLOT;
    int PLAYERCHAR_JUMP_STAT;
    int PLAYERCHAR_STAMINA_STAT;
    int PLAYERCHAR_SPEED_STAT;
};

struct MessageReqLogoutPlayer
{
    MessageHeader MsgHeader;
    char PLAYER_ID[30];
};


struct MessageResLogoutPlayer
{
    MessageHeader MsgHead;
    char LOGOUT_PLAYER_ID[30];
    int PROCESS_FLAG;
};


struct MessageReqSessionChattingLog
{
    MessageHeader MsgHead;
    int CHATCHANNEL_TYPE;
    int SESSION_ID;
    char PLAYER_ID[30];
    char PLAYER_NAME[30];
    char CHAT_MSG[100];
};

struct MessageResSessionChattingLog
{
    MessageHeader MsgHead;
    int CHATCHANNEL_TYPE;
    int SESSION_ID;
    char PLAYER_ID[30];
    char PLAYER_NAME[30];
    char CHAT_MSG[100];
};

struct MessageReqCreateSession
{
    MessageHeader MsgHead;
    char HOST_PLAYER_ID[30];
    char HOST_PLAYER_NAME[30];
    char SESSION_NAME[30];
    char SESSION_PASSWORD[30];
    char SESSION_MAPNAME[30];
    int SESSION_PLAYER;
};

struct MessageResCreateSession
{
    MessageHeader MsgHead;
    int SESSION_ID;
    char HOST_PLAYER_ID[30];
    char HOST_PLAYER_NAME[30];
    char SESSION_NAME[30];
    char SESSION_PASSWORD[30];
    char SESSION_MAPNAME[30];
    int SESSION_PLAYER;
    int SESSION_STATE;
};

struct MessageReqUpdatePlayerState
{
    MessageHeader MsgHead;
    char PLAYER_ID[30];
    char PLAYER_NAME[30];
    int PLAYERCHAR_TYPE;
    int PLAYERCHAR_BODY_SLOT;
    int PLAYERCHAR_HEAD_SLOT;
    int PLAYERCHAR_JUMP_STAT;
    int PLAYERCHAR_STAMINA_STAT;
    int PLAYERCHAR_SPEED_STAT;
};

struct MessageResUpdatePlayerState
{
    MessageHeader MsgHead;
    int PROCESS_FLAG;
};

struct MessageReqUpdatePlayerStateReward
{
    MessageHeader MsgHead;
    char PLAYER_ID[30];
    int EARN_PLAYER_GOLD;
    int EARN_PLAYER_EXP;
    int EARN_PLAYER_LEVEL;
};

struct MessageResUpdatePlayerStateReward
{
    MessageHeader MsgHead;
    char PLAYER_ID[30];
    int UPDATED_PLAYER_GOLD;
    int UPDATED_PLAYER_EXP;
    int UPDATED_PLAYER_LEVEL;
};


struct MessageReqSelectPlayerState
{
    MessageHeader MsgHeader;
    char PLAYER_ID[30];
};

struct MessageResSelectPlayerState
{
    MessageHeader MsgHead;
    char PLAYER_ID[30];
    char PLAYER_NAME[30];
    int PLAYER_GOLD;
    int PLAYER_EXP;
    int PLAYER_LEVEL;
    int PLAYERCHAR_TYPE;
    int PLAYERCHAR_BODY_SLOT;
    int PLAYERCHAR_HEAD_SLOT;
    int PLAYERCHAR_JUMP_STAT;
    int PLAYERCHAR_STAMINA_STAT;
    int PLAYERCHAR_SPEED_STAT;
};

#pragma pack(pop)