
#ifndef GAME_H_
#define GAME_H_

#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>

#define ZED 1 // true if the code is executed on a zedboard
#define SERVER_CHAR 'x'
#define CLIENT_CHAR 'o'
#define EMPTY_CHAR '_'
#define SERVER_ADDR "169.254.39.1"
#define PORT 8889

typedef struct {
	char mat[13];
	int socket;
	int game_finished;
	int server_win;
	int client_win;
	int server_plays;
} game;

//client and server
void display_game(game * game);

//client and server
void display_matrix(char * mat);

//for server
int send_game(game * game);

//for client
int receive_game( game * game);

//for client
int send_move(game * g, char move);

//for server NOT USED
int receive_move(game * g, char move);

// is_server=1 for server, 0 for client
int play_move(game * g, int is_server, char move);

int init_game(game * g);


int test_win(game * g, int is_server);

void print_board(game *g, int posx, int posy);

#endif /* GAME_H_ */

