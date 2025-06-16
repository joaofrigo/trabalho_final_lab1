// game_state.c
#include "game_state.h"

int game_state_num_cores(const GameState *gs) {
    int cores = INITIAL_COLORS + (gs->etapa - 1);
    int max = INITIAL_COLORS + (NUM_ETAPAS_MAX - 1);
    if (cores > max) cores = max;
    return cores;
}

void game_state_init(GameState *gs) {
    gs->etapa = 1;
    gs->score = 0;
    srand((unsigned) time(NULL));
    int ncores = game_state_num_cores(gs);
    for (int r = 0; r < NUM_LINHAS; r++) {
        for (int c = 0; c < NUM_COLUNAS; c++) {
            gs->board[r][c] = rand() % ncores;
        }
    }
}

void game_state_rotate_row(GameState *gs, int row, int direction) {
    if (row < 0 || row >= NUM_LINHAS) return;
    int temp[NUM_COLUNAS];
    for (int c = 0; c < NUM_COLUNAS; c++) {
        int src = (c - direction + NUM_COLUNAS) % NUM_COLUNAS;
        temp[c] = gs->board[row][src];
    }
    for (int c = 0; c < NUM_COLUNAS; c++) {
        gs->board[row][c] = temp[c];
    }
}

bool game_state_rotate_column(GameState *gs, int col, int direction) {
    if (col < 0 || col >= NUM_COLUNAS) return false;
    int dest = (col + direction + NUM_COLUNAS) % NUM_COLUNAS;
    bool has_occupied = false;
    bool has_empty = false;
    for (int r = 0; r < NUM_LINHAS; r++) {
        if (gs->board[r][col] != EMPTY_CELL) has_occupied = true;
        if (gs->board[r][dest] == EMPTY_CELL) has_empty = true;
    }
    if (!has_occupied || !has_empty) return false;
    int value = gs->board[NUM_LINHAS-1][col];
    if (value == EMPTY_CELL) return false;
    gs->board[NUM_LINHAS-1][col] = EMPTY_CELL;
    gs->board[0][dest] = value;
    return true;
}

void game_state_apply_gravity(GameState *gs) {
    for (int c = 0; c < NUM_COLUNAS; c++) {
        int write_r = NUM_LINHAS - 1;
        for (int r = NUM_LINHAS - 1; r >= 0; r--) {
            if (gs->board[r][c] != EMPTY_CELL) {
                if (write_r != r) {
                    gs->board[write_r][c] = gs->board[r][c];
                    gs->board[r][c] = EMPTY_CELL;
                }
                write_r--;
            }
        }
        for (int r = write_r; r >= 0; r--) {
            gs->board[r][c] = EMPTY_CELL;
        }
    }
}

int game_state_remove_uniform_columns(GameState *gs) {
    int total_removed = 0;
    for (int c = 0; c < NUM_COLUNAS; c++) {
        int first = gs->board[0][c];
        if (first == EMPTY_CELL) continue;
        bool uniform = true;
        for (int r = 1; r < NUM_LINHAS; r++) {
            if (gs->board[r][c] != first) {
                uniform = false;
                break;
            }
        }
        if (uniform) {
            for (int r = 0; r < NUM_LINHAS; r++) {
                gs->board[r][c] = EMPTY_CELL;
                total_removed++;
            }
        }
    }
    if (total_removed > 0) {
        gs->score += total_removed * gs->etapa;
    }
    return total_removed;
}

bool game_state_first_row_empty(const GameState *gs) {
    for (int c = 0; c < NUM_COLUNAS; c++) {
        if (gs->board[0][c] != EMPTY_CELL) return false;
    }
    return true;
}

void game_state_spawn_first_row(GameState *gs) {
    if (!game_state_first_row_empty(gs)) return;
    int rand_col = rand() % NUM_COLUNAS;
    int ncores = game_state_num_cores(gs);
    gs->board[0][rand_col] = rand() % ncores;
}

int game_state_clear_row(GameState *gs, int row) {
    if (row < 0 || row >= NUM_LINHAS) return 0;
    int removed = 0;
    for (int c = 0; c < NUM_COLUNAS; c++) {
        if (gs->board[row][c] != EMPTY_CELL) {
            gs->board[row][c] = EMPTY_CELL;
            removed++;
        }
    }
    if (removed > 0) {
        gs->score -= 2;
    }
    return removed;
}