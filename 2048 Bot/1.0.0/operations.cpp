#include "operations.h"

void instantiate_tables() {
    for (row_t row = 0; row < TABLE_SIZE; ++row) {
        // Store each square
        row_t square[ROW_SIZE] = {
            // Each of these squares is &'d with 0xf = 1111 such that only the last 4 bits are possibly positive
                (row) & 0xf,
                (row >>  SQUARE_BITS) & 0xf,
                (row >>  2*SQUARE_BITS) & 0xf,
                (row >> 3*SQUARE_BITS) & 0xf
        };
        // Now, each square is extracted.
    }
}