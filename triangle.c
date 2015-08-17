/**
 * FileName: triangle.c        
 *
 * @file
 * Main program loop and initialization code
 *
 * @author LE
 *                                
 * REVISION HISTORY:
 *
 * - LE  06/30/15 -> original development 
 */

//system header files
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>

//project header files
#include "triangle.h"

//#debugging MACRO
#if MAINDEBUG
	#include <syslog.h>
	#define MAINDBG(x...) syslog(LOG_INFO, x)
#else
	#define MAINDBG(x...)
#endif


#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); } while (0)


// Local function prototypes
static uint8_t 	num_moves(const GAME_STATE* state);
static void 	init_game(GAME_STATE* state, uint8_t missing_row, uint8_t missing_column);
static int	check_for_winning_state(const GAME_STATE* state, uint8_t final_row, uint8_t final_column);
static int	get_new_state(const uint8_t move, const GAME_STATE* current_state, GAME_STATE* new_state);
static void 	printf_board(const GAME_STATE* state);

/**
 * Program entry point
 * 
 */
int main (int argc, char **argv)
{
	GAME_STATE moves[MAX_MOVES];	//this will hold all of the moves that we will try
	uint8_t	done = false;
	uint8_t current_depth = 0;
	int i;
	uint8_t missing_row = 255;
	uint8_t missing_column = 255;
	uint8_t final_row = 255;
	uint8_t final_column = 255;
	uint64_t steps = 0;
	int syntax_error = false;

	//make sure the user passed us 2 (or 4) arguments that were integers 0-4
	if (argc != 3 && argc != 5)
	{
		syntax_error = true;
	}
	else
	{
		if (sscanf (argv[1], "%hhu", &missing_row) != 1) 
		{
			syntax_error = true;
		}

		if (sscanf (argv[2], "%hhu", &missing_column) != 1) 
		{
			syntax_error = true;
		}
		if (missing_row > 4 || missing_column > 4)
		{
			syntax_error = true;
		}
		if (5 == argc)
		{
			if (sscanf (argv[3], "%hhu", &final_row) != 1) 
			{
				syntax_error = true;
			}

			if (sscanf (argv[4], "%hhu", &final_column) != 1) 
			{
				syntax_error = true;
			}
		}
	}
	if (syntax_error)
	{
		printf("Usage: \"triangle <row> <column>\" or  \n");
		printf("\t \"triangle <row> <column> <final_row> <final_column>\" \n\n");
		printf("\t where <row> and <column> are integers 0-4\n\n");
		exit (1);
	}

	//sanity check values
	if (missing_column > missing_row)
	{
		printf("Invalid column. (Max column# for row %d is %d.)\n", missing_row, missing_row);
		exit (1);
	}
	
	//initialize the game
	for (i = 0; i < MAX_MOVES; i++)
	{
		memset(&moves[i],0, sizeof(GAME_STATE));
	}

	
	init_game(&moves[0], missing_row, missing_column);
	
	//Start Triangle Solver application.
	MAINDBG("MAIN: ********* TRIANGLE SOLVER APPLICATION STARTING ************.\n");
	
	printf("%s\n", moves[0].description);
	printf_board(&moves[0]);
	if (final_row <= 4 && final_column<=4)
	{
		printf("Search will only stop if the last peg is at space (%d,%d).\n", final_row, final_column);
	}
	else
	{
		printf("Search will stop when only one peg is left, regardless of location.\n");
	}

	while (!done)
	{
		//Check to see if we have a final answer...
		if (WINNING_STATE == check_for_winning_state(&moves[current_depth], final_row, final_column))
		{
			MAINDBG("MAIN: We have a winning state!\n");
			done = true;
			break;
		}		
	
		steps ++;

		//if our deepest node has no options, it's a dead end... go up one and mark that we've tried this path.
		if ((0 == moves[current_depth].num_possible_moves) || \
			(moves[current_depth].num_moves_tried == moves[current_depth].num_possible_moves))
		{
			if (0 == current_depth)
			{
				MAINDBG("MAIN: Error - something went wrong and we think there are more options at depth %d. Exiting.\n", current_depth);
				printf("No valid result found. Steps tried: %llu. \n", steps);
				exit(1);
			}

			MAINDBG("MAIN: Move at level %d is a dead end. Moving up one.\n", current_depth);
			
			moves[current_depth-1].num_moves_tried++;
			current_depth--;
		}

		//If our deepest node has more options left to try, try one of them.
		if (moves[current_depth].num_moves_tried < moves[current_depth].num_possible_moves)
		{
			MAINDBG("MAIN: Node at depth %d has more options to try... trying option %d\n", current_depth, moves[current_depth].num_moves_tried);
			if (MAX_MOVES-1 == current_depth)
			{
				MAINDBG("MAIN: Error - something went wrong and we think there are more options at depth %d. Exiting.\n", current_depth);
				exit(1);
			}

			if (false == get_new_state(moves[current_depth].num_moves_tried, &moves[current_depth], &moves[current_depth + 1]))
			{
				MAINDBG("MAIN: Error occurred at depth %d. Exiting.\n", current_depth);
				exit(1);
			}

			current_depth++;
		}
	}

	for (i = 0; i < MAX_MOVES; i++)
	{
		printf("Winning move #%d: %s\n", i, moves[i].description);
		printf_board(&moves[i]);
	}

	printf("Steps required to find answer: %llu\n\n", steps);
	
	exit (0);
	
}

