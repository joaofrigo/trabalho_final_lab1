// history.c
#include "history.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "janela.h"

static int compare_records(const void *a, const void *b) {
    const Record *ra = a;
    const Record *rb = b;
    return rb->score - ra->score;
}

void history_init(History *h) {
    h->data = NULL;
    h->size = 0;
    h->capacity = 0;
    FILE *f = fopen(RECORDS_FILE, "r");
    if (!f) return;
    char line[128];
    while (fgets(line, sizeof(line), f)) {
        char name[MAX_NAME_LEN]; int score, etapa;
        if (sscanf(line, "%31[^;];%d;%d", name, &score, &etapa) == 3) {
            if (h->size == h->capacity) {
                size_t newcap = h->capacity == 0 ? 4 : h->capacity * 2;
                Record *tmp = realloc(h->data, newcap * sizeof(Record));
                if (!tmp) break;
                h->data = tmp;
                h->capacity = newcap;
            }
            strncpy(h->data[h->size].name, name, MAX_NAME_LEN);
            h->data[h->size].score = score;
            h->data[h->size].etapa = etapa;
            h->size++;
        }
    }
    fclose(f);
    if (h->size > 1)
        qsort(h->data, h->size, sizeof(Record), compare_records);
}

void history_free(History *h) {
    free(h->data);
    h->data = NULL;
    h->size = h->capacity = 0;
}

void history_add(History *h, const char *name, int score, int etapa) {
    // adicionar
    if (h->size == h->capacity) {
        size_t newcap = h->capacity == 0 ? 4 : h->capacity * 2;
        Record *tmp = realloc(h->data, newcap * sizeof(Record));
        if (!tmp) return;
        h->data = tmp;
        h->capacity = newcap;
    }
    Record rec;
    strncpy(rec.name, name, MAX_NAME_LEN);
    rec.name[MAX_NAME_LEN-1] = '\0';
    rec.score = score;
    rec.etapa = etapa;
    h->data[h->size++] = rec;
    qsort(h->data, h->size, sizeof(Record), compare_records);
    // salvar
    FILE *f = fopen(RECORDS_FILE, "w");
    if (!f) return;
    for (size_t i = 0; i < h->size; i++) {
        fprintf(f, "%s;%d;%d\n", h->data[i].name, h->data[i].score, h->data[i].etapa);
    }
    fclose(f);
}

void history_show(const History *h) {
    j_seleciona_fonte(NULL, 18);
    bool wait = true;
    size_t idx = 0;
    while (wait) {
        // desenha lista
        ponto_t pos = { .x = 20, .y = 20 };
        j_texto(pos, (cor_t){1,1,1,1}, "-- RECORDES --");
        for (size_t i = 0; i < h->size && i < 10; i++) {
            char buf[128];
            snprintf(buf, sizeof(buf), "%zu. %s - Score: %d - Stage: %d", i+1, h->data[i].name, h->data[i].score, h->data[i].etapa);
            ponto_t p = { .x = 20, .y = 50 + (int)i * 30 };
            j_texto(p, (cor_t){1,1,1,1}, buf);
        }
        j_texto((ponto_t){20, 50 + 10*30}, (cor_t){1,1,1,1}, "Pressione Enter para voltar");
        j_mostra();
        tecla_t t = j_tecla();
        if (t == T_ENTER) wait = false;
    }
    j_seleciona_fonte(NULL, 15);
}
