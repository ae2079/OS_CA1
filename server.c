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
#define MAX_NUM_OF_CLIENTS 1000
#define DELAY 0.01
#define COMPUTER_REQUEST "computer\n"
#define ELECTRICAL_REQUEST "electrical\n"
#define CIVIL_REQUEST "civil\n"
#define MECHANIC_REQUEST "mechanic\n"
#define COMP_FILE "comp.txt"
#define ELEC_FILE "elec.txt"
#define CIVI_FILE "civi.txt"
#define MECH_FILE "mech.txt"

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

int make_room(int *members, int* num_of_rooms, int server_port) {
    char buff[10] = {0};
    (*num_of_rooms)++;

    for(int i = 0; i < NUM_OF_ROOM_MEMBERS; i++){
        memset(buff, 0, 10);
        sprintf(buff, "%d", server_port + *num_of_rooms);
        send(members[i], buff, strlen(buff), 0);
        sleep(0.01);
        memset(buff, 0, 10);
        sprintf(buff, "%d", i);
        send(members[i], buff, strlen(buff), 0);
        sleep(0.01);
    }

    return *num_of_rooms;
}

void save_to_proper_file(char *buffer, int i, int comp_set[MAX_NUM_OF_CLIENTS], int elec_set[MAX_NUM_OF_CLIENTS], int civi_set[MAX_NUM_OF_CLIENTS], int mech_set[MAX_NUM_OF_CLIENTS], int comp_set_ind, int elec_set_ind, int civi_set_ind, int mech_set_ind){
    char file_name[100];
    for(int j = 0; j < comp_set_ind; j++){
        if(comp_set[j] == i){
            strcpy(file_name, COMP_FILE);
        }
    }
    for(int j = 0; j < elec_set_ind; j++){
        if(elec_set[j] == i){
            strcpy(file_name, ELEC_FILE);
        }
    }
    for(int j = 0; j < civi_set_ind; j++){
        if(civi_set[j] == i){
            strcpy(file_name, CIVI_FILE);
        }
    }
    for(int j = 0; j < mech_set_ind; j++){
        if(mech_set[j] == i){
            strcpy(file_name, MECH_FILE);
        }
    }
    int file = open(file_name, O_CREAT | O_APPEND | O_WRONLY);
    write(file, buffer, strlen(buffer));
    close(file);
}

void clear_from_sets(int i, int *comp_set, int *elec_set, int *civi_set, int *mech_set, int comp_set_ind, int elec_set_ind, int civi_set_ind, int mech_set_ind){
    for(int j = 0; j < comp_set_ind; j++){
        if(comp_set[j] == i){
            comp_set[j] = -1;
        }
    }
    for(int j = 0; j < elec_set_ind; j++){
        if(elec_set[j] == i){
            elec_set[j] = -1;
        }
    }
    for(int j = 0; j < civi_set_ind; j++){
        if(civi_set[j] == i){
            civi_set[j] = -1;
        }
    }
    for(int j = 0; j < mech_set_ind; j++){
        if(mech_set[j] == i){
            mech_set[j] = -1;
        }
    }
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, max_sd, server_port;
    char buffer[NUM_OF_ROOM_MEMBERS*1024] = {0};
    fd_set master_set, working_set;
    int comp_set_ind, elec_set_ind, civi_set_ind, mech_set_ind, num_of_rooms;
    comp_set_ind = elec_set_ind = civi_set_ind = mech_set_ind = num_of_rooms = 0;
    int comp_set[MAX_NUM_OF_CLIENTS], elec_set[MAX_NUM_OF_CLIENTS], civi_set[MAX_NUM_OF_CLIENTS], mech_set[MAX_NUM_OF_CLIENTS];
    
    server_port = atoi(argv[1]);
    server_fd = setupServer(server_port);

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
                    bytes_received = recv(i , buffer, 1024*NUM_OF_ROOM_MEMBERS, 0);
                    
                    if (bytes_received == 0) { // EOF
                        printf("client fd = %d closed\n", i);
                        close(i);
                        FD_CLR(i, &master_set);
                        clear_from_sets(i, comp_set, elec_set, civi_set, mech_set, comp_set_ind, elec_set_ind, civi_set_ind, mech_set_ind);
                        continue;
                    }

                    printf("client %d: %s\n", i, buffer);

                    if (strcmp(buffer, COMPUTER_REQUEST) == 0){ // computer request
                        comp[comp_index] = i;
                        comp_index++;
                        comp_set[comp_set_ind] = i;
                        comp_set_ind++;
                        if (comp_index == NUM_OF_ROOM_MEMBERS){
                            make_room(comp, &num_of_rooms, server_port);
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
                        elec_set[elec_set_ind] = i;
                        elec_set_ind++;
                        if (elec_index == NUM_OF_ROOM_MEMBERS){
                            make_room(elec, &num_of_rooms, server_port);
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
                        civi_set[civi_set_ind] = i;
                        civi_set_ind++;
                        if (civi_index == NUM_OF_ROOM_MEMBERS){
                            make_room(civi, &num_of_rooms, server_port);
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
                        mech_set[mech_set_ind] = i;
                        mech_set_ind++;
                        if (mech_index == NUM_OF_ROOM_MEMBERS){
                            make_room(mech, &num_of_rooms, server_port);
                            printf("create mechanic room with id : %d\n" , mech_set_ind);
                            mech_index = 0;
                        }
                        else{
                            printf("add a client to mechanic waiting queue.\n");
                            printf("in this queue exist %d client.\n", mech_index);
                        }
                    }
                    else{
                        save_to_proper_file(buffer, i, comp_set, elec_set, civi_set, mech_set, comp_set_ind, elec_set_ind, civi_set_ind, mech_set_ind);
                    }
                    memset(buffer, 0, 1024*NUM_OF_ROOM_MEMBERS);
                }
            }
        }

    }

    return 0;
}