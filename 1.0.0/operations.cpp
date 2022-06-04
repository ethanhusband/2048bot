#include "operations.h"

static row_t row_left_table [65536];
static row_t row_right_table[65536];
static board_t col_up_table[65536];
static board_t col_down_table[65536];

void instantiate_tables() {
    for (row_t row = 0; row < TABLE_SIZE; row++) {
        // Store each square
        row_t square[ROW_SIZE] = {
            // Each of these squares is &'d with 0xf = 1111 such that only the last 4 bits are possibly positive
                (row) & 0xf,
                (row >>  SQUARE_BITS) & 0xf,
                (row >>  2*SQUARE_BITS) & 0xf,
                (row >> 3*SQUARE_BITS) & 0xf
        };
        // Now, each square is extracted

        // Create the result of moving a row right
        // All we need is stable sorting algorithm for result, with key as nonzero > zero
            std::stable_sort(square, square+ROW_SIZE, row_shift_comp);
        
        // Concatenate the sorted squares for the resulting row
        row_t left_shift_result = (square[0]) | 
                                (square[1] <<  SQUARE_BITS) | 
                                (square[2] <<  2*SQUARE_BITS) | 
                                (square[3] << 3*SQUARE_BITS);
        row_t right_shift_result =  (square[3]) | 
                                (square[2] <<  SQUARE_BITS) | 
                                (square[1] <<  2*SQUARE_BITS) | 
                                (square[0] << 3*SQUARE_BITS);

        // Add this row iteration to the tables

        row_left_table[row] = left_shift_result;
        row_right_table[row] = right_shift_result;
        
    }
}

// Comparison function for sorting a row to the left
bool row_shift_comp(const row_t a, const row_t b) {
    // Should return true if in order. Below is the only out of order case
    return !((a == 0) && (b != 0));
}


static inline board_t execute_move_left(board_t board) {
    board_t ret = 0;
    ret |= board_t(row_left_table[(board >>  0) & SQUARE_MASK]) <<  0;
    ret |= board_t(row_left_table[(board >> 16) & SQUARE_MASK]) << 16;
    ret |= board_t(row_left_table[(board >> 32) & SQUARE_MASK]) << 32;
    ret |= board_t(row_left_table[(board >> 48) & SQUARE_MASK]) << 48;
    return ret;
}

static inline board_t execute_move_right(board_t board) {
    board_t ret = 0;
    ret |= board_t(row_right_table[(board >>  0) & SQUARE_MASK]) <<  0;
    ret |= board_t(row_right_table[(board >> 16) & SQUARE_MASK]) << 16;
    ret |= board_t(row_right_table[(board >> 32) & SQUARE_MASK]) << 32;
    ret |= board_t(row_right_table[(board >> 48) & SQUARE_MASK]) << 48;
    return ret;
}