#include <stdlib.h> 		/* For random number generation */
#include "./tetris.h"
#include "./include/led_control.h"

static char GAME_OVER;
char board[ROWS][COLS];
struct tetronimo curr;
unsigned long ms = 0;

ISR(TIMER1_OVF_VECT)
{
	display();
	SET_DISPLAY_TIMER;
}

ISR(TIMER2_OVF_VECT)
{
	++ms;
	SET_MS_TIMER;
}

int main(void)
{
	INIT_DISPLAY;
	INIT_INTERRUPT;
	INIT_BUTTONS;
	while(1) {
		GAME_OVER = 0;
		init_array();
		while(!GAME_OVER) {
			gen_rand_tetronimo();
			set_curr(1);
			while(not_reached_bottom() && check_tetronimo_below()) {
				display_curr();
				unsigned long future_ms0 = ms+50;
				static unsigned long future_ms1;
				while(ms < future_ms0) {
      					check_input(&future_ms0, &future_ms1);
				}
			}
			set_curr(2);
			check_game_over();
			clear_full_rows();
		}
	}
	return 0;
}

void init_array()
{
	int i;
	int j;
	for(i=0; i<ROWS; ++i) {
		for(j=0; j<COLS; ++j)
			board[i][j] = 0;
	}
}

char gen_rand_num()
{
	SET_BIT(ADMUX, 6);
	SET_BIT(ADCSRA, 7);
	SET_BIT(ADCSRA, 6);
	while(GET_BIT(ADCSRA, 6)); 
	srand(ADC);
	return (rand()+ms)%7;
}

void set_piece(struct coordinate *blk_0, struct coordinate *blk_1,
	       struct coordinate *blk_2, struct coordinate *blk_3,
	       char a0, char a1, char b0, char b1, char c0, char c1, char d0, char d1) {
	blk_0->row = a0;
	blk_0->col = a1;
	blk_1->row = b0;
	blk_1->col = b1;
	blk_2->row = c0;
	blk_2->col = c1;
	blk_3->row = d0;
	blk_3->col = d1;
		}

void gen_rand_tetronimo(void)
{
	char name;
	char rotation = 0;
	struct coordinate blk_0;
	struct coordinate blk_1;
	struct coordinate blk_2;
	struct coordinate blk_3;
	
	switch(gen_rand_num()) {
	case I_PIECE:
		name = I_PIECE;
		set_piece(&blk_0, &blk_1, &blk_2, &blk_3,
			  0,3,1,3,2,3,3,3);
		break;
	case O_PIECE:
		name = O_PIECE;
		set_piece(&blk_0, &blk_1, &blk_2, &blk_3,
			  1,3,1,4,2,3,2,4);
		break;
	case T_PIECE:
		name = T_PIECE;
		set_piece(&blk_0, &blk_1, &blk_2, &blk_3,
			  1,4,2,3,2,4,2,5);
		break;		
	case L_PIECE:
		name = L_PIECE;
		set_piece(&blk_0, &blk_1, &blk_2, &blk_3,
			  0,3,1,3,2,3,2,4);
		break;
	case J_PIECE:
		name = J_PIECE;
		set_piece(&blk_0, &blk_1, &blk_2, &blk_3,
			  0,4,1,4,2,4,2,3);
		break;
	case S_PIECE:
		name = S_PIECE;
		set_piece(&blk_0, &blk_1, &blk_2, &blk_3,
			  1,4,1,5,2,3,2,4);
		break;
	case Z_PIECE:
		name = Z_PIECE;
		set_piece(&blk_0, &blk_1, &blk_2, &blk_3,
			  1,3,1,4,2,4,2,5);
		break;		
	}
	// Check positions that block will be placed whether there are already blocks there
	curr.name = name;
	curr.rotation = rotation;
	curr.blk_0 = blk_0;
	curr.blk_1 = blk_1;
	curr.blk_2 = blk_2;
	curr.blk_3 = blk_3;
}

char not_reached_bottom(void)
{
	return (curr.blk_3.row < ROWS-1);
}

void set_curr(char c)
{
	board[curr.blk_0.row][curr.blk_0.col] = c; 
	board[curr.blk_1.row][curr.blk_1.col] = c;
	board[curr.blk_2.row][curr.blk_2.col] = c;
	board[curr.blk_3.row][curr.blk_3.col] = c;
}

void display_curr(void)
{
	set_curr(0);;
	DROP_CURR;
	set_curr(1);
}

