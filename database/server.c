#include <dirent.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <dirent.h>
 
#define D_BUFF 300

const int S_BUFF = sizeof(char) * D_BUFF; 
int fd_now = -1;
int id_now = -1;
 
const char *dirNow = "/home/wisnupramoedya/sisop/fp-sisop-E01-2021/database/databases";
const char *TABLE_OF_USERS = "./list_user_pass.csv";
const char *PERM_TABLE = "/list_user_db.csv";
const char *LOG_FILE = "./databases/db.log";
char thisDataB[300];

int create_tcp_server_socket();
void *daemonize(pid_t *pid, pid_t *sid);
void *prog(void *argv);
bool login(int fd, char *nama, char *sandi);
void createAcc(int fd, char *nama, char *sandi);
int getUserId(const char *directoryp, char *nama, char *sandi);
int getLastId(const char *directoryp);
int whatId(const char *directoryp, char *nama);
bool isDBx(const char *directoryp, char *thisdb);
bool isGranted(const char *directoryp, char *thisdb);
void logging(char *nama, const char *command);

int main()
{
    // pid_t pid, sid;
    // daemonize(&pid, &sid);
    socklen_t addrlen;
    struct sockaddr_in new_addr;
    pthread_t t_id;
    char dummy[D_BUFF];
    int server_fd = create_tcp_server_socket();
    int new_fd;
 
    while (1) {
        new_fd = accept(server_fd, (struct sockaddr *)&new_addr, &addrlen);
        if (new_fd >= 0) {
            printf("Accepted a new connection, fd: %d\n", new_fd);
            pthread_create(&t_id, NULL, &prog, (void *) &new_fd);
        } else {
            fprintf(stderr, "Accept failed [%s]\n", strerror(errno));
        }
    } 
    return 0;
}

void logging(char *nama, const char *command) {
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    FILE *fp = fopen(LOG_FILE, "a");
    fprintf(fp, "%d-%02d-%02d %02d:%02d:%02d:%s:%s\n", timeinfo->tm_year + 1900,
            timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour,
            timeinfo->tm_min, timeinfo->tm_sec, nama, command);

    fclose(fp);
}
 
void *prog(void *argv)
{
    chdir(dirNow); 
    int fd = *(int *) argv;
    char query[D_BUFF], dummy[D_BUFF];
 
    while (read(fd, query, D_BUFF) != 0) {
        puts(query);
 
        strcpy(dummy, query);
        char *req = strtok(dummy, " ");
 
        if (strcmp(req, "LOGIN") == 0) {
            char *nama = strtok(NULL, " ");
            char *sandi = (strcmp(nama, "root") != 0) 
                            ? strtok(NULL, " ") : "root";
            if (login(fd, nama, sandi) == false){
                puts(nama);
                break;
            }
        }
        else if (strcmp(req, "CREATE") == 0) {
            req = strtok(NULL, " ");
 
            if (strcmp(req, "USER") == 0) {
                if (id_now == 0) {
                    char *nama = strtok(NULL, " ");
                    char *sandi = strtok(NULL, " ");
                    for (int i = 0; i < 2; i++) {
                        sandi = strtok(NULL, " ");
                    }
                    // puts(nama);
                    // puts(sandi);
                    createAcc(fd, nama, sandi);
                } else {
                    write(fd, "Cant access\n\n", S_BUFF);
                }
            }
            else if(strcmp(req, "DATABASE") == 0){
                req = strtok(NULL, " ");
                int cek = mkdir(req, 0777);
                if (!cek){
                    char directoryp[D_BUFF];
                    sprintf(directoryp, "%s/%s%s", dirNow, req, PERM_TABLE);
                    puts(directoryp);
                    // printf("%d\n", id_now);
                    FILE *baru = fopen(directoryp, "a");
                    fprintf(baru, "%d\n", id_now);
                    fclose(baru);
                    write(fd, "Database created\n\n", S_BUFF);
                }
                else {
                    write(fd, "Unable to create database\n", S_BUFF);
                }

            }
            else {
                write(fd, "Wrong query\n\n", S_BUFF);
            }
        }
        else if(strcmp(req, "GRANT") == 0){
            req = strtok(NULL, " ");
            if(strcmp(req, "PERMISSION") == 0){
                if (id_now == 0) {
                    char *thisdb = strtok(NULL, " ");
                    char *into = strtok(NULL, " ");
                    char *thisuser = strtok(NULL, " ");
                    if (strcmp(into, "INTO") == 0) {
                        char directoryp[D_BUFF];
                        sprintf(directoryp, "%s/%s%s", dirNow, thisdb, PERM_TABLE);
                        puts(directoryp);
                        FILE *baru = fopen(directoryp, "a+");
                        int thisid = -1;
                        thisid = whatId(TABLE_OF_USERS, thisuser);
                        if (thisid != -1) {
                            fprintf(baru, "%d\n", thisid);
                            write(fd, "User Granted\n\n", S_BUFF);
                        } else {
                            write(fd, "Wrong user\n\n", S_BUFF);
                        }
                        fclose(baru);
                    } else {
                        write(fd, "Wrong query\n\n", S_BUFF);
                    }
                } else {
                    write(fd, "Cant access\n\n", S_BUFF);
                }
            } else {
                write(fd, "Wrong query\n\n", S_BUFF);
            }
        } else if (strcmp(req, "USE") == 0) {
            req = strtok(NULL, " ");
            if (isDBx(dirNow, req) && isGranted(dirNow, req)) {
                strcpy(thisDataB, req);
                puts(thisDataB);
                write(fd, "Database Granted\n\n", S_BUFF);
            } else {
                write(fd, "Wrong Database or Cant access\n\n",
                      S_BUFF);
            }
        } else {
            write(fd, "Wrong query\n\n", S_BUFF);
        }
    }
    if (fd == fd_now) {
        fd_now = id_now = -1;
    }
    printf("Close connection, fd: %d\n", fd);
    close(fd);
}
 
/****   Controllers   *****/
void createAcc(int fd, char *nama, char *sandi)
{
    FILE *fp = fopen(TABLE_OF_USERS, "a+");
    int ID = getUserId(TABLE_OF_USERS, nama, sandi);
 
    if (ID != -1) {
        write(fd, "User already registered\n\n", S_BUFF);
    } else {
        ID = getLastId(TABLE_OF_USERS) + 1;
        fprintf(fp, "%d,%s,%s\n", ID, nama, sandi);
        write(fd, "create Account success\n\n", S_BUFF);
    }
    fclose(fp);
}
 
bool login(int fd, char *nama, char *sandi)
{
    if (fd_now != -1) {
        write(fd, "Server can only handle 1 user, please wait.\n", S_BUFF);
        return false;
    }
 
    int ID = -1;
    if (strcmp(nama, "root") == 0) {
        ID = 0;
    } else { // Check data in DB
        FILE *fp = fopen(TABLE_OF_USERS, "r");
        if (fp != NULL) ID = getUserId(TABLE_OF_USERS, nama, sandi);
        fclose(fp);
    }
 
    if (ID == -1) {
        write(fd, "Wrong password or username\n", S_BUFF);
        return false;
    } else {
        write(fd, "Proceed to Menu\n", S_BUFF);
        fd_now = fd;
        id_now = ID;
    }
    return true;
}
 
/*****  HELPER  *****/
int getUserId(const char *directoryp, char *nama, char *sandi)
{
    int ID = -1;
    FILE *fp = fopen(directoryp, "r");
 
    if (fp != NULL) {
        char db[D_BUFF], input[D_BUFF];
        sprintf(input, "%s,%s", nama, sandi);
 
        while (fscanf(fp, "%s", db) != EOF) {
            char *temp = strstr(db, ",") + 1; 
            if (strcmp(temp, input) == 0) {
                ID = atoi(strtok(db, ",")); 
                break;
            }
        }
        fclose(fp);
    }
    return ID;
}
 
int getLastId(const char *directoryp)
{
    int ID = 1;
    FILE *fp = fopen(directoryp, "r");
 
    if (fp != NULL) {
        char db[D_BUFF];
        while (fscanf(fp, "%s", db) != EOF) {
            ID = atoi(strtok(db, ","));  
        }
    }
    return ID;
}

int whatId(const char *directoryp, char *nama) {
    int ID = -1;
    FILE *fp = fopen(directoryp, "r");

    if (fp != NULL) {
        char db[D_BUFF];

        while (fscanf(fp, "%s", db) != EOF) {
            char *temp = strstr(db, ",") + 1; 
            char *temp2 = strstr(db, nama);
            if (strcmp(temp, temp2) == 0) {
                ID = atoi(strtok(db, ",")); 
                break;
            }
        }
        fclose(fp);
    }
    return ID;
}

bool isDBx(const char *directoryp, char *thisdb) {
    char temp[S_BUFF];
    sprintf(temp, "%s/%s", directoryp, thisdb);
    DIR *dir = opendir(temp);
    if (dir) {
        return true;
        closedir(dir);
    } else {
        return false;
    }
}

bool isGranted(const char *directoryp, char *thisdb) {
    bool ID = false;
    char temp[S_BUFF];
    sprintf(temp, "%s/%s%s", directoryp, thisdb, PERM_TABLE);
    FILE *fp = fopen(temp, "r");

    if (fp != NULL) {
        char db[D_BUFF];

        while (fscanf(fp, "%s", db) != EOF) {
            int angka = atoi(db);
            if (angka == id_now) {
                ID = true;
                break;
            }
        }
        fclose(fp);
    }
    return ID;
}

int create_tcp_server_socket()
{
    struct sockaddr_in saddr;
    int fd, ret_val;
    int opt = 1;
 
    /* Step1: create a TCP socket */
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == -1) {
        fprintf(stderr, "socket failed [%s]\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    printf("Created a socket, fd: %d\n", fd);
 
    /* Initialize the socket address structure */
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(7000);
    saddr.sin_addr.s_addr = INADDR_ANY;
 
    /* Step2: bind the socket to port 7000 on the local host */
    ret_val = bind(fd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
    if (ret_val != 0) {
        fprintf(stderr, "bind failed [%s]\n", strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }
 
    /* Step3: listen for incoming connections */
    ret_val = listen(fd, 5);
    if (ret_val != 0) {
        fprintf(stderr, "listen failed [%s]\n", strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }
    return fd;
}
 
void *daemonize(pid_t *pid, pid_t *sid)
{
    int status;
    *pid = fork();
 
    if (*pid != 0) {
        exit(EXIT_FAILURE);
    }
    if (*pid > 0) {
        exit(EXIT_SUCCESS);
    }
    umask(0);
 
    *sid = setsid();
    if (*sid < 0 || chdir(dirNow) < 0) {
        exit(EXIT_FAILURE);
    }
 
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}