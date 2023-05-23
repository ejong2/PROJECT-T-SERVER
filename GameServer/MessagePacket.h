#pragma once

enum class EMessageID : int
{
    UNDEFINED                   = 0,        //��Ȯ��
    C2S_REQ_SIGNUP              = 11001,    //Ŭ���̾�Ʈ_���� ȸ������ ��û �޽���
    C2S_REQ_LOGIN               = 11002,    //Ŭ���̾�Ʈ_���� �α��� ��û �޽���

    S2C_RES_CLINET_CONNECT      = 20001,    //����_Ŭ���̾�Ʈ ���� ���� �޽���
    S2C_RES_CLINET_DISCONNET    = 20002,    //����_Ŭ���̾�Ʈ �������� ���� �޽���

    S2C_REQ_SIGNUP              = 21001,    //����_Ŭ���̾�Ʈ ȸ������ ��û �޽���
    S2C_REQ_LOGIN               = 21002,    //����_Ŭ���̾�Ʈ �α��� ��û �޽���
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