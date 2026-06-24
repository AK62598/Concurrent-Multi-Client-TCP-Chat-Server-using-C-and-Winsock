#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <mutex>
#include <map>
#include <string>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

map<int, SOCKET> clients;
mutex clientsMutex;
bool serverRunning = true;
SOCKET globalServerSocket = INVALID_SOCKET;

void handleClient(SOCKET clientSocket, int clientId)
{
    char buffer[1024];

    while (true)
    {
        int bytesIn = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesIn <= 0)
        {
            cout << "\nClient " << clientId << " disconnected." << endl;
            break;
        }
        buffer[bytesIn] = '\0';
        cout << "\nClient " << clientId << ": " << buffer << endl;
        cout << "Type '" << clientId << ": your message' to reply to them." << endl;
        cout << "You: ";
        cout.flush();
    }

    closesocket(clientSocket);

    lock_guard<mutex> lock(clientsMutex);
    clients.erase(clientId);
}

void typingLoop()
{
    string line;
    while (true)
    {
        if (!getline(cin, line)) break;

        if (line == "exit")
        {
            cout << "Shutting down server..." << endl;
            serverRunning = false;

            lock_guard<mutex> lock(clientsMutex);
            for (auto& pair : clients)
            {
                closesocket(pair.second);
            }
            clients.clear();

            if (globalServerSocket != INVALID_SOCKET)
                closesocket(globalServerSocket);

            break;
        }

        // Expected format: "1: hello there"
        size_t colonPos = line.find(':');
        if (colonPos == string::npos)
        {
            cout << "Format must be 'clientId: message'. Example: 1: hi" << endl;
            cout << "You: ";
            continue;
        }

        int targetId = -1;
        try
        {
            targetId = stoi(line.substr(0, colonPos));
        }
        catch (...)
        {
            cout << "Invalid client id." << endl;
            cout << "You: ";
            continue;
        }

        string message = line.substr(colonPos + 1);
        if (!message.empty() && message[0] == ' ')
            message = message.substr(1);

        lock_guard<mutex> lock(clientsMutex);
        auto it = clients.find(targetId);
        if (it == clients.end())
        {
            cout << "No client with id " << targetId << " is connected." << endl;
        }
        else
        {
            send(it->second, message.c_str(), (int)message.length(), 0);
            cout << "Sent to client " << targetId << "." << endl;
        }
        cout << "You: ";
    }
}

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    globalServerSocket = serverSocket;

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));
    listen(serverSocket, SOMAXCONN);

    cout << "Server waiting on port 8080..." << endl;
    //cout << "Once clients connect, reply using format  clientId: message" << endl;
    //cout << "Example:  1: hello there" << endl;
    cout << "Type 'exit' at any time to shut down the server." << endl;

    thread inputThread(typingLoop);
    inputThread.detach();

    int clientId = 0;

    while (serverRunning)
    {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (!serverRunning) break;
        if (clientSocket == INVALID_SOCKET) continue;

        clientId++;
        cout << "\nClient " << clientId << " connected!" << endl;
        cout << "You: ";

        {
            lock_guard<mutex> lock(clientsMutex);
            clients[clientId] = clientSocket;
        }

        thread t(handleClient, clientSocket, clientId);
        t.detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
