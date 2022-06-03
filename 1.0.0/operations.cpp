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
        // Now, each square is extracted

        // Create the result of moving a row left
        for (int i = 0; i < ROW_SIZE; i++) {
            // All we need is stable sorting algorithm for result, with key 0 or nonzero
            std::stable_sort(square[0], square[3], row_shift_comp);
        }
        
        // Concatenate the squares for the resulting row
        row_t result = (square[0]) | 
                        (square[1] <<  SQUARE_BITS) | 
                        (square[2] <<  2*SQUARE_BITS) | 
                        (square[3] << 3*SQUARE_BITS);
    }
}

bool row_shift_comp(const row_t &a, const row_t &b) {
    return ((a != 0) && (b == 0));
}