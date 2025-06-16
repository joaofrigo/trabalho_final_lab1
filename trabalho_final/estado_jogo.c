#include "estado_jogo.h"

int obter_numero_cores(const EstadoJogo *estado) {
    int total = CORES_INICIAIS + ((*estado).etapa - 1);
    int maximo = CORES_INICIAIS + (NUM_ETAPAS_MAX - 1);
    if (total > maximo) {
        total = maximo;
    }
    return total;
}

void inicializar_estado(EstadoJogo *estado) {
    (*estado).etapa = 1;
    (*estado).pontuacao = 0;
    srand((unsigned) time(NULL));
    int numeroCores = obter_numero_cores(estado);
    for (int linha = 0; linha < NUM_LINHAS; linha++) {
        for (int coluna = 0; coluna < NUM_COLUNAS; coluna++) {
            (*estado).tabuleiro[linha][coluna] = rand() % numeroCores;
        }
    }
}

void rotacionar_linha(EstadoJogo *estado, int indiceLinha, int direcao) {
    if (indiceLinha < 0 || indiceLinha >= NUM_LINHAS) {
        return;
    }
    int temporario[NUM_COLUNAS];
    for (int coluna = 0; coluna < NUM_COLUNAS; coluna++) {
        int origem = coluna - direcao;
        while (origem < 0) {
            origem += NUM_COLUNAS;
        }
        origem = origem % NUM_COLUNAS;
        temporario[coluna] = (*estado).tabuleiro[indiceLinha][origem];
    }
    for (int coluna = 0; coluna < NUM_COLUNAS; coluna++) {
        (*estado).tabuleiro[indiceLinha][coluna] = temporario[coluna];
    }
}

bool rotacionar_coluna(EstadoJogo *estado, int indiceColuna, int direcao) {
    if (indiceColuna < 0 || indiceColuna >= NUM_COLUNAS) {
        return false;
    }
    int destino = indiceColuna + direcao;
    while (destino < 0) {
        destino += NUM_COLUNAS;
    }
    destino = destino % NUM_COLUNAS;
    bool existeOcupado = false;
    bool existeVazio = false;
    for (int linha = 0; linha < NUM_LINHAS; linha++) {
        if ((*estado).tabuleiro[linha][indiceColuna] != CELULA_VAZIA) {
            existeOcupado = true;
        }
        if ((*estado).tabuleiro[linha][destino] == CELULA_VAZIA) {
            existeVazio = true;
        }
    }
    if (!existeOcupado || !existeVazio) {
        return false;
    }
    int valor = (*estado).tabuleiro[NUM_LINHAS - 1][indiceColuna];
    if (valor == CELULA_VAZIA) {
        return false;
    }
    (*estado).tabuleiro[NUM_LINHAS - 1][indiceColuna] = CELULA_VAZIA;
    (*estado).tabuleiro[0][destino] = valor;
    return true;
}

void aplicar_gravidade(EstadoJogo *estado) {
    for (int coluna = 0; coluna < NUM_COLUNAS; coluna++) {
        int posicaoEscrita = NUM_LINHAS - 1;
        for (int linha = NUM_LINHAS - 1; linha >= 0; linha--) {
            if ((*estado).tabuleiro[linha][coluna] != CELULA_VAZIA) {
                if (posicaoEscrita != linha) {
                    (*estado).tabuleiro[posicaoEscrita][coluna] = (*estado).tabuleiro[linha][coluna];
                    (*estado).tabuleiro[linha][coluna] = CELULA_VAZIA;
                }
                posicaoEscrita--;
            }
        }
        for (int linha = posicaoEscrita; linha >= 0; linha--) {
            (*estado).tabuleiro[linha][coluna] = CELULA_VAZIA;
        }
    }
}

int remover_colunas_uniformes(EstadoJogo *estado) {
    int totalRemovido = 0;
    for (int coluna = 0; coluna < NUM_COLUNAS; coluna++) {
        int primeiroValor = (*estado).tabuleiro[0][coluna];
        if (primeiroValor == CELULA_VAZIA) {
            continue;
        }
        bool uniforme = true;
        for (int linha = 1; linha < NUM_LINHAS; linha++) {
            if ((*estado).tabuleiro[linha][coluna] != primeiroValor) {
                uniforme = false;
                break;
            }
        }
        if (uniforme) {
            for (int linha = 0; linha < NUM_LINHAS; linha++) {
                (*estado).tabuleiro[linha][coluna] = CELULA_VAZIA;
                totalRemovido++;
            }
        }
    }
    if (totalRemovido > 0) {
        (*estado).pontuacao += totalRemovido * (*estado).etapa;
    }
    return totalRemovido;
}

bool primeira_linha_vazia(const EstadoJogo *estado) {
    for (int coluna = 0; coluna < NUM_COLUNAS; coluna++) {
        if ((*estado).tabuleiro[0][coluna] != CELULA_VAZIA) {
            return false;
        }
    }
    return true;
}

void gerar_peca_primeira_linha(EstadoJogo *estado) {
    if (!primeira_linha_vazia(estado)) {
        return;
    }
    int colunaAleatoria = rand() % NUM_COLUNAS;
    int numeroCores = obter_numero_cores(estado);
    (*estado).tabuleiro[0][colunaAleatoria] = rand() % numeroCores;
}

int limpar_linha(EstadoJogo *estado, int indiceLinha) {
    if (indiceLinha < 0 || indiceLinha >= NUM_LINHAS) {
        return 0;
    }
    int totalRemovido = 0;
    for (int coluna = 0; coluna < NUM_COLUNAS; coluna++) {
        if ((*estado).tabuleiro[indiceLinha][coluna] != CELULA_VAZIA) {
            (*estado).tabuleiro[indiceLinha][coluna] = CELULA_VAZIA;
            totalRemovido++;
        }
    }
    if (totalRemovido > 0) {
        (*estado).pontuacao -= 2;
    }
    return totalRemovido;
}