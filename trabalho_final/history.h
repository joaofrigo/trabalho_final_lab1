// history.h
#ifndef HISTORY_H
#define HISTORY_H

#include <stddef.h>
#include "config.h"

typedef struct {
    char name[MAX_NAME_LEN];
    int score;
    int etapa;
} Record;

typedef struct {
    Record *data;
    size_t size;
    size_t capacity;
} History;

// Inicializa histórico, carrega de arquivo
void history_init(History *h);
// Libera memória
void history_free(History *h);
// Adiciona recorde e salva em arquivo
void history_add(History *h, const char *name, int score, int etapa);
// Exibe histórico na tela, aguarda Enter para voltar
void history_show(const History *h);

#endif // HISTORY_H