#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "janela.h"
#include "config.h"
#include "estado_jogo.h"
#include "historico.h"

static const cor_t CORES[] = {
    {1.0f, 0.0f, 0.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 1.0f, 1.0f},
    {1.0f, 0.5f, 0.0f, 1.0f},
    {0.5f, 0.0f, 0.5f, 1.0f}
};
static const int QTD_CORES = sizeof(CORES) / sizeof(CORES[0]);

void mostrar_instrucoes(Historico *historico) {
    j_seleciona_fonte(NULL, 18);
    bool esperando = true;
    while (esperando) {
        j_texto((ponto_t){20, 40}, (cor_t){1,1,1,1}, "Controles:");
        j_texto((ponto_t){20, 80}, (cor_t){1,1,1,1}, "M: alterna modo Linha/Coluna");
        j_texto((ponto_t){20, 110}, (cor_t){1,1,1,1}, "Setas: mover selecionado / girar");
        j_texto((ponto_t){20, 140}, (cor_t){1,1,1,1}, "X: limpar linha (modo linha) - penalidade 2 pontos");
        j_texto((ponto_t){20, 170}, (cor_t){1,1,1,1}, "ESC: encerrar partida e salvar score");
        j_texto((ponto_t){20, 200}, (cor_t){1,1,1,1}, "C: ver recordes");
        j_texto((ponto_t){20, 240}, (cor_t){1,1,1,1}, "Pressione Enter para iniciar...");
        j_mostra();
        tecla_t tecla = j_tecla();
        if (tecla == T_ENTER) {
            esperando = false;
        } else if (tecla == 'c' || tecla == 'C') {
            mostrar_historico(historico);
        } else if (tecla == T_ESC) {
            esperando = false;
        }
    }
}

void solicitar_nome(char *nomeSaida) {
    int pos = 0;
    nomeSaida[0] = '\0';
    j_seleciona_fonte(NULL, 18);
    bool concluido = false;
    while (!concluido) {
        char prompt[64];
        snprintf(prompt, sizeof(prompt), "Digite seu nome (max %d): %s", TAM_MAX_NOME-1, nomeSaida);
        j_texto((ponto_t){20, 40}, (cor_t){1,1,1,1}, prompt);
        j_mostra();
        tecla_t tecla = j_tecla();
        if (tecla != T_NADA) {
            if (tecla == T_ENTER) {
                if (pos > 0) {
                    concluido = true;
                }
            } else if (tecla == T_ESC) {
                concluido = true;
            } else if (tecla == T_BS || tecla == T_BACKSPACE) {
                if (pos > 0) {
                    pos--;
                    nomeSaida[pos] = '\0';
                }
            } else {
                unsigned char code = (unsigned char) tecla;
                if (code >= 32 && code < 127) {
                    if (pos < TAM_MAX_NOME-1) {
                        nomeSaida[pos] = (char) tecla;
                        pos++;
                        nomeSaida[pos] = '\0';
                    }
                }
            }
        }
    }
}

int main(void) {
    int larguraJanela = NUM_COLUNAS * TAM_CELULA + 200;
    int alturaJanela = NUM_LINHAS * TAM_CELULA + 200;
    tamanho_t tamanho = { .largura = (float)larguraJanela, .altura = (float)alturaJanela };
    j_inicializa(tamanho, "Jogo de Tabuleiro");

    Historico historico;
    inicializar_historico(&historico);

    mostrar_instrucoes(&historico);

    bool encerrarPrograma = false;
    while (!encerrarPrograma) {
        EstadoJogo estado;
        inicializar_estado(&estado);
        int indiceLinhaSelecionada = 0;
        int indiceColunaSelecionada = 0;
        bool modoLinha = true;
        double inicioEtapa = j_relogio();
        int removidasNaEtapa = 0;
        int ultimaEtapa = 1;
        bool encerrarPorESC = false;
        while (true) {
            double agora = j_relogio();
            double decorrido = agora - inicioEtapa;
            double restante = DURACAO_ETAPA_SEG - decorrido;
            if (restante <= 0) {
                int minimo = (NUM_LINHAS * NUM_COLUNAS) / 2;
                bool passou = (removidasNaEtapa >= minimo);
                j_seleciona_fonte(NULL, 18);
                bool aguardando = true;
                while (aguardando) {
                    if (passou) {
                        char texto1[128];
                        snprintf(texto1, sizeof(texto1), "Etapa %d concluida! Removidas %d (min %d)", estado.etapa, removidasNaEtapa, minimo);
                        j_texto((ponto_t){20, 40}, (cor_t){1,1,1,1}, texto1);
                        int bonus = BONUS_BASE * estado.etapa;
                        char texto2[128];
                        snprintf(texto2, sizeof(texto2), "Bonus: %d. Pontuacao atual: %d. Enter para proxima etapa.", bonus, estado.pontuacao + bonus);
                        j_texto((ponto_t){20, 80}, (cor_t){1,1,1,1}, texto2);
                    } else {
                        char texto1[128];
                        snprintf(texto1, sizeof(texto1), "Etapa %d falhada. Removidas %d (min %d). Fim de jogo.", estado.etapa, removidasNaEtapa, minimo);
                        j_texto((ponto_t){20, 40}, (cor_t){1,1,1,1}, texto1);
                        char texto2[128];
                        snprintf(texto2, sizeof(texto2), "Pontuacao final: %d. Enter para continuar.", estado.pontuacao);
                        j_texto((ponto_t){20, 80}, (cor_t){1,1,1,1}, texto2);
                    }
                    j_mostra();
                    tecla_t tecla = j_tecla();
                    if (tecla == T_ENTER) {
                        aguardando = false;
                    }
                }
                j_seleciona_fonte(NULL, 18);
                ultimaEtapa = estado.etapa;
                if (passou) {
                    int bonus = BONUS_BASE * estado.etapa;
                    estado.pontuacao += bonus;
                    estado.etapa++;
                    if (estado.etapa > NUM_ETAPAS_MAX) {
                        estado.etapa = NUM_ETAPAS_MAX;
                    }
                    removidasNaEtapa = 0;
                    inicioEtapa = j_relogio();
                    continue;
                } else {
                    break;
                }
            }
            tecla_t tecla = j_tecla();
            while (tecla != T_NADA) {
                if (tecla == T_ESC) {
                    encerrarPorESC = true;
                    break;
                }
                if (tecla == 'm' || tecla == 'M') {
                    modoLinha = !modoLinha;
                } else if (tecla == 'c' || tecla == 'C') {
                    mostrar_historico(&historico);
                } else if (tecla == T_CIMA) {
                    if (modoLinha) {
                        indiceLinhaSelecionada = (indiceLinhaSelecionada - 1 + NUM_LINHAS) % NUM_LINHAS;
                    } else {
                        indiceColunaSelecionada = (indiceColunaSelecionada - 1 + NUM_COLUNAS) % NUM_COLUNAS;
                    }
                } else if (tecla == T_BAIXO) {
                    if (modoLinha) {
                        indiceLinhaSelecionada = (indiceLinhaSelecionada + 1) % NUM_LINHAS;
                    } else {
                        indiceColunaSelecionada = (indiceColunaSelecionada + 1) % NUM_COLUNAS;
                    }
                } else if (tecla == T_ESQUERDA || tecla == T_DIREITA) {
                    int direcao;
                    if (tecla == T_DIREITA) {
                        direcao = +1;
                    } else {
                        direcao = -1;
                    }
                    if (modoLinha) {
                        rotacionar_linha(&estado, indiceLinhaSelecionada, direcao);
                        aplicar_gravidade(&estado);
                        int removido;
                        do {
                            removido = remover_colunas_uniformes(&estado);
                            if (removido > 0) {
                                removidasNaEtapa += removido;
                            }
                            aplicar_gravidade(&estado);
                        } while (removido > 0);
                        if (primeira_linha_vazia(&estado)) {
                            gerar_peca_primeira_linha(&estado);
                        }
                        aplicar_gravidade(&estado);
                        do {
                            removido = remover_colunas_uniformes(&estado);
                            if (removido > 0) {
                                removidasNaEtapa += removido;
                            }
                            aplicar_gravidade(&estado);
                        } while (removido > 0);
                    } else {
                        bool rotacionou = rotacionar_coluna(&estado, indiceColunaSelecionada, direcao);
                        if (rotacionou) {
                            aplicar_gravidade(&estado);
                            int removido;
                            do {
                                removido = remover_colunas_uniformes(&estado);
                                if (removido > 0) {
                                    removidasNaEtapa += removido;
                                }
                                aplicar_gravidade(&estado);
                            } while (removido > 0);
                            if (primeira_linha_vazia(&estado)) {
                                gerar_peca_primeira_linha(&estado);
                            }
                            aplicar_gravidade(&estado);
                            do {
                                removido = remover_colunas_uniformes(&estado);
                                if (removido > 0) {
                                    removidasNaEtapa += removido;
                                }
                                aplicar_gravidade(&estado);
                            } while (removido > 0);
                        }
                    }
                } else if ((tecla == 'x' || tecla == 'X') && modoLinha) {
                    int removido = limpar_linha(&estado, indiceLinhaSelecionada);
                    if (removido > 0) {
                        removidasNaEtapa += removido;
                        aplicar_gravidade(&estado);
                        int remov2;
                        do {
                            remov2 = remover_colunas_uniformes(&estado);
                            if (remov2 > 0) {
                                removidasNaEtapa += remov2;
                            }
                            aplicar_gravidade(&estado);
                        } while (remov2 > 0);
                        if (primeira_linha_vazia(&estado)) {
                            gerar_peca_primeira_linha(&estado);
                        }
                        aplicar_gravidade(&estado);
                        do {
                            remov2 = remover_colunas_uniformes(&estado);
                            if (remov2 > 0) {
                                removidasNaEtapa += remov2;
                            }
                            aplicar_gravidade(&estado);
                        } while (remov2 > 0);
                    }
                }
                tecla = j_tecla();
            }
            if (encerrarPorESC) {
                break;
            }
            for (int linha = 0; linha < NUM_LINHAS; linha++) {
                for (int coluna = 0; coluna < NUM_COLUNAS; coluna++) {
                    int valor = estado.tabuleiro[linha][coluna];
                    cor_t cor;
                    if (valor == CELULA_VAZIA) {
                        cor = (cor_t){0.2f, 0.2f, 0.2f, 1.0f};
                    } else {
                        int idx = valor % QTD_CORES;
                        cor = CORES[idx];
                    }
                    ponto_t posicao;
                    posicao.x = 100 + coluna * TAM_CELULA;
                    posicao.y = 50 + linha * TAM_CELULA;
                    tamanho_t tamanhoCelula;
                    tamanhoCelula.largura = TAM_CELULA - 2;
                    tamanhoCelula.altura = TAM_CELULA - 2;
                    retangulo_t ret;
                    ret.inicio.x = posicao.x + 1;
                    ret.inicio.y = posicao.y + 1;
                    ret.tamanho = tamanhoCelula;
                    cor_t corBorda;
                    corBorda.vermelho = 0;
                    corBorda.verde = 0;
                    corBorda.azul = 0;
                    corBorda.opacidade = 1;
                    j_retangulo(ret, 2.0f, corBorda, cor);
                    if (modoLinha && linha == indiceLinhaSelecionada) {
                        ponto_t p1; ponto_t p2;
                        p1.x = 100;
                        p1.y = 50 + indiceLinhaSelecionada * TAM_CELULA + TAM_CELULA / 2;
                        p2.x = 100 + NUM_COLUNAS * TAM_CELULA;
                        p2.y = p1.y;
                        j_linha(p1, p2, 2.0f, (cor_t){1,1,1,1});
                    }
                    if (!modoLinha && coluna == indiceColunaSelecionada) {
                        ponto_t p1; ponto_t p2;
                        p1.x = 100 + indiceColunaSelecionada * TAM_CELULA + TAM_CELULA / 2;
                        p1.y = 50;
                        p2.x = p1.x;
                        p2.y = 50 + NUM_LINHAS * TAM_CELULA;
                        j_linha(p1, p2, 2.0f, (cor_t){1,1,1,1});
                    }
                }
            }
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "Pontuacao: %d  Etapa: %d  Tempo: %.0f", estado.pontuacao, estado.etapa, restante > 0 ? restante : 0);
            j_texto((ponto_t){20, 50 + NUM_LINHAS * TAM_CELULA + 20}, (cor_t){1,1,1,1}, buffer);
            j_mostra();
        }
        if (encerrarPorESC) {
            char nome[TAM_MAX_NOME];
            solicitar_nome(nome);
            if (strlen(nome) > 0) {
                adicionar_registro(&historico, nome, estado.pontuacao, ultimaEtapa);
            }
            encerrarPrograma = true;
            break;
        }
        char nome[TAM_MAX_NOME];
        solicitar_nome(nome);
        if (strlen(nome) > 0) {
            adicionar_registro(&historico, nome, estado.pontuacao, ultimaEtapa);
        }
        mostrar_instrucoes(&historico);
    }
    liberar_historico(&historico);
    j_finaliza();
    return 0;
}
