#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

//#define MAX_NUM_OF_ROOMS 10
#define ROOM_PORTS_START 400
#define NUM_OF_ROOM_MEMBERS 3
#define DELAY 0.01
#define COMPUTER_REQUEST "computer\n"
#define ELECTRICAL_REQUEST "electrical\n"
#define CIVIL_REQUEST "civil\n"
#define MECHANIC_REQUEST "mechanic\n"


int setupServer(int port) {
    struct sockaddr_in address;
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    
    listen(server_fd, 4);

    return server_fd;
}

int acceptClient(int server_fd) {
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*) &address_len);

    return client_fd;
}

int make_room(int *members, int* num_of_rooms) {
    /*int sock, broadcast = 1, opt = 1;
    struct sockaddr_in bc_address;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(ROOM_PORTS_START + *num_of_rooms ); 
    bc_address.sin_addr.s_addr = inet_addr("192.168.1.255");*/

    int room = socket(AF_INET, SOCK_DGRAM, 0);
    char buff[1024] = {0};
    for(int i = 0; i < NUM_OF_ROOM_MEMBERS; i++){
        memset(buff, 0, 1024);
        sprintf(buff, "%d", ROOM_PORTS_START + *num_of_rooms);
        send(members[i], buff, strlen(buff), 0);
        sleep(0.01);
        memset(buff, 0, 1024);
        sprintf(buff, "%d", i);
        send(members[i], buff, strlen(buff), 0);
        sleep(0.01);
        /*memset(buff, 0, 1024);
        sprintf(buff, "%d", room);
        sleep(0.001);
        send(members[i], buff, strlen(buff), 0);*/
    }

    (*num_of_rooms)++;
    return *num_of_rooms;
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, max_sd;
    char buffer[1024] = {0};
    fd_set master_set, working_set, comp_set, elec_set, civi_set, mech_set;
    int comp_set_ind, elec_set_ind, civi_set_ind, mech_set_ind, num_of_rooms;
    comp_set_ind = elec_set_ind = civi_set_ind = mech_set_ind = num_of_rooms = 0;

    server_fd = setupServer(atoi(argv[1]));

    FD_ZERO(&master_set);
    max_sd = server_fd;
    FD_SET(server_fd, &master_set);

    write(1, "Server is running\n", 18);

    int comp[NUM_OF_ROOM_MEMBERS];
    int comp_index = 0;
    int elec[NUM_OF_ROOM_MEMBERS];
    int elec_index = 0;
    int civi[NUM_OF_ROOM_MEMBERS];
    int civi_index = 0;
    int mech[NUM_OF_ROOM_MEMBERS];
    int mech_index = 0;

    while (1) {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_sd; i++) {
            if (FD_ISSET(i, &working_set)) {
                
                if (i == server_fd) {  // new clinet
                    new_socket = acceptClient(server_fd);
                    FD_SET(new_socket, &master_set);
                    if (new_socket > max_sd)
                        max_sd = new_socket;
                    printf("New client connected. fd = %d\n", new_socket);
                }
                
                else { // client sending msg
                    int bytes_received;
                    bytes_received = recv(i , buffer, 1024, 0);
                    
                    if (bytes_received == 0) { // EOF
                        printf("client fd = %d closed\n", i);
                        close(i);
                        FD_CLR(i, &master_set);
                        continue;
                    }

                    printf("client %d: %s\n", i, buffer);
                    //printf("compare result : %d \n", strcmp(buffer, "a\n"));
                    if (strcmp(buffer, COMPUTER_REQUEST) == 0){ // computer request
                        comp[comp_index] = i;
                        comp_index++;
                        if (comp_index == NUM_OF_ROOM_MEMBERS){
                            int new_room = make_room(comp, &num_of_rooms);
                            FD_SET(new_room, &comp_set);
                            comp_set_ind++;
                            printf("create computer room with id : %d\n" , comp_set_ind);
                            comp_index = 0;
                        }
                        else{
                            printf("add a client to computer waiting queue.\n");
                            printf("in this queue exist %d client.\n", comp_index);
                        }
                    }
                    else if (strcmp(buffer, ELECTRICAL_REQUEST) == 0){ // electrical request
                        elec[elec_index] = i;
                        elec_index++;
                        if (elec_index == NUM_OF_ROOM_MEMBERS){
                            int new_room = make_room(elec, &num_of_rooms);
                            FD_SET(new_room, &elec_set);
                            elec_set_ind++;
                            printf("create electrical room with id : %d\n" , elec_set_ind);
                            elec_index = 0;
                        }
                        else{
                            printf("add a client to electrical waiting queue.\n");
                            printf("in this queue exist %d client.\n", elec_index);
                        }
                    }
                    else if (strcmp(buffer, CIVIL_REQUEST) == 0){ // civil request
                        civi[civi_index] = i;
                        civi_index++;
                        if (civi_index == NUM_OF_ROOM_MEMBERS){
                            int new_room = make_room(civi, &num_of_rooms);
                            FD_SET(new_room, &civi_set);
                            civi_set_ind++;
                            printf("create civil room with id : %d\n" , civi_set_ind);
                            civi_index = 0;
                        }
                        else{
                            printf("add a client to civil waiting queue.\n");
                            printf("in this queue exist %d client.\n", civi_index);
                        }
                    }
                    else if (strcmp(buffer, MECHANIC_REQUEST) == 0){ // mechanic request
                        mech[mech_index] = i;
                        mech_index++;
                        if (mech_index == NUM_OF_ROOM_MEMBERS){
                            int new_room = make_room(mech, &num_of_rooms);
                            FD_SET(new_room, &mech_set);
                            mech_set_ind++;
                            printf("create mechanic room with id : %d\n" , mech_set_ind);
                            mech_index = 0;
                        }
                        else{
                            printf("add a client to mechanic waiting queue.\n");
                            printf("in this queue exist %d client.\n", mech_index);
                        }
                    }
                    memset(buffer, 0, 1024);
                }
            }
        }

    }

    return 0;
}