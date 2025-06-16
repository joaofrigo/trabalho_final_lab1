// game_state.h
#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "config.h"

typedef struct {
    int board[NUM_LINHAS][NUM_COLUNAS];
    int etapa;
    int score;
} GameState;

// Inicializa estado: etapa=1, score=0, e preenche board aleatoriamente
void game_state_init(GameState *gs);
// Retorna número de cores para a etapa atual
int game_state_num_cores(const GameState *gs);
// Rotaciona linha: direction +1 para direita, -1 para esquerda
void game_state_rotate_row(GameState *gs, int row, int direction);
// Rotaciona coluna: direction +1 para direita, -1 para esquerda. Retorna true se executado com sucesso
bool game_state_rotate_column(GameState *gs, int col, int direction);
// Aplica gravidade: peças caem para preencher espaços vazios abaixo
void game_state_apply_gravity(GameState *gs);
// Remove colunas uniformes: se todas células ocupadas e mesma cor, limpa e retorna número de peças removidas
int game_state_remove_uniform_columns(GameState *gs);
// Verifica se primeira linha está toda vazia
bool game_state_first_row_empty(const GameState *gs);
// Insere peça aleatória na primeira linha (em coluna escolhida aleatoriamente)
void game_state_spawn_first_row(GameState *gs);
// Limpa linha: remove todas peças da linha, retorna número de peças removidas e aplica penalidade de 2 pontos
int game_state_clear_row(GameState *gs, int row);

#endif // GAME_STATE_H