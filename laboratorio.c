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
#include <stdio.h>      // Inclui funções de entrada e saída (printf, scanf, fopen, etc.)
#include <string.h>     // Inclui funções para manipulação de strings (strcpy, strcmp, etc.)
#include <stdlib.h>     // Inclui funções de alocação dinâmica (malloc, realloc, free) e outras utilidades
#include "laboratorio.h" // Inclui o cabeçalho com as definições das estruturas e protótipos

// Variável global que armazena a quantidade total de laboratórios (inclui slots ativos e inativos)
int qtd_labs = 0;

// Função para limpar o buffer de entrada (remove caracteres remanescentes, como '\n')
void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF); // Lê e descarta até encontrar nova linha ou fim de arquivo
}

// --- FUNÇÕES DE PERSISTÊNCIA ---

// Salva os dados atuais em um arquivo binário
void salvarDados(Laboratorio *Laboratorios, Data *Datas, Horario *Horarios) {
    FILE *f = fopen("dados_projeto.dat", "wb"); // Abre arquivo para escrita binária
    if (f == NULL) { // Verifica se houve erro na abertura
        printf("\nErro ao abrir arquivo para salvar!\n");
        return;
    }
    // Escreve a quantidade de laboratórios no arquivo (cabeçalho)
    fwrite(&qtd_labs, sizeof(int), 1, f);
    // Escreve os arrays de laboratórios, datas e horários (apenas os primeiros qtd_labs elementos)
    fwrite(Laboratorios, sizeof(Laboratorio), qtd_labs, f);
    fwrite(Datas, sizeof(Data), qtd_labs, f);
    fwrite(Horarios, sizeof(Horario), qtd_labs, f);
    fclose(f); // Fecha o arquivo
}

// Função auxiliar para comparar reservas pelo horário de início (usada no qsort)
int compararHorariosAgenda(const void *a, const void *b) {
    ReservaLab *resA = (ReservaLab *)a; // Converte ponteiro genérico para ponteiro de ReservaLab
    ReservaLab *resB = (ReservaLab *)b;
    // Converte hora e minuto para minutos desde meia-noite
    int minA = resA->inicio.hora * 60 + resA->inicio.minuto;
    int minB = resB->inicio.hora * 60 + resB->inicio.minuto;
    return minA - minB; // Retorna diferença (ordem crescente)
}

// Carrega os dados do arquivo binário, realocando os arrays conforme necessário
void carregarDados(Laboratorio **Laboratorios, Data **Datas, Horario **Horarios, int *capacidade) {
    FILE *f = fopen("dados_projeto.dat", "rb"); // Abre arquivo para leitura binária
    if (f == NULL) { // Se o arquivo não existe, inicializa os laboratórios com valores padrão
        inicializaLaboratorio(*Laboratorios, *capacidade);
        return;
    }
    // Lê a quantidade de laboratórios do arquivo
    fread(&qtd_labs, sizeof(int), 1, f);
    if (qtd_labs <= 10) { //Verifica a quantidade de laboratórios carregados (qtd_labs) para definir a nova capacidade do array
        *capacidade = 10;   // Mantém capacidade mínima de 10
    } else {
        // Arredonda para o próximo múltiplo de 10
        *capacidade = ((qtd_labs + 9) / 10) * 10;
    }
    // Realoca os arrays para a nova capacidade
    Laboratorio *tmpLab = realloc(*Laboratorios, (*capacidade) * sizeof(Laboratorio));
    Data *tmpData = realloc(*Datas, (*capacidade) * sizeof(Data));
    Horario *tmpHor = realloc(*Horarios, (*capacidade) * sizeof(Horario));
    // Verifica se alguma realocação falhou
    if (tmpLab == NULL || tmpData == NULL || tmpHor == NULL) {
        free(tmpLab);   // Libera o que foi alocado (para evitar vazamento)
        free(tmpData);
        free(tmpHor);
        printf("\nErro ao realocar memoria no carregamento!\n");
        fclose(f);
        return;
    }
    // Atualiza os ponteiros originais
    *Laboratorios = tmpLab;
    *Datas = tmpData;
    *Horarios = tmpHor;
    // Lê os dados dos arrays do arquivo (exatamente qtd_labs elementos)
    fread(*Laboratorios, sizeof(Laboratorio), qtd_labs, f);
    fread(*Datas, sizeof(Data), qtd_labs, f);
    fread(*Horarios, sizeof(Horario), qtd_labs, f);
    fclose(f); // Fecha o arquivo
    // Ordena a agenda de cada laboratório ativo (para garantir consistência)
    for (int i = 0; i < qtd_labs; i++) {
        if ((*Laboratorios)[i].status == LAB_ATIVO) {
            qsort((*Laboratorios)[i].agenda, (*Laboratorios)[i].qtd_reservas,
                  sizeof(ReservaLab), compararHorariosAgenda);
        }
    }
    // Inicializa os slots restantes (de qtd_labs até capacidade-1) como indisponíveis
    for (int i = qtd_labs; i < *capacidade; i++) {
        (*Laboratorios)[i].status = LAB_INDISPONIVEL;
        (*Laboratorios)[i].qtd_reservas = 0;
        for (int j = 0; j < MAX_RESERVAS; j++) {
            (*Laboratorios)[i].agenda[j].ativa = 0; // Desativa todas as reservas
        }
    }
}

// --- UTILITÁRIOS ---

// Função para comparar laboratórios por ID (usada na ordenação)
int compararIDs(const void *a, const void *b) {
    Laboratorio *labA = (Laboratorio *)a;
    Laboratorio *labB = (Laboratorio *)b;
    return labA->id - labB->id;  // ordem crescente de ID
}

// Exibe os horários livres de um laboratório, considerando sua agenda de reservas
void exibirHorariosLivres(Laboratorio *lab) {
    // Garante que a agenda está ordenada
    qsort(lab->agenda, lab->qtd_reservas, sizeof(ReservaLab), compararHorariosAgenda);
    // Converte abertura e fechamento para minutos
    int inicio_min = lab->abertura.hora * 60 + lab->abertura.minuto;
    int fim_min = lab->fechamento.hora * 60 + lab->fechamento.minuto;
    int tempo_atual = inicio_min; // Começa do horário de abertura
    printf("\nHorarios livres:");
    if (lab->qtd_reservas == 0) {
        // Se não há reservas, todo o período é livre
        printf("\n [ %02d:%02d ate %02d:%02d ]",
               lab->abertura.hora, lab->abertura.minuto,
               lab->fechamento.hora, lab->fechamento.minuto);
    } else {
        // Percorre as reservas e imprime os intervalos entre elas
        for (int i = 0; i < lab->qtd_reservas; i++) {
            int res_ini = lab->agenda[i].inicio.hora * 60 + lab->agenda[i].inicio.minuto;
            int res_fim = lab->agenda[i].fim.hora * 60 + lab->agenda[i].fim.minuto;
            // Se há um intervalo antes desta reserva, imprime
            if (res_ini > tempo_atual) {
                printf("\n [ %02d:%02d ate %02d:%02d ]",
                       tempo_atual / 60, tempo_atual % 60,
                       res_ini / 60, res_ini % 60);
            }
            // Avança o tempo atual para o fim da reserva (se for maior)
            if (res_fim > tempo_atual) tempo_atual = res_fim;
        }
        // Após a última reserva, se ainda houver tempo até o fechamento, imprime
        if (tempo_atual < fim_min) {
            printf("\n [ %02d:%02d ate %02d:%02d ]",
                   tempo_atual / 60, tempo_atual % 60,
                   fim_min / 60, fim_min % 60);
        }
    }
}

// MELHORIA: Agora imprime a agenda de reservas do laboratório
void imprimirDadosLab(Laboratorio lab, Data d, Horario h) {
    printf("\nID: %d | Nome: %s | Cap: %d", lab.id, lab.nome, lab.capacidade);
    printf("\nEquipamentos: %s", lab.equipamentos);
    printf("\nFuncionamento: %02d:%02d as %02d:%02d", lab.abertura.hora, lab.abertura.minuto, lab.fechamento.hora, lab.fechamento.minuto);
    if (lab.qtd_reservas > 0) {
        printf("\nRESERVAS PARA %02d/%02d/%d:", d.dia, d.mes, d.ano);
        for (int i = 0; i < lab.qtd_reservas; i++) {
            if (lab.agenda[i].ativa) {
                printf("\n  -> %02d:%02d ate %02d:%02d para %s", 
                    lab.agenda[i].inicio.hora, lab.agenda[i].inicio.minuto,
                    lab.agenda[i].fim.hora, lab.agenda[i].fim.minuto,
                    lab.agenda[i].solicitante);
            }
        }
    } else {
        printf("\nStatus: LIVRE (Sem reservas)");
    }
    printf("\n-----------------------------------------------\n");
}

// Inicializa todos os slots do array de laboratórios com valores padrão
void inicializaLaboratorio(Laboratorio *Laboratorios, int capacidade) {
    for(int i = 0; i < capacidade; i++) {
        Laboratorios[i].id = 0;               // ID 0 indica slot vazio
        Laboratorios[i].status = LAB_INDISPONIVEL; // Inicialmente indisponível
        Laboratorios[i].status_reserva = LAB_DISPONIVEL; // Disponível para reserva (quando ativo)
        Laboratorios[i].indice_original = i;   // Guarda o índice original
        Laboratorios[i].qtd_reservas = 0;      // Nenhuma reserva
        strcpy(Laboratorios[i].nome, "");      // Nome vazio
        // Limpa também o array de agenda para evitar lixo de memória
        for(int j = 0; j < MAX_RESERVAS; j++) {
            Laboratorios[i].agenda[j].ativa = 0; // Todas as reservas inativas
        }
    }
}

// Valida se uma data (dia e mês) é válida (ano fixo 2026, fevereiro com 28 dias)
int validarData(int d, int m) {
    if (m < 1 || m > 12) return 0; // Mês inválido
    if (d < 1 || d > 31) return 0; // Dia fora do intervalo
    // Meses com 30 dias
    if ((m == 4 || m == 6 || m == 9 || m == 11) && d > 30) return 0;
    // Fevereiro (considerando 28 dias, sem ano bissexto)
    if (m == 2 && d > 28) return 0;
    return 1; // Data válida
}

// Valida se um horário (hora e minuto) é válido
int validarHorario(int h, int min) {
    if (h < 0 || h > 23) return 0; // Hora inválida
    if (min < 0 || min > 59) return 0; // Minuto inválido
    return 1; // Horário válido
}

// --- GERENCIAMENTO ---

