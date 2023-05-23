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

void handleAuthenticationResponse(MessageResPlayer* response)
{
    if (response->PROCESS_FLAG == 1)
    {
        std::cout << (response->MsgHead.MessageID == static_cast<int>(EMessageID::S2C_REQ_SIGNUP) ? "Signup" : "Login") << " success!" << std::endl;
    }
    else
    {
        std::cout << (response->MsgHead.MessageID == static_cast<int>(EMessageID::S2C_REQ_SIGNUP) ? "Signup" : "Login") << " failed. User already exists or incorrect username or password." << std::endl;
    }
}

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

int sendRequest(SOCKET clientSocket, MessageHeader* requestMessage)
{
    return send(clientSocket, (char*)requestMessage, requestMessage->MessageSize, 0);
}

void processResponse(char* buffer)
{
    MessageHeader* header = reinterpret_cast<MessageHeader*>(buffer);
    switch (static_cast<EMessageID>(header->MessageID))
    {
    case EMessageID::S2C_REQ_SIGNUP:
    case EMessageID::S2C_REQ_LOGIN:
        handleAuthenticationResponse(reinterpret_cast<MessageResPlayer*>(buffer));
        break;
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

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Failed to initialize winsock! Error: " << WSAGetLastError() << std::endl;
        return -1;
    }

    // Create a client socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Failed to create socket! Error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    // Set server details
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP_ADDRESS, &serverAddr.sin_addr) <= 0)
    {
        std::cerr << "Failed to set server address! Error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    // Connect to server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "Failed to connect to server! Error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    char buffer[1024];
    std::string choice;

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

