

##### DATA STRUCTURES

#### BOARD REPRESENTATION

Since each entry in the board is necessarily a power of 2,
the entire board can be represented in a 64 bit number,
where each 4 bit entry computes the power of 2 possessed by the tile.
Note this limits each tile to a maximum of 2^15 = 32768.
However, the theoretical maximum possible tile in a 4x4 grid
is only 2^(4x4) = 65536 (unless you get a 4 at the end, a neglected case).
Since this has only ever been achieved by 3 AI's, the consideration will be deferred

#### OPERATION TABLES

Operation tables allow for the result of a move to be calculated in O(1) time within the decision tree.

With 16 bits per row, each row or column has 2^16 possible states. 
Hence a table of size 2^16 = 65536 can be used to index 
the transformed result of a move in a single direction for every possible row. 
This enables the board to quickly be transformed in each move node, 
which ultimately enables greater search depth per time.
A table will be created for each move direction to save time 
that would otherwise be wasted reversing and transposing matrices to apply one operation table.

The left shift of any given row can be implemented efficiently by using any stable sorting algorithm,
where the key is simply whether the square is zero or nonzero, nonzero obviously taking priority. 
std::stable_sort() achieves this.

Ultimately an efficient time complexity for calculating these tables does not affect performance
of the bot itself and has a small upper bound, so is largely unnecessary.

##### CODEBASE STANDARDS

- Each operation/transformation in the algorithm is explained, for the sake of sanity
- The use of static inline functions increases runtime speed, as per the (C++ 17) docs:
    - "The original intent of the inline keyword was to serve as an indicator to the optimizer that inline substitution of a    function is preferred over function call, that is, instead of executing the function call CPU instruction to transfer control to the function body, a copy of the function body is executed without generating the call. This avoids overhead created by the function call (passing the arguments and retrieving the result) but it may result in a larger executable as the code for the function has to be repeated multiple times."

#### Other

## Heuristic improvements that may or may not work

- For skipping moves where board is the same, there are two options:
  - Compare for equality against table (currently in use)
  - Bitwise xor the result board with the old board and store that in the table, then it will be 0 if move does nothing. Some algorithms have this but I don't yet see how it would improve runtime. Worth testing once implemented to see if better.

    