char check_tetronimo_below(void)
{
	if(not_reached_bottom()) {
		if(board[curr.blk_0.row+1][curr.blk_0.col] == 2)
			return 0;
		if(board[curr.blk_1.row+1][curr.blk_1.col] == 2)
			return 0;
		if(board[curr.blk_2.row+1][curr.blk_2.col] == 2)
			return 0;
		if(board[curr.blk_3.row+1][curr.blk_3.col] == 2)
			return 0;
	}
	return 1;
		   
}

void check_game_over(void)
{
	int i;
	for(i=0; i<COLS; ++i) {
		if(board[2][i] == 2) {
			GAME_OVER = 1;
			break;
		}
	}
}

void check_input(unsigned long *future_ms0, unsigned long *future_ms1) 
{
	if(ms < *future_ms1)
		return;
	else {
		*future_ms1 = ms+BUTTON_DEBOUNCE;
		if(GET_BIT(PINA, LEFT)) {
			if(check_left()) {
				set_curr(0);
				MOVE_LEFT;
				set_curr(1);
			}
		} else if(GET_BIT(PINA, DOWN)) {
			*future_ms0 = 0;
		} else if(GET_BIT(PINA, RIGHT)) {
			if(check_right()) {
				set_curr(0);
				MOVE_RIGHT;
				set_curr(1);
			}
		} else if(GET_BIT(PINA, R_LEFT)) {
			rotate_shape();
		}
		
	}
}

char check_left(void)
{
	if(check_left_helper(curr.blk_0.col, curr.blk_0.row))
	   return 0;
	if(check_left_helper(curr.blk_1.col, curr.blk_1.row))
	   return 0;
	if(check_left_helper(curr.blk_2.col, curr.blk_2.row))
	   return 0;
	if(check_left_helper(curr.blk_3.col, curr.blk_3.row))
	   return 0;
	return 1;
}

char check_left_helper(char curr_col, char curr_row)
{
	if(0 >= curr_col || 2 == board[curr_row][curr_col-1])
	   return 1;
	return 0;
}

char check_right(void)
{
	if(check_right_helper(curr.blk_0.col, curr.blk_0.row))
	   return 0;
	if(check_right_helper(curr.blk_1.col, curr.blk_1.row))
	   return 0;
	if(check_right_helper(curr.blk_2.col, curr.blk_2.row))
	   return 0;
	if(check_right_helper(curr.blk_3.col, curr.blk_3.row))
	   return 0;
	return 1;
}

char check_right_helper(char curr_col, char curr_row)
{
	if(curr_col >= COLS-1 || 2 == board[curr_row][curr_col+1])
		return 1;
	return 0;
}

void rotate_shape()
{
	char rotation = get_new_rotation();
	struct tetronimo rotated_tetronimo; /* See if test piece fits on board first */
	
	switch(curr.name) {
	case I_PIECE:
		if(rotate_I(&rotated_tetronimo, rotation))
			return;
		break;
	case T_PIECE:
		if(rotate_T(&rotated_tetronimo, rotation))
			return;
		break;
	case L_PIECE:
		if(rotate_L(&rotated_tetronimo, rotation))
			return;
		break;
	case J_PIECE:
		if(rotate_J(&rotated_tetronimo, rotation))
			return;
		break;
	case S_PIECE:
		if(rotate_S(&rotated_tetronimo, rotation))
			return;
		break;
	case Z_PIECE:
		if(rotate_Z(&rotated_tetronimo, rotation))
			return;
		break;
	default:
		return;
	}
	
	set_curr(0);
	curr.rotation = rotation;
	curr.blk_0 = rotated_tetronimo.blk_0;
	curr.blk_1 = rotated_tetronimo.blk_1;
	curr.blk_2 = rotated_tetronimo.blk_2;
	curr.blk_3 = rotated_tetronimo.blk_3;
	set_curr(1);	
}

char rotate_I(struct tetronimo *rotated_tetronimo, char rotation)
{
	/* Special Case */
	if(0 == curr.rotation && 7 == curr.blk_0.col)
		return 1;

	if(0 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_2.row-2, curr.blk_2.col,
			  curr.blk_2.row-1, curr.blk_2.col,
			  curr.blk_2.row, curr.blk_2.col,
			  curr.blk_2.row+1, curr.blk_2.col);
	} else if(1 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_1.row, curr.blk_1.col+2,
			  curr.blk_1.row, curr.blk_2.col,
			  curr.blk_1.row, curr.blk_2.col+1,
			  curr.blk_1.row, curr.blk_2.col-1);
	} else if(2 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_2.row-1, curr.blk_2.col,
			  curr.blk_2.row+1, curr.blk_2.col,
			  curr.blk_2.row, curr.blk_2.col,
			  curr.blk_2.row+2, curr.blk_2.col);
	} else if(3 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_1.row, curr.blk_1.col+1,
			  curr.blk_1.row, curr.blk_1.col,
			  curr.blk_1.row, curr.blk_1.col-1,
			  curr.blk_1.row, curr.blk_1.col-2);
	}
	if(check_rotated_tetronimo(*rotated_tetronimo))
		return 1;
	return 0;
}

char rotate_T(struct tetronimo *rotated_tetronimo, char rotation)
{
	/* Special Case */
	if(3 == curr.rotation && 7 == curr.blk_3.col)
		return 1;
	
	if(0 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_2.row-1, curr.blk_2.col,
			  curr.blk_2.row, curr.blk_2.col-1,
			  curr.blk_2.row, curr.blk_2.col,
			  curr.blk_2.row, curr.blk_2.col+1);
	} else if(1 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_0.row, curr.blk_0.col,
			  curr.blk_2.row-1, curr.blk_2.col,
			  curr.blk_2.row, curr.blk_2.col,
			  curr.blk_3.row, curr.blk_3.col);		
	} else if(2 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_2.row, curr.blk_2.col+1,
			  curr.blk_1.row, curr.blk_1.col,
			  curr.blk_2.row, curr.blk_2.col,
			  curr.blk_3.row, curr.blk_3.col);		
	} else if(3 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_0.row, curr.blk_0.col,
			  curr.blk_1.row, curr.blk_1.col,
			  curr.blk_2.row, curr.blk_2.col,
			  curr.blk_2.row+1, curr.blk_2.col);		
	}
	if(check_rotated_tetronimo(*rotated_tetronimo))
		return 1;
	return 0;
}

char rotate_L(struct tetronimo *rotated_tetronimo, char rotation)
{
	/* Special Case */
	if(2 == curr.rotation && 7 == curr.blk_2.col)
		return 1;
	if(0 == rotation) {
		char row = curr.blk_2.row;
		char col = curr.blk_2.col;
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  row-1, col,
			  row, col,
			  row+1, col,
			  row+1, col+1);

	} else if(1 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_2.row, curr.blk_2.col+1,
			  curr.blk_2.row, curr.blk_2.col-1,
			  curr.blk_2.row, curr.blk_2.col,
			  curr.blk_2.row+1, curr.blk_2.col-1);		
	} else if(2 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_2.row-1, curr.blk_2.col-1,
			  curr.blk_2.row-1, curr.blk_2.col,
			  curr.blk_2.row, curr.blk_2.col,
			  curr.blk_2.row+1, curr.blk_2.col);		
	} else if(3 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_3.row-1, curr.blk_3.col,
			  curr.blk_2.row, curr.blk_2.col-1,
			  curr.blk_2.row, curr.blk_2.col,
			  curr.blk_3.row, curr.blk_3.col);		
	}
	if(check_rotated_tetronimo(*rotated_tetronimo))
		return 1;
	return 0;
}

char rotate_J(struct tetronimo *rotated_tetronimo, char rotation)
{
	/* Special Case */
	if(0 == curr.rotation && 7 == curr.blk_0.col)
		return 1;
	
	if(0 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_1.row-1, curr.blk_1.col,
			  curr.blk_1.row, curr.blk_1.col,
			  curr.blk_1.row+1, curr.blk_1.col,
			  curr.blk_1.row+1, curr.blk_1.col-1);		
	} else if(1 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_1.row-1, curr.blk_1.col-1,
			  curr.blk_1.row, curr.blk_1.col,
			  curr.blk_1.row, curr.blk_1.col-1,
			  curr.blk_1.row, curr.blk_1.col+1);		
	} else if(2 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_1.row-1, curr.blk_1.col,
			  curr.blk_1.row, curr.blk_1.col,
			  curr.blk_1.row-1, curr.blk_1.col+1,
			  curr.blk_1.row+1, curr.blk_1.col);		
	} else if(3 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_1.row, curr.blk_1.col-1,
			  curr.blk_1.row, curr.blk_1.col,
			  curr.blk_1.row, curr.blk_1.col+1,
			  curr.blk_1.row+1, curr.blk_1.col+1);		
	}
	if(check_rotated_tetronimo(*rotated_tetronimo))
		return 1;
	return 0;
}


char rotate_S(struct tetronimo *rotated_tetronimo, char rotation)
{
	if(0 == rotation) {
		char row = curr.blk_0.row;
		char col = curr.blk_0.col;
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  row, col,
			  row, col+1,
			  row+1, col-1,
			  row+1, col);
	} else if(1 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_0.row, curr.blk_0.col,
			  curr.blk_0.row-1, curr.blk_0.col,
			  curr.blk_0.row, curr.blk_0.col+1,
			  curr.blk_0.row+1, curr.blk_0.col+1);		
	} else if(2 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_2.row+1, curr.blk_2.col-2,
			  curr.blk_1.row, curr.blk_1.col,
			  curr.blk_2.row, curr.blk_2.col,
			  curr.blk_2.row+1, curr.blk_2.col-1);		
	} else if(3 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_2.row-1, curr.blk_2.col,
			  curr.blk_2.row, curr.blk_2.col,
			  curr.blk_2.row, curr.blk_2.col+1,
			  curr.blk_2.row+1, curr.blk_2.col+1);		
	}
	if(check_rotated_tetronimo(*rotated_tetronimo))
		return 1;
	return 0;
}

char rotate_Z(struct tetronimo *rotated_tetronimo, char rotation)
{
	if(0 == rotation) {
		char row = curr.blk_1.row;
		char col = curr.blk_1.col;
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  row-1, col,
			  row-1, col+1,
			  row, col+1,
			  row, col+2);
	} else if(1 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_1.row-1, curr.blk_1.col+1,
			  curr.blk_1.row, curr.blk_1.col,
			  curr.blk_1.row, curr.blk_1.col+1,
			  curr.blk_1.row+1, curr.blk_1.col);		
	} else if(2 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_0.row, curr.blk_0.col,
			  curr.blk_2.row+1, curr.blk_2.col,
			  curr.blk_2.row, curr.blk_2.col,
			  curr.blk_2.row+1, curr.blk_2.col+1);		
	} else if(3 == rotation) {
		set_piece(&rotated_tetronimo->blk_0, &rotated_tetronimo->blk_1,
			  &rotated_tetronimo->blk_2, &rotated_tetronimo->blk_3,
			  curr.blk_2.row, curr.blk_2.col-1,
			  curr.blk_1.row, curr.blk_1.col,
			  curr.blk_2.row, curr.blk_2.col,
			  curr.blk_2.row+1, curr.blk_2.col-1);		
	}
	if(check_rotated_tetronimo(*rotated_tetronimo))
		return 1;
	return 0;
}

void set_coordinates(void)
{
	set_curr(0);
	curr.blk_3.row = curr.blk_1.row;
	curr.blk_3.col = curr.blk_1.col-2;

	curr.blk_2.row = curr.blk_1.row;
	curr.blk_2.col = curr.blk_1.col-1;

	curr.blk_0.row = curr.blk_1.row;
	curr.blk_0.col = curr.blk_1.col+1;
	set_curr(1);
}

char get_new_rotation()
{
	char rotation = curr.rotation;
	 --rotation;
	if(-1 == rotation)
		rotation = 3;
	return rotation;
}

char check_rotated_tetronimo(struct tetronimo r_tet)
{
	if(!check_rotated_coordinate(r_tet.blk_0))
	   return 1;
	if(!check_rotated_coordinate(r_tet.blk_1))
	   return 1;
	if(!check_rotated_coordinate(r_tet.blk_2))
	   return 1;
	if(!check_rotated_coordinate(r_tet.blk_3))
	   return 1;
	return 0;
}

char check_rotated_coordinate(struct coordinate r_coord)
{
	if(r_coord.col < 0 || r_coord.col > COLS
	   || 2 == board[r_coord.row][r_coord.col])
		return 0;
	return 1;

}

void clear_full_rows(void)
{
	int i;
	for(i=0; i<ROWS; ++i) {
		if(check_row_full(i)) {
			clear_row(i);
			shift_row(i);
		}
	}
}

char check_row_full(char row)
{
	int i;
	for(i=0; i<COLS; ++i) {
		if(board[row][i] != 2)
			return 0;
	}
	return 1;
}

char clear_row(char row)
{
	int i;
	for(i=0; i<COLS; ++i) {
		board[row][i] = 0;
	}
}

void shift_row(char row)
{
	int i;
	int j;
	for(i=row-1; i>=0; --i) {
		for(j=0; j<COLS; ++j) {
			if(board[i][j] == 2) {
				board[i][j] = 0;
				board[i+1][j] = 2;
			}
		}
	}
}
