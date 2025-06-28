#ifndef HISTORICO_H
#define HISTORICO_H

#include <stddef.h>
#include "config.h"

typedef struct {
    char nome[TAM_MAX_NOME];
    int pontuacao;
    int etapa;
} Registro;

typedef struct {
    Registro *dados;
    size_t tamanho;
    size_t capacidade;
} Historico;

void inicializar_historico(Historico *h);
void liberar_historico(Historico *h);
void adicionar_registro(Historico *h, const char *nome, int pontuacao, int etapa);
void mostrar_historico(const Historico *h);

#endif