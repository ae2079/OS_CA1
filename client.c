#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define NUM_OF_ROOM_MEMBERS 3

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

// write printf with sprintf and write?
int main(int argc, char const *argv[]) {
    int server, port, order, room, broadcast = 1, opt = 1;
    char buff[1024] = {0};
    struct sockaddr_in bc_address;

    server = connectServer(atoi(argv[1]));

    //fd_set working_set;
    //FD_ZERO(&working_set);
    //FD_SET(0, &working_set);
    //FD_SET(server, &working_set);

    //select(2, &working_set, NULL, NULL, NULL);

    printf("what type of room do you want?\n");
    read(0, buff, 1024);
    send(server, buff, strlen(buff), 0);
    memset(buff, 0, 1024);

    recv(server, buff, 1024, 0);
    port = atoi(buff);
    memset(buff, 0, 1024);
    recv(server, buff, 1024, 0);
    order = atoi(buff);
    memset(buff, 0, 1024);
    /*recv(server, buff, 1024, 0);
    room = atoi(buff);
    memset(buff, 0, 1024);*/
    //port = 404;
    /*for(int i = 0; i< 10; i++){
        room = socket(AF_INET, SOCK_DGRAM, 0);
    }*/
    room = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(room, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(room, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(port); 
    bc_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("You entered to a room.\nRoom id : %d\nYour id in room : %d\n" , room, order);

    bind(room, (struct sockaddr *)&bc_address, sizeof(bc_address));

    for(int i = 0; i < NUM_OF_ROOM_MEMBERS; i++){
        if(i == order){
            char answers[NUM_OF_ROOM_MEMBERS][1024];
            printf("it is your turn to ask a question.\nwrite your question :\n");
            read(0, buff, 1024);
            sprintf(answers[0], "question : %s\n", buff);
            sendto(room, buff, strlen(buff), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
            ////////////////
            //sleep(1);
            printf("sent!\n to room : %d \n", room);
            //////////
            memset(buff, 0, 1024);
            for(int j = 0; j < NUM_OF_ROOM_MEMBERS - 1; j++){
                recv(room, buff, 1024, 0);
                sprintf(answers[j+1], "answer%d : %s\n", j+1, buff); 
                printf("answer%d : %s\n", j+1, buff);
                memset(buff, 0, 1024);
            }
            printf("select best answer : ");
            read(0, buff, 1024);
            sendto(room, buff, strlen(buff), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
            int index = atoi(buff);
            memset(buff, 0, 1024);
            answers[index][strlen(answers[index])] = '*';
            //send_answers_to_server(answers);
        }
        else{
            ////////////////
            printf("wait! in room : %d\n", room);
            //////////
            //sleep(1);
            //////////
            recv(room, buff, 1024, 0);
            ////////////////
            printf("recive!\n");
            //////////
            printf("question : %s\n", buff);
            memset(buff, 0, 1024);
            int turn = (i < order) ? (order - 1) : order;
            for(int j = 0; j < NUM_OF_ROOM_MEMBERS; j++){
                if(j == turn){
                    printf("it is your turn to answer to the question.\nwrite your answer : ");
                    read(0, buff, 1024);
                    sendto(room, buff, strlen(buff), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
                    memset(buff, 0, 1024);
                }
                else{
                    recv(room, buff, 1024, 0);
                    printf("answer%d : %s\n", j+1, buff);
                    memset(buff, 0, 1024);
                }   
            }
        }
    }
    
    // chat in the room



    printf("end of client work.\n");
    send(server, buff, strlen(buff), 0);
    return 0;
}