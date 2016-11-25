
#include<stdio.h>

#include<string.h>    //strlen
#include "game.h"
#include<unistd.h>    //write

#include<time.h>


#if WINDOWS
#include<winsock2.h>
#include <windows.h>
#else
#include<pthread.h>
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#endif





void * game_fn(void * data){
	game * g = data;
	int  move = 90;
	while (1){
		puts("Play something dude\n");
		fflush(stdin);
		if (g->game_finished) break;
		scanf("%d",&move);
		printf("server plays %d",move);
		if(move == 'q') break;
		else if (move == 'n') init_game(g);	
		else{
			play_move(g,1,move);
			display_game(g);
		}

		usleep(1000000);
		fflush(stdin);
	}
	puts("close socket");
     	close(g->socket);
	return NULL;
}

void * socket_fn(void * data){
	game * g = data;
	char client_next_move = 50;
	while(!g->game_finished){
		//receive request
		int read_size = recv(g->socket, &client_next_move, 1, 0);
		if (client_next_move == 33) send_game(g);//game status request
		else{
			//try to play move
			printf("try to play move %d",client_next_move);
			if (play_move(g,0,client_next_move) >= 0) display_game(g);
			send_game(g);
		}
	}
	return NULL;
}

int main(int argc , char *argv[])
{
#if WINDOWS

	WSADATA wsa;
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        printf("Failed. Error Code : %d",WSAGetLastError());
        return 1;
    }
#endif
#if !WINDOWS
    pthread_t game_thread, socket_thread;
#endif

    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[2000];
    game g;
    c=0;
    //Init game
    init_game(&g);

    //Create socket
#if WINDOWS
    s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
#else
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
#endif

    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( PORT );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
    //accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");
    puts("Starting game");
    g.socket = client_sock;
    

    //starting threads
    pthread_create(&socket_thread, NULL, socket_fn, &g);
    pthread_create(&game_thread, NULL, game_fn, &g);

	//join threads
	if (pthread_join(socket_thread, NULL) <0) puts("Error Joining socket thread");
	if (pthread_join(game_thread, NULL) <0) puts("Error Joining game thread");


     //close(socket_desc);
    return 0;
}
