#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#define PORT 8080
#define SIZE_SENT 1024
#define SIZE_RECV 300

char message_sent[SIZE_SENT];
char message_recv[SIZE_RECV];

int create_socket_connection(struct sockaddr_in *address, int server_fd);
void createDaemon(pid_t *pid, pid_t *sid);

int main(int argc, char const *argv[]) {
    // Untuk daemon
    // pid_t pid, sid;
    // createDaemon(&pid, &sid);

    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = create_socket_connection(&address, server_fd);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                 (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        valread = read(new_socket, message_recv, SIZE_RECV);
        printf("get:\n%s\n", message_recv);
        sprintf(message_sent, "done\n");
        send(new_socket, message_sent, strlen(message_sent), 0);
        printf("data has been sent\n");
    }
    return 0;
}

int create_socket_connection(struct sockaddr_in *address, int server_fd) {
    int opt = 1;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)address, sizeof(*address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

void createDaemon(pid_t *pid, pid_t *sid) {
    int status;
    *pid = fork();

    /* Keluar saat fork gagal
     * (nilai variabel pid < 0) */
    if (*pid < 0) {
        exit(EXIT_FAILURE);
    }

    /* Keluar saat fork berhasil
     * (nilai variabel pid adalah PID dari child process) */
    if (*pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);

    *sid = setsid();
    if (*sid < 0) {
        exit(EXIT_FAILURE);
    }

    if ((chdir("/")) < 0) {
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}