static uint8_t 	num_moves(const GAME_STATE* state)
{
	uint8_t result = 0;
	int row = 0;
	int column = 0;
	uint8_t column_max[5] = {0,1,2,3,4};	//define the maximum column for each row
	

	//Moves are only possible if there is a blank space. Look for a blank space...
	for (row = 0; row < 5; row++)
	{
		for (column = 0; column <= column_max[row]; column++)
		{
			if (NO_PEG_PRESENT == state->peg[row][column])
			{
				/*
				* we have a blank space! Check for valid moves in the following order:
				* 	1) diagonal down l->r
				* 	2) diagonal down r->l
				* 	3) diagonal up l->r
				*	4) diagonal up r->l
				*	5) horizontal l->r
				*	6) horizontal r->l
				*/

				//Diagonal down, l->r
				if ((row >= 2) && (column >= 2))
				{
					if ((PEG_PRESENT == state->peg[row-1][column-1]) && \
						(PEG_PRESENT == state->peg[row-2][column-2]))
					{
						result++;
					}
				}
				//Diagonal down, r->l
				if ((row >= 2) && (column <= (column_max[row] - 2)))
				{
					if ((PEG_PRESENT == state->peg[row-1][column]) && \
						(PEG_PRESENT == state->peg[row-2][column]))
					{
						result++;
					}
				}
				//Diagonal up, l->r
				if ((row <= 2) && (column <= 2))
				{
					if ((PEG_PRESENT == state->peg[row+1][column]) && \
						(PEG_PRESENT == state->peg[row+2][column]))
					{
						result++;
					}
				}
				//Diagonal up, r->l
				if ((row <= 2) && (column >= (column_max[row] - 2)))
				{
					if ((PEG_PRESENT == state->peg[row+1][column+1]) && \
						(PEG_PRESENT == state->peg[row+2][column+2]))
					{
						result++;
					}
				}
				//Horizontal, l->r
				if (column >= 2)
				{
					if ((PEG_PRESENT == state->peg[row][column-1]) && \
						(PEG_PRESENT == state->peg[row][column-2]))
					{
						result++;
					}
				}
				//Horizontal, r->l
				if (column <= 2)
				{
					if ((PEG_PRESENT == state->peg[row][column+1]) && \
						(PEG_PRESENT == state->peg[row][column+2]))
					{
						result++;
					}
				}
			}
		}
	}
	
	return result;
}

