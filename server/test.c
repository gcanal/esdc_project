
#include "game.h"
#include <stdio.h>



int main(int argc, char * argv[]){

	game g;
	init_game(&g);
	g.server_plays = 0;
	display_game(&g);
	play_move(&g,0,1);
	display_game(&g);
	play_move(&g,0,2);
	display_game(&g);
	play_move(&g, 1,0);	
	display_game(&g);
	play_move(&g,0,4);
	display_game(&g);
	
	play_move(&g,1,3);
	display_game(&g);
	play_move(&g,0,6);
	display_game(&g);

	play_move(&g,1,7);
	display_game(&g);
	play_move(&g,0,5);
	
	display_game(&g);
	play_move(&g,1,2);
	display_game(&g);
	play_move(&g,0,8);
	display_game(&g);
	//game finished no winner
	//
	
	play_move(&g,1,7);
	display_game(&g);
	return 0;
}
