/*
    To run, open WSL Linux distro, navigate to correct folder path (cd /mnt/c/Users/InsertYourUsername/InsertFolderName)
    Run commands: 
    gcc minesweeper.c -o minesweeper.exe -lncurses
    ./minesweeper.exe
    Must install ncurses lib in your Linux distro to work (sudo apt update && sudo apt install libncurses5-dev libncursesw5-dev)

    Made by CCC
    AI used for debugging and code review, but not for writing the code itself

    Note: I don't expect this to win anything. I just wanted to use a low-level language. C is not the language to use at hackathons.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <ncurses.h>
#include "minesweeper.h"

int main(int argc, char *argv[]) {
    // Initialize ncurses screen
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0); // Hide the terminal blinking cursor

    // Initialize a board
    Board board;
    i32 rows = 10;
    i32 cols = 10;
    i32 total_mines = 12;
    init_board(&board, rows, cols, total_mines);

    // Track cursor position
    i32 cursor_r = 0;
    i32 cursor_c = 0;
    i32 ch;

    // Main game loop
    while (board.game_over == GAME_PLAYING) {
        clear(); // Clear the screen for the new frame

        // Header information
        mvprintw(1, 4, "=== TERMINAL MINESWEEPER ===");
        mvprintw(2, 4, "Controls: Arrow Keys/WASD to Move | SPACE/ENTER to Reveal | F to Flag | Q to Quit");
        mvprintw(3, 4, "Mines Remaining: %d", board.flags_left);

        // Draw the board grid
        for (i32 r = 0; r < board.rows; r++) {
            for (i32 c = 0; c < board.cols; c++) {
                i32 print_r = r + 6; 
                i32 print_c = c * 3 + 4; // Multiply by 3 to space columns nicely
                // Use move() to explicitly place cursor for every cell
                move(print_r, print_c);

                Cell *cell = &board.grid[r][c];
                // Highlight cell currently under cursor
                if (r == cursor_r && c == cursor_c) {
                    attron(A_REVERSE);
                }
                if (cell->is_revealed) {
                    if (cell->is_mine) {
                        printw(" * ");
                    } 
                    else if (cell->neighbour_mines > 0) {
                        printw(" %d ", cell->neighbour_mines);
                    } 
                    else {
                        printw(" . ");
                    }
                } 
                else if (cell->is_flagged) {
                    attron(COLOR_PAIR(3)); // Red flag
                    printw(" F ");
                    attroff(COLOR_PAIR(3));
                } 
                else {
                    printw(" # "); // Hidden cell
                }
                // Turn off cursor highlight
                if (r == cursor_r && c == cursor_c) {
                    attroff(A_REVERSE);
                }
            }
        }

        refresh(); // Refresh screen to render output

        // Listen for user input
        ch = getch();

        switch (ch) {
            // Can use arrow keys and/or WASD for movement
            case KEY_UP:
            case 'w':
            case 'W':
                if (cursor_r > 0) {
                    cursor_r--;
                }
                break;
            case KEY_DOWN:
            case 's':
            case 'S':
                if (cursor_r < board.rows - 1) {
                    cursor_r++;
                }
                break;
            case KEY_LEFT:
            case 'a':
            case 'A':
                if (cursor_c > 0) {
                    cursor_c--;
                }
                break;
            case KEY_RIGHT:
            case 'd':
            case 'D':
                if (cursor_c < board.cols - 1) {
                    cursor_c++;
                }
                break;

            // Reveal cell = space or enter
            case ' ':
            case '\n':
            case KEY_ENTER:
                reveal_cell(&board, cursor_r, cursor_c);
                if (board.game_over == GAME_PLAYING) {
                    check_win_condition(&board);
                }
                break;

            // Toggle flag = F key
            case 'f':
            case 'F':
                toggle_flag(&board, cursor_r, cursor_c);
                break;

            // Quit = Q key
            case 'q':
            case 'Q':
                endwin();
                return 0;
        }
    }

    // Game over handling
    clear();
    mvprintw(5, 4, "========================================");
    if (board.game_over == GAME_WON) {
        mvprintw(6, 4, "  YOU CLEARED THE MINEFIELD. GGS.   ");
    } 
    else {
        // Reveal all mines on loss
        for (i32 r = 0; r < board.rows; r++) {
            for (i32 c = 0; c < board.cols; c++) {
                if (board.grid[r][c].is_mine) {
                    board.grid[r][c].is_revealed = 1;
                }
            }
        }
        mvprintw(6, 4, "    YOU HIT A MINE. RIP.    ");
    }
    mvprintw(7, 4, "========================================");
    mvprintw(9, 4, "Press any key to exit...");
    refresh();
    getch();

    // Clean up ncurses window before exiting
    endwin();
    return 0;
}

void init_board(Board *b, i32 rows, i32 cols, i32 mines) {
    b->rows = rows;
    b->cols = cols;
    b->total_mines = mines;
    b->flags_left = mines;
    b->game_over = GAME_PLAYING;
    b->first_click = 0;

    for (i32 r = 0; r < rows; r++) {
        for (i32 c = 0; c < cols; c++) {
            b->grid[r][c].is_mine = 0;
            b->grid[r][c].is_revealed = 0;
            b->grid[r][c].is_flagged = 0;
            b->grid[r][c].neighbour_mines = 0;
        }
    }
}

// Guarantee safe first click, so place mines after the player's initial click
void place_mines(Board *b, i32 safe_r, i32 safe_c) {
    srand((u32)time(NULL));
    i32 placed = 0;
    
    while (placed < b->total_mines) {
        i32 r = rand() % b->rows;
        i32 c = rand() % b->cols;

        // Do not place a mine on the clicked cell or its immediate neighbours
        if (abs(r - safe_r) <= 1 && abs(c - safe_c) <= 1) {
            continue;
        }

        if (!b->grid[r][c].is_mine) {
            b->grid[r][c].is_mine = 1;
            placed++;
        }
    }
    calculate_neighbours(b);
    b->first_click = 1;
}

void calculate_neighbours(Board *b) {
    for (i32 r = 0; r < b->rows; r++) {
        for (i32 c = 0; c < b->cols; c++) {
            if (b->grid[r][c].is_mine) {
                continue;
            }
            i32 count = 0;
            for (i32 dr = -1; dr <= 1; dr++) {
                for (i32 dc = -1; dc <= 1; dc++) {
                    i32 nr = r + dr;
                    i32 nc = c + dc;
                    if (nr >= 0 && nr < b->rows && nc >= 0 && nc < b->cols) {
                        if (b->grid[nr][nc].is_mine) {
                            count++;
                        }
                    }
                }
            }
            b->grid[r][c].neighbour_mines = count;
        }
    }
}

i32 reveal_cell(Board *b, i32 r, i32 c) {
    // Check oob conditions
    if (r < 0 || r >= b->rows || c < 0 || c >= b->cols) {
        return REVEAL_IGNORED;
    }
    
    Cell *cell = &b->grid[r][c];
    if (cell->is_revealed || cell->is_flagged) {
        return REVEAL_IGNORED;
    }

    // If very first click, generate mines away from here
    if (!b->first_click) {
        place_mines(b, r, c);
    }

    cell->is_revealed = 1;
    if (cell->is_mine) {
        b->game_over = GAME_LOST;
        return REVEAL_BOOM;
    }

    // If the cell has 0 neighbour mines, automatically cascade-reveal neighbours
    if (cell->neighbour_mines == 0) {
        for (i32 dr = -1; dr <= 1; dr++) {
            for (i32 dc = -1; dc <= 1; dc++) {
                if (dr != 0 || dc != 0) {
                    reveal_cell(b, r + dr, c + dc);
                }
            }
        }
    }
    return REVEAL_SUCCESS;
}

void toggle_flag(Board *b, i32 r, i32 c) {
    Cell *cell = &b->grid[r][c];
    if (!cell->is_revealed) {
        cell->is_flagged = !cell->is_flagged;
        if (cell->is_flagged) {
            b->flags_left -= 1;
        } 
        else {
            b->flags_left += 1;
        }
    }
}

i32 check_win_condition(Board *b) {
    for (i32 r = 0; r < b->rows; r++) {
        for (i32 c = 0; c < b->cols; c++) {
            Cell *cell = &b->grid[r][c];
            // If a safe cell is still hidden, game is not won yet
            if (!cell->is_mine && !cell->is_revealed) {
                return GAME_PLAYING;
            }
        }
    }
    b->game_over = GAME_WON;
    return GAME_WON;
}