static void init_game(GAME_STATE* state, uint8_t missing_row, uint8_t missing_column)
{
	int row, column;
	uint8_t column_max[5] = {0,1,2,3,4};	//define the maximum column for each row
	uint8_t moves;

	//initialize beginning state
	for (row = 0; row < 5; row++)
	{
		for (column = 0; column < 5; column++)
		{
			if (column <= column_max[row])
			{
				state->peg[row][column] = PEG_PRESENT;
			}
			else
		{
				state->peg[row][column] = PEG_INVALID;
			}
		}
	}

	//take out one of the pegs
	state->peg[missing_row][missing_column] = NO_PEG_PRESENT;

	//calculate number of possible moves...
	moves = num_moves(state);
	state->num_possible_moves = moves;
	state->num_moves_tried = 0;
	snprintf(state->description, 50, "Initial board - peg (%d,%d) missing.", missing_row, missing_column);
	MAINDBG("MAIN: Initial setup has %d allowable moves. (Desciption: %s)\n", moves, state->description);
}

static int check_for_winning_state(const GAME_STATE* state, uint8_t final_row, uint8_t final_column)
{
	int row, column;
	uint8_t num_pegs_left = 0;
	
	if (state->num_possible_moves > 0)
	{
		return NOT_WINNING_STATE;
	}
	
	//initialize beginning state
	for (row = 0; row < 5; row++)
	{
		for (column = 0; column < 5; column++)
		{
			if (PEG_PRESENT == state->peg[row][column])
			{
				num_pegs_left++;
			}
			if (1 < num_pegs_left)
			{
				return NOT_WINNING_STATE;
			}
		}
	}

	// if we get here, we have 1 or fewer pegs in the board!
	if (final_row <= 4 && final_column <= 4)
	{
		if (state->peg[final_row][final_column] == PEG_PRESENT)
		{
			return WINNING_STATE;
		}
		else
		{
			return NOT_WINNING_STATE;
		}
	}
	else
	{
		return WINNING_STATE;
	}
}

