/**
 * FileName: triangle.h
 *
 * @file
 * Triangle configuration header file
 *
 * @author LE
 *
 * REVISION HISTORY:
 *
 * - LE  06/30/15 -> original development
 */

#ifndef _TRIANGLE_H_
#define _TRIANGLE_H_

/* Platform includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Project includes */


/* */
#define true			1
#define false			0

#define PEG_INVALID		2
#define NO_PEG_PRESENT		0
#define PEG_PRESENT		1

#define WINNING_STATE		0
#define NOT_WINNING_STATE	1

#define MAX_MOVES		14	//Each game starts with 14 pegs; 13 good moves will result in 1 peg left, so 14 states total (including initial)

/* Game Status Structure */
#pragma pack(push, 1)	//do this to avoid byte boundary nonsense
typedef struct 
{
	//two-dimensional array looks as follows (all other values are don't cares)
	uint8_t peg [5][5];				//		(0,0)
							//	     (1,0) (1,1)
							//	   (2,0)(2,1)(2,2)
							// 	 (3,0)(3,1)(3,2)(3,3)
							//    (4,0)(4,1)(4,2)(4,3)(4,4)
	uint8_t num_possible_moves;			// number of valid moves possible from this game state
	uint8_t num_moves_tried;			// how many of these moves have you chased down to see if they result in success?
	char description[50];				// human readable string to let us know which move we are trying...
} GAME_STATE;
#pragma pack(pop)

#endif /*_TRIANGLE_H_ */
