#if!defined(MINESWEEPER_H)

#define MINESWEEPER_H
#define MAX_ROWS 30
#define MAX_COLS 40
#define GAME_PLAYING 0
#define GAME_WON 1
#define GAME_LOST -1
#define REVEAL_IGNORED 0
#define REVEAL_SUCCESS 1
#define REVEAL_BOOM -1

#include <stdint.h>
#include <stdbool.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef i8 b8;
typedef i32 b32;

typedef struct {
    b8 is_mine;
    b8 is_revealed;
    b8 is_flagged;
    i32 neighbour_mines; // Count of adjacent mines (0-8)
} Cell;

typedef struct {
    i32 rows;
    i32 cols;
    i32 total_mines;
    i32 flags_left;
    i32 game_over; // Uses GAME_PLAYING, GAME_WON, GAME_LOST defines
    i32 first_click; // 0 for unsafe first click, 1 safe first click
    Cell grid[MAX_ROWS][MAX_COLS];
} Board;

void init_board(Board *b, i32 rows, i32 cols, i32 mines);
void place_mines(Board *b, i32 safe_r, i32 safe_c);
void calculate_neighbours(Board *b);
i32 reveal_cell(Board *b, i32 r, i32 c);
void toggle_flag(Board *b, i32 r, i32 c);
i32 check_win_condition(Board *b);

#endif