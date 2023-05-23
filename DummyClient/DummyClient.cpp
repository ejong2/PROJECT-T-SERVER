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

#pragma comment (lib, "ws2_32.lib")

#ifdef _RUN_AS_LOCALHOST
const char* SERVER_IP_ADDRESS = "127.0.0.1";
#else
const char* SERVER_IP_ADDRESS = "192.168.0.10";
#endif
const int SERVER_PORT = 5001;

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

    // Create signup request
    MessageReqSignup signupRequest;
    memset(&signupRequest, 0, sizeof(signupRequest));
    signupRequest.MessageID = static_cast<int>(EMessageID::C2S_REQ_SIGNUP);
    signupRequest.MessageSize = sizeof(signupRequest);
    strcpy_s(signupRequest.USER_ID, sizeof(signupRequest.USER_ID), "user1");
    strcpy_s(signupRequest.USER_PASSWORD, sizeof(signupRequest.USER_PASSWORD), "password1");

    // Send signup request
    if (send(clientSocket, (char*)&signupRequest, sizeof(signupRequest), 0) == SOCKET_ERROR)
    {
        std::cerr << "Failed to send signup request! Error: " << WSAGetLastError() << std::endl;
    }
    else
    {
        std::cout << "Signup request sent!" << std::endl;
    }

    // Create login request
    MessageReqLogin loginRequest;
    memset(&loginRequest, 0, sizeof(loginRequest));
    loginRequest.MessageID = static_cast<int>(EMessageID::C2S_REQ_LOGIN);
    loginRequest.MessageSize = sizeof(loginRequest);
    strcpy_s(loginRequest.USER_ID, sizeof(loginRequest.USER_ID), "user1");
    strcpy_s(loginRequest.USER_PASSWORD, sizeof(loginRequest.USER_PASSWORD), "password1");

    // Send login request
    if (send(clientSocket, (char*)&loginRequest, sizeof(loginRequest), 0) == SOCKET_ERROR)
    {
        std::cerr << "Failed to send login request! Error: " << WSAGetLastError() << std::endl;
    }
    else
    {
        std::cout << "Login request sent!" << std::endl;
    }

    // Cleanup and close connections
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
