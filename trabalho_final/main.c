// main.c
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "janela.h"
#include "config.h"
#include "game_state.h"
#include "history.h"

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

// Exibe instruções antes de começar e aguarda Enter ou 'C' para ver recordes
void show_instructions(History *history) {
    j_seleciona_fonte(NULL, 18);
    bool waiting = true;
    while (waiting) {
        j_texto((ponto_t){20, 40}, (cor_t){1,1,1,1}, "Controles:");
        j_texto((ponto_t){20, 80}, (cor_t){1,1,1,1}, "M: alterna modo Linha/Coluna");
        j_texto((ponto_t){20, 110}, (cor_t){1,1,1,1}, "Setas: mover selecionado / girar");
        j_texto((ponto_t){20, 140}, (cor_t){1,1,1,1}, "X: limpa linha (modo linha) - penalidade 2 pontos");
        j_texto((ponto_t){20, 170}, (cor_t){1,1,1,1}, "ESC: sair");
        j_texto((ponto_t){20, 200}, (cor_t){1,1,1,1}, "C: ver recordes");
        j_texto((ponto_t){20, 240}, (cor_t){1,1,1,1}, "Pressione Enter para iniciar...");
        j_mostra();
        tecla_t t = j_tecla();
        if (t == T_ENTER) waiting = false;
        else if (t == 'c' || t == 'C') {
            history_show(history);
        } else if (t == T_ESC) {
            waiting = false;
        }
    }
    j_seleciona_fonte(NULL, 15);
}

// Captura nome do jogador via teclado
void prompt_name(char *out_name) {
    int pos = 0;
    out_name[0] = '\0';
    j_seleciona_fonte(NULL, 18);
    bool done = false;
    while (!done) {
        char prompt[64];
        snprintf(prompt, sizeof(prompt), "Digite seu nome (max %d chars): %s", MAX_NAME_LEN-1, out_name);
        // desenha prompt
        j_texto((ponto_t){20, 40}, (cor_t){1,1,1,1}, prompt);
        j_mostra();
        tecla_t t = j_tecla();
        if (t != T_NADA) {
            if (t == T_ENTER) {
                if (pos > 0) done = true;
            } else if (t == T_ESC) {
                done = true;
            } else if (t == T_BS || t == T_BACKSPACE) {
                if (pos > 0) {
                    pos--; out_name[pos] = '\0';
                }
            } else if ((unsigned char)t >= 32 && (unsigned char)t < 127) {
                if (pos < MAX_NAME_LEN-1) {
                    out_name[pos++] = (char)t;
                    out_name[pos] = '\0';
                }
            }
        }
    }
    j_seleciona_fonte(NULL, 15);
}

