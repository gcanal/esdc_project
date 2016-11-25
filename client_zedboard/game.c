#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>    //write
#if ZED
#include "lwip/inet.h"
#include "lwip/sockets.h"
#else
#include <arpa/inet.h> //inet_addr
#include <sys/socket.h>
#endif

#define SEND_BUFSIZE (9)


#include <errno.h>
#include <string.h>
//Not API
void empty_matrix(char * mat){
        int j = 0;
        for ( j = 0; j < 9 ; j++){
                *(mat+j) = ' ';
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
	int c;
        for ( c = 0 ; c < 9 ; c++){
                if (*(mat+c) == EMPTY_CHAR) return 0;
        }
        return 1;
}


//API
void clear_and_display_game(game * g)
{
	int c = 0;
   for (c = 0 ; c < 60 ; c++) printf("\n");//clear screen
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
   //works for ZEDBOARD or Linux
   write(g->socket, g->mat, 13);
   return 0;
}

//for client
int receive_game(game * g){
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
	//works for ZEDBOARD or Linux
	printf("[*] SENDING MOVE...%d = %c\n", move, move);
	int errcode = lwip_write(g->socket,  &move, 1);
	printf("socket = %d", g->socket);
    if ((errcode) < 0) {
    	printf("Error sending move code = %d.\n", errcode);
    	printf("Error sending move string = %s.\n", strerror(errno));
    }
	return errcode;
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
	printf("Initializing game...");
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

void print_board(game *g, int posx, int posy){
	clear();
	printf("\n\rClient WINS: %d\t Server Wins:%d.",g->client_win, g->server_win);
	char board[48];
	int c= 0;
	for (c=0 ; c<48 ; c++) board[c] = ' ';
	for (c=0 ; c<3 ; c++) {board[c*16+6] = '|';board[c*16+10] = '|';}
	for (c=0;c<9;c++){
		board[7+16*(c/3)+c%3] = *(g->mat+c) ;
	}
	int m = 11,n = 27;
	if (g->server_plays){
		board[m]='R';board[m+1]='O';board[m+2]='B';board[m+3]='B';board[m+4]='Y';board[n+4]='S';
	}
	else{
		board[m]='Y';board[m+1]='O';board[m+2]='U';
	}

	board[n]='P';board[n+1]='L';board[n+2]='A';board[n+3]='Y';
	int p = 0, q=16;
	if (g->client_win && g->server_win){
		 board[q]='D';board[q+1]='R';board[q+2]='A';board[q+3]='W';
	}
	else if(g->client_win || g->server_win){
		p=0;
		if(g->client_win){
			board[p]='Y';board[p+1]='O';board[p+2]='U';
		}
		if(g->server_win){
			board[p]='R';board[p+1]='O';board[p+2]='B';board[p+3]='B';board[p+4]='Y';board[q+3]='S';
		}
		board[q]='W';board[q+1]='I';board[q+2]='N';
	}
	//display cursor
	board[16*posy+posx] = '*';
	for (c = 0 ; c < 48; c++) {
		print_char(board[c],c/16,c%16);
	}
}
