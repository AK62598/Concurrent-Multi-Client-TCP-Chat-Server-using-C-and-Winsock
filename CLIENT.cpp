#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <cstring>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
    WSADATA wsa;
    int iVal = 0;

    iVal = WSAStartup(MAKEWORD(2, 2), &wsa);

    if (0 != iVal)
    {
        return 0;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    connect(clientSocket, (sockaddr*)&server, sizeof(server));

    char message[1024];
    char buffer[1024];

    while (true)
    {
        cout << "You: ";
        cin.getline(message, sizeof(message));

        send(clientSocket, message, (int)strlen(message), 0);

        if (strcmp(message, "exit") == 0)
            break;

        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived <= 0)
        {
            cout << "Server disconnected.\n";
            break;
        }

        buffer[bytesReceived] = '\0';

        cout << "Server: " << buffer << endl;
    }
}
