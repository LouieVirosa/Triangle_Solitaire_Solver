# Triangle_Solitaire_Solver

A Depth-First-Search (DFS) solver for the peg solitaire puzzles often found in the Cracker Barrel chain of restaurants.
This command line tool will print out all moves necessary to win the game leaving only one peg on the board. 

To install:

  make

then copy the "triangle" executable to the appropriate directory (/usr/bin).

To run the app:

  triangle <row> <column>

where <row> and <column> are integers 0-4 which represent which pin starts out missing. For example, to start the game
with the top pin missing, you would invoke it as:

  triangle 0 0
  
To start a game with the 2nd from the right pin missing on the bottom row, invoke it as follows:

  triangle 4 3
  
You can also try to solve the puzzle by specifying which pin you would like to be the last one standing. This is done as 
follows: 
  
  triangle <row> <column> <final_row> <final_column>
  
Note that not all combinations of <final_row> <final_column> have a solution for a given starting peg.
