#ifndef TETRIS_GUARD
#define TETRIS_GUARD

/* Buttons */
#define LEFT 4
#define DOWN 5
#define RIGHT 6
#define R_LEFT 7
#define R_RIGHT 3

#define BUTTON_DEBOUNCE 30
#define MOVE_LEFT ({\
	--curr.blk_0.col;\
	--curr.blk_1.col;\
	--curr.blk_2.col;\
	--curr.blk_3.col;\
		})

#define MOVE_RIGHT ({\
	++curr.blk_0.col;\
	++curr.blk_1.col;\
	++curr.blk_2.col;\
	++curr.blk_3.col;\
		})


#define INIT_BUTTONS ({\
	CLR_BIT(DDRA, 4);\
		})
/* Display and ms Counter Interrupts */
extern unsigned long ms; // stores current number of milliseconds elapsed from power on. In reality inaccurate due to display interrupt.

#define INIT_INTERRUPT ({\
	sei();\
	TCCR1B = 0x05;\
	SET_DISPLAY_TIMER;\
	TCCR2 = 0x04;\
	SET_MS_TIMER;\
	TIMSK |= (1<<TOIE1) | (1<<TOIE2);\
		})
#define TIMER1_OVF_VECT _VECTOR(9)
#define TIMER2_OVF_VECT _VECTOR(5)
#define SET_DISPLAY_TIMER TCNT1 = 65489;
#define SET_MS_TIMER TCNT2 = 131;

#define ROWS 18
#define COLS 8

#define I_PIECE 0
#define O_PIECE 1
#define T_PIECE 2
#define L_PIECE 3
#define J_PIECE 4
#define S_PIECE 5
#define Z_PIECE 6

#define DROP_CURR ({\
	++curr.blk_0.row;\
	++curr.blk_1.row;\
	++curr.blk_2.row;\
	++curr.blk_3.row;\
		})
		
struct coordinate {
	int row;
	int col;
};

struct tetronimo {
	char name;
	char rotation;
	struct coordinate blk_0;
	struct coordinate blk_1;
	struct coordinate blk_2;
	struct coordinate blk_3; /* Always the bottommost block */
};

extern char board[ROWS][COLS];
extern struct tetronimo curr;

void init_array();
char gen_rand_num();
void gen_rand_tetronimo(void);
char not_reached_bottom(void);
void set_curr(char c);
void display_curr(void);
char check_tetronimo_below(void);
void check_game_over(void);
void check_input(unsigned long *future_ms0, unsigned long *future_ms1);
char check_left(void);
char check_left_helper(char curr_col, char curr_row);
char check_right(void);
char check_right_helper(char curr_col, char curr_row);
void rotate_shape();
char rotate_I(struct tetronimo *rotated_tetronimo, char rotation);
char rotate_T(struct tetronimo *rotated_tetronimo, char rotation);
char rotate_L(struct tetronimo *rotated_tetronimo, char rotation);
char rotate_J(struct tetronimo *rotated_tetronimo, char rotation);
char rotate_S(struct tetronimo *rotated_tetronimo, char rotation);
char rotate_Z(struct tetronimo *rotated_tetronimo, char rotation);
void set_coordinates(void);
char get_new_rotation();
char check_rotated_tetronimo(struct tetronimo r_tet);
char check_rotated_coordinate(struct coordinate r_coord);
void clear_full_rows(void);
char check_row_full(char row);
char clear_row(char row);
void shift_row(char row);
#endif
