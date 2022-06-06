#include "operations.h"

static row_t row_left_table [65536];
static row_t row_right_table[65536];
// Since columns are vertical, we need a board_t to store them. There will still only be 2^16 of them.
static board_t col_up_table[65536];
static board_t col_down_table[65536];

#define TESTING true

int main() {
#if TESTING
    instantiate_tables();
    board_t board = 0x1102211000011031;
    printf("ORIGINAL BOARD:\n" );
    print_bitboard(board);
    printf("SHIFTED UP:\n");
    print_bitboard(play_move_up(board));
#endif
#if !TESTING
#endif
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
    for (unsigned row = 0; row < TABLE_SIZE; row++) {
        // Store each square
        unsigned square[ROW_SIZE] = {
            // Each of these squares is &'d with 0xf = 1111 such that only the last 4 bits are possibly positive
                (row) & 0xf,
                (row >>  SQUARE_BITS) & 0xf,
                (row >>  2*SQUARE_BITS) & 0xf,
                (row >> 3*SQUARE_BITS) & 0xf,  
        };

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
        std::stable_sort(square, square+ROW_SIZE, left_shift_comp);
        
        // Concatenate the left-shift sorted squares for the resulting row
        row_t left_shift_result =  (square[0]) | 
                                    (square[1] <<  SQUARE_BITS) | 
                                    (square[2] <<  2*SQUARE_BITS) | 
                                    (square[3] << 3*SQUARE_BITS);

        // Reverse the order by compressing to the right, to emulate a right shift
        std::stable_sort(square, square+ROW_SIZE, right_shift_comp);
        row_t right_shift_result = (square[0]) | 
                                    (square[1] <<  SQUARE_BITS) | 
                                    (square[2] <<  2*SQUARE_BITS) | 
                                    (square[3] << 3*SQUARE_BITS);

        // Add this row iteration to the tables
        row_left_table[row] = left_shift_result;
        row_right_table[row] = right_shift_result;

        // Note that while we store this at 'row' in the col tables, these tables will always be referenced after a transpose. 
        // Hence they will actually be indexed by columns, just in the form of rows, returning the result in the form of a column.
        col_up_table[row] = transpose_board(left_shift_result); 
        col_down_table[row] = transpose_board(right_shift_result);
    }
}

// Comparison function for sorting a row to the left
bool left_shift_comp(const row_t a, const row_t b) {
    // Should return false if in order. Below is the only out of order case
    return ((a == 0) && (b != 0));
}

// Comparison function for sorting a row to the right
bool right_shift_comp(const row_t a, const row_t b) {
    // Should return false if in order. Below is the only out of order case
    return ((a != 0) && (b == 0));
}


static inline board_t play_move_up(board_t board) {
    board_t result = 0;
    // We transpose such that we can use a 16bit integer as the col_table index, enabling a small enough table.
    // What is returned is the shifted up column in the 64bit board, column is stored on the right of the board.
    board_t transposed_board = transpose_board(board);
    result |= col_up_table[(transposed_board) & ROW_MASK];
    result |= col_up_table[(transposed_board >> 16ULL) & ROW_MASK] <<  4;
    result |= col_up_table[(transposed_board >> 32ULL) & ROW_MASK] <<  8;
    result |= col_up_table[(transposed_board >> 48ULL)] << 12;
    return result;
}

static inline board_t play_move_down(board_t board) {
    board_t result = 0;
    // We transpose such that we can use a 16bit integer as the col_table index, enabling a small enough table.
    // What is returned is the shifted down column in the 64bit board, column is stored on the right of the board.
    board_t transposed_board = transpose_board(board);
    result |= col_down_table[(transposed_board) & ROW_MASK];
    result |= col_down_table[(transposed_board >> 16ULL) & ROW_MASK] <<  4;
    result |= col_down_table[(transposed_board >> 32ULL) & ROW_MASK] <<  8;
    result |= col_down_table[(transposed_board >> 48ULL)] << 12;
    return result;
}

static inline board_t play_move_left(board_t board) {
    board_t result = 0;
    // Concatenate the result of shifting each row left
    result |= board_t(row_left_table[(board >>  0) & ROW_MASK]) <<  0;
    result |= board_t(row_left_table[(board >> 16) & ROW_MASK]) << 16;
    result |= board_t(row_left_table[(board >> 32) & ROW_MASK]) << 32;
    result |= board_t(row_left_table[(board >> 48)]) << 48;
    return result;
}

static inline board_t play_move_right(board_t board) {
    board_t result = 0;
    // Concatenate the result of shifting each row right
    result |= board_t(row_right_table[(board >>  0) & ROW_MASK]) <<  0;
    result |= board_t(row_right_table[(board >> 16) & ROW_MASK]) << 16;
    result |= board_t(row_right_table[(board >> 32) & ROW_MASK]) << 32;
    result |= board_t(row_right_table[(board >> 48)]) << 48;
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