// config.h
#ifndef CONFIG_H
#define CONFIG_H

#define NUM_LINHAS 5
#define NUM_COLUNAS 6
#define INITIAL_COLORS 4
#define NUM_ETAPAS_MAX 5
#define CELL_SIZE 60  // pixels por célula

// Valor para célula vazia
#define EMPTY_CELL -1

// Duração de cada etapa em segundos
#define STAGE_DURATION_SEC 60.0
// Bônus base para avanço de etapa
#define BONUS_BASE 50
// Arquivo de recordes
#define RECORDS_FILE "records.txt"
// Comprimento máximo de apelido
#define MAX_NAME_LEN 32

#endif // CONFIG_H