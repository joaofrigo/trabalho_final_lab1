#include "historico.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "janela.h"

static int comparar_registros(const void *a, const void *b) {
    const Registro *ra = a;
    const Registro *rb = b;
    return (*rb).pontuacao - (*ra).pontuacao;
}

void inicializar_historico(Historico *h) {
    (*h).dados = NULL;
    (*h).tamanho = 0;
    (*h).capacidade = 0;
    FILE *arquivo = fopen(ARQUIVO_RECORDES, "r");
    if (arquivo == NULL) {
        return;
    }
    char linha[128];
    while (fgets(linha, sizeof(linha), arquivo)) {
        char nome[TAM_MAX_NOME];
        int pontuacao;
        int etapa;
        if (sscanf(linha, "%31[^;];%d;%d", nome, &pontuacao, &etapa) == 3) {
            if ((*h).tamanho == (*h).capacidade) {
                size_t novaCap = ((*h).capacidade == 0) ? 4 : (*h).capacidade * 2;
                Registro *temporario = realloc((*h).dados, novaCap * sizeof(Registro));
                if (temporario == NULL) {
                    break;
                }
                (*h).dados = temporario;
                (*h).capacidade = novaCap;
            }
            strncpy((*h).dados[(*h).tamanho].nome, nome, TAM_MAX_NOME);
            (*h).dados[(*h).tamanho].pontuacao = pontuacao;
            (*h).dados[(*h).tamanho].etapa = etapa;
            (*h).tamanho++;
        }
    }
    fclose(arquivo);
    if ((*h).tamanho > 1) {
        qsort((*h).dados, (*h).tamanho, sizeof(Registro), comparar_registros);
    }
}

void liberar_historico(Historico *h) {
    free((*h).dados);
    (*h).dados = NULL;
    (*h).tamanho = 0;
    (*h).capacidade = 0;
}

void adicionar_registro(Historico *h, const char *nome, int pontuacao, int etapa) {
    if ((*h).tamanho == (*h).capacidade) {
        size_t novaCap = ((*h).capacidade == 0) ? 4 : (*h).capacidade * 2;
        Registro *temporario = realloc((*h).dados, novaCap * sizeof(Registro));
        if (temporario == NULL) {
            return;
        }
        (*h).dados = temporario;
        (*h).capacidade = novaCap;
    }
    Registro reg;
    strncpy(reg.nome, nome, TAM_MAX_NOME);
    reg.nome[TAM_MAX_NOME-1] = '\0';
    reg.pontuacao = pontuacao;
    reg.etapa = etapa;
    (*h).dados[(*h).tamanho] = reg;
    (*h).tamanho++;
    qsort((*h).dados, (*h).tamanho, sizeof(Registro), comparar_registros);
    FILE *arquivo = fopen(ARQUIVO_RECORDES, "w");
    if (arquivo == NULL) {
        return;
    }
    for (size_t i = 0; i < (*h).tamanho; i++) {
        fprintf(arquivo, "%s;%d;%d\n", (*h).dados[i].nome, (*h).dados[i].pontuacao, (*h).dados[i].etapa);
    }
    fclose(arquivo);
}

void mostrar_historico(const Historico *h) {
    j_seleciona_fonte(NULL, 18);
    bool aguardar = true;
    while (aguardar) {
        j_texto((ponto_t){20, 20}, (cor_t){1,1,1,1}, "-- RECORDES --");
        int limite = 10;
        if ((int)(*h).tamanho < limite) {
            limite = (*h).tamanho;
        }
        for (int i = 0; i < limite; i++) {
            char buffer[128];
            snprintf(buffer, sizeof(buffer), "%d. %s - Pontuacao: %d - Etapa: %d", i+1, (*h).dados[i].nome, (*h).dados[i].pontuacao, (*h).dados[i].etapa);
            j_texto((ponto_t){20, 50 + i * 30}, (cor_t){1,1,1,1}, buffer);
        }
        j_texto((ponto_t){20, 50 + 10*30}, (cor_t){1,1,1,1}, "Pressione Enter para voltar");
        j_mostra();
        tecla_t t = j_tecla();
        if (t == T_ENTER) {
            aguardar = false;
        }
    }
}
