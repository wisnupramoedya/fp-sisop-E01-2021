#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define SIZE 50
#define SIZE_SENT 15
#define SIZE_RECV 1024

char message_sent[SIZE_SENT];
char message_recv[SIZE_RECV];

int create_socket_connection(struct sockaddr_in *server_address, int sock);
bool format_login(int argc, char const *argv[], char *message);

int main(int argc, char const *argv[]) {
    int sock = 0, valread;
    struct sockaddr_in server_address;
    sock = create_socket_connection(&server_address, sock);
    if (!format_login(argc, argv, message_sent)) {
        printf("wrong username/password\n");
        return -1;
    }

    // cek login
    if (connect(sock, (struct sockaddr *)&server_address,
                sizeof(server_address)) < 0) {
        printf("connection failed \n");
        return -1;
    }
    send(sock, message_sent, strlen(message_sent), 0);
    valread = read(sock, message_recv, SIZE_RECV);
    printf("result:\n%s", message_recv);

    while (1) {
        // fflush(stdin);
        printf("oursql# ");
        // scanf("%s", message_sent);
        // getchar();
        // char *eof = fgets(message_sent, sizeof(message_sent), stdin);
        // if (eof == NULL) return -EXIT_FAILURE;

        printf("mesage sent");
        send(sock, message_sent, strlen(message_sent), 0);
        valread = read(sock, message_recv, SIZE_RECV);
        printf("result:\n%s", message_recv);
    }

    return 0;
}

bool format_login(int argc, char const *argv[], char *message) {
    printf("argc:%d argv:%s %s\n", argc, argv[0], argv[1]);
    if (argc == 1) {
        printf("check root access...\n");
        sprintf(message, "root");
    } else if (argc == 5 && !strcmp(argv[1], "-u") && !strcmp(argv[3], "-p")) {
        printf("check %s access...\n", argv[2]);
        sprintf(message, "%s:%s", argv[2], argv[4]);
    } else {
        return 0;
    }
    return 1;
}

int create_socket_connection(struct sockaddr_in *server_address, int sock) {
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket creation error\n");
        exit(EXIT_FAILURE);
    }

    memset(server_address, '0', sizeof(server_address));
    server_address->sin_family = AF_INET;
    server_address->sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_address->sin_addr) <= 0) {
        printf("invalid address/address not supported \n");
        exit(EXIT_FAILURE);
    }

    return sock;
}