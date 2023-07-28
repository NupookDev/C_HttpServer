#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_BUFF 1024 

const char mimeTypes[5][2][20] = {
    { "html", "text/html" },
    { "css", "text/css" },
    { "js", "text/javascript" },
    { "webp", "image/webp" },
    { "webm", "video/webm" }
};

char *getMime(char *ext) {
    char defMime[] = "text/plain;charset=UTF-8";
    char *extReturn;

    for (int i = 0; i < 3; ++i) {
        if (strcmp(ext, mimeTypes[i][0]) == 0) {
            const char *selectedMime = mimeTypes[i][1];

            extReturn = (char *)malloc(sizeof(selectedMime));

            if (extReturn == NULL) {
                return NULL;
            }

            strcpy(extReturn, selectedMime);

            return extReturn;
        }
    }

    extReturn = (char *)malloc(sizeof(defMime));

    if (extReturn == NULL) {
        return NULL;
    }

    strcpy(extReturn, defMime);

    return extReturn;
}

char *getRoute(char *str) {
    int size = strlen(str);
    char *result = (char *)malloc(size);

    if (result == NULL) {
        return NULL;
    }

    for (int i = 0; i < strlen(str) - 1; ++i) {
        result[i] = *(str + (i + 1));
    }

    result[size - 1] = '\0';

    return result;
}

int main() {
    WSADATA wsaData;
    SOCKET serverSock, client;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    char buffer[MAX_BUFF + 1];
    int bytesRecieved;
    const char tempHttpHead[] = "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n";
    const char httpHtmlHead[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    const char httpErr[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain;charset=UTF-8\r\n\r\nPage not found 🗿";
    const int tmpHeadS = strlen(tempHttpHead), htmlHeadS = strlen(httpHtmlHead), httpErrS = strlen(httpErr);

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

    while (1) {
        if ((client = accept(serverSock, (struct sockaddr*)&clientAddr, &clientAddrSize)) == INVALID_SOCKET) {
            closesocket(serverSock);
            printf("failed to accept\n");
            continue;
        }

        if ((bytesRecieved = recv(client, buffer, MAX_BUFF, 0)) == SOCKET_ERROR) {
            closesocket(client);
            printf("recv failed\n");
            continue;
        }

        if (bytesRecieved == 0) {
            closesocket(client);
            printf("no bytes recieved\n");
            continue;
        }

        buffer[bytesRecieved] = '\0';

        char *firstLine = strtok(buffer, "\n");

        if (firstLine == NULL) {
            closesocket(client);
            continue;
        }

        char *req = strtok(firstLine, " ");
        req = strtok(NULL, " ");

        if (req == NULL) {
            closesocket(client);
            continue;
        }

        if (strcmp(req, "/") == 0) {
            char res[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\njoe mama";

            if (send(client, res, strlen(res), 0) == SOCKET_ERROR) {
                printf("failed to send\n");
            }
        } else {
            char *reqFile = getRoute(req);

            if (reqFile != NULL) {
                FILE *stacFile;
                char stacRoute[50];

                snprintf(stacRoute, sizeof(stacRoute), "public/%s", reqFile);
                stacFile = fopen(stacRoute, "rb");

                if (stacFile == NULL) {
                    send(client, httpErr, httpErrS, 0);
                } else {
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

                        ext = strtok(NULL, ".");

                        if (ext == NULL) {
                            send(client, httpHtmlHead, htmlHeadS, 0);
                        } else {
                            char *resMime = getMime(ext);

                            if (resMime != NULL) {
                                char *resHead = (char *)malloc(tmpHeadS + strlen(resMime) + 1);

                                if (resHead != NULL) {
                                    sprintf(resHead, tempHttpHead, resMime);     
                                    send(client, resHead, strlen(resHead), 0);
                                    free(resHead);
                                }

                                free(resMime);
                            }
                        }

                        send(client, fileBytes, size, 0);

                        fclose(stacFile);
                        free(fileBytes);
                    }
                }

                free(reqFile);
            }
        }

        closesocket(client);
        
        if (getchar() == 'q') {
            break;
        }
    }

    closesocket(serverSock);
    WSACleanup();
    printf("cleaned-up the server\n");

    return 0;
}