static int get_new_state(const uint8_t move, const GAME_STATE* current_state, GAME_STATE* new_state)
{
	int row = 0;
	int column = 0;
	uint8_t column_max[5] = {0,1,2,3,4};	//define the maximum column for each row
	uint8_t result = 0;
	
	if (move > current_state->num_possible_moves)
	{
		MAINDBG("MAIN: ERROR! Tried move %d but there were only %d moves possible.\n", move, current_state->num_possible_moves);
		return false;
	}


	//Moves are only possible if there is a blank space. Look for a blank space...
	for (row = 0; row < 5; row++)
	{
		for (column = 0; column <= column_max[row]; column++)
		{
			if (NO_PEG_PRESENT == current_state->peg[row][column])
			{
				/*
				* we have a blank space! Check for valid moves in the following order:
				* 	1) diagonal down l->r
				* 	2) diagonal down r->l
				* 	3) diagonal up l->r
				*	4) diagonal up r->l
				*	5) horizontal l->r
				*	6) horizontal r->l
				*/

				//Diagonal down, l->r
				if ((row >= 2) && (column >= 2))
				{
					if ((PEG_PRESENT == current_state->peg[row-1][column-1]) && \
						(PEG_PRESENT == current_state->peg[row-2][column-2]))
					{
						result++;
						if ((move + 1) == result)
						{
							memcpy(new_state, current_state, sizeof(GAME_STATE));
							new_state->peg[row][column] = PEG_PRESENT;
							new_state->peg[row-1][column-1] = NO_PEG_PRESENT;
							new_state->peg[row-2][column-2] = NO_PEG_PRESENT;
							new_state->num_possible_moves = num_moves(new_state);
							new_state->num_moves_tried = 0;
							snprintf(new_state->description,50,"Peg (%d,%d) moves Diagonal Down L->R",(row-2),(column-2));
						}
					}
				}
				//Diagonal down, r->l
				if ((row >= 2) && (column <= (column_max[row] - 2)))
				{
					if ((PEG_PRESENT == current_state->peg[row-1][column]) && \
						(PEG_PRESENT == current_state->peg[row-2][column]))
					{
						result++;
						if ((move + 1) == result)
						{
							memcpy(new_state, current_state, sizeof(GAME_STATE));
							new_state->peg[row][column] = PEG_PRESENT;
							new_state->peg[row-1][column] = NO_PEG_PRESENT;
							new_state->peg[row-2][column] = NO_PEG_PRESENT;
							new_state->num_possible_moves = num_moves(new_state);
							new_state->num_moves_tried = 0;
							snprintf(new_state->description,50,"Peg (%d,%d) moves Diagonal Down R->L",(row-2),(column));
						}
					}
				}
				//Diagonal up, l->r
				if ((row <= 2) && (column <= 2))
				{
					if ((PEG_PRESENT == current_state->peg[row+1][column]) && \
						(PEG_PRESENT == current_state->peg[row+2][column]))
					{
						result++;
						if ((move + 1) == result)
						{
							memcpy(new_state, current_state, sizeof(GAME_STATE));
							new_state->peg[row][column] = PEG_PRESENT;
							new_state->peg[row+1][column] = NO_PEG_PRESENT;
							new_state->peg[row+2][column] = NO_PEG_PRESENT;
							new_state->num_possible_moves = num_moves(new_state);
							new_state->num_moves_tried = 0;
							snprintf(new_state->description,50,"Peg (%d,%d) moves Diagonal Up L->R",(row+2),(column));
						}
					}
				}
				//Diagonal up, r->l
				if ((row <= 2) && (column >= (column_max[row] - 2)))
				{
					if ((PEG_PRESENT == current_state->peg[row+1][column+1]) && \
						(PEG_PRESENT == current_state->peg[row+2][column+2]))
					{
						result++;
						if ((move + 1) == result)
						{
							memcpy(new_state, current_state, sizeof(GAME_STATE));
							new_state->peg[row][column] = PEG_PRESENT;
							new_state->peg[row+1][column+1] = NO_PEG_PRESENT;
							new_state->peg[row+2][column+2] = NO_PEG_PRESENT;
							new_state->num_possible_moves = num_moves(new_state);
							new_state->num_moves_tried = 0;
							snprintf(new_state->description,50,"Peg (%d,%d) moves Diagonal Up R->L",(row+2),(column+2));
						}
					}
				}
				//Horizontal, l->r
				if (column >= 2)
				{
					if ((PEG_PRESENT == current_state->peg[row][column-1]) && \
						(PEG_PRESENT == current_state->peg[row][column-2]))
					{
						result++;
						if ((move + 1) == result)
						{
							memcpy(new_state, current_state, sizeof(GAME_STATE));
							new_state->peg[row][column] = PEG_PRESENT;
							new_state->peg[row][column-1] = NO_PEG_PRESENT;
							new_state->peg[row][column-2] = NO_PEG_PRESENT;
							new_state->num_possible_moves = num_moves(new_state);
							new_state->num_moves_tried = 0;
							snprintf(new_state->description,50,"Peg (%d,%d) moves Horizontal L->R",(row),(column-2));
						}
					}
				}
				//Horizontal, r->l
				if (column <= 2)
				{
					if ((PEG_PRESENT == current_state->peg[row][column+1]) && \
						(PEG_PRESENT == current_state->peg[row][column+2]))
					{
						result++;
						if ((move + 1) == result)
						{
							memcpy(new_state, current_state, sizeof(GAME_STATE));
							new_state->peg[row][column] = PEG_PRESENT;
							new_state->peg[row][column+1] = NO_PEG_PRESENT;
							new_state->peg[row][column+2] = NO_PEG_PRESENT;
							new_state->num_possible_moves = num_moves(new_state);
							new_state->num_moves_tried = 0;
							snprintf(new_state->description,50,"Peg (%d,%d) moves Horizontal R->L",(row),(column+2));
						}
					}
				}
			}
		}
	}
	
	return result;
}

static void printf_board(const GAME_STATE* state)
{
	printf("\n    %d\n", state->peg[0][0]);
	printf("   %d %d\n", state->peg[1][0], state->peg[1][1]);
	printf("  %d %d %d\n", state->peg[2][0], state->peg[2][1], state->peg[2][2]);
	printf(" %d %d %d %d\n", state->peg[3][0], state->peg[3][1], state->peg[3][2], state->peg[3][3]);
	printf("%d %d %d %d %d\n", state->peg[4][0], state->peg[4][1], state->peg[4][2], state->peg[4][3], state->peg[4][4]);
}
