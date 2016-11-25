
#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>    //write


#if WINDOWS
#include<winsock2.h>
#else
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#endif




//Not API
void empty_matrix(char * mat){
        int j = 0;
        for (int j = 0; j < 9 ; j++){
                *(mat+j) = EMPTY_CHAR;
        }
}


int test_win_char(char  mat[], char a)
{
        int ret =  (mat[0] == a) && (mat[1] == a) && (mat[2] == a);
        ret = ret || ((mat[3] == a) && (mat[4] == a) && (mat[5] == a));
        ret = ret || ((mat[6] == a) && (mat[7] == a) && (mat[8] == a));

        ret = ret || ((mat[0] == a) && (mat[3] == a) && (mat[6] == a));
        ret = ret || ((mat[1] == a) && (mat[4] == a) && (mat[7] == a));
        ret = ret || ((mat[2] == a) && (mat[5] == a) && (mat[8] == a));


        ret = ret || ((mat[0] == a) && (mat[4] == a) && (mat[8] == a));
        ret = ret || ((mat[2] == a) && (mat[4] == a) && (mat[6] == a));
        return ret;
}




int is_matrix_full(char * mat){
        for (int c = 0 ; c < 9 ; c++){
                if (*(mat+c) == EMPTY_CHAR) return 0;
        }
        return 1;
}

//API
void clear_and_display_game(game * g)
{

	for (int c = 0 ; c < 60 ; c++) printf("\n");//clear screen
	display_game(g);
}

void display_game(game* g ){
	if (g->game_finished){
		printf(" GAME FINISHED - ");
		if (g->server_win && g->client_win) printf("DRAW ");
		else if (g->server_win) printf("Server Wins");
		else if (g->client_win) printf("Client Wins");
		else printf("No winner");
		printf("\n");
	}
	else{
		printf("GAME IS ON - ");
		if (g->server_plays) printf("Server plays");
		else printf("Client plays");
		printf("\n");
	}
	display_matrix(g->mat);
}


void display_matrix(char * mat){
        printf("%c|%c|%c\n________\n%c|%c|%c\n________\n%c|%c|%c\n\n", *(mat), *(mat+1), *(mat+2), *(mat+3), *(mat+4), *(mat+5), *(mat+6), *(mat+7), *(mat+8));
}

//for server 
int send_game(game * g){
	g->mat[9] = g->game_finished;
	g->mat[10] = g->server_win;
	g->mat[11] = g->client_win;
	g->mat[12] = g->server_plays;
	write(g->socket, g->mat, 13);
	return 0;
}


//for client
int receive_game( game * g){
	int read_size = recv(g->socket, g->mat, 13, 0); //what last arg is for ?
	g->game_finished = g->mat[9];
	g->server_win = g->mat[10];
	g->client_win = g->mat[11];
	g->server_plays = g->mat[12];
	return read_size;

}

//for client
int send_move(game *g, char move){
	//no check
	return send(g->socket, &move, 1, 0);
}

//for server NOT USED
int receive_move(game *  g, char move){
	return 0;
}

//Used by server
int play_move(game * g, int server_plays, char move){
	if(g->game_finished){
		puts("game is finished, we no longer play\n");
		return -1;
	}
	if (g->server_plays != server_plays){
		puts("not your turn");
		return -1;
	}
	char player_char = ' ';
	player_char = server_plays ? SERVER_CHAR : CLIENT_CHAR;
	if (move < 0 || move > 8){
		puts(" not a square coordinate \n");
		return -1;
	}	
	if (g->mat[move] != EMPTY_CHAR){
		puts("square is occupied");
		return -1;
	}
	//updating matrix and current_player
	g->mat[move] = player_char;
	g->game_finished = is_matrix_full(g->mat);
	g->server_win = test_win_char(g->mat, SERVER_CHAR);
	g->client_win = test_win_char(g->mat, CLIENT_CHAR);
	g->game_finished |= g->server_win;
	g->game_finished |= g->client_win;
	g->server_plays = !g->server_plays;
	return 0;
}

int init_game(game * g){
	empty_matrix(g->mat);
	g->game_finished = 0;
	g->server_win = 0;
	g->client_win = 0;
	g->server_plays = 1;
	return 0;
}
int test_win(game * g, int is_server){
	return (is_server ? test_win_char(g->mat, SERVER_CHAR) : test_win_char(g->mat, CLIENT_CHAR));
}
