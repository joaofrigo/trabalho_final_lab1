// main.c
#include <stdio.h>
#include <stdbool.h>
#include "janela.h"
#include "config.h"
#include "game_state.h"

// mapeamento de cores: pelo menos 8
static const cor_t COLORS[] = {
    {1.0f, 0.0f, 0.0f, 1.0f}, // vermelho
    {0.0f, 1.0f, 0.0f, 1.0f}, // verde
    {0.0f, 0.0f, 1.0f, 1.0f}, // azul
    {1.0f, 1.0f, 0.0f, 1.0f}, // amarelo
    {1.0f, 0.0f, 1.0f, 1.0f}, // magenta
    {0.0f, 1.0f, 1.0f, 1.0f}, // ciano
    {1.0f, 0.5f, 0.0f, 1.0f}, // laranja
    {0.5f, 0.0f, 0.5f, 1.0f}  // roxo
};
static const int COLOR_COUNT = sizeof(COLORS) / sizeof(COLORS[0]);

int main(void) {
    GameState gs;
    game_state_init(&gs);

    int largura = NUM_COLUNAS * CELL_SIZE;
    int altura = NUM_LINHAS * CELL_SIZE;
    tamanho_t tamanho = { .largura = (float)largura, .altura = (float)altura };
    j_inicializa(tamanho, "Jogo de Tabuleiro");

    bool sair = false;
    while (!sair) {
        tecla_t t = j_tecla();
        if (t == T_ESC) {
            sair = true;
        }
        // desenha tabuleiro
        for (int r = 0; r < NUM_LINHAS; r++) {
            for (int c = 0; c < NUM_COLUNAS; c++) {
                int color_idx = gs.board[r][c] % COLOR_COUNT;
                cor_t cor = COLORS[color_idx];
                ponto_t inicio = { .x = c * CELL_SIZE, .y = r * CELL_SIZE };
                tamanho_t tam = { .largura = CELL_SIZE - 2, .altura = CELL_SIZE - 2 };
                retangulo_t rect = { .inicio = { inicio.x + 1, inicio.y + 1 }, .tamanho = tam };
                cor_t borda = {0, 0, 0, 1};
                j_retangulo(rect, 2.0f, borda, cor);
            }
        }
        j_mostra();
    }

    j_finaliza();
    return 0;
}
