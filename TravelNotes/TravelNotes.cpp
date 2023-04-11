//#pragma once
#define WIN32_LEAN_AND_MEAN

#include <string>
#include <iostream>
#include <pqxx/pqxx>

#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

int main()
{
    setlocale(LC_ALL, "Russian");

    /************************/

    WSADATA wsaData;
    ADDRINFO hints;
    ADDRINFO* addResult = NULL;
    SOCKET ClientSocket = INVALID_SOCKET;
    SOCKET ListenSocket = INVALID_SOCKET;

    const char* sendBuffer = "Hello from Server!";

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
    hints.ai_flags = AI_PASSIVE;

    result = getaddrinfo(NULL, "555", &hints, &addResult);
    if (result != 0) {
        std::cout << "getaddrinfo failed with error: " << result << std::endl;
        WSACleanup();
        return 1;
    }

    ListenSocket = socket(addResult->ai_family, addResult->ai_socktype, addResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        std::cout << "Socket creation failed\n";
        freeaddrinfo(addResult);
        WSACleanup();
        return 1;
    }

    result = bind(ListenSocket, addResult->ai_addr, addResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        std::cout << "Binding socket failed" << std::endl;
        closesocket(ListenSocket);
        ListenSocket = INVALID_SOCKET;
        freeaddrinfo(addResult);
        WSACleanup();
        return 1;
    }

    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        std::cout << "Listening socket failed" << std::endl;
        closesocket(ListenSocket);
        freeaddrinfo(addResult);
        WSACleanup();
        return 1;
    }

    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        std::cout << "Accepting socket failed" << std::endl;
        closesocket(ListenSocket);
        freeaddrinfo(addResult);
        WSACleanup();
        return 1;
    }

    closesocket(ListenSocket);

    do {
        ZeroMemory(recvBuffer, 512);
        result = recv(ClientSocket, recvBuffer, 512, 0);
        if (result > 0) {
            std::cout << "Received " << result << " bytes\n";
            std::cout << "Received data: " << recvBuffer << "\n";

            result = send(ClientSocket, sendBuffer, (int)strlen(sendBuffer), 0);
            if (result == SOCKET_ERROR) {
                std::cout << "Failed to send data back" << std::endl;
                closesocket(ClientSocket);
                freeaddrinfo(addResult);
                WSACleanup();
                return 1;
            }
        }
        else if (result == 0)
            std::cout << "Connection closing...\n";
        else {
            std::cout << "recv failed with error\n";
            closesocket(ClientSocket);
            freeaddrinfo(addResult);
            WSACleanup();
            return 1;
        }

    } while (result > 0);

    result = shutdown(ClientSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        std::cout << "shutdown client socket failed\n";
        closesocket(ClientSocket);
        freeaddrinfo(addResult);
        WSACleanup();
        return 1;
    }

    closesocket(ClientSocket);
    freeaddrinfo(addResult);
    WSACleanup();


    /************************/

    std::string connectionString = "host=localhost port=5433 dbname=travel_notes user=postgres password =password";

    try
    {
        pqxx::connection connectionObject(connectionString.c_str());

        while (1) {
            std::cout << "Select operation:\n1 - Show data\n2 - Insert fact\n3 - Edit fact\n4 - Delete fact\n5 - Exit\n";
            int ans; std::cin >> ans;
            pqxx::work worker(connectionObject);
            switch (ans)
            {
            case 1: {
                pqxx::result response = worker.exec("SELECT * FROM interesting_facts");

                for (size_t i = 0; i < response.size(); i++)
                {
                    std::cout << "Id: " << response[i][0] << " Country: " << response[i][1] << " City: " << response[i][2] << " Fact: " << response[i][3] << std::endl;
                }
                break;
            }
            case 2: {
                std::cout << "Enter Id:\n"; int id; std::cin >> id;
                std::cout << "Enter Country:\n"; std::string country; std::cin >> country;
                std::cout << "Enter City:\n"; std::string city; std::cin >> city;
                std::cout << "Enter Fact:\n"; std::string fact;  std::getline(std::cin, fact); std::getline(std::cin, fact);

                worker.exec("INSERT INTO interesting_facts VALUES(" + std::to_string(id) + ",'" + country + "','" + city + "','" + fact + "')");
                worker.commit();
                break;
            }
            case 3: {
                std::cout << "Enter id of fact to edit:\n";
                int id; std::cin >> id;

                std::cout << "Select operation:\n1 - Edit country\n2 - Edit city\n3 - Edit fact\n";
                int edit; std::cin >> edit;
                switch (edit) {
                case 1: {
                    std::cout << "Enter new name:\n"; std::string country; std::getline(std::cin, country); std::getline(std::cin, country);
                    worker.exec("UPDATE interesting_facts SET \"Country\" = '" + country + "' where \"Id\" = " + std::to_string(id));
                    worker.commit();
                    break;
                }
                case 2: {
                    std::cout << "Enter new city:\n"; std::string city; std::getline(std::cin, city); std::getline(std::cin, city);
                    worker.exec("UPDATE interesting_facts SET \"City\" = '" + city + "' where \"Id\" = " + std::to_string(id));
                    worker.commit();
                    break;
                }
                case 3: {
                    std::cout << "Enter new fact:\n"; std::string fact; std::getline(std::cin, fact); std::getline(std::cin, fact);
                    worker.exec("UPDATE interesting_facts SET \"Fact\" = '" + fact + "' where \"Id\" = " + std::to_string(id));
                    worker.commit();
                    break;
                }
                default:
                    break;
                }
                break;
            }
            case 4: {
                std::cout << "Enter id of fact to delete:\n";
                int id; std::cin >> id;
                worker.exec("DELETE from interesting_facts where Id = " + std::to_string(id));
                worker.commit();
                break;
            }
            case 5: return 0;
            default:
                break;
            }

        }

    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    system("pause");
}