#define _RUN_AS_LOCALHOST

#define _WINSOCK_DEPRECATED_NO_WARNINGS
//--C Header------------------------------------------------------------------
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
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

using namespace std;

#pragma comment (lib, "ws2_32.lib")

#ifdef _RUN_AS_LOCALHOST
const char* SERVER_IP_ADDRESS = "127.0.0.1";
#else
const char* SERVER_IP_ADDRESS = "192.168.0.10";
#endif
const int SERVER_PORT = 5001;

// 사용자 인증 응답을 처리하는 함수입니다.
void handleAuthenticationResponse(MessageResPlayer* response)
{
    // 응답으로 받은 프로세스 플래그를 기준으로 성공/실패 메시지를 출력합니다.
    // 1이면 성공, 아니면 실패입니다.
    if (response->PROCESS_FLAG == 1)
    {
        std::cout << (response->MsgHead.MessageID == static_cast<int>(EMessageID::S2C_REQ_SIGNUP) ? "Signup" : "Login") << " success!" << std::endl;
    }
    else
    {
        std::cout << (response->MsgHead.MessageID == static_cast<int>(EMessageID::S2C_REQ_SIGNUP) ? "Signup" : "Login") << " failed. User already exists or incorrect username or password." << std::endl;
    }
}

// 회원가입 요청 메시지를 구성하는 함수입니다.
void inputCredentials(MessageReqSignup* request)
{
    memset(request, 0, sizeof(*request));
    request->MessageID = static_cast<int>(EMessageID::C2S_REQ_SIGNUP);
    request->MessageSize = sizeof(*request);
    std::cout << "Enter UserID: ";
    std::cin >> request->USER_ID;
    std::cout << "Enter Password: ";
    std::cin >> request->USER_PASSWORD;
}

// 로그인 요청 메시지를 구성하는 함수입니다.
void inputCredentials(MessageReqLogin* request)
{
    memset(request, 0, sizeof(*request));
    request->MessageID = static_cast<int>(EMessageID::C2S_REQ_LOGIN);
    request->MessageSize = sizeof(*request);
    std::cout << "Enter UserID: ";
    std::cin >> request->USER_ID;
    std::cout << "Enter Password: ";
    std::cin >> request->USER_PASSWORD;
}

// 클라이언트가 선택한 옵션(회원가입 or 로그인)에 따라 해당 요청 메시지를 생성하는 함수입니다.
MessageHeader* createRequestMessage(std::string choice)
{
    if (choice == "1") // Signup
    {
        MessageReqSignup* signupRequest = new MessageReqSignup();
        inputCredentials(signupRequest);
        return signupRequest;
    }
    else if (choice == "2") // Login
    {
        MessageReqLogin* loginRequest = new MessageReqLogin();
        inputCredentials(loginRequest);
        return loginRequest;
    }
    return nullptr;
}

// 요청 메시지를 서버에 전송하는 함수입니다.
int sendRequest(SOCKET clientSocket, MessageHeader* requestMessage)
{
    return send(clientSocket, (char*)requestMessage, requestMessage->MessageSize, 0);
}

// 서버로부터의 응답을 처리하는 함수입니다.
void processResponse(char* buffer)
{
    MessageHeader* header = reinterpret_cast<MessageHeader*>(buffer);
    switch (static_cast<EMessageID>(header->MessageID))
    {
    // 회원가입 혹은 로그인 요청에 대한 응답을 처리합니다.
    case EMessageID::S2C_REQ_SIGNUP:
    case EMessageID::S2C_REQ_LOGIN:
        handleAuthenticationResponse(reinterpret_cast<MessageResPlayer*>(buffer));
        break;
    // 서버 연결 성공 및 연결 해제 메시지를 처리합니다.
    case EMessageID::S2C_RES_CLINET_CONNECT:
        std::cout << "Server: Connected successfully!" << std::endl;
        break;
    case EMessageID::S2C_RES_CLINET_DISCONNET:
        std::cout << "Server: Disconnected successfully!" << std::endl;
        break;
    default:
        std::cerr << "Unknown response from server!" << std::endl;
        break;
    }
}

int main()
{
    WSAData wsaData;
    SOCKET clientSocket;
    sockaddr_in serverAddr;

    // Winsock 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Failed to initialize winsock! Error: " << WSAGetLastError() << std::endl;
        return -1;
    }

    // 클라이언트 소켓 생성
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Failed to create socket! Error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    // 서버 상세 정보 설정
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP_ADDRESS, &serverAddr.sin_addr) <= 0)
    {
        std::cerr << "Failed to set server address! Error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    // 서버에 연결
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "Failed to connect to server! Error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    char buffer[1024];
    std::string choice;

    // 메인 루프: 사용자의 입력에 따라 회원가입 또는 로그인을 진행하거나, 클라이언트를 종료합니다.
    while (true)
    {
        std::cout << "\n1. Signup\n2. Login\n3. Quit\n";
        std::cin >> choice;
        MessageHeader* requestMessage;
        if (choice == "1" || choice ==  "2")
        {
            requestMessage = createRequestMessage(choice);
            if (sendRequest(clientSocket, requestMessage) == SOCKET_ERROR)
            {
                std::cerr << "Failed to send request! Error: " << WSAGetLastError() << std::endl;
                continue;
            }
            std::cout << (choice == "1" ? "Signup" : "Login") << " request sent!" << std::endl;
        }
        else if (choice == "3") // Quit
        {
            // 사용자가 종료를 선택하면 소켓을 닫고 Winsock을 정리한 뒤 프로그램을 종료합니다.
            closesocket(clientSocket);
            WSACleanup();
            return 0;
        }
        else
        {
            std::cout << "Invalid choice, please try again." << std::endl;
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived == SOCKET_ERROR)
        {
            std::cerr << "Failed to receive response! Error: " << WSAGetLastError() << std::endl;
            continue;
        }

        processResponse(buffer);
    }
}

