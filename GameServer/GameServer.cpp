//#define _RUN_AS_LOCALHOST

#define _WINSOCK_DEPRECATED_NO_WARNINGS
//--C Header------------------------------------------------------------------
#include <WinSock2.h> 
#include <mswsock.h>
#include <ws2tcpip.h>
#include <Windows.h>
//--C++ Header----------------------------------------------------------------
#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <mutex>
//--Custom Header-------------------------------------------------------------
#include "StringHelper.h"
#include "MessagePacket.h"
#include "ClientData.h"
//--Plugin Header-------------------------------------------------------------
#include "jdbc/mysql_connection.h"
#include "jdbc/cppconn/driver.h"
#include "jdbc/cppconn/exception.h"
#include "jdbc/cppconn/prepared_statement.h"
//--Plugin Library------------------------------------------------------------
#pragma comment (lib, "WS2_32.lib")
#pragma comment (lib, "debug/mysqlcppconn.lib")
//--Network-------------------------------------------------------------------
WSAData                      NET_WSADATA = { 0, };
SOCKET                       NET_SERVERSOCKET = NULL;
SOCKADDR_IN                  NET_SERVERADDR = { 0, };
#ifdef _RUN_AS_LOCALHOST
const char* NET_SERVER_IPV4 = "127.0.0.1";
#else
const char* NET_SERVER_IPV4 = "192.168.0.10";
#endif 
const int                    NET_SERVER_PORT = 5001;
const int                    NET_PACKET_SIZE = 512;
std::map<SOCKET, ClientData> CLIENT_POOL;
std::mutex                   MUTEX_NETWORK_HANDLER;
//--Thread--------------------------------------------------------------------       
std::map<SOCKET, std::thread> THREAD_POOL;
std::mutex                    MUTEX_THREAD_HANDLER;
//--DBMS----------------------------------------------------------------------
#ifdef _RUN_AS_LOCALHOST
const std::string            DB_SERVERNAME = "tcp://127.0.0.1:3306";
const std::string            DB_USERNAME = "root";
const std::string            DB_PASSWORD = "1234";
#else
const std::string            DB_SERVERNAME = "tcp://127.0.0.1:3306";
const std::string            DB_USERNAME = "root";
const std::string            DB_PASSWORD = "1234";
#endif 
const std::string            DB_DBNAME = "PROJECT_T";
sql::Driver* DB_DRIVER = nullptr;
sql::Connection* DB_CONN = nullptr;
sql::Statement* DB_STMT = nullptr;
sql::PreparedStatement* DB_PSTMT = nullptr;
sql::ResultSet* DB_RS = nullptr;
std::mutex                   MUTEX_DB_HANDLER;
//--MainProgram---------------------------------------------------------------
bool                         G_PROGRAMRUNNING = true;

using namespace std;

// �޼��� ���� ����
sql::ResultSet* ExecuteQuery(const std::string& sqlQuery, const std::vector<std::string>& params);
int ExecuteUpdate(const std::string& sqlQuery, const std::vector<std::string>& params);
bool HandleSignupRequest(char* recvData, SOCKET clientSocket);
bool HandleLoginRequest(char* recvData, SOCKET clientSocket);
int ProcessPacket(SOCKET clientSocket, char* recvData);
void ThreadProcessClientSocket(SOCKET clientSocket);

