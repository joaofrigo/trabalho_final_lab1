#ifndef ESTADO_JOGO_H
#define ESTADO_JOGO_H

#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "config.h"

typedef struct {
    int tabuleiro[NUM_LINHAS][NUM_COLUNAS];
    int etapa;
    int pontuacao;
} EstadoJogo;

void inicializar_estado(EstadoJogo *estado);
int obter_numero_cores(const EstadoJogo *estado);
void rotacionar_linha(EstadoJogo *estado, int indiceLinha, int direcao);
bool rotacionar_coluna(EstadoJogo *estado, int indiceColuna, int direcao);
void aplicar_gravidade(EstadoJogo *estado);
int remover_colunas_uniformes(EstadoJogo *estado);
bool primeira_linha_vazia(const EstadoJogo *estado);
void gerar_peca_primeira_linha(EstadoJogo *estado);
int limpar_linha(EstadoJogo *estado, int indiceLinha);

#endif
