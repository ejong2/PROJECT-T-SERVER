#pragma once

enum class EMessageID : int
{
    UNDEFINED = 0,        //��Ȯ��
    C2S_REQ_SIGNUP = 11001,    //Ŭ���̾�Ʈ_���� ȸ������ ��û �޽���
    C2S_REQ_LOGIN = 11002,    //Ŭ���̾�Ʈ_���� �α��� ��û �޽���

    S2C_RES_CLINET_CONNECT = 20001,    //����_Ŭ���̾�Ʈ ���� ���� �޽���
    S2C_RES_CLINET_DISCONNET = 20002,    //����_Ŭ���̾�Ʈ �������� ���� �޽���
};

enum class EProcessFlag : int
{
    UNDEFINED = 0,  //������
    PROCESS_OK = 1,  //ó���Ϸ�
    PROCESS_FAIL = 2   //ó������
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