int main()
{
    cout << "PROJECT-T Server v2023-05-31" << endl;

    try
    {
        DB_DRIVER = get_driver_instance();
        DB_CONN = DB_DRIVER->connect(DB_SERVERNAME, DB_USERNAME, DB_PASSWORD);
        DB_CONN->setSchema(DB_DBNAME);
        cout << "[SYS] MySQLDB [" << DB_SERVERNAME << "] Connected!" << endl;
    }
    catch (sql::SQLException e)
    {
        cout << "[ERR] DB_CONNection Error Occurred. ErrorMsg : " << e.what() << endl;
        exit(-11);
    }

    // Create the table if not exists
    std::string create_table_query = "CREATE TABLE IF NOT EXISTS User ("
        "id INT AUTO_INCREMENT PRIMARY KEY, "
        "login_id VARCHAR(50) NOT NULL, "
        "password VARCHAR(50) NOT NULL)";
    try
    {
        DB_STMT = DB_CONN->createStatement();
        DB_STMT->execute(create_table_query);
    }
    catch (sql::SQLException& e)
    {
        cout << "[ERR] Table Creation Error Occurred. ErrorMsg : " << e.what() << endl;
        exit(-12);
    }

    if (WSAStartup(MAKEWORD(2, 2), &NET_WSADATA) != 0)
    {
        cout << "[ERR] WSAStartup Error Occurred. ErrorCode : " << GetLastError() << endl;
        exit(-1);
    }
    NET_SERVERSOCKET = socket(AF_INET, SOCK_STREAM, 0);
    if (NET_SERVERSOCKET == INVALID_SOCKET)
    {
        cout << "[ERR] ServerSocket Creation Error Occurred. ErrorCode : " << GetLastError() << endl;
        exit(-2);
    }

    NET_SERVERADDR.sin_family = AF_INET;
    NET_SERVERADDR.sin_addr.S_un.S_addr = inet_addr(NET_SERVER_IPV4);
    NET_SERVERADDR.sin_port = htons(NET_SERVER_PORT);
    if (::bind(NET_SERVERSOCKET, (SOCKADDR*)&NET_SERVERADDR, sizeof(NET_SERVERADDR)) != 0)
    {
        std::cout << "[ERR] ServerSocket Bind Error Occurred. ErrorCode : " << GetLastError() << std::endl;
        exit(-3);
    }

    if (listen(NET_SERVERSOCKET, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cout << "[ERR] ServerSocket Listen Error Occurred. ErrorCode : " << GetLastError() << std::endl;
        exit(-4);
    };

    std::cout << "[SYS] ServerSocket [" << NET_SERVERSOCKET << "] Listen Started!" << std::endl;

    while (G_PROGRAMRUNNING)
    {
        SOCKADDR_IN ClientAddr = { 0, };
        int szClientAddr = sizeof(ClientAddr);
        SOCKET ClientSocket = accept(NET_SERVERSOCKET, (SOCKADDR*)&ClientAddr, &szClientAddr);
        if (ClientSocket == INVALID_SOCKET)
        {
            closesocket(ClientSocket);
            continue;
        }
        {
            std::lock_guard<std::mutex> lock(MUTEX_NETWORK_HANDLER);
            CLIENT_POOL[ClientSocket] = ClientData(ClientSocket);
            cout << "[SYS] ClientSocket [" << ClientSocket << "] Connected!" << endl;
        }

        // Create a new thread for each client
        THREAD_POOL[ClientSocket] = std::thread(ThreadProcessClientSocket, ClientSocket);
        THREAD_POOL[ClientSocket].detach(); // detach the thread and continue
    }

    return 0;
}

// �� �Լ��� �־��� SQL ������ �Ű� ������ ����Ͽ� �����ͺ��̽� ������ �����մϴ�.
// �� ������ ��� ��Ʈ�� ��ȯ�� ������ ����Ǹ�, �̴� SELECT ������ �ַ� ���˴ϴ�.
sql::ResultSet* ExecuteQuery(const std::string& sqlQuery, const std::vector<std::string>& params)
{
    std::lock_guard<std::mutex> lock(MUTEX_DB_HANDLER);
    sql::PreparedStatement* DB_PSTMT = nullptr;
    sql::ResultSet* DB_RS = nullptr;

    try
    {
        DB_PSTMT = DB_CONN->prepareStatement(sqlQuery);
        for (size_t i = 0; i < params.size(); ++i)
        {
            DB_PSTMT->setString(i + 1, params[i]);
        }
        DB_RS = DB_PSTMT->executeQuery();
    }
    catch (sql::SQLException& ex)
    {
        std::cout << "[ERR] SQL Error. ErrorMsg : " << ex.what() << std::endl;
    }

    return DB_RS;
}

// �� �Լ��� �־��� SQL ������ �Ű� ������ ����Ͽ� �����ͺ��̽� ������Ʈ�� �����մϴ�.
// �� ������ ��ȭ�� ���� ���� ��ȯ�ϸ�, �̴� INSERT, UPDATE, DELETE ������ �ַ� ���˴ϴ�.
int ExecuteUpdate(const std::string& sqlQuery, const std::vector<std::string>& params)
{
    std::lock_guard<std::mutex> lock(MUTEX_DB_HANDLER);
    sql::PreparedStatement* DB_PSTMT = nullptr;
    int updatedRows = 0;

    try
    {
        DB_PSTMT = DB_CONN->prepareStatement(sqlQuery);
        for (size_t i = 0; i < params.size(); ++i)
        {
            DB_PSTMT->setString(i + 1, params[i]);
        }
        updatedRows = DB_PSTMT->executeUpdate();
    }
    catch (sql::SQLException& ex)
    {
        std::cout << "[ERR] SQL Error. ErrorMsg : " << ex.what() << std::endl;
    }

    return updatedRows;
}

// �� �Լ��� �������� Ŭ���̾�Ʈ�� �޽����� �����ϴ� ����� ����մϴ�.
// ���� �޽������� �޽��� ID, ó�� ���� �� ���� �ڵ尡 ���Ե˴ϴ�.
void SendResponse(SOCKET clientSocket, EMessageID messageId, EProcessFlag processFlag, ErrorCode errorCode)
{
    MessageResPlayer respMsg;
    respMsg.MsgHead.MessageID = (int)messageId;
    respMsg.MsgHead.MessageSize = sizeof(MessageResPlayer);
    respMsg.PROCESS_FLAG = (int)processFlag;
    respMsg.ERROR_CODE = errorCode;
    send(clientSocket, reinterpret_cast<char*>(&respMsg), sizeof(MessageResPlayer), 0);
}

// �� �Լ��� ȸ�� ���� ��û�� ó���ϴ� ����� ����մϴ�.
// ���� �����͸� �м��Ͽ� ȸ�� ���� ������ �����ͺ��̽��� �����մϴ�.
bool HandleSignupRequest(char* recvData, SOCKET clientSocket)
{
    MessageReqSignup reqMsg;
    memcpy(&reqMsg, recvData, sizeof(MessageReqSignup));

    std::string sqlQuery = "SELECT 1 FROM User WHERE login_id = ? LIMIT 1";
    sql::ResultSet* DB_RS = ExecuteQuery(sqlQuery, { reqMsg.USER_ID });

    if (DB_RS != nullptr && DB_RS->next() == false)
    {
        sqlQuery = "INSERT INTO User(login_id, password) VALUES (?, ?)";
        int updatedRows = ExecuteUpdate(sqlQuery, { reqMsg.USER_ID, reqMsg.USER_PASSWORD });
        if (updatedRows > 0)
        {
            SendResponse(clientSocket, EMessageID::S2C_REQ_SIGNUP, EProcessFlag::PROCESS_OK, ErrorCode::NONE);

            return true;
        }
    }

    SendResponse(clientSocket, EMessageID::S2C_REQ_SIGNUP, EProcessFlag::PROCESS_FAIL, ErrorCode::SIGNUP_DUPLICATE_USERID);
    return false;
}

// �� �Լ��� �α��� ��û�� ó���ϴ� ����� ����մϴ�.
// ���� �����͸� �м��Ͽ� �����ͺ��̽��� ����� ����� ������ ���մϴ�.
bool HandleLoginRequest(char* recvData, SOCKET clientSocket)
{
    MessageReqLogin reqMsg;
    memcpy(&reqMsg, recvData, sizeof(MessageReqLogin));

    std::string sqlQuery = "SELECT 1 FROM User WHERE login_id = ? AND password = ? LIMIT 1";
    sql::ResultSet* DB_RS = ExecuteQuery(sqlQuery, { reqMsg.USER_ID, reqMsg.USER_PASSWORD });

    if (DB_RS != nullptr && DB_RS->next() == true)
    {
        SendResponse(clientSocket, EMessageID::S2C_REQ_LOGIN, EProcessFlag::PROCESS_OK, ErrorCode::NONE);
        return true;
    }

    SendResponse(clientSocket, EMessageID::S2C_REQ_LOGIN, EProcessFlag::PROCESS_FAIL, ErrorCode::LOGIN_FAIL);
    return false;
}

// ���� ��Ŷ ó�� �Լ��Դϴ�.
// ���ϰ� ���۸� �޾�, ���۸� ����ȭ�� �޽����� ���ڵ��ϰ�, �޽��� ������ ���� �޽����� ó���մϴ�.
int ProcessPacket(SOCKET clientSocket, char* recvData)
{
    int retval = 1;

    MessageHeader* msgHeader = reinterpret_cast<MessageHeader*>(recvData);

    switch ((EMessageID)msgHeader->MessageID)
    {
        case EMessageID::C2S_REQ_SIGNUP:

            std::cout << "----------------------------------------\n";
            std::cout << "[TYPE] C2S_REQ_SIGNUP" << std::endl;

            std::cout << "USER_ID : " << reinterpret_cast<MessageReqSignup*>(recvData)->USER_ID << std::endl;
            std::cout << "USER_PASSWORD : " << reinterpret_cast<MessageReqSignup*>(recvData)->USER_PASSWORD << std::endl;
            std::cout << "----------------------------------------\n";

            retval += HandleSignupRequest(recvData, clientSocket);
            break;

        case EMessageID::C2S_REQ_LOGIN:
            std::cout << "----------------------------------------\n";
            std::cout << "[TYPE] C2S_REQ_LOGIN" << std::endl;

            std::cout << "USER_ID : " << reinterpret_cast<MessageReqLogin*>(recvData)->USER_ID << std::endl;
            std::cout << "USER_PASSWORD : " << reinterpret_cast<MessageReqLogin*>(recvData)->USER_PASSWORD << std::endl;
            std::cout << "----------------------------------------\n";

            retval += HandleLoginRequest(recvData, clientSocket);
            break;

        default:
            std::cout << "[ERR] Invalid Message Format! " << std::endl;
            retval = 1;
            break;
    }

    return retval;
}

// �� �Լ��� �� Ŭ���̾�Ʈ ���Ͽ� ���� ������ �����忡�� ����˴ϴ�.
// Ŭ���̾�Ʈ�κ��� �޽����� �����ϰ�, �ش� �޽����� ó���ϸ�, �ʿ信 ���� Ŭ���̾�Ʈ���� ������ �����մϴ�.
void ThreadProcessClientSocket(SOCKET clientSocket)
{
    char recvBuffer[NET_PACKET_SIZE] = { 0, };

    // Message handling loop
    while (G_PROGRAMRUNNING)
    {
        // Receive messages from client
        int recvLen = recv(clientSocket, recvBuffer, NET_PACKET_SIZE, 0);
        if (recvLen <= 0)
        {
            // Handle disconnect
            std::lock_guard<std::mutex> lock(MUTEX_NETWORK_HANDLER);

            std::cout << "[SYS] ClientSocket [" << clientSocket << "] Disconnected!" << std::endl;

            // Remove the disconnected client from CLIENT_POOL
            CLIENT_POOL.erase(clientSocket);

            // Prepare a disconnect message packet
            MessageHeader msgHead = {};
            msgHead.MessageID = (int)EMessageID::S2C_RES_CLINET_DISCONNET;
            msgHead.MessageSize = sizeof(MessageHeader);
            msgHead.SenderSocketID = (int)NET_SERVERSOCKET;

            // Send the disconnect message to all remaining clients
            for (const auto& client : CLIENT_POOL)
            {
                msgHead.ReceiverSocketID = (int)client.first;
                send(client.first, (char*)&msgHead, msgHead.MessageSize, 0);
            }

            closesocket(clientSocket);
            break;
        }

        // Process the received packet
        if (ProcessPacket(clientSocket, recvBuffer) < 0)
        {
            // Handle packet processing error
            // ...
        }
    }

    // Clean up
    {
        std::lock_guard<std::mutex> lock(MUTEX_THREAD_HANDLER);
        THREAD_POOL.erase(clientSocket);
    }
}