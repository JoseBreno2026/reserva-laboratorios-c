/*
 * ==================================================
 * Título:        2º V.A. de Introdução a Programação
 *                (Sistema para reserva de laboratórios)
 * Autores:       Jose Almeida de Freitas Neto e
 *                Jose Breno Da Silva Almeida
 * Disciplina:    Itrodução a Programação
 * Professor:     Renê Gusmão
 * Instituição:   Universidade Federal do Agreste de Pernambuco (UFAPE)
 * Data:          23/02/2026
 * Versão:        10.0
 *
 * Descrição:
 *   Implementa um Sistema para reserva de 
 *   laboratórios.
 * ==================================================
 */
#ifndef LABORATORIO_H            // Início do cabeçalho de inclusão única: se LABORATORIO_H não estiver definido, define-o e inclui o conteúdo.
#define LABORATORIO_H            // Define a macro LABORATORIO_H para evitar inclusões múltiplas.

#include <stdio.h>               // Inclui a biblioteca padrão de entrada e saída (necessária para FILE*, usada nos protótipos de salvar/carregar).

#define MAX_RESERVAS 10          // Define o número máximo de reservas que um laboratório pode ter (tamanho do vetor agenda).

extern int qtd_labs;             // Declaração de uma variável global (definida em laboratorio.c) que armazena a quantidade de laboratórios cadastrados (ativos ou inativos).

// Estrutura que representa uma data (dia, mês, ano).
typedef struct {
    int dia, mes, ano;           // Campos: dia, mês e ano (todos inteiros).
} Data;

// Estrutura que representa um horário (hora e minuto).
typedef struct {
    int hora, minuto;            // Campos: hora (0-23) e minuto (0-59).
} Horario;

// Enumeração para os possíveis status de um laboratório ou de sua disponibilidade.
typedef enum {
    LAB_ATIVO = 1,               // Laboratório ativo (cadastrado e em uso).
    LAB_INDISPONIVEL = 0,        // Laboratório inativo (removido ou slot vazio).
    LAB_DISPONIVEL = 1,          // Laboratório disponível para reserva (pode ser usado como status_reserva).
    LAB_NAO_DISPONIVEL = 0       // Laboratório não disponível para reserva.
} StatusLab;

// Estrutura que representa uma reserva individual.
typedef struct {
    int id;                        // Identificador único da reserva (não usado explicitamente no código atual).
    char solicitante[64];           // Nome do solicitante da reserva (até 63 caracteres + '\0').
    int idLaboratorio;              // ID do laboratório ao qual a reserva pertence.
    Data data;                      // Data da reserva.
    Horario inicio;                 // Horário de início da reserva.
    Horario fim;                    // Horário de término da reserva.
    int qtd_pessoas;                // Quantidade de pessoas que participarão.
    int ativa;                      // Flag que indica se a reserva está ativa (1) ou não (0) – usada para reservas canceladas ou slots vazios.
} ReservaLab;

// Estrutura que representa um laboratório.
typedef struct {
    int id;                         // Identificador único do laboratório (gerado sequencialmente).
    char nome[64];                  // Nome do laboratório (até 63 caracteres).
    int capacidade;                  // Capacidade máxima de pessoas.
    char equipamentos[256];          // Descrição dos equipamentos disponíveis (até 255 caracteres).
    StatusLab status;                // Status do laboratório: LAB_ATIVO ou LAB_INDISPONIVEL.
    StatusLab status_reserva;        // Status de disponibilidade para reserva: LAB_DISPONIVEL ou LAB_NAO_DISPONIVEL.
    Horario horario_original;        // Horário original de abertura (pode ser usado para restaurar).
    Horario abertura;                // Horário de abertura do laboratório.
    Horario fechamento;              // Horário de fechamento do laboratório.
    int indice_original;              // Índice original no array (usado para manter referência após ordenações).
    ReservaLab agenda[MAX_RESERVAS];  // Vetor de reservas do laboratório (até MAX_RESERVAS).
    int qtd_reservas;                 // Quantidade atual de reservas ativas neste laboratório.
} Laboratorio;

// --- PROTÓTIPOS DAS FUNÇÕES (declarações) ---

// Utilitários de entrada e validação
void limparBuffer();                // Limpa o buffer de entrada (descarrega caracteres remanescentes).
int validarData(int d, int m);      // Verifica se uma data (dia e mês) é válida (considerando ano fixo 2026).
int validarHorario(int h, int min); // Verifica se hora e minuto são válidos (0-23, 0-59).

// Funções auxiliares para verificação de disponibilidade
int temHorarioLivre(Laboratorio *lab);       // Retorna 1 se o laboratório possui pelo menos um horário livre no dia, 0 caso contrário.
void exibirHorariosLivres(Laboratorio *lab); // Exibe na tela todos os intervalos livres do laboratório.

// Persistência de dados
void salvarDados(Laboratorio *Laboratorios, Data *Datas, Horario *Horarios);       // Salva os arrays em arquivo binário.
void carregarDados(Laboratorio **Laboratorios, Data **Datas, Horario **Horarios, int *capacidade); // Carrega os dados do arquivo, realocando se necessário.

// Inicialização e listagens
void inicializaLaboratorio(Laboratorio *Laboratorios, int capacidade);              // Inicializa todos os slots do array com valores padrão.
void imprimirDadosLab(Laboratorio lab, Data d, Horario h);                          // Imprime os dados completos de um laboratório, incluindo reservas.
void listarLaboratorio(Laboratorio *Laboratorios, Data *Datas, Horario *Horarios); // Lista todos os laboratórios ativos em ordem alfabética.
void listarLaboratoriosDisponiveis(Laboratorio *Laboratorios, Data *Datas, Horario *Horarios); // Lista laboratórios com pelo menos um horário livre.
void listarLaboratoriosOcupados(Laboratorio *Laboratorios, Data *Datas, Horario *Horarios);   // Lista laboratórios que possuem reservas.

// Operações de CRUD (Create, Read, Update, Delete) sobre laboratórios
int adicionarLab(Laboratorio **Laboratorios, Data **Datas, Horario **Horarios, int *qtd_labs, int *capacidade, Laboratorio novo, Data nova_data, Horario novo_hora); // Adiciona um novo laboratório.
int buscarLaboratorio(Laboratorio *Laboratorios, int id);            // Busca um laboratório ativo pelo ID, retorna o índice ou -1.
int removerLaboratorio(Laboratorio *Laboratorios, int id);           // Remove (torna indisponível) um laboratório pelo ID.
int atualizarLaboratorio(Laboratorio *Laboratorios, int id, Data *Datas, Horario *Horarios); // Atualiza os dados de um laboratório existente.

// Operações de reserva
int reservaLaboratorio(Laboratorio *Laboratorios, int id, Data *Datas); // Realiza uma reserva em um laboratório.
int cancelarReserva(Laboratorio *Laboratorios, int id, Horario *HorariosAtuais); // Cancela uma reserva existente.

#endif // Fim da diretiva de inclusão única.