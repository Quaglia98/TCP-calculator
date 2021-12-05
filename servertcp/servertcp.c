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
#include <ctype.h>
#include <signal.h>
#include <math.h>
#include "protocol.h"
#include "utilities.h"



void clearwinsock() {
	#if defined WIN32
	WSACleanup();
	#endif
}


void errorhandler(char *errorMessage) {
     printf ("%s", errorMessage);
}


int main(int argc, char *argv[]) {

	char SendBuff [BUFFERSIZE] ;
	char ReceiveBuff [BUFFERSIZE] ;
	int operand1, operand2, port;
	float result = -1 ;
	char operator = ' ';
    int neg1 = 0;
    int neg2 = 0;

	if (argc > 1) {
	port = atoi(argv[1]); // if argument specified convert argument to binary
	}
	else port = PROTOPORT;       // use default port number

	if (port < 0) {
	printf("bad port number %s \n", argv[1]);
	return 0;
	}



	#if defined WIN32 // initialize Winsock
	WSADATA wsa_data;
	int result2 = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result2 != 0) {
		errorhandler("Error at WSAStartup()\n");
		return 0;
	}
	#endif

	//Create the socket
	int server_socket;
	server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_socket < 0) {
	errorhandler("socket creation failed.\n");
	clearwinsock();
	return -1;
	}

	//Set an address to the socket
	struct sockaddr_in serveradd;
	memset(&serveradd, 0, sizeof(serveradd)); // ensures that extra bytes contain 0
	serveradd.sin_family = AF_INET;
	serveradd.sin_addr.s_addr = inet_addr("127.0.0.1");
	serveradd.sin_port = htons(port); /* converts values between the host and
	network byte order. Specifically, htons() converts 16-bit quantities
	from host byte order to network byte order. */

	if (bind(server_socket, (struct sockaddr*) &serveradd, sizeof(serveradd)) < 0) {
		errorhandler("bind() failed.\n");
		closesocket(server_socket);
		clearwinsock();
		return -1;
	}

	//set the listen socket
	if (listen (server_socket, QLEN) < 0) {
		errorhandler("listen() failed.\n");
		closesocket(server_socket);
		clearwinsock();
		return -1;
	}

	//Accept a new connection
	struct sockaddr_in clientadd; // structure for the client address

	int client_socket;       // socket descriptor for the client
	int client_len;          // the size of the client address
	printf("Waiting for a client to connect...");
	while (1) {
		client_len = sizeof(clientadd); // set the size of the client address
		if ((client_socket = accept(server_socket, (struct sockaddr*)&clientadd, &client_len)) < 0) {
			errorhandler("accept() failed.\n");
			//Close connection
			closesocket(client_socket);
			clearwinsock();
			return 0;
		}

		printf("\nConnection established with: %s:%d\n", inet_ntoa(clientadd.sin_addr), ntohs(clientadd.sin_port));
		while(1){

			memset(SendBuff,0, sizeof(SendBuff));
			memset(ReceiveBuff,0, sizeof(ReceiveBuff));
		    printf("Waiting for client request:\n");

			if (recv(client_socket, ReceiveBuff, sizeof(ReceiveBuff),0) < 0 ) {
				errorhandler ("Error while receiving data from client.\n");
				break;
			}

			     //Processing the expression:
			     if(strcmp("=", ReceiveBuff) == 0) {   /* checking if client sent = for closing the connection*/
			        printf("\nConnection Closed \n");
			        printf("Waiting for a client to connect...\n");
			        closesocket(client_socket);
			        break;
			     }

			     //checking if the expression is not valid

			     if(parseExp(ReceiveBuff, &operator, &operand1, &operand2, &neg1, &neg2)  < 0 ){

			    	//reset booleans neg1 and neg2
			    	neg1 = 0; neg2 = 0;

			    	errorhandler("\nError while processing the expression\n");
			        char* errore = "Expression not valid, try again";
			        sprintf(SendBuff, "%s",errore);
			        if(send(client_socket, SendBuff, sizeof(SendBuff), 0) < 0){
			        	errorhandler("Error while sending data to the client\n");
			       	    break;
			        }


			     }

			     //expression valid
			     else{

			    	     //checking if the operand1 and/or the operand2 are negative
                         if(neg1){
			    		 int temp1 = 0;
			    		 temp1 = -abs(operand1);
			    		 operand1 = temp1;
			    		 neg1 = 0;
                         }
                         if(neg2){
			    	 	 int temp2 = 0;
			    	 	 temp2 = -abs(operand2);
			    	 	 operand2 = temp2;
			    	 	 neg2 = 0;
                         }


			    	 printf("Expression: (%c %d %d)\n",operator ,operand1 , operand2) ;
			    	 switch(operator){
			    	 case '+': result = add(&operand1,&operand2);
			    	 break;
			    	 case '-': result = sub(&operand1,&operand2);
			    	 break;
			    	 case '/': result = division(&operand1,&operand2);
			    	 break;
			    	 case 'x': result = mult(&operand1,&operand2);
			    	 break;
			    	 default: break;
			    	 }

			    	 printf("Sending result to the client: (%f)\n", result ) ;
			    	 sprintf(SendBuff, "%f",result); //result into the sendbuff

			    	 if(send(client_socket, SendBuff, sizeof(SendBuff), 0) < 0){
			    		 errorhandler("Error while sending data to the client \n");
			    		 break;
			    	 }
			    	 printf("\n___________________________________________________________\n")  ;
			     }
		}

		closesocket(client_socket);
	}

	closesocket(server_socket);

 return 0;

}
