#include "gameAi.h"

static row_t row_left_table [65536];
static row_t row_right_table[65536];
// Since columns are vertical, we need a board_t to store them. There will still only be 2^16 of them.
static board_t col_up_table[65536];
static board_t col_down_table[65536];
static float score_table[65536];

#define USING_FRONTEND false

int main(int argc, char *argv[]) {
    instantiate_tables();
#if USING_FRONTEND
    /*printf("STARTING WITH BOARD: \n");
    print_bitboard(299068625676867);
    int move = select_move(299068625676867);
    printf("RESULT IS MOVE %d\n", move);
    print_bitboard(play_move(move, 299068625676867));*/
    play_game();
#endif
#if !USING_FRONTEND
    
    board_t board;
    std::cin >> board;
    printf("RECEIVED BITBOARD: \n");
    print_bitboard(board);
    int result = select_move(board);
    printf("RESULT: %d", result);
    return result;
#endif
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

        // Add this row to the score table
        float score = 0.0f;
        for (int i = 0; i < 4; ++i) {
            int tile = square[i];
            if (tile >= 2) {
                // The score is the total sum of the tile and all intermediate merged tiles
                score += (tile - 1) * (1 << tile);
            }
        }
        score_table[row] = score;

        // Merge any squares before compressing
        for (int i = 0; i < ROW_SIZE-1; i++) {
            if (square[i] == 0) continue;
            int j = i + 1;
            while (square[j] == 0) {
                j++;
            }
            if (square[j] == square[i]) {
                if (square[j] == 0xf) continue; // handle the 32768 limit
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

static float score_board(board_t board) {
    return score_table[(board >>  0) & ROW_MASK] +
           score_table[(board >> 16) & ROW_MASK] +
           score_table[(board >> 32) & ROW_MASK] +
           score_table[(board >> 48) & ROW_MASK];
}

void play_game() {
    board_t board = init_board();
    // We now have our starting board

    while(true) {
        printf("BOARD IS NOW: %llu\n", board);
        print_bitboard(board);
        board_t newboard;
        // The score increases when 2 tiles merge, by an amount equal to the value of the merged tile. 
        // Given this, there is a way to calculate the score explicitly using just the tiles.
        // However, getting a 4 to begin with means it wasnt a merged tile, and needs to be recorded to subtract from score.
        int score_penalty = 0;
        int best_move = select_move(board);        

        if(best_move < 0)
            // get_move_tree_score returns negative if no available moves
            break;

        newboard = play_move(best_move, board);

        board_t new_square = get_new_square();
        if (new_square == 2) score_penalty += 4;
        board = insert_rand_square(newboard, new_square);
    }
    printf("Game over\n");
}

board_t init_board() {
    board_t board = 0;
    board_t starting_square_one = get_new_square();
    board_t starting_square_two = get_new_square();
    board = insert_rand_square(insert_rand_square(board, starting_square_one), starting_square_two);
    return board;
}

int select_move(board_t board) {
    float max_util = -1;
    int best_move = -1;
    for (int i = 0; i < MOVE_DIRECTIONS; i++) {
        if (play_move(i, board) == board) continue;
        float move_score = get_move_tree_score(play_move(i, board), 0, false, 1);
        if (move_score > max_util) {
            best_move = i;
            max_util = move_score;
        }
    }
    return best_move;
}


// Dont calculate moves where the cumulative probability of the random squares occurring is below this threshold
#define CPROB 0.0001f
// This is where expectimax algorithm is implemented to create a move tree
float get_move_tree_score(board_t board, int depth, int is_max, float cprob) {
    if (depth == MAX_DEPTH || cprob < CPROB) return score_board(board);

    // Max node, pick the highest score move
    if (is_max) {
        int highest_utility = -1;
        board_t best_move;
        for (int i = 0; i < ROW_SIZE; i++) {
            board_t tmp = play_move(i, board);
            // Ensure board is not the same, set highest utility to the board with max score
            if (tmp != board && get_move_tree_score(tmp, depth+1, TRUE, cprob) > highest_utility) {
                highest_utility = score_board(board);
                best_move = tmp;
            }
        }
        return highest_utility;
    }
    // Chance node. Returns the average of possible panel spawns
    else {
        float expectation_sum=0;
        float empty_squares=0;
        for (int i = 0; i < BOARD_BITS; i+=SQUARE_BITS) {
            // Check for empty square, add random val to it
            if (!((board << i) & SQUARE_MASK)) {
                empty_squares++;
                int two_board = board | (1 << i);
                int four_board = (board) | (2 << i);
                expectation_sum += 0.9*get_move_tree_score(two_board, depth+1, FALSE, 0.9*cprob) + \
                                    0.1*get_move_tree_score(four_board, depth+1, FALSE, 0.1*cprob);
            }
        }
        return expectation_sum/empty_squares;
    }
}


static inline board_t transpose_board(board_t board) {
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

board_t insert_rand_square(board_t board, board_t new_square) {
    int empties = count_empty_squares(board);
    int index = 1;
    if (empties > 1) {
        index = rand()%empties;
    }
    int empties_found=0;
    for (int i = 0; i < BOARD_BITS; i += SQUARE_BITS) {
        if (!((board >> i) & SQUARE_MASK)) empties_found++;
        if (empties_found == index) {
            board |= (new_square << i);
        }
    }
    return board;
}

board_t get_new_square() {
    srand(time(NULL));
    board_t new_square;
    rand()%10 < 9 ? new_square = 1 : new_square = 2;
    return new_square;
}

int count_empty_squares(board_t board) {
    int empty_squares = 0;
    for (int i = 0; i < BOARD_BITS; i += SQUARE_BITS) {
        if (!((board >> i) & SQUARE_MASK)) empty_squares++;
    }
    return empty_squares;
}

// Pick one of the moves. Moves use index codes such that this function can be looped through with fewer operations.
static inline board_t play_move(int move, board_t board) {
    switch(move) {
    case 0:
        return play_move_up(board);
    case 1:
        return play_move_down(board);
    case 2:
        return play_move_left(board);
    case 3:
        return play_move_right(board);
    default:
        return 0;
    }  
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