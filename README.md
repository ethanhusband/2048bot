
# 2048 Bot

To operate, simply open any terminal where python, enter

    python3 2048.py

## ALGORITHM

The algorithm is an adaptation of the nneonneo 2048 bot - https://github.com/nneonneo/2048-ai

### EXPECTIMAX 

The algorithm implements an expectimax tree to search for the best move.

Expectimax trees alternate between move nodes and chance nodes. Move nodes simply have a value which is equivalent the highest utility child node, while chance nodes have a value equivalent to the expected value of all their child nodes, which are all possible arrangements following the chance node.

As the tree is searched through, branched where the probability that such a branch occurs is less than a particular threshold, are pruned. The depth of the search is also proportional to the number of distinct tiles, since this is inherently when the game is more difficult and more careful calculation needs to be taken. For example, a board filled with twos and fours generally has an obvious response - merge all those tiles, however an increased number of distinct tiles makes what move to make far less obvious. This dynamic choice of search depth is arguably the strongest heuristic of the AI.

Caching on chance nodes is also used since calculating the expecation within a move tree is computationally expensive. When a chance node is calculated, the board that node possesses is cached with its corresponding score, such that recalculation is dynamically avoided.

## DATA STRUCTURES

### BOARD REPRESENTATION

Since each entry in the board is necessarily a power of 2,
the entire board can be represented in a 64 bit number,
where each 4 bit entry computes the power of 2 possessed by the tile.
Note this limits each tile to a maximum of 2^15 = 32768.
However, the theoretical maximum possible tile in a 4x4 grid
is only 2^(4x4) = 65536 (unless you get a 4 at the end, a neglected case).
Since this has only ever been achieved by 3 AI's, the consideration will be deferred

### OPERATION TABLES

Operation tables allow for the result of a move to be calculated in O(1) time within the decision tree.

With 16 bits per row, each row or column has 2^16 possible states. 
Hence a table of size 2^16 = 65536 can be used to index 
the transformed result of a move in a single direction for every possible row. 
This enables the board to quickly be transformed in each move node, 
which ultimately enables greater search depth per time.
A table will be created for each move direction to save time 
that would otherwise be wasted reversing and transposing matrices to apply one operation table.

The left shift of any given row can be implemented efficiently by using any stable sorting algorithm,
where the key is simply whether the square is zero or nonzero, nonzero obviously taking priority. std::stable_sort() achieves this.

Row tables are looked up simply by indexing with the row itself, and receiving a result that is the corresponding row transformation.

The column tables are admittedly convoluted, due to the fact a column cannot be bound to a 16bit representation
in the same way a row can. How this is managed is to transform the board up or down, the board is first transposed.
Transposing the board converts columns into rows such that each column can be treated as a 16bit number.
This enables the space of possible columns to be small enough to have a lookup table. The 

Ultimately an efficient time complexity for calculating these tables does not affect performance
of the bot itself and has a small upper bound, so is largely unnecessary.

### SCORING TABLES

Scoring tables are used in a similar manner to the board transformation tables. Since the score is computed as the sum result of every merge, it can be explicitly calculated that a given tile contributes to the score by an amount equal to (tile - 1) * (2^tile). Since the score can be evaluated by iterating over the squares, each row can just be used as an index, similar to transformation lookup, and what is returned is the score of that row. Summing over each row gets the total score.

#### HEURISTIC SCORES

Many heuristics such as monotonicity across files, the number of merges available etc improve the board evaluation to play more like a human. The values used for these heuristics are taken directly from the nneonneo algorithm.

## FRONT END

The frontend GUI uses tkinter python library to mock the 2048 table

In order for moves performed to be shown in the frontend, the front end actually managed the state of the grid. Each move, the front end will call the select_move function from the cpp backend using the subprocess library.

Each subprocess call submits the board code and returns the move to be played, and the Grid class in python manages the global state of the board from there.

### PYTHON AI TAKEOVER AT 32768 TILE

It may be noticed that the cpp representation of the board using bitboards limits each square to a maximum of the 32768 tile.
As a final push to get the 65536 tile if the opportunity presents itself, it is planned that the program should switch to a much more rudimentary python AI to make the final push in merging these two tiles when they both exist and are proximal, since the cpp AI is incapable of doing so.


## CODEBASE STANDARDS

- Each operation/transformation in the algorithm is explained, for the sake of sanity
- The use of static inline functions increases runtime speed, as per the (C++ 17) docs:
    - "The original intent of the inline keyword was to serve as an indicator to the optimizer that inline substitution of a    function is preferred over function call, that is, instead of executing the function call CPU instruction to transfer control to the function body, a copy of the function body is executed without generating the call. This avoids overhead created by the function call (passing the arguments and retrieving the result) but it may result in a larger executable as the code for the function has to be repeated multiple times."

## Other

### Heuristic improvements that may or may not work

- For skipping moves where board is the same, there are two options:
  - Compare for equality against table (currently in use)
  - Bitwise xor the result board with the old board and store that in the table, then it will be 0 if move does nothing. Some algorithms have this but I don't yet see how it would improve runtime. Worth testing once implemented to see if better.

    