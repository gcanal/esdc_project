#include<stdio.h>
#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h>
#include<time.h>
#include "game.h"

void * game_fn(void *data){
	game * g = data;
	int  move = 50;
	while (!g->game_finished){
		scanf("%d",&move);
		printf("sending move %d", move);
		send_move(g,move);
		receive_game(g);
		display_game(g);
		fflush(stdin);
                }  
	return NULL;
}

void * socket_fn(void * data){
	game * g = data;
	int connection = 1;
	while (connection >= 0){
		//wait 1 s
		usleep(1000000);
		//send update request
		connection = send_move(g, 33);
		receive_game(g);
		display_game(g);
	}
	return NULL;
}

int main(int argc , char *argv[])
{
	pthread_t game_thread, socket_thread;
	game g;
    int socket_desc;
    int read_size =0;
    struct sockaddr_in server;
    char message[200];
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    } 
    server.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    server.sin_family = AF_INET;
    server.sin_port = htons( PORT );
 
    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("connect error");
        return 1;
    }
     
    puts("Connected\n");
    //Init game
	init_game(&g);
    g.socket = socket_desc;

    //starting threads
    pthread_create(&socket_thread, NULL, socket_fn, &g);
    pthread_create(&game_thread, NULL, game_fn, &g);

        //join threads
        if (pthread_join(socket_thread, NULL) <0) puts("Error Joining socket thread");
        if (pthread_join(game_thread, NULL) <0) puts("Error Joining game thread");


    return 0;
}
