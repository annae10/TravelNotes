#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

int main()
{
    WSADATA wsaData;
    ADDRINFO hints;
    ADDRINFO* addResult = NULL;
    SOCKET ConnectSocket = INVALID_SOCKET;

    const char* sendBuffer = "Hello from Client!";

    char recvBuffer[512];

    int result;

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cout << "WSAStartup failed, result = " << result << std::endl;
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    result = getaddrinfo("localhost", "555", &hints, &addResult);
    if (result != 0) {
        std::cout << "getaddrinfo failed with error: " << result << std::endl;
        WSACleanup();
        return 1;
    }

    ConnectSocket = socket(addResult->ai_family, addResult->ai_socktype, addResult->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        std::cout << "Socket creation failed\n";
        freeaddrinfo(addResult);
        WSACleanup();
        return 1;
    }

    result = connect(ConnectSocket, addResult->ai_addr, addResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        std::cout << "Unable connect to server" << std::endl;
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
        freeaddrinfo(addResult);
        WSACleanup();
        return 1;
    }

    result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
    if (result == SOCKET_ERROR) {
        std::cout << "send failed, error: " << result << std::endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addResult);
        WSACleanup();
        return 1;
    }

    std::cout << "Sent: " << result << " bytes\n";

    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        std::cout << "Shutdown error: " << result << "\n";
        closesocket(ConnectSocket);
        freeaddrinfo(addResult);
        WSACleanup();
        return 1;
    }


    do {
        ZeroMemory(recvBuffer, 512);
        result = recv(ConnectSocket, recvBuffer, 512, 0);
        if (result > 0) {
            std::cout << "Received " << result << " bytes\n";
            std::cout << "Received data: " << recvBuffer << "\n";
        }
        else if (result == 0)
            std::cout << "Connection closed\n";
        else
            std::cout << "recv failed with error\n";
    } while (result>0);
    closesocket(ConnectSocket);
    freeaddrinfo(addResult);
    WSACleanup();
    return 0;
}

