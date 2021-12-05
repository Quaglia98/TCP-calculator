#if defined WIN32
#include <winsock.h>
#else
#define closesocket close
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#endif
#include <stdio.h>
#include <stdlib.h> // for atoi()
#include "protocol.h"



void errorhandler(char *error_message) {
	printf("%s",error_message);
}




void clearwinsock() {
	#if defined WIN32
	WSACleanup();
	#endif
}




int main(int argc, char *argv[]) {

    int port;
    char*IP;

    //IP and port specified
	if (argc > 2) {
	port = atoi(argv[2]);
	IP = argv[1];
	}

	else if (argc > 1){
		printf("Port or IP missing\n");
		return 0;
	}

	//default parameters
	else{
		IP = "127.0.0.1";
		port = PROTOPORT;
	}

	if (port < 0) {
	printf("bad port number %s \n", argv[1]);
	return 0;
	}


	#if defined WIN32
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2 ,2), &wsa_data);
	if (result != 0) {
		printf ("error at WSASturtup\n");
	}
	//
	#endif

	char rcvbuff[BUFFERSIZE];
	char sendbuff[BUFFERSIZE];



	//Create the socket
	int client_socket;
	client_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client_socket < 0) {
		errorhandler("socket creation failed.\n");
		closesocket(client_socket);
		clearwinsock();
		//
	}


	//Make the address of the server
	struct sockaddr_in serveradd;
	memset(&serveradd, 0, sizeof(serveradd));
	serveradd.sin_family = AF_INET;
	serveradd.sin_addr.s_addr = inet_addr(IP); // IP server
	serveradd.sin_port = htons(port); // Server port
    printf("Server: %s:%d\n", inet_ntoa(serveradd.sin_addr), htons(serveradd.sin_port));
	printf("Connection request...\n");

	//Connect to server
	if (connect(client_socket, (struct sockaddr *)&serveradd, sizeof(serveradd)) < 0) {
		errorhandler( "Failed to connect.\n" );
		closesocket(client_socket);
		clearwinsock();
		system("pause");
		return -1;
	}

	else printf("Successfully connected to the server\n");




	do{
		printf("\nEnter your expression or close the connection with '='.\n") ;
		printf("[operator] space [operand1] space [operand2]\n");
		fgets(sendbuff, sizeof(sendbuff), stdin);
		sendbuff[strlen(sendbuff)-1] = '\0';

			if(send(client_socket, sendbuff, strlen(sendbuff), 0) < 0){
				errorhandler("Error while sending Buffer.\n");
				break;
			}
			if(recv (client_socket, rcvbuff, sizeof(rcvbuff),0) < 0){
				errorhandler("Error while receiving Buffer.\n");
				break;
			}

			if(strcmp("=", sendbuff)!=0){
				printf("(%s, %d)", inet_ntoa(serveradd.sin_addr) , ntohs(serveradd.sin_port));
				printf("  %s\n", rcvbuff);
			}


	}
	while (strcmp("=", sendbuff)!= 0);

	//Close the connection
	closesocket(client_socket);
	clearwinsock();
	printf("\n");    // Print a final linefeed
	system("pause");
	return(0);

}
