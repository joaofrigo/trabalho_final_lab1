#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "janela.h"
#include "config.h"
#include "estado_jogo.h"
#include "historico.h"

static const cor_t CORES[] = {
    {1,0,0,1}, {0,1,0,1}, {0,0,1,1}, {1,1,0,1},
    {1,0,1,1}, {0,1,1,1}, {1,0.5,0,1}, {0.5,0,0.5,1}
};
static const int QTD_CORES = sizeof(CORES) / sizeof(CORES[0]);

// Exibe instruções
static bool mostrar_instrucoes(Historico *h)
{
    j_seleciona_fonte(NULL,18);
    while (1) {
        j_texto((ponto_t){20,40},(cor_t){1,1,1,1},"Controles:");
        j_texto((ponto_t){20,80},(cor_t){1,1,1,1},"M: alterna modo Linha/Coluna");
        j_texto((ponto_t){20,110},(cor_t){1,1,1,1},"Setas: mover selecionado / girar");
        j_texto((ponto_t){20,140},(cor_t){1,1,1,1},"X: limpar linha (modo linha) - penalidade 2 pontos");
        j_texto((ponto_t){20,170},(cor_t){1,1,1,1},"ESC: encerrar partida e salvar score");
        j_texto((ponto_t){20,200},(cor_t){1,1,1,1},"C: ver recordes");
        j_texto((ponto_t){20,240},(cor_t){1,1,1,1},"Pressione Enter para iniciar...");
        j_mostra();
        tecla_t t = j_tecla();
        if (t == T_ENTER) {
            return true;
        } else if (t == T_ESC) {
            return false;
        } else if (t == 'c' || t == 'C') {
            mostrar_historico(h);
        }
    }
}


// Solicita nome do jogador
static void solicitar_nome(char *nome)
{
    int pos = 0;
    nome[0] = '\0';
    j_seleciona_fonte(NULL,18);
    bool concluido = false;
    while (!concluido) {
        char prompt[64];
        snprintf(prompt,sizeof(prompt),"Digite seu nome (max %d): %s",TAM_MAX_NOME-1,nome);
        j_texto((ponto_t){20,40},(cor_t){1,1,1,1},prompt);
        j_mostra();
        tecla_t t = j_tecla();
        if (t == T_ENTER && pos > 0) {
            concluido = true;
        } else if (t == T_ESC) {
            concluido = true;
        } else if (t == T_BS || t == T_BACKSPACE) {
            if (pos > 0) nome[--pos] = '\0';
        } else {
            int c = t;
            if (c >= 32 && c < 127 && pos < TAM_MAX_NOME-1) {
                nome[pos++] = (char)c;
                nome[pos] = '\0';
            }
        }
    }
}

// Processa rotação e remoção
static void processar_movimento(EstadoJogo *e,bool modoLinha,int idx,int dir,int *removidas)
{
    bool ok;
    if (modoLinha) {
        rotacionar_linha(e,idx,dir);
        ok = true;
    } else {
        ok = rotacionar_coluna(e,idx,dir);
    }
    if (!ok) return;
    aplicar_gravidade(e);
    int r;
    do { r = remover_colunas_uniformes(e);
        if (r>0) *removidas += r;
        aplicar_gravidade(e);
    } while (r>0);
    if (primeira_linha_vazia(e)) gerar_peca_primeira_linha(e);
    aplicar_gravidade(e);
    do { r = remover_colunas_uniformes(e);
        if (r>0) *removidas += r;
        aplicar_gravidade(e);
    } while (r>0);
}

// Desenha o tabuleiro e HUD
static void desenhar(EstadoJogo *e,bool modoLinha,int selL,int selC,double tempo)
{
    int i,j;
    for (i=0;i<NUM_LINHAS;i++){
        for (j=0;j<NUM_COLUNAS;j++){
            int v = e->tabuleiro[i][j];
            cor_t cor = (v==CELULA_VAZIA)?(cor_t){0.2,0.2,0.2,1}:CORES[v%QTD_CORES];
            retangulo_t r;
            r.inicio.x = 100+j*TAM_CELULA+1;
            r.inicio.y = 50 +i*TAM_CELULA+1;
            r.tamanho.largura = TAM_CELULA-2;
            r.tamanho.altura  = TAM_CELULA-2;
            j_retangulo(r,2,(cor_t){0,0,0,1},cor);
        }
    }
    ponto_t p1,p2;
    if (modoLinha) {
        p1.x=100; p1.y=50+selL*TAM_CELULA+TAM_CELULA/2;
        p2.x=100+NUM_COLUNAS*TAM_CELULA; p2.y=p1.y;
    } else {
        p1.x=100+selC*TAM_CELULA+TAM_CELULA/2; p1.y=50;
        p2.x=p1.x; p2.y=50+NUM_LINHAS*TAM_CELULA;
    }
    j_linha(p1,p2,2,(cor_t){1,1,1,1});
    char buf[64];
    snprintf(buf,sizeof(buf),"Pontuacao: %d  Etapa: %d  Tempo: %.0f",
             e->pontuacao,e->etapa,tempo);
    j_texto((ponto_t){20,50+NUM_LINHAS*TAM_CELULA+20},(cor_t){1,1,1,1},buf);
    j_mostra();
}

// Roda uma etapa, retorna false se ESC
static bool rodar_etapa(EstadoJogo *e,Historico *h,
                        bool *modoLinha,int *selL,int *selC,int *rem)
{
    double inicio = j_relogio();
    while (1) {
        double remT = DURACAO_ETAPA_SEG - (j_relogio()-inicio);
        if (remT<=0) return true;
        tecla_t t=j_tecla();
        if (t==T_ESC) return false;
        if (t=='m'||t=='M') *modoLinha = !*modoLinha;
        if (t=='c'||t=='C') mostrar_historico(h);
        if (t==T_CIMA) {
            if (*modoLinha) *selL = (*selL-1+NUM_LINHAS)%NUM_LINHAS;
            else *selC = (*selC-1+NUM_COLUNAS)%NUM_COLUNAS;
        }
        if (t==T_BAIXO) {
            if (*modoLinha) *selL = (*selL+1)%NUM_LINHAS;
            else *selC = (*selC+1)%NUM_COLUNAS;
        }
        if (t==T_ESQUERDA||t==T_DIREITA) {
            int d = (t==T_DIREITA)?1:-1;
            processar_movimento(e,*modoLinha,
                                *modoLinha?*selL:*selC,d,rem);
        }
        if (*modoLinha && (t=='x'||t=='X')) {
            int r = limpar_linha(e,*selL);
            if (r>0) { *rem += r; aplicar_gravidade(e);} 
        }
        desenhar(e,*modoLinha,*selL,*selC,remT);
    }
}

// Exibe resultado e aplica bônus
static void finalizar_etapa(EstadoJogo *e,int rem)
{
    int minimo=(NUM_LINHAS*NUM_COLUNAS)/2;
    j_seleciona_fonte(NULL,18);
    while (1) {
        char t1[128],t2[128];
        if (rem>=minimo) {
            snprintf(t1,sizeof(t1),"Etapa %d concluida! Removidas %d (min %d)",
                     e->etapa,rem,minimo);
            int bonus=BONUS_BASE*e->etapa;
            snprintf(t2,sizeof(t2),"Bonus: %d. Pontuacao atual: %d. Enter para proxima etapa.",
                     bonus,e->pontuacao+bonus);
        } else {
            snprintf(t1,sizeof(t1),"Etapa %d falhada. Removidas %d (min %d). Fim de jogo.",
                     e->etapa,rem,minimo);
            snprintf(t2,sizeof(t2),"Pontuacao final: %d. Enter para continuar.",e->pontuacao);
        }
        j_texto((ponto_t){20,40},(cor_t){1,1,1,1},t1);
        j_texto((ponto_t){20,80},(cor_t){1,1,1,1},t2);
        j_mostra();
        if (j_tecla()==T_ENTER) break;
    }
    if (rem >= minimo) {
        int bonus = BONUS_BASE * e->etapa;
        e->pontuacao += bonus;
        if (e->etapa < NUM_ETAPAS_MAX) e->etapa++;
    }
}

// Loop principal de etapas
static void executar_partida(Historico *h)
{
    EstadoJogo e;
    inicializar_estado(&e);
    bool modoLinha=true;
    int selL=0, selC=0, rem;
    int ultimaEtapa=1;

    while (1) {
        rem=0;
        bool cont = rodar_etapa(&e,h,&modoLinha,&selL,&selC,&rem);
        if (!cont) break;
        ultimaEtapa = e.etapa;
        finalizar_etapa(&e,rem);
        if (rem < (NUM_LINHAS*NUM_COLUNAS)/2) break;
    }
    char nome[TAM_MAX_NOME];
    solicitar_nome(nome);
    if (strlen(nome)>0) adicionar_registro(h,nome,e.pontuacao,ultimaEtapa);
}

int main(void)
{
    tamanho_t t;
    t.largura = (float)(NUM_COLUNAS*TAM_CELULA+200);
    t.altura  = (float)(NUM_LINHAS*TAM_CELULA+200);
    j_inicializa(t,"Jogo de Tabuleiro");
    Historico h;
    inicializar_historico(&h);

    while (1) {
        if (!mostrar_instrucoes(&h)) break;
        executar_partida(&h);
    }

    liberar_historico(&h);
    j_finaliza();
    return 0;
}

