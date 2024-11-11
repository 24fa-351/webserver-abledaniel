#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h> 
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define PORT 6116
#define LISTEN_BACKLOG 5

//Global Varialbes
int requests = 0;
int total_recieved = 0;
int total_sent = 0;

void handleConnection(int* a_client_ptr){

    int a_client = *a_client_ptr;
    free(a_client_ptr);

    printf("Handling Connection: %d\n", a_client);
    char buffer[1024];
    int bytes_read = read(a_client, buffer, sizeof(buffer));
    printf("Received: %s\n", buffer);
    write(a_client, buffer, bytes_read);
    printf("Done with Connection %d\n", a_client);

    //web server
    char path[1000];
    char method[1000];
    char http_version[1000];


    //adds buffer, finds what type of method GET, POST etc and finds the path of the html file /index.html. Lastly finds the version of the HTTP protcol 1.1, 2 etc
    sscanf(buffer, "%s %s %s", method, path, http_version);

    //strcmp checks if the method and get are equal and strncmp checks the first 6 characters so only /calc/
    // in C both return as false if equal so ! is used
    if (!strcmp(method, "GET") && !strncmp(path, "/calc/", 6)){
        int a, b;
        sscanf(path, "/calc/%d/%d", &a, &b);
        dprintf(a_client, "HTTP/1.1 200 OK\nContent-Type: text/plain\n\n%d\n", a+b);
        write(a_client, buffer, bytes_read);
        total_recieved += bytes_read;
        total_sent += sizeof(write(a_client, buffer, bytes_read));
        requests += 1;
        printf("The Result is %d\n", a+b);
    }

    if (!strcmp(method, "GET") && !strncmp(path, "/stats/", 7)){
        total_recieved += bytes_read;
        dprintf(a_client, "HTTP/1.1 200 OK\nContent-Type: text/plain\n\n\n");
        write(a_client, buffer, bytes_read);
        total_sent += sizeof(write(a_client, buffer, bytes_read));
        requests += 1;
        printf("Total Recieved Bytes is %d\nTotal Sent Bytes is %d\nTotal Requests is %d\n", total_recieved, total_sent, requests);
    }

    if (!strcmp(method, "GET") && !strncmp(path, "/static/", 8)){
        dprintf(a_client, "HTTP/1.1 200 OK\nContent-Type: text/plain\n\n\n");
        int file = open(path+8, O_RDONLY);
        write(a_client, buffer, bytes_read);
        close(file);
        total_recieved += bytes_read;
        total_sent += sizeof(write(a_client, buffer, bytes_read));
        requests += 1;
    }
}

int main(int argc, char*argv[])
{   //I'm assuming it gets the current socket from the 
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in socket_address;
    // example of socketaddr

    memset(&socket_address, 0, sizeof(socket_address));
    //sets all of socket_address to 0 

    
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons(PORT);
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);

    
    int return_val = bind(socket_fd, (struct sockaddr*)&socket_address, sizeof(socket_address));
    
    return_val = listen(socket_fd, LISTEN_BACKLOG);

    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    //The third argument in the function points to a socklen_t so you must use a referencing variable since referencing
    //allows the address to update its len
    // int client_fd = accept(socket_fd, 
    // (struct sockaddr*)&client_address, &client_address_len);

    // handleConnection(&client_fd);

    //runs while loop to create threads for multiprocessing
    while(1){
        pthread_t thread;
        int *client_fd_buff = malloc(sizeof(int));

        *client_fd_buff = accept(socket_fd, (struct sockaddr*)&client_address, &client_address_len);

        //thread stores value, No attribute, the function, and the argument it'll use 
        //the thread function must acceot and return void* but also can dynamically choose 
        pthread_create(&thread, NULL, (void* (*) (void*))handleConnection, (void*)client_fd_buff);

        printf("Accepted Connection on %d\n", *client_fd_buff);

    }
    return 0;
}
