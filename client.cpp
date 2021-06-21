#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// 1. read the command line
// 2. split the command line into port and command
// 3. send the command to server

using namespace std;

struct arg_holder {
	int port;
	char* command;
};  

void * clientSocket(int port, char* command) {
	struct sockaddr_in serverAddr;
	
	serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
	
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1) { 
        printf("Error: create\n");
    }

    socklen_t addr_size = sizeof(serverAddr);
	
    if(connect(sock, (struct sockaddr *) &serverAddr, addr_size) < 0){
		printf("Error: connect\n");
	}
  
		char send_buffer[1024], receive_buffer[1024];
		memset(send_buffer, '\0', sizeof(send_buffer));
		
		strcpy(send_buffer, command);       
    
		if(send(sock, send_buffer, 1024, 0) < 0){
		  printf("ERROR: sending data\n");
		}

		if(recv(sock, receive_buffer, 1024, 0) < 0){
		  printf("ERROR: receiving data\n");       
		}
 
		std::cout << receive_buffer << std::endl;      
    
	
	close(sock);
	return 0;
}

void * thread_caller(void * arg) {
	
	struct arg_holder port_command = *(struct arg_holder *)arg;  
	
	return clientSocket(port_command.port, port_command.command);
}

int main(int argc, char *argv[]){
	struct arg_holder * port_command = (arg_holder*)malloc(sizeof(*port_command));
	
	port_command->port = stoi(argv[1]);
	port_command->command = argv[2];
	
	pthread_t thread;
	
	if(pthread_create(&thread, NULL, thread_caller, port_command) != 0){
		printf("Error: create thread\n");		
	}
	
	pthread_join(thread, NULL);
	
	free(port_command);
	delete port_command;
	
	return 0;
}