#include "operations.h"

static row_t row_left_table [65536];
static row_t row_right_table[65536];
static board_t col_up_table[65536];
static board_t col_down_table[65536];

#define TESTING true

int main() {
#if TESTING
    instantiate_tables();
#endif
#if !TESTING
#endif
}

static inline board_t row_to_column(row_t row) {
    board_t tmp = row;
    return (tmp | (tmp << 12ULL) | (tmp << 24ULL) | (tmp << 36ULL));
}

board_t transpose_board(board_t board) {
    board_t transposed_board = 0;
    board_t square;
    // Need to place the square at (i, j) at (j, i)
    for (int i = 0; i < ROW_SIZE; i++) {
        for (int j = 0; j < ROW_SIZE; j++) {
            square = (board >> (ROW_BITS*i + SQUARE_BITS*j)) & SQUARE_MASK;
            transposed_board |= (square << (ROW_BITS*j + SQUARE_BITS*i));
        }
    }
    return transposed_board;
}

void instantiate_tables() {
    for (unsigned row = 0x1122; row < 0x1123; row++) {
        // Store each square
        unsigned square[ROW_SIZE] = {
            // Each of these squares is &'d with 0xf = 1111 such that only the last 4 bits are possibly positive
                (row >> 3*SQUARE_BITS) & 0xf,
                (row >>  2*SQUARE_BITS) & 0xf,
                (row >>  SQUARE_BITS) & 0xf,
                (row) & 0xf,
        };
        printf("ROW IS: \n");
        print_bitboard(row);

        // Merge any squares before compressing
        for (int i = 0; i < ROW_SIZE-1; i++) {
            if (square[i] == 0) continue;
            int j = i + 1;
            while (square[j] == 0) {
                j++;
            }
            if (square[j] == square[i]) {
                square[i]++;
                square[j] = 0;
            }
        }
        // Compress to the left, squares are now in order they would be in after a left shift
        std::stable_sort(square, square+ROW_SIZE, row_left_shift_comp);
        
        // Concatenate the left-shift sorted squares for the resulting row
        row_t left_shift_result = (square[0] << 3*SQUARE_BITS) | 
                                  (square[1] << 2*SQUARE_BITS) | 
                                  (square[2] << SQUARE_BITS) | 
                                  (square[3]);

        // Reverse the order to emulate a right shift
        row_t right_shift_result =  (square[0]) | 
                                    (square[1] <<  SQUARE_BITS) | 
                                    (square[2] <<  2*SQUARE_BITS) | 
                                    (square[3] << 3*SQUARE_BITS);

        // Add this row iteration to the tables
        row_left_table[row] = left_shift_result;
        row_right_table[row] = right_shift_result;
        
    }
}


row_t merge_right(row_t row) {

}

// Comparison function for sorting a row to the left
bool row_left_shift_comp(const row_t a, const row_t b) {
    // Should return false if in order. Below is the only out of order case
    return ((a == 0) && (b != 0));
}

// Comparison function for sorting a row to the right
bool row_right_shift_comp(const row_t a, const row_t b) {
    // Should return false if in order. Below is the only out of order case
    return ((a != 0) && (b == 0));
}


static inline board_t play_move_up(board_t board) {
    board_t result = 0;
    board_t t = transpose_board(board);
    result |= col_up_table[(t >>  0) & ROW_MASK] <<  0;
    result |= col_up_table[(t >> 16) & ROW_MASK] <<  4;
    result |= col_up_table[(t >> 32) & ROW_MASK] <<  8;
    result |= col_up_table[(t >> 48) & ROW_MASK] << 12;
    return result;
}

static inline board_t play_move_down(board_t board) {
    board_t result = 0;
    board_t t = transpose_board(board);
    result |= col_down_table[(t >>  0) & ROW_MASK] <<  0;
    result |= col_down_table[(t >> 16) & ROW_MASK] <<  4;
    result |= col_down_table[(t >> 32) & ROW_MASK] <<  8;
    result |= col_down_table[(t >> 48) & ROW_MASK] << 12;
    return result;
}

static inline board_t play_move_left(board_t board) {
    board_t result = 0;
    // Concatenate the result of shifting each row left
    result |= board_t(row_left_table[(board >>  0) & ROW_MASK]) <<  0;
    result |= board_t(row_left_table[(board >> 16) & ROW_MASK]) << 16;
    result |= board_t(row_left_table[(board >> 32) & ROW_MASK]) << 32;
    result |= board_t(row_left_table[(board >> 48) & ROW_MASK]) << 48;
    return result;
}

static inline board_t play_move_right(board_t board) {
    board_t result = 0;
    // Concatenate the result of shifting each row right
    result |= board_t(row_right_table[(board >>  0) & ROW_MASK]) <<  0;
    result |= board_t(row_right_table[(board >> 16) & ROW_MASK]) << 16;
    result |= board_t(row_right_table[(board >> 32) & ROW_MASK]) << 32;
    result |= board_t(row_right_table[(board >> 48) & ROW_MASK]) << 48;
    return result;
}

void print_bitboard(board_t board) {
    int board_nums[BOARD_SIZE];
    int square;
    // Store all the squares in an array. Storing occurs in reverse order.
    for (int i = 0; i < BOARD_SIZE; i++) {
        square = SQUARE_MASK & (board >> i*SQUARE_BITS);
        board_nums[BOARD_SIZE - i - 1] = square;
    }  
    printf(BOARD_SEPERATOR);
    for (int i = 0; i < ROW_SIZE; i++) {
        putchar('\n');
        for (int j = 0; j < ROW_SIZE; j++) {
            printf(" %5d |", (int)pow(2, board_nums[ROW_SIZE*i + j]));
        }
        putchar('\n');
        if (i != ROW_SIZE - 1) {
            printf(ROW_SEPERATOR);
        }
    }
    printf(BOARD_SEPERATOR);
    putchar('\n');
}