int main(void) {
    int win_w = NUM_COLUNAS * CELL_SIZE + 200;
    int win_h = NUM_LINHAS * CELL_SIZE + 200;
    tamanho_t tamanho = { .largura = (float)win_w, .altura = (float)win_h };
    j_inicializa(tamanho, "Jogo de Tabuleiro");

    History history;
    history_init(&history);

    show_instructions(&history);

    bool exit_program = false;
    while (!exit_program) {
        GameState gs;
        game_state_init(&gs);
        int selected_row = 0;
        int selected_col = 0;
        bool mode_row = true;
        double stage_start = j_relogio();
        int removed_in_stage = 0;
        bool defeated = false;
        int last_stage = 1;
        while (true) {
            double now = j_relogio();
            double elapsed = now - stage_start;
            double remaining = STAGE_DURATION_SEC - elapsed;
            if (remaining <= 0) {
                int half = (NUM_LINHAS * NUM_COLUNAS) / 2;
                bool passed = (removed_in_stage >= half);
                // Confirmação
                j_seleciona_fonte(NULL, 18);
                bool wait = true;
                while (wait) {
                    if (passed) {
                        char msg1[128];
                        snprintf(msg1, sizeof(msg1), "Etapa %d concluida! Removidas %d (min %d)", gs.etapa, removed_in_stage, half);
                        j_texto((ponto_t){20, 40}, (cor_t){1,1,1,1}, msg1);
                        int bonus = BONUS_BASE * gs.etapa;
                        char msg2[128];
                        snprintf(msg2, sizeof(msg2), "Bonus: %d. Score atual: %d. Enter para proxima etapa.", bonus, gs.score + bonus);
                        j_texto((ponto_t){20, 80}, (cor_t){1,1,1,1}, msg2);
                    } else {
                        char msg1[128];
                        snprintf(msg1, sizeof(msg1), "Etapa %d falhada. Removidas %d (min %d). Fim de jogo.", gs.etapa, removed_in_stage, half);
                        j_texto((ponto_t){20, 40}, (cor_t){1,1,1,1}, msg1);
                        char msg2[128];
                        snprintf(msg2, sizeof(msg2), "Score final: %d. Enter para continuar.", gs.score);
                        j_texto((ponto_t){20, 80}, (cor_t){1,1,1,1}, msg2);
                        defeated = true;
                    }
                    j_mostra();
                    tecla_t tk = j_tecla();
                    if (tk == T_ENTER) wait = false;
                }
                j_seleciona_fonte(NULL, 15);
                last_stage = gs.etapa;
                if (passed) {
                    int bonus = BONUS_BASE * gs.etapa;
                    gs.score += bonus;
                    gs.etapa++;
                    if (gs.etapa > NUM_ETAPAS_MAX) gs.etapa = NUM_ETAPAS_MAX;
                    removed_in_stage = 0;
                    stage_start = j_relogio();
                    continue;
                } else {
                    break;
                }
            }
            tecla_t t = j_tecla();
            while (t != T_NADA) {
                if (t == T_ESC) { exit_program = true; break; }
                else if (t == 'm' || t == 'M') { mode_row = !mode_row; }
                else if (t == 'c' || t == 'C') { history_show(&history); }
                else if (t == T_CIMA) {
                    if (mode_row) selected_row = (selected_row - 1 + NUM_LINHAS) % NUM_LINHAS;
                    else selected_col = (selected_col - 1 + NUM_COLUNAS) % NUM_COLUNAS;
                }
                else if (t == T_BAIXO) {
                    if (mode_row) selected_row = (selected_row + 1) % NUM_LINHAS;
                    else selected_col = (selected_col + 1) % NUM_COLUNAS;
                }
                else if (t == T_ESQUERDA || t == T_DIREITA) {
                    int dir = (t == T_DIREITA) ? +1 : -1;
                    if (mode_row) {
                        game_state_rotate_row(&gs, selected_row, dir);
                        game_state_apply_gravity(&gs);
                        int rem;
                        do { rem = game_state_remove_uniform_columns(&gs); if (rem>0) removed_in_stage+=rem; game_state_apply_gravity(&gs);} while(rem>0);
                        if (game_state_first_row_empty(&gs)) game_state_spawn_first_row(&gs);
                        game_state_apply_gravity(&gs);
                        do { rem = game_state_remove_uniform_columns(&gs); if(rem>0) removed_in_stage+=rem; game_state_apply_gravity(&gs);} while(rem>0);
                    } else {
                        if (game_state_rotate_column(&gs, selected_col, dir)) {
                            game_state_apply_gravity(&gs);
                            int rem;
                            do { rem = game_state_remove_uniform_columns(&gs); if(rem>0) removed_in_stage+=rem; game_state_apply_gravity(&gs);} while(rem>0);
                            if (game_state_first_row_empty(&gs)) game_state_spawn_first_row(&gs);
                            game_state_apply_gravity(&gs);
                            do { rem = game_state_remove_uniform_columns(&gs); if(rem>0) removed_in_stage+=rem; game_state_apply_gravity(&gs);} while(rem>0);
                        }
                    }
                }
                else if ((t=='x'||t=='X') && mode_row) {
                    int rem = game_state_clear_row(&gs, selected_row);
                    if (rem>0) {
                        removed_in_stage += rem;
                        game_state_apply_gravity(&gs);
                        int rem2;
                        do { rem2=game_state_remove_uniform_columns(&gs); if(rem2>0) removed_in_stage+=rem2; game_state_apply_gravity(&gs);} while(rem2>0);
                        if (game_state_first_row_empty(&gs)) game_state_spawn_first_row(&gs);
                        game_state_apply_gravity(&gs);
                        do { rem2=game_state_remove_uniform_columns(&gs); if(rem2>0) removed_in_stage+=rem2; game_state_apply_gravity(&gs);} while(rem2>0);
                    }
                }
                t = j_tecla();
            }
            if (exit_program) break;
            // Render
            for (int r = 0; r < NUM_LINHAS; r++) {
                for (int c = 0; c < NUM_COLUNAS; c++) {
                    int idx = gs.board[r][c];
                    cor_t cor = (idx==EMPTY_CELL? (cor_t){0.2f,0.2f,0.2f,1.0f}: COLORS[idx%COLOR_COUNT]);
                    ponto_t inicio={.x=100 + c*CELL_SIZE,.y=50 + r*CELL_SIZE};
                    tamanho_t tam={.largura=CELL_SIZE-2,.altura=CELL_SIZE-2};
                    retangulo_t rect={.inicio={inicio.x+1,inicio.y+1},.tamanho=tam};
                    j_retangulo(rect,2.0f,(cor_t){0,0,0,1},cor);
                    if (mode_row && r==selected_row) j_linha((ponto_t){100,50+selected_row*CELL_SIZE+CELL_SIZE/2}, (ponto_t){100+NUM_COLUNAS*CELL_SIZE,50+selected_row*CELL_SIZE+CELL_SIZE/2},2.0f,(cor_t){1,1,1,1});
                    if (!mode_row && c==selected_col) j_linha((ponto_t){100+selected_col*CELL_SIZE+CELL_SIZE/2,50}, (ponto_t){100+selected_col*CELL_SIZE+CELL_SIZE/2,50+NUM_LINHAS*CELL_SIZE},2.0f,(cor_t){1,1,1,1});
                }
            }
            char buf[64];
            snprintf(buf,sizeof(buf),"Score: %d  Stage: %d  Time: %.0f",gs.score,gs.etapa,remaining>0?remaining:0);
            j_texto((ponto_t){20,50+NUM_LINHAS*CELL_SIZE+20},(cor_t){1,1,1,1},buf);
            j_mostra();
            if (exit_program) break;
        }
        if (exit_program) break;
        // Após fim, prompt nome
        char name[MAX_NAME_LEN];
        prompt_name(name);
        if (strlen(name) > 0) history_add(&history, name, gs.score, last_stage);
        // Volta à tela de instruções
        show_instructions(&history);
    }
    history_free(&history);
    j_finaliza();
    return 0;
}
