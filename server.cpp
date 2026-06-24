#include <iostream>
#include <winsock2.h>
#include <cstring>
#include <ws2tcpip.h>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    bind(serverSocket, (sockaddr*)&server, sizeof(server));

    listen(serverSocket, 1);

    cout << "Server waiting on port 8080...\n";

    SOCKET clientSocket;
    sockaddr_in client;
    int c = sizeof(sockaddr_in);

    clientSocket = accept(serverSocket, (sockaddr*)&client, &c);

    cout << "Client connected!\n";

    char message[1024];

    int bytesReceived = recv(clientSocket, message, sizeof(message), 0);

    if (bytesReceived > 0)
    {
        message[bytesReceived] = '\0';
        cout << "Client says: " << message << endl;
    }

    const char* reply = "Hello Client!";
    send(clientSocket, reply, strlen(reply), 0);

    closesocket(clientSocket);
    closesocket(serverSocket);

    WSACleanup();

    return 0;
}