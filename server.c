#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

const char mimeTypes[3][2][20] = {
    { "html", "text/html" },
    { "css", "text/css" },
    { "js", "text/javascript" }
};

char *getMime(char *ext) {
    char defMime[] = "text/plain;charset=UTF-8";
    char *extReturn;

    for (int i = 0; i < 3; ++i) {
        if (strcmp(ext, mimeTypes[i][0]) == 0) {
            const char *selectedMime = mimeTypes[i][1];

            extReturn = (char *)malloc(sizeof(selectedMime));
            strcpy(extReturn, selectedMime);

            return extReturn;
        }
    }

    extReturn = (char *)malloc(sizeof(defMime));
    strcpy(extReturn, defMime);

    return extReturn;
}

int main() {
    WSADATA wsaData;
    SOCKET serverSock, client;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    char buffer[1024];
    int bytesRecieved;
    char tempHttpResHead[] = "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n";
    char httpResHead[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    char termi;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("wsa startup failed\n");
        return 1;
    }

    if ((serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
        WSACleanup();
        printf("failed to create socket\n");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(serverSock);
        WSACleanup();
        printf("bind failed");
        return 1;
    }

    if (listen(serverSock, 1) == SOCKET_ERROR) {
        closesocket(serverSock);
        WSACleanup();
        printf("listen failed\n");
        return 1;
    }

    printf("server listening on port 8080\n");

    while (termi != 'q') {
        if ((client = accept(serverSock, (struct sockaddr*)&clientAddr, &clientAddrSize)) == INVALID_SOCKET) {
            closesocket(serverSock);
            printf("failed to accept\n");
            continue;
        }

        if ((bytesRecieved = recv(client, buffer, sizeof(buffer), 0)) == SOCKET_ERROR) {
            closesocket(client);
            printf("recv failed\n");
            continue;
        }

        if (bytesRecieved == 0) {
            closesocket(client);
            printf("no bytes recieved\n");
            continue;
        }

        char *firstLine = strtok(buffer, "\n");

        if (firstLine == NULL) {
            closesocket(client);
            continue;
        }

        char *req = strtok(firstLine, " ");

        if (req == NULL) {
            closesocket(client);
            continue;
        }

        req = strtok(NULL, " ");

        if (req != NULL) {
            if (strcmp(req, "/") == 0) {
                char res[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\njoe mama";

                if (send(client, res, strlen(res), 0) == SOCKET_ERROR) {
                    printf("failed to send\n");
                }
            } else {
                FILE *stacFile;
                char *reqFile = strtok(req, "/");

                if (reqFile != NULL) {
                    char stacRoute[50];

                    snprintf(stacRoute, sizeof(stacRoute), "public/%s", reqFile);
                    stacFile = fopen(stacRoute, "rb");

                    if (stacFile != NULL) {
                        char *fileBytes;
                        int size;

                        fseek(stacFile, 0, SEEK_END);
                        size = ftell(stacFile);
                        rewind(stacFile);

                        fileBytes = (char *)malloc(size);
                        
                        if (fileBytes != NULL) {
                            for (int i = 0; i < size; ++i) {
                                fileBytes[i] = fgetc(stacFile);
                            }

                            char *ext = strtok(reqFile, ".");
                            char *resHead;

                            ext = strtok(NULL, ".");

                            if (ext == NULL) {
                                resHead = (char *)malloc(sizeof(httpResHead));
                                strcpy(resHead, httpResHead);
                            } else {
                                char *resMime = getMime(ext);
                                resHead = (char *)malloc(strlen(tempHttpResHead) + strlen(resMime) + 1);

                                sprintf(resHead, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", resMime);     
                                free(resMime);
                            }

                            send(client, resHead, strlen(resHead), 0);
                            send(client, fileBytes, size, 0);

                            free(resHead);
                            free(fileBytes);
                            fclose(stacFile);
                        }
                    }
                }
            }
        }

        closesocket(client);
        termi = _getch();
    }

    closesocket(serverSock);
    WSACleanup();
    printf("cleaned-up the server\n");

    return 0;
}