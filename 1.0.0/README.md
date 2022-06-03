### BOARD REPRESENTATION

    Since each entry in the board is necessarily a power of 2,
    the entire board can be represented in a 64 bit number,
    where each 4 bit entry computes the power of 2 possessed by the tile.
    Note this limits each tile to a maximum of 2^15 = 32768.
    However, the theoretical maximum possible tile in a 4x4 grid
    is only 2^(4x4) = 65536 (unless you get a 4 at the end, a neglected case).
    Since this has only ever been achieved by 3 AI's, the consideration will be deferred

### OPERATION TABLES

    With 16 bits per row, each row or column has 2^16 possible states. 
    Hence a table of size 2^16 = 65536 can be used to index 
    the transformed result of a move in a single direction of every possible row. 
    This enables the board to quickly be transformed in each move node, 
    which ultimately enables greater search depth per time.
    A table will be created for each move direction to save time 
    that would otherwise be wasted reversing and transpose matrices to apply one operation table */