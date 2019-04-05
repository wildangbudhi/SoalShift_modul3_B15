#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#define PORT 8000

void *runCommand( void *ptr );

int *stock, new_socket;

int main(int argc, char const *argv[]) {
    int server_fd, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);  
      
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    key_t key = 1234;

    int shmid = shmget(key, sizeof(int), IPC_CREAT | 0666);
    stock = shmat(shmid, NULL, 0);    

    pthread_t threads;
    pthread_create(&threads, NULL, runCommand, NULL);
    pthread_join(threads, NULL);

    shmdt(stock);
    shmctl(shmid, IPC_RMID, NULL);
    
    return 0;
}

void *runCommand( void *ptr ){

    char buffer[1024], *massage;

    while(1){
        read( new_socket , buffer, 1024);

         if(!strcmp(buffer, "beli")){
            if(*stock > 0){
                *stock = *stock - 1;
                massage = "transaksi berhasil";
            }
            else massage = "transaksi gagal";

            send(new_socket , massage , strlen(massage) , 0 );
         }
    }
}