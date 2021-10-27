#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

int connectServer(int port) {
    int fd;
    struct sockaddr_in server_address;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(port); 
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) { // checking for errors
        printf("Error in connecting to server\n");
    }

    return fd;
}


int main(int argc, char const *argv[]) {
    int fd;
    char buff[1024] = {0};
    int room = 0;

    fd = connectServer(8080);

    fd_set working_set;
    FD_ZERO(&working_set);
    FD_SET(0, &working_set);
    FD_SET(fd, &working_set);

    select(2, &working_set, NULL, NULL, NULL);

    while(room == 0){
        if(FD_ISSET(0, &working_set)){
            read(0, buff, 1024);
            send(fd, buff, strlen(buff), 0);
            memset(buff, 0, 1024);
        }
        if(FD_ISSET(0, &working_set)){
            recv(fd, buff, 1024, 0);
            room = atoi(buff);
            printf("room : %d\n" , room);
            memset(buff, 0, 1024);
        }
    }

    
    // chat in the room



    printf("end of client work.");

    return 0;
}