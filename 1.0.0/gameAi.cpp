#include "gameAi.h"

// Stop caching after depth reaches this limit (hits too improbable for this to be more efficient)
#define CACHE_DEPTH_LIM 15
// Dont calculate moves where the cumulative probability of the random squares occurring is below this threshold
#define CPROB_THRESHOLD 0.0001f

static row_t row_left_table [65536];
static row_t row_right_table[65536];
// Since columns are vertical, we need a board_t to store them. There will still only be 2^16 of them.
static board_t col_up_table[65536];
static board_t col_down_table[65536];
// Regular score table is kept for the sake of testing and comparison, heur table is primarily used however.
static float score_table[65536];
static float heur_score_table[65536];

#define USING_FRONTEND true

int main(int argc, char *argv[]) {
    instantiate_tables();
#if !USING_FRONTEND
    play_game();
#endif
#if USING_FRONTEND
    
    board_t board;
    std::cin >> board;
    printf("RECEIVED BITBOARD: \n");
    print_bitboard(board);
    int result = select_move(board);
    printf("RESULT: %d\n", result);
    return result;
#endif
}

// Heuristic scoring settings - values taken from existing 2048 ai
static const float SCORE_LOST_PENALTY = 200000.0f;
static const float SCORE_MONOTONICITY_POWER = 4.0f;
static const float SCORE_MONOTONICITY_WEIGHT = 47.0f;
static const float SCORE_SUM_POWER = 3.5f;
static const float SCORE_SUM_WEIGHT = 11.0f;
static const float SCORE_MERGES_WEIGHT = 700.0f;
static const float SCORE_EMPTY_WEIGHT = 270.0f;

void instantiate_tables() {
    for (unsigned row = 0; row < TABLE_SIZE; row++) {
        // Store each square
        unsigned square[ROW_SIZE] = {
            // Each of these squares is &'d with 0xf = 1111 such that only the last 4 bits are possibly positive.
            // This technique is used often throughout the software to extract rows or squares.
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

        // Add row to heuristic score table
        float sum = 0;
        int empty = 0;
        int merges = 0;

        int prev = 0;
        int counter = 0;
        for (int i = 0; i < ROW_SIZE; ++i) {
            int rank = square[i];
            sum += pow(rank, SCORE_SUM_POWER);
            // Count the amount of empty squares
            if (rank == 0) {
                empty++;
            } else {
                // Test the amount of merges possible
                if (prev == rank) {
                    counter++;
                } else if (counter > 0) {
                    merges += 1 + counter;
                    counter = 0;
                }
                prev = rank;
            }
        }
        if (counter > 0) {
            merges += 1 + counter;
        }
        
        // Weight the monotonicity in heur score
        float monotonicity_left = 0;
        float monotonicity_right = 0;
        for (int i = 1; i < ROW_SIZE; ++i) {
            if (square[i-1] > square[i]) {
                monotonicity_left += pow(square[i-1], SCORE_MONOTONICITY_POWER) - pow(square[i], SCORE_MONOTONICITY_POWER);
            } else {
                monotonicity_right += pow(square[i], SCORE_MONOTONICITY_POWER) - pow(square[i-1], SCORE_MONOTONICITY_POWER);
            }
        }

        heur_score_table[row] = SCORE_LOST_PENALTY +
            SCORE_EMPTY_WEIGHT * empty +
            SCORE_MERGES_WEIGHT * merges -
            SCORE_MONOTONICITY_WEIGHT * std::min(monotonicity_left, monotonicity_right) -
            SCORE_SUM_WEIGHT * sum;

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
    return heur_score_table[(board >>  0) & ROW_MASK] +
           heur_score_table[(board >> 16) & ROW_MASK] +
           heur_score_table[(board >> 32) & ROW_MASK] +
           heur_score_table[(board >> 48) & ROW_MASK];
}

void play_game() {
    board_t board = init_board();
    // We now have our starting board
    while(true) {
        printf("BOARD IS NOW: %llu\n", board);
        print_bitboard(board);
        // The score increases when 2 tiles merge, by an amount equal to the value of the merged tile. 
        // Given this, there is a way to calculate the score explicitly using just the tiles.
        // However, getting a 4 to begin with means it wasnt a merged tile, and needs to be recorded to subtract from score.
        int score_penalty = 0;
        int best_move = select_move(board);        

        if(best_move < 0)
            // get_move_tree_score returns negative if no available moves
            break;
        printf("PLAYING MOVE %d\n", best_move);
        board = play_move(best_move, board);

        board_t new_square = get_new_square();
        if (new_square == 2) score_penalty += 4;
        board = insert_rand_square(board, new_square);
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
    float max_util = 0;
    int best_move = -1;

    for (int i = 0; i < MOVE_DIRECTIONS; i++) {
        if (play_move(i, board) == board) continue;

        float move_score = score_root_move(board, i);
        if (move_score > max_util) {
            best_move = i;
            max_util = move_score;
        }
    }
    return best_move;
}

float score_root_move(board_t board, int move) {
    // state, play_move(board, i), 1.0f
    eval_state state;
    state.depth_limit = std::max(3, count_distinct_tiles(board) - 2);
    board_t move_board = play_move(move, board);
    float move_score = score_chance_node(state, move_board, 1.0f) + 1e-6;
    printf("Move %d: result %f: eval'd %ld moves (%d cache hits, %d cache size) (maxdepth=%d)\n", move, move_score, \
                state.moves_evaled, state.cachehits, (int)state.cache_table.size(), state.maxdepth);
    return move_score;
}

static float score_max_node(eval_state &state, board_t board, float cprob) {
    float highest_utility = 0.0f;
    state.curdepth++;
    for (int move = 0; move < MOVE_DIRECTIONS; ++move) {
        state.moves_evaled++;
        board_t newboard = play_move(move, board);

        if (board != newboard) {
            highest_utility = std::max(highest_utility, score_chance_node(state, newboard, cprob));
        }
    }
    state.curdepth--;
    return highest_utility;
}

static float score_chance_node(eval_state &state, board_t board, float cprob) {
    if (state.curdepth >= state.depth_limit || cprob < CPROB_THRESHOLD) {
            state.maxdepth = std::max(state.curdepth, state.maxdepth);
            return score_board(board);
        }

    // Take the expected score from the cache if possible
    if (state.curdepth < CACHE_DEPTH_LIM) {
        const cache_table_t::iterator &i = cache_table.find(board);
        if (i != cache_table.end()) {
            cache_entry_t entry = i->second;
            if(entry.depth <= state.curdepth) {
                state.cachehits++;
                return entry.heuristic;
            }
        }
    }

    int empties = count_empty_squares(board);
    cprob /= empties;

    float expectation = 0.0f;
    board_t tmp = board;
    board_t two_board = 1;
    while (two_board) {
        // If we hit an empty square, test it
        if ((tmp & 0xf) == 0) {
            expectation += score_max_node(state, board |  two_board, cprob * 0.9f) * 0.9f;
            expectation += score_max_node(state, board | (two_board << 1), cprob * 0.1f) * 0.1f;
        }
        tmp >>= SQUARE_BITS;
        two_board <<= SQUARE_BITS;
    }
    expectation = expectation / empties;

    // Add this result to the cache
    if (state.curdepth < CACHE_DEPTH_LIM) {
        cache_entry_t entry = {static_cast<uint8_t>(state.curdepth), expectation};
        cache_table[board] = entry;
    }

    return expectation;

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
    // Index is used to place the new sqaure at the <index>th empty square we find
    if (empties > 1) {
        index = rand()%empties;
    }
    int empties_found=0;
    for (int i = 0; i < BOARD_BITS; i += SQUARE_BITS) {
        if (!((board >> i) & SQUARE_MASK)) empties_found++;
        if (empties_found == index) {
            board |= (new_square << i);
            break;
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

static inline int count_distinct_tiles(board_t board) {
    uint16_t bitset = 0;
    while (board) {
        // This will push a 1 to the same location for the same square value
        bitset |= 1<<(board & SQUARE_MASK);
        board >>= SQUARE_BITS;
    }

    // Don't count empty tiles.
    bitset >>= 1;

    int count = 0;
    while (bitset) {
        // Check the amount of 1's in the bitset, corresponding to amount of distinct tiles
        bitset &= bitset - 1;
        count++;
    }
    return count;
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