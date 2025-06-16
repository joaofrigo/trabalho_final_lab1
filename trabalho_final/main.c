// main.c
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "janela.h"
#include "config.h"
#include "game_state.h"

static const cor_t COLORS[] = {
    {1.0f, 0.0f, 0.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 1.0f, 1.0f},
    {1.0f, 0.5f, 0.0f, 1.0f},
    {0.5f, 0.0f, 0.5f, 1.0f}
};
static const int COLOR_COUNT = sizeof(COLORS) / sizeof(COLORS[0]);

// Exibe instruções antes de começar e aguarda Enter
void show_instructions() {
    j_seleciona_fonte(NULL, 18);
    bool waiting = true;
    while (waiting) {
        // Render de fundo
        // Limpa é feito em j_mostra
        ponto_t pos = { .x = 20, .y = 40 };
        j_texto(pos, (cor_t){1,1,1,1}, "Controles:");
        ponto_t pos2 = { .x = 20, .y = 70 };
        j_texto(pos2, (cor_t){1,1,1,1}, "M: alterna modo Linha/Coluna");
        ponto_t pos3 = { .x = 20, .y = 100 };
        j_texto(pos3, (cor_t){1,1,1,1}, "Setas: escolher linha/coluna ou girar");
        ponto_t pos4 = { .x = 20, .y = 130 };
        j_texto(pos4, (cor_t){1,1,1,1}, "X: limpa linha (modo linha) - penalidade 2 pontos");
        ponto_t pos5 = { .x = 20, .y = 160 };
        j_texto(pos5, (cor_t){1,1,1,1}, "ESC: sair");
        ponto_t pos6 = { .x = 20, .y = 200 };
        j_texto(pos6, (cor_t){1,1,1,1}, "Pressione Enter para iniciar...");
        j_mostra();
        tecla_t t = j_tecla();
        if (t == T_ENTER) waiting = false;
        if (t == T_ESC) {
            waiting = false;
        }
    }
    j_seleciona_fonte(NULL, 15);
}

int main(void) {
    tamanho_t tamanho = { .largura = NUM_COLUNAS * CELL_SIZE, .altura = NUM_LINHAS * CELL_SIZE + 60 };
    j_inicializa(tamanho, "Jogo de Tabuleiro");

    show_instructions();

    bool exit_program = false;
    while (!exit_program) {
        GameState gs;
        game_state_init(&gs);
        int selected_row = 0;
        int selected_col = 0;
        bool mode_row = true; // true: selecionar linha; false: selecionar coluna
        double stage_start = j_relogio();
        int removed_in_stage = 0;
        while (true) {
            double now = j_relogio();
            double elapsed = now - stage_start;
            double remaining = STAGE_DURATION_SEC - elapsed;
            if (remaining <= 0) {
                int half = (NUM_LINHAS * NUM_COLUNAS) / 2;
                if (removed_in_stage >= half) {
                    int bonus = BONUS_BASE * gs.etapa;
                    gs.score += bonus;
                    gs.etapa++;
                    if (gs.etapa > NUM_ETAPAS_MAX) gs.etapa = NUM_ETAPAS_MAX;
                    removed_in_stage = 0;
                    stage_start = j_relogio();
                } else {
                    break;
                }
            }
            tecla_t t = j_tecla();
            while (t != T_NADA) {
                if (t == T_ESC) {
                    exit_program = true;
                    break;
                } else if (t == 'm' || t == 'M') {
                    mode_row = !mode_row;
                } else if (t == T_CIMA) {
                    if (mode_row) {
                        selected_row = (selected_row - 1 + NUM_LINHAS) % NUM_LINHAS;
                    } else {
                        selected_col = (selected_col - 1 + NUM_COLUNAS) % NUM_COLUNAS;
                    }
                } else if (t == T_BAIXO) {
                    if (mode_row) {
                        selected_row = (selected_row + 1) % NUM_LINHAS;
                    } else {
                        selected_col = (selected_col + 1) % NUM_COLUNAS;
                    }
                } else if (t == T_ESQUERDA || t == T_DIREITA) {
                    int dir = (t == T_DIREITA) ? +1 : -1;
                    if (mode_row) {
                        game_state_rotate_row(&gs, selected_row, dir);
                        // Após rotação de linha
                        // Aplicar gravidade e remoções até estabilizar
                        game_state_apply_gravity(&gs);
                        int rem;
                        do {
                            rem = game_state_remove_uniform_columns(&gs);
                            if (rem > 0) removed_in_stage += rem;
                            game_state_apply_gravity(&gs);
                        } while (rem > 0);
                        // Spawn apenas uma vez se primeira linha vazia
                        if (game_state_first_row_empty(&gs)) {
                            game_state_spawn_first_row(&gs);
                        }
                        game_state_apply_gravity(&gs);
                        do {
                            rem = game_state_remove_uniform_columns(&gs);
                            if (rem > 0) removed_in_stage += rem;
                            game_state_apply_gravity(&gs);
                        } while (rem > 0);
                    } else {
                        // Rotação de coluna: só funciona se coluna destino tem espaço
                        if (game_state_rotate_column(&gs, selected_col, dir)) {
                            game_state_apply_gravity(&gs);
                            int rem;
                            do {
                                rem = game_state_remove_uniform_columns(&gs);
                                if (rem > 0) removed_in_stage += rem;
                                game_state_apply_gravity(&gs);
                            } while (rem > 0);
                            if (game_state_first_row_empty(&gs)) {
                                game_state_spawn_first_row(&gs);
                            }
                            game_state_apply_gravity(&gs);
                            do {
                                rem = game_state_remove_uniform_columns(&gs);
                                if (rem > 0) removed_in_stage += rem;
                                game_state_apply_gravity(&gs);
                            } while (rem > 0);
                        }
                    }
                } else if ((t == 'x' || t == 'X') && mode_row) {
                    int rem = game_state_clear_row(&gs, selected_row);
                    if (rem > 0) {
                        removed_in_stage += rem;
                        game_state_apply_gravity(&gs);
                        int rem2;
                        do {
                            rem2 = game_state_remove_uniform_columns(&gs);
                            if (rem2 > 0) removed_in_stage += rem2;
                            game_state_apply_gravity(&gs);
                        } while (rem2 > 0);
                        if (game_state_first_row_empty(&gs)) {
                            game_state_spawn_first_row(&gs);
                        }
                        game_state_apply_gravity(&gs);
                        do {
                            rem2 = game_state_remove_uniform_columns(&gs);
                            if (rem2 > 0) removed_in_stage += rem2;
                            game_state_apply_gravity(&gs);
                        } while (rem2 > 0);
                    }
                }
                t = j_tecla();
            }
            if (exit_program) break;
            // Render
            for (int r = 0; r < NUM_LINHAS; r++) {
                for (int c = 0; c < NUM_COLUNAS; c++) {
                    int idx = gs.board[r][c];
                    cor_t cor = (idx == EMPTY_CELL ? (cor_t){0.2f,0.2f,0.2f,1.0f} : COLORS[idx % COLOR_COUNT]);
                    ponto_t inicio = { .x = c * CELL_SIZE, .y = r * CELL_SIZE };
                    tamanho_t tam = { .largura = CELL_SIZE - 2, .altura = CELL_SIZE - 2 };
                    retangulo_t rect = { .inicio = { inicio.x + 1, inicio.y + 1 }, .tamanho = tam };
                    cor_t borda = {0, 0, 0, 1};
                    j_retangulo(rect, 2.0f, borda, cor);
                    // Highlight: linha inteira ou coluna inteira
                    if (mode_row && r == selected_row) {
                        ponto_t p1 = { .x = 0, .y = selected_row * CELL_SIZE + CELL_SIZE/2 };
                        ponto_t p2 = { .x = NUM_COLUNAS * CELL_SIZE, .y = selected_row * CELL_SIZE + CELL_SIZE/2 };
                        j_linha(p1, p2, 2.0f, (cor_t){1,1,1,1});
                    }
                    if (!mode_row && c == selected_col) {
                        ponto_t p1 = { .x = selected_col * CELL_SIZE + CELL_SIZE/2, .y = 0 };
                        ponto_t p2 = { .x = selected_col * CELL_SIZE + CELL_SIZE/2, .y = NUM_LINHAS * CELL_SIZE };
                        j_linha(p1, p2, 2.0f, (cor_t){1,1,1,1});
                    }
                }
            }
            char buf[64];
            snprintf(buf, sizeof(buf), "Score: %d  Stage: %d  Time: %.0f", gs.score, gs.etapa, remaining > 0 ? remaining : 0);
            ponto_t pos = { .x = 5, .y = NUM_LINHAS * CELL_SIZE + 20 };
            j_texto(pos, (cor_t){1,1,1,1}, buf);
            j_mostra();
            if (exit_program) break;
        }
        if (exit_program) break;
    }
    j_finaliza();
    return 0;
}
