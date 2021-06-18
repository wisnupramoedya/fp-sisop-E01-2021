#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define D_BUFF 300

const int S_BUFF = sizeof(char) * D_BUFF;
char nama[D_BUFF] = {0};
char *tipe = NULL;
bool stat = false;

int create_tcp_Csocket();
void *input_handler(void *client_fd);
void *output_handler(void *client_fd);
void receiveSOutput(int fd, char *inp);
bool login(int fd, int argc, char *argv[]);

int main(int argc, char *argv[]) {
    pthread_t t_id[2];
    int client_fd = create_tcp_Csocket();

    if (!login(client_fd, argc, argv)) {
        return -1;
    }

    pthread_create(&(t_id[0]), NULL, &output_handler, (void *)&client_fd);
    pthread_create(&(t_id[1]), NULL, &input_handler, (void *)&client_fd);

    pthread_join(t_id[0], NULL);
    pthread_join(t_id[1], NULL);

    close(client_fd);
    return 0;
}

bool login(int fd, int argc, char *argv[]) {
    char dummy[D_BUFF];
    if (geteuid() == 0) { 
        write(fd, "login, root", S_BUFF);
        puts("login, root");
        strcpy(nama, "root");
        tipe = "root";
    } else if (argc == 5 && strcmp(argv[1], "-u") == 0 &&
               strcmp(argv[3], "-p") == 0) { 
        sprintf(dummy, "login, %s %s", argv[2], argv[4]);
        write(fd, dummy, S_BUFF);
        puts(dummy);
        strcpy(nama, argv[2]);
        tipe = "user";
    } else {
        puts("Invalid argument");
        return false;
    }
    read(fd, dummy, S_BUFF);
    puts(dummy);
    return strcmp(dummy, "Login success\n") == 0;
}

void *input_handler(void *client_fd) {
    int fd = *(int *)client_fd;
    char mssg[D_BUFF] = {0};

    while (1) {
        if (stat) continue;
        printf("%s@%s: ", tipe, nama);
        fgets(mssg, D_BUFF, stdin);
        char *tmp = strtok(mssg, "\n");
        if (tmp != NULL) {
            strcpy(mssg, tmp);
        }
        if (strcmp(mssg, "quit") == 0) {
            exit(EXIT_SUCCESS);
        }
        send(fd, mssg, S_BUFF, 0);
        stat = true;
    }
}

void *output_handler(void *client_fd) {
    int fd = *(int *)client_fd;
    char mssg[D_BUFF] = {0};

    while (1) {
        memset(mssg, 0, S_BUFF);
        receiveSOutput(fd, mssg);
        printf("%s", mssg);
        fflush(stdout);
        stat = false;
    }
}

void receiveSOutput(int fd, char *inp) {
    if (recv(fd, inp, D_BUFF, 0) == 0) {
        printf("Disconnected\\n");
        exit(EXIT_SUCCESS);
    }
}

int create_tcp_Csocket() {
    struct sockaddr_in saddr;
    int fd, ret_val;
    int opt = 1;
    struct hostent *local_host;

    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == -1) {
        fprintf(stderr, "socket failed [%s]\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    printf("Created a socket, fd: %d\n", fd);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(7000);
    local_host = gethostbyname("127.0.0.1");
    saddr.sin_addr = *((struct in_addr *)local_host->h_addr);
    // saddr.sin_addr.s_addr = INADDR_ANY;

    ret_val =
        connect(fd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
    if (ret_val == -1) {
        fprintf(stderr, "connect failed [%s]\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fd;
}