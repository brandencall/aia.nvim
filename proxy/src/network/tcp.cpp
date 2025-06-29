#include "tcp.h"
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <limits.h>
#include <netinet/in.h>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace network {

int createServerSocket(const char *ip, int port) {
    int serverSocket;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cout << "Error creating socket" << std::endl;
        return -1;
    }
    sockaddr_in service{};
    service.sin_family = AF_INET;
    service.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &service.sin_addr) <= 0) {
        std::cerr << "Invalid IP address\n";
        return -1;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(serverSocket, (sockaddr *)&service, sizeof(service)) == -1) {
        perror("bind failed");
        close(serverSocket);
        return -1;
    }
    if (listen(serverSocket, 5) == -1) {
        std::cout << "Error on listen" << std::endl;
        close(serverSocket);
        return -1;
    }

    std::cout << "Server listening on " << ip << ":" << port << std::endl;
    return serverSocket;
}

int acceptClient(int serverSocket) {
    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (sockaddr *)&clientAddr, &clientLen);
    if (clientSocket < 0) {
        std::cerr << "Accept failed\n";
        return -1;
    }

    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);

    std::string allowedIP = "10.0.0.209";
    if (std::string(clientIP) != allowedIP) {
        std::cout << "Rejected connection " << clientIP << std::endl;
        close(clientSocket);
        return -1;
    }

    std::cout << "Accepted connection from " << clientIP << std::endl;
    return clientSocket;
}

void clientSession(int clientSocket, ModelManager* modelManager) {
    Router *router = new Router(clientSocket, *modelManager);
    while (true) {
        auto prompt = handleClient(clientSocket);
        if (!prompt)
            break;
        router->routeRequest(*prompt);
    }
    delete router;
    close(clientSocket);
}

std::optional<ClientRequest> handleClient(int clientSocket) {
    char buffer[1024];
    std::memset(buffer, 0, 1024);
    ssize_t bytesReceived = recv(clientSocket, buffer, 1024 - 1, 0);

    if (bytesReceived == 0) {
        std::cout << "Client disconnected gracefully" << std::endl;
        return std::nullopt;
    } else if (bytesReceived < 0) {
        perror("recv failed");
        return std::nullopt;
    } else {
        std::cout << "Client said: " << std::string(buffer) << std::endl;
        std::string confirmation = "Processing...";
        sendMsg(clientSocket, confirmation);
        std::cout << confirmation << std::endl;
        json j = json::parse(buffer);
        ClientRequest request = j.get<ClientRequest>();
        return request;
    }
}

void sendMsg(int clientSocket, std::string msg) { send(clientSocket, msg.c_str(), msg.size(), 0); }

} // namespace network
