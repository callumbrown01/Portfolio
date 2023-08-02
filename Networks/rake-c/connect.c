#include "globals.h"

//Get the socket number for the host and port provided
int get_sock(char *host, int port)
{
    int sock;
    time_t t;

    srand((unsigned) time(&t));
    
    //Structures for handling internet addresses
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, host, &server_address.sin_addr);
    server_address.sin_port = htons(port);

    //Creation of socket
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("ERROR: Socket creation failed\n");
        return -1;
    }
    
    //Connecting to the socket
    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        printf("ERROR: Unable to connect to server\n");
        return -1;
    }
    printf("Connected to %s\n", host); //Tells us which host we connected to

    return sock;
}

void close_connection(int sock)
{
    close(sock);
    printf("Disconnected from Host\n");
}