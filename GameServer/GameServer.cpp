#define _RUN_AS_LOCALHOST

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
const std::string            DB_DBNAME = "MYPROJECT";
sql::Driver* DB_DRIVER = nullptr;
sql::Connection* DB_CONN = nullptr;
sql::Statement* DB_STMT = nullptr;
sql::PreparedStatement* DB_PSTMT = nullptr;
sql::ResultSet* DB_RS = nullptr;
std::mutex                   MUTEX_DB_HANDLER;
//--MainProgram---------------------------------------------------------------
bool                         G_PROGRAMRUNNING = true;

using namespace std;

int ProcessPacket(SOCKET clientSocket, char* recvData)
{
    int retval = 1;

    // Extract the header from the received data
    MessageHeader* msgHeader = reinterpret_cast<MessageHeader*>(recvData);
    std::string sqlQuery;
    sql::PreparedStatement* DB_PSTMT = nullptr;
    sql::ResultSet* DB_RS = nullptr;
    int updatedRows = 0;

    switch ((EMessageID)msgHeader->MessageID)
    {
    case EMessageID::C2S_REQ_SIGNUP:  // Sign up request
    {
        MessageReqSignup reqMsg;
        memcpy(&reqMsg, recvData, sizeof(MessageReqSignup));

        std::lock_guard<std::mutex> lock(MUTEX_DB_HANDLER);
        try
        {
            // Check if user already exists
            sqlQuery = "SELECT 1 FROM 유저 WHERE 로그인_아이디 = ? LIMIT 1";
            DB_PSTMT = DB_CONN->prepareStatement(sqlQuery);
            DB_PSTMT->setString(1, reqMsg.USER_ID);
            DB_RS = DB_PSTMT->executeQuery();

            // If user does not exist, insert new user
            if (DB_RS == nullptr || (DB_RS != nullptr && DB_RS->next() == false))
            {
                sqlQuery = "INSERT INTO 유저(로그인_아이디,비밀번호)VALUES(?,?)";
                DB_PSTMT = DB_CONN->prepareStatement(sqlQuery);
                DB_PSTMT->setString(1, reqMsg.USER_ID);
                DB_PSTMT->setString(2, reqMsg.USER_PASSWORD);
                updatedRows += DB_PSTMT->executeUpdate();
            }
        }
        catch (sql::SQLException ex)
        {
            std::cout << "[ERR] SQL Error On C2S_REQ_SIGNUP. ErrorMsg : " << ex.what() << std::endl;
        }
        if (DB_RS) { DB_RS->close(); DB_RS = nullptr; }
        if (DB_PSTMT) { DB_PSTMT->close(); DB_PSTMT = nullptr; }

        // Prepare response message...
        if (updatedRows > 0)  // If signup was successful
        {
            MessageResInsertPlayer respMsg;
            respMsg.MsgHead.MessageID = (int)EMessageID::S2C_REQ_SIGNUP;
            respMsg.MsgHead.MessageSize = sizeof(MessageResInsertPlayer);
            respMsg.PROCESS_FLAG = 1;  // Success

            retval += send(clientSocket, reinterpret_cast<char*>(&respMsg), sizeof(MessageResInsertPlayer), 0);
        }
        else  // Signup failed
        {
            MessageResInsertPlayer respMsg;
            respMsg.MsgHead.MessageID = (int)EMessageID::S2C_REQ_SIGNUP;
            respMsg.MsgHead.MessageSize = sizeof(MessageResInsertPlayer);
            respMsg.PROCESS_FLAG = 0;  // Failure

            retval += send(clientSocket, reinterpret_cast<char*>(&respMsg), sizeof(MessageResInsertPlayer), 0);
        }
        break;
    }
    case EMessageID::C2S_REQ_LOGIN:  // Login request
    {
        MessageReqLogin reqMsg;
        memcpy(&reqMsg, recvData, sizeof(MessageReqLogin));

        std::lock_guard<std::mutex> lock(MUTEX_DB_HANDLER);
        try
        {
            // Try to find the user with given ID and password
            sqlQuery = "SELECT 1 FROM 유저 WHERE 로그인_아이디 = ? AND 비밀번호 = ? LIMIT 1";
            DB_PSTMT = DB_CONN->prepareStatement(sqlQuery);
            DB_PSTMT->setString(1, reqMsg.USER_ID);
            DB_PSTMT->setString(2, reqMsg.USER_PASSWORD);
            DB_RS = DB_PSTMT->executeQuery();

            // If user exists and password matches, successful login
            if (DB_RS != nullptr && DB_RS->next() == true)
            {
                // Login success!
            }
        }
        catch (sql::SQLException ex)
        {
            std::cout << "[ERR] SQL Error On C2S_REQ_LOGIN. ErrorMsg : " << ex.what() << std::endl;
        }
        if (DB_RS) { DB_RS->close(); DB_RS = nullptr; }
        if (DB_PSTMT) { DB_PSTMT->close(); DB_PSTMT = nullptr; }

        // Prepare response message...
        if (DB_RS != nullptr)  // If login was successful
        {
            MessageResInsertPlayer respMsg;
            respMsg.MsgHead.MessageID = (int)EMessageID::S2C_REQ_LOGIN;
            respMsg.MsgHead.MessageSize = sizeof(MessageResInsertPlayer);
            respMsg.PROCESS_FLAG = 1;  // Success

            retval += send(clientSocket, reinterpret_cast<char*>(&respMsg), sizeof(MessageResInsertPlayer), 0);
        }
        else  // Login failed
        {
            MessageResInsertPlayer respMsg;
            respMsg.MsgHead.MessageID = (int)EMessageID::S2C_REQ_LOGIN;
            respMsg.MsgHead.MessageSize = sizeof(MessageResInsertPlayer);
            respMsg.PROCESS_FLAG = 0;  // Failure

            retval += send(clientSocket, reinterpret_cast<char*>(&respMsg), sizeof(MessageResInsertPlayer), 0);
        }
        break;
    }
    default:
        std::cout << "[ERR] Invalid Message Format! " << std::endl;
        retval = 1;
        break;
    }

    return retval;
}

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

int main()
{
    cout << "myProject Server v2023-05-22" << endl;

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