// Adiciona um novo laboratório, reutilizando slots inativos ou aumentando a capacidade se necessário
int adicionarLab(Laboratorio **Laboratorios, Data **Datas, Horario **Horarios, int *qtd_labs, int *capacidade, Laboratorio novo, Data nova_data, Horario novo_hora) {
    int i;
    // 1. Procura um slot inativo (status != LAB_ATIVO) entre os primeiros *qtd_labs
    for (i = 0; i < *qtd_labs; i++) {
        if ((*Laboratorios)[i].status != LAB_ATIVO) {
            break; // Encontrou um slot vazio
        }
    }
    // Se não achou inativo, i == *qtd_labs (todos os slots até *qtd_labs-1 estão ativos)
    if (i == *qtd_labs) {
        // Verifica se precisa aumentar capacidade
        if (*qtd_labs == *capacidade) { // Array cheio
            int nova_cap = *capacidade + 10; // Aumenta em 10
            Laboratorio *tmpLab = realloc(*Laboratorios, nova_cap * sizeof(Laboratorio));
            Data *tmpData = realloc(*Datas, nova_cap * sizeof(Data));
            Horario *tmpHor = realloc(*Horarios, nova_cap * sizeof(Horario));
            if (tmpLab == NULL || tmpData == NULL || tmpHor == NULL) {
                free(tmpLab); // Libera o que foi alocado (pode haver alguns bem-sucedidos)
                free(tmpData);
                free(tmpHor);
                printf("\nErro ao realocar memoria!\n");
                return 0; // Falha
            }
            // Atualiza os ponteiros
            *Laboratorios = tmpLab;
            *Datas = tmpData;
            *Horarios = tmpHor;
            // Inicializa os novos slots (do antigo *capacidade até nova_cap-1) como indisponíveis
            for (int j = *capacidade; j < nova_cap; j++) {
                (*Laboratorios)[j].status = LAB_INDISPONIVEL;
                (*Laboratorios)[j].qtd_reservas = 0;
                for (int k = 0; k < MAX_RESERVAS; k++) {
                    (*Laboratorios)[j].agenda[k].ativa = 0;
                }
            }
            *capacidade = nova_cap; // Atualiza a capacidade
            printf("\nCapacidade aumentada para %d registros.\n", nova_cap);
        }
        i = *qtd_labs;      // Próximo índice disponível
        (*qtd_labs)++;       // Incrementa a quantidade de slots ocupados
    }
    // 2. Preenche o slot encontrado (seja inativo ou novo) com os dados do novo laboratório
    (*Laboratorios)[i] = novo;
    (*Laboratorios)[i].id = i + 1;               // ID sequencial baseado no índice (1-based)
    (*Laboratorios)[i].status = LAB_ATIVO;       // Marca como ativo
    (*Laboratorios)[i].status_reserva = LAB_DISPONIVEL; // Disponível para reserva
    (*Laboratorios)[i].indice_original = i;       // Guarda o índice original
    (*Laboratorios)[i].qtd_reservas = 0;          // Nenhuma reserva ainda
    // Limpa a agenda (segurança)
    for (int j = 0; j < MAX_RESERVAS; j++) {
        (*Laboratorios)[i].agenda[j].ativa = 0;
    }
    // Armazena data e horário nos arrays correspondentes
    (*Datas)[i] = nova_data;
    (*Datas)[i].ano = 2026;  // ano fixo
    (*Horarios)[i] = novo_hora;  // armazena o horário de abertura
    printf("\nSucesso! Laboratorio '%s' adicionado.\n", (*Laboratorios)[i].nome);
    return 1; // Sucesso
}

// Lista todos os laboratórios ativos em ordem crescente de ID
void listarLaboratorio(Laboratorio *Laboratorios, Data *Datas, Horario *Horarios) {
    // Aloca um array temporário para copiar apenas os laboratórios ativos
    Laboratorio *copia = malloc(qtd_labs * sizeof(Laboratorio));
    if (copia == NULL) {
        printf("\nErro de alocacao de memoria.\n");
        return;
    }
    int cont = 0;
    // Copia apenas os laboratórios ativos
    for(int i = 0; i < qtd_labs; i++) {
        if(Laboratorios[i].status == LAB_ATIVO) {
            copia[cont++] = Laboratorios[i];
        }
    }
    if(cont == 0) {
        printf("\nNenhum laboratorio cadastrado.\n");
        free(copia);
        return;
    }
    // Ordena a cópia por ID
    qsort(copia, cont, sizeof(Laboratorio), compararIDs);
    printf("\n--- LISTAGEM GERAL DE LABORATORIOS ---");
    // Usa indice_original para acessar Data e Horario corretos
    for(int i = 0; i < cont; i++) {
        int idx = copia[i].indice_original; // Índice original no array
        imprimirDadosLab(Laboratorios[idx], Datas[idx], Horarios[idx]);
    }
    free(copia); // Libera a memória da cópia
}

// Lista laboratórios que possuem pelo menos um horário livre no dia
void listarLaboratoriosDisponiveis(Laboratorio *Laboratorios, Data *Datas, Horario *Horarios) {
    printf("\n--- LABORATORIOS COM HORARIO LIVRE ---");
    int encontrou = 0;
    for (int i = 0; i < qtd_labs; i++) {
        if (Laboratorios[i].status == LAB_ATIVO && temHorarioLivre(&Laboratorios[i])) {
            // Exibe informações básicas
            printf("\nID: %d | Nome: %s | Capacidade: %d", 
                   Laboratorios[i].id, Laboratorios[i].nome, Laboratorios[i].capacidade);
            printf("\nEquipamentos: %s", Laboratorios[i].equipamentos);
            printf("\nFuncionamento: %02d:%02d as %02d:%02d",
                   Laboratorios[i].abertura.hora, Laboratorios[i].abertura.minuto,
                   Laboratorios[i].fechamento.hora, Laboratorios[i].fechamento.minuto);
            // Exibe os horários livres
            exibirHorariosLivres(&Laboratorios[i]);
            printf("\n-----------------------------------------------\n");
            encontrou = 1;
        }
    }
    if (!encontrou) printf("\nNenhum laboratorio com horario livre no momento.\n");
}

// Lista laboratórios que possuem reservas ativas
void listarLaboratoriosOcupados(Laboratorio *Laboratorios, Data *Datas, Horario *Horarios) {
    printf("\n--- LABORATORIOS COM RESERVAS ---");
    int encontrou = 0;
    for(int i = 0; i < qtd_labs; i++) {
        if(Laboratorios[i].status == LAB_ATIVO && Laboratorios[i].qtd_reservas > 0) {
            imprimirDadosLab(Laboratorios[i], Datas[i], Horarios[i]);
            encontrou = 1;
        }
    }
    if(!encontrou) printf("\nNao ha laboratorios reservados.\n");
}

// Verifica se um laboratório tem pelo menos um horário livre (considerando reservas existentes)
int temHorarioLivre(Laboratorio *lab) {
    if (lab->qtd_reservas == 0) return 1; // Sem reservas, totalmente livre
    // Ordena a agenda (segurança)
    qsort(lab->agenda, lab->qtd_reservas, sizeof(ReservaLab), compararHorariosAgenda);
    int abertura = lab->abertura.hora * 60 + lab->abertura.minuto;
    int fechamento = lab->fechamento.hora * 60 + lab->fechamento.minuto;
    int ultimo_fim = abertura; // Fim do último intervalo livre (começa na abertura)
    for (int i = 0; i < lab->qtd_reservas; i++) {
        int ini = lab->agenda[i].inicio.hora * 60 + lab->agenda[i].inicio.minuto;
        int fim = lab->agenda[i].fim.hora * 60 + lab->agenda[i].fim.minuto;
        if (ini > ultimo_fim) return 1; // Há um intervalo antes desta reserva
        if (fim > ultimo_fim) ultimo_fim = fim; // Avança para o fim desta reserva
    }
    // Depois da última reserva
    if (ultimo_fim < fechamento) return 1; // Há intervalo após a última reserva
    return 0; // Nenhum intervalo livre
}

// Busca um laboratório ativo pelo ID. Retorna o índice ou -1 se não encontrado.
int buscarLaboratorio(Laboratorio *Laboratorios, int id) {
    for(int i = 0; i < qtd_labs; i++) {
        if(Laboratorios[i].status == LAB_ATIVO && Laboratorios[i].id == id) return i;
    }
    return -1;
}

// Remove (torna indisponível) um laboratório pelo ID
int removerLaboratorio(Laboratorio *Laboratorios, int id) {
    int indice = buscarLaboratorio(Laboratorios, id);
    if (indice != -1) {
        Laboratorios[indice].status = LAB_INDISPONIVEL; // Marca como indisponível
        Laboratorios[indice].qtd_reservas = 0;          // Zera reservas
        // Limpa a agenda para evitar dados residuais
        for (int j = 0; j < MAX_RESERVAS; j++) {
            Laboratorios[indice].agenda[j].ativa = 0;
        }
        return 1; // Sucesso
    }
    return 0; // Não encontrado
}

// Atualiza os dados de um laboratório existente
int atualizarLaboratorio(Laboratorio *Laboratorios, int id, Data *Datas, Horario *Horarios) {
    int i = buscarLaboratorio(Laboratorios, id);
    if (i == -1) {
        printf("\nErro: Laboratorio com ID %d nao encontrado.\n", id);
        return 0;
    }
    // Criamos variáveis temporárias para armazenar os novos dados antes de validar
    Laboratorio tempLab = Laboratorios[i]; 
    Data tempData = Datas[i];
    printf("\n--- ATUALIZANDO LABORATORIO (ID: %d) ---", id);
    // 1. Nome
    printf("\nNovo nome: "); 
    limparBuffer(); 
    scanf("%63[^\n]", tempLab.nome); // Lê string com espaços
    // 2. Capacidade
    printf("Nova capacidade: "); 
    if (scanf("%d", &tempLab.capacidade) != 1) { 
        printf("Erro: Valor numerico invalido!\n"); 
        limparBuffer(); 
        return 0; 
    }
    // 3. Equipamentos
    printf("Novos equipamentos: "); 
    limparBuffer();
    scanf("%255[^\n]", tempLab.equipamentos);
    // 4. Data
    printf("Nova data (dia mes): "); 
    if (scanf("%d %d", &tempData.dia, &tempData.mes) != 2) { 
        printf("Erro: informe valores numericos\n"); 
        limparBuffer(); 
        return 0; 
    }
    if (!validarData(tempData.dia, tempData.mes)) {
        printf("Erro: Data invalida!\n");
        return 0;
    }
    tempData.ano = 2026; // Ano fixo
    // 5. Horários
    printf("Novo horario de ABERTURA (Hora Min): "); 
    if (scanf("%d %d", &tempLab.abertura.hora, &tempLab.abertura.minuto) != 2) { 
        printf("Erro: informe valores numericos\n"); 
        limparBuffer(); 
        return 0; 
    }
    if (!validarHorario(tempLab.abertura.hora, tempLab.abertura.minuto)) {
        printf("Erro: Horario de abertura invalido!\n");
        return 0;
    }
    printf("Novo horario de FECHAMENTO (Hora Min): "); 
    if (scanf("%d %d", &tempLab.fechamento.hora, &tempLab.fechamento.minuto) != 2) { 
        printf("Erro: informe valores numericos\n"); 
        limparBuffer(); 
        return 0; 
    }
    if (!validarHorario(tempLab.fechamento.hora, tempLab.fechamento.minuto)) {
        printf("Erro: Horario de fechamento invalido!\n");
        return 0;
    }
    // 6. Validação Lógica de Período
    int min_abre = tempLab.abertura.hora * 60 + tempLab.abertura.minuto;
    int min_fecha = tempLab.fechamento.hora * 60 + tempLab.fechamento.minuto;
    if (min_fecha <= min_abre) {
        printf("\nERRO: O fechamento deve ser posterior a abertura! Alteracoes descartadas.\n");
        return 0;
    }
    // --- MOMENTO DA ATUALIZAÇÃO REAL ---
    // Se o código chegou até aqui, significa que TODAS as validações passaram.
    // Agora sim, movemos os dados temporários para os arrays oficiais.
    Laboratorios[i] = tempLab;
    Datas[i] = tempData;
    Horarios[i] = tempLab.abertura; // Armazena o horário de abertura
    Laboratorios[i].horario_original = tempLab.abertura;
    limparBuffer();
    printf("\nSUCESSO: Todas as informações foram validadas e o laboratorio foi atualizado!\n");
    return 1;
}

// Realiza uma reserva em um laboratório
int reservaLaboratorio(Laboratorio *Laboratorios, int id, Data *Datas) {
    int idx = buscarLaboratorio(Laboratorios, id);
    if (idx == -1) {
        printf("\nErro: Laboratorio nao encontrado!\n");
        return -1;
    }
    Laboratorio *lab = &Laboratorios[idx];
    // Garantir que a agenda está ordenada (importante para a lógica de intervalos)
    qsort(lab->agenda, lab->qtd_reservas, sizeof(ReservaLab), compararHorariosAgenda);
    // 1. Mostrar Informações Básicas
    printf("\n--- RESERVA DE LABORATORIO ---");
    printf("\nNome: %s", lab->nome);
    printf("\nCapacidade Maxima: %d pessoas", lab->capacidade);
    printf("\nData Disponivel: %02d/%02d/%d", Datas[idx].dia, Datas[idx].mes, Datas[idx].ano);
    // 2. Exibir horários disponíveis (baseado na agenda ordenada)
    printf("\n\nJanelas de Horarios Livres:");
    int inicio_min = lab->abertura.hora * 60 + lab->abertura.minuto;
    int fim_min = lab->fechamento.hora * 60 + lab->fechamento.minuto;
    int tempo_atual = inicio_min;
    if (lab->qtd_reservas == 0) {
        printf("\n [ %02d:%02d ate %02d:%02d ]",
               lab->abertura.hora, lab->abertura.minuto,
               lab->fechamento.hora, lab->fechamento.minuto);
    } else {
        for (int i = 0; i < lab->qtd_reservas; i++) {
            int res_ini = lab->agenda[i].inicio.hora * 60 + lab->agenda[i].inicio.minuto;
            int res_fim = lab->agenda[i].fim.hora * 60 + lab->agenda[i].fim.minuto;
            if (res_ini > tempo_atual) {
                printf("\n [ %02d:%02d ate %02d:%02d ]",
                       tempo_atual / 60, tempo_atual % 60,
                       res_ini / 60, res_ini % 60);
            }
            if (res_fim > tempo_atual) tempo_atual = res_fim;
        }
        if (tempo_atual < fim_min) {
            printf("\n [ %02d:%02d ate %02d:%02d ]",
                   tempo_atual / 60, tempo_atual % 60,
                   fim_min / 60, fim_min % 60);
        }
    }
    // 3. Dados do Solicitante
    char nome_solicitante[64];
    int pessoas;
    printf("\n\nNome do Solicitante: ");
    limparBuffer();
    scanf("%63[^\n]", nome_solicitante);
    printf("Quantidade de pessoas: ");
    if (scanf("%d", &pessoas) != 1) {
        printf("\nERRO: Entrada invalida!\n");
        limparBuffer();
        return -1;
    }
    if (pessoas > lab->capacidade) {
        printf("\nERRO: Capacidade excedida (%d > %d)!\n", pessoas, lab->capacidade);
        return -1;
    }
    // 4. Escolha do horário de início
    int h_ini, m_ini;
    printf("Horario de inicio desejado (Hora Min): ");
    if (scanf("%d %d", &h_ini, &m_ini) != 2) {
        limparBuffer();
        return -1;
    }
    int inicio_desejado = h_ini * 60 + m_ini;
    // Validação básica: dentro do expediente?
    if (inicio_desejado < inicio_min || inicio_desejado >= fim_min) {
        printf("\nERRO: Fora do horario de funcionamento (%02d:%02d - %02d:%02d)!\n",
               lab->abertura.hora, lab->abertura.minuto,
               lab->fechamento.hora, lab->fechamento.minuto);
        return -1;
    }
    // 5. Encontrar o próximo evento (reserva ou fechamento) após o início desejado
    int proximo_evento = fim_min; // começa com o fechamento
    for (int i = 0; i < lab->qtd_reservas; i++) {
        int res_ini = lab->agenda[i].inicio.hora * 60 + lab->agenda[i].inicio.minuto;
        if (res_ini > inicio_desejado) {
            if (res_ini < proximo_evento)
                proximo_evento = res_ini;
            break; // como a agenda está ordenada, o primeiro que encontramos é o mais próximo
        }
    }
    // 6. Verificar se o início não conflita com reserva existente
    for (int i = 0; i < lab->qtd_reservas; i++) {
        int res_ini = lab->agenda[i].inicio.hora * 60 + lab->agenda[i].inicio.minuto;
        int res_fim = lab->agenda[i].fim.hora * 60 + lab->agenda[i].fim.minuto;
        if (inicio_desejado >= res_ini && inicio_desejado < res_fim) {
            printf("\nERRO: Horario de inicio coincide com uma reserva existente!\n");
            return -1;
        }
    }
    // 7. Duração pretendida: 2 horas (120 minutos)
    int duracao_desejada = 120;
    int fim_desejado = inicio_desejado + duracao_desejada;
    // Se o fim desejado ultrapassar o próximo evento, ajusta para o máximo disponível
    int fim_real;
    if (fim_desejado > proximo_evento) {
        fim_real = proximo_evento;
        if (fim_real <= inicio_desejado) {
            printf("\nERRO: Nao ha tempo disponivel a partir desse horario.\n");
            return -1;
        }
        printf("\nAviso: apenas %d minutos disponiveis (ate as %02d:%02d). Reserva sera ajustada.\n",
               fim_real - inicio_desejado, fim_real / 60, fim_real % 60);
    } else {
        fim_real = fim_desejado;
    }
    // 8. Criar a reserva
    if (lab->qtd_reservas >= MAX_RESERVAS) {
        printf("\nERRO: Limite de reservas para este laboratorio atingido.\n");
        return -1;
    }
    int pos = lab->qtd_reservas;
    strcpy(lab->agenda[pos].solicitante, nome_solicitante);
    lab->agenda[pos].qtd_pessoas = pessoas;
    lab->agenda[pos].inicio.hora = h_ini;
    lab->agenda[pos].inicio.minuto = m_ini;
    lab->agenda[pos].fim.hora = fim_real / 60;
    lab->agenda[pos].fim.minuto = fim_real % 60;
    lab->agenda[pos].ativa = 1; // Marca como ativa
    lab->qtd_reservas++; // Incrementa contador
    // 9. Ordenar a agenda novamente
    qsort(lab->agenda, lab->qtd_reservas, sizeof(ReservaLab), compararHorariosAgenda);
    printf("\nReserva para %s confirmada com sucesso!", nome_solicitante);
    if (fim_real < fim_desejado)
        printf(" (duracao ajustada para %d minutos)", fim_real - inicio_desejado);
    printf("\n");
    return idx; // Retorna o índice do laboratório
}

// Cancela uma reserva existente em um laboratório
int cancelarReserva(Laboratorio *Laboratorios, int id, Horario *HorariosAtuais) {
    int idx = buscarLaboratorio(Laboratorios, id);
    if (idx == -1) {
        printf("\nErro: Laboratorio nao encontrado!\n");
        return 0;
    }
    Laboratorio *lab = &Laboratorios[idx];
    if (lab->qtd_reservas == 0) {
        printf("\nEste laboratorio nao possui reservas para cancelar.\n");
        return 0;
    }
    // 1. Listar todas as reservas atuais
    printf("\n--- RESERVAS DO LABORATORIO: %s ---", lab->nome);
    for (int i = 0; i < lab->qtd_reservas; i++) {
        printf("\n[%d] Solicitante: %s | Horario: %02d:%02d as %02d:%02d | Pessoas: %d", 
                i + 1, 
                lab->agenda[i].solicitante,
                lab->agenda[i].inicio.hora, lab->agenda[i].inicio.minuto,
                lab->agenda[i].fim.hora, lab->agenda[i].fim.minuto,
                lab->agenda[i].qtd_pessoas);
    }
    // 2. Perguntar qual deseja cancelar
    int escolha;
    printf("\n\nDigite o numero da reserva que deseja cancelar (ou 0 para sair): ");
    if (scanf("%d", &escolha) != 1 || escolha == 0) {
        limparBuffer();
        return 0;
    }
    if (escolha < 1 || escolha > lab->qtd_reservas) {
        printf("\nOpcao invalida!\n");
        return 0;
    }
    int indice_reserva = escolha - 1; // Ajusta para o índice do array (0 a N-1)
    // 3. Ajustar o array (Remover e puxar as próximas para frente)
    // Se eu cancelar a reserva [1] de um total de 3, a [2] vira a [1] e a [3] vira a [2]
    for (int i = indice_reserva; i < lab->qtd_reservas - 1; i++) {
        lab->agenda[i] = lab->agenda[i + 1];
    }
    // 4. Atualizar contagem e status
    lab->qtd_reservas--;
    if (lab->qtd_reservas == 0) {
        lab->status_reserva = LAB_DISPONIVEL; // Se não há mais reservas, marca como disponível
    }
    printf("\nReserva cancelada com sucesso! Horarios do laboratorio atualizados.\n");
    return 1;
}