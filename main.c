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
#include <stdio.h>
#include <stdlib.h>
#include "laboratorio.h"

int main (void) {
    // Capacidade inicial do array (número de slots).
    int capacidade = 10;

    // Alocação dinâmica dos três arrays: laboratórios, datas e horários.
    Laboratorio *Laboratorios = malloc(capacidade * sizeof(Laboratorio));
    Data *Datas = malloc(capacidade * sizeof(Data));
    Horario *Horarios = malloc(capacidade * sizeof(Horario));

    // Verifica se todas as alocações foram bem-sucedidas.
    if (Laboratorios == NULL || Datas == NULL || Horarios == NULL) {
        printf("Erro ao alocar memoria inicial!\n");
        return 1; // Encerra o programa com código de erro.
    }
    
    // Tenta carregar dados existentes do arquivo.
    // Se o arquivo não existir, inicializa os arrays com valores padrão.
    carregarDados(&Laboratorios, &Datas, &Horarios, &capacidade);

    int op; // Variável para armazenar a opção do menu.

    do {
        // Exibição do menu principal.
        printf("\n=== SISTEMA DE GESTAO DE LABORATORIOS ===");
        printf("\n1 - Adicionar Laboratorio");
        printf("\n2 - Listar Todos");
        printf("\n3 - Buscar por Laboratorio");
        printf("\n4 - Remover Laboratorio");
        printf("\n5 - Atualizar Laboratorio");
        printf("\n6 - Reservar Laboratorio");
        printf("\n7 - Cancelar Reserva");
        printf("\n8 - Listar APENAS Disponiveis");
        printf("\n9 - Listar APENAS Ocupados");
        printf("\n0 - Sair");
        printf("\n> Escolha uma opcao: ");
        
        // Leitura da opção com validação.
        if (scanf("%d", &op) != 1) {
            printf("\nErro: Digite apenas numeros!\n");
            limparBuffer(); // Limpa o buffer em caso de entrada inválida.
            op = -1; // Define opção inválida para não executar nenhum case.
            continue; // Volta ao início do loop.
        }

        switch (op) {
           case 1: { // Adicionar laboratório
                Laboratorio a; // Variável temporária para o novo laboratório.
                Data dat;      // Variável temporária para a data.
    
                dat.ano = 2026; // Ano fixo conforme regra do projeto.

                printf("\n--- CADASTRO DE NOVO LABORATORIO ---");
                printf("\nNome: "); 
                // Lê o nome (até 63 caracteres, permitindo espaços).
                scanf(" %63[^\n]", a.nome);
    
                printf("Capacidade: "); 
                // Lê a capacidade e valida se é um número.
                if(scanf("%d", &a.capacidade) != 1) { 
                    printf("Erro: informe um valor numerico\n");
                    limparBuffer(); 
                    break; 
                }

                printf("Equipamentos: "); 
                // Lê a descrição dos equipamentos (até 255 caracteres).
                scanf(" %255[^\n]", a.equipamentos);

                printf("Data de disponibilidade (Dia Mes): ");
                // Lê dia e mês.
                if(scanf("%d %d", &dat.dia, &dat.mes) != 2) { 
                    printf("Erro: informe um valor numerico\n");
                    limparBuffer(); 
                    break; 
                }

                // Valida a data.
                if (!validarData(dat.dia, dat.mes)) { 
                    printf("Erro: Data %02d/%02d fora do calendario!\n", dat.dia, dat.mes); 
                    break; 
                }

                printf("Hora que o lab ABERTURA (Hora Minuto): ");
                // Lê hora e minuto de abertura.
                if(scanf("%d %d", &a.abertura.hora, &a.abertura.minuto) != 2) { 
                    printf("Erro: informe um valor numerico\n");
                    limparBuffer(); 
                    break; 
                }

                // Valida o horário de abertura.
                if (!validarHorario(a.abertura.hora, a.abertura.minuto)) { 
                    printf("Erro: Hora %dh:%dm fora do padrao (00-23)!\n", a.abertura.hora, a.abertura.minuto); 
                    break; 
                }

                printf("Hora que o lab FECHAMENTO (Hora Minuto): ");
                // Lê hora e minuto de fechamento.
                if(scanf("%d %d", &a.fechamento.hora, &a.fechamento.minuto) != 2) { 
                    printf("Erro: informe um valor numerico\n");
                    limparBuffer(); 
                    break; 
                }
    
                // Valida o horário de fechamento.
                if (!validarHorario(a.fechamento.hora, a.fechamento.minuto)) { 
                    printf("Erro: Hora %dh:%dm fora do padrao (00-23)!\n", a.fechamento.hora, a.fechamento.minuto); 
                    break; 
                }

                // Inicialização de campos de controle (redundante, mas seguro).
                a.horario_original = a.abertura;   // horário original = abertura (evita lixo).
                a.qtd_reservas = 0;                // redundante, mas mantido.

                // Tenta adicionar ao array global, passando a.abertura como o Horario (novo_hora).
                if(adicionarLab(&Laboratorios, &Datas, &Horarios, &qtd_labs, &capacidade, a, dat, a.abertura)) {
                    printf("\n>>> Laboratorio cadastrado com sucesso! <<<\n");
                    salvarDados(Laboratorios, Datas, Horarios); // Persiste os dados.
                } else {
                    printf("\nErro: Nao foi possivel adicionar o laboratorio.\n");
                }
            } break;

            case 2: // Listar todos os laboratórios (ordenados por ID).
                listarLaboratorio(Laboratorios, Datas, Horarios); 
                break;
            
            case 3: { // Buscar laboratório por ID.
                int ID; 
                printf("\nID para buscar: ");
                
                // Validação da entrada do ID.
                if(scanf("%d", &ID) != 1) { 
                    printf("Erro: ID deve ser um numero inteiro!\n"); 
                    limparBuffer(); 
                    break; 
                }
                
                int idx = buscarLaboratorio(Laboratorios, ID); 
                
                if(idx != -1) {
                    printf("\n--- RESULTADO DA BUSCA ---");
                    // Exibe os dados completos do laboratório encontrado.
                    imprimirDadosLab(Laboratorios[idx], Datas[idx], Horarios[idx]);
                } else {
                    printf("\nErro: Laboratorio com ID %d nao encontrado!\n", ID);
                }
            } break;
            
            case 4: { // Remover laboratório (tornar indisponível).
                int ID; 
                printf("\nID para remover: "); 
                if(scanf("%d", &ID) != 1) { 
                    printf("ID invalido!\n"); 
                    limparBuffer(); 
                    break; 
                }
                if(removerLaboratorio(Laboratorios, ID)) {
                    printf("\nRemovido!\n");
                    salvarDados(Laboratorios, Datas, Horarios); // Persiste a remoção.
                } else 
                    printf("\nErro: laboratorio nao existe!\n"); 
            } break;

            case 5: { // Atualizar dados de um laboratório.
                int ID; 
                printf("\nID para atualizar: "); 
                if(scanf("%d", &ID) != 1) { 
                    printf("ID invalido!\n"); 
                    limparBuffer(); 
                    break; 
                }
                if(atualizarLaboratorio(Laboratorios, ID, Datas, Horarios)) {
                    printf("\nAtualizado!\n");
                    salvarDados(Laboratorios, Datas, Horarios); // Persiste as alterações.
                }
            } break;
            
            case 6: { // Reservar laboratório.
                int ID; 
                printf("\nID para reserva: "); 
                scanf("%d", &ID);
                
                if(reservaLaboratorio(Laboratorios, ID, Datas) != -1) {
                    printf("\nReserva confirmada com sucesso!\n");
                    salvarDados(Laboratorios, Datas, Horarios); // Persiste a reserva.
                } else {
                    printf("\nNao foi possivel concluir a reserva.\n");
                }
            } break;

           case 7: { // Cancelar reserva.
                int ID; 
                printf("\nID para cancelar: "); 
                if(scanf("%d", &ID) != 1) { 
                    printf("ID invalido!\n"); 
                    limparBuffer(); 
                    break; 
                }
                if(cancelarReserva(Laboratorios, ID, Horarios)) {
                    printf("\nCancelada!\n");
                    salvarDados(Laboratorios, Datas, Horarios); // Persiste o cancelamento.
                } else 
                    printf("\nNao encontrado ou ja livre.\n");
            } break;

            case 8: // Listar apenas laboratórios com horário livre.
                listarLaboratoriosDisponiveis(Laboratorios, Datas, Horarios); 
                break;

            case 9: // Listar apenas laboratórios com reservas.
                listarLaboratoriosOcupados(Laboratorios, Datas, Horarios); 
                break;

            case 0: // Sair do programa.
                printf("\nSaindo e salvando...\n"); 
                break;

            default: // Opção inválida (não prevista no menu).
                printf("\nOpcao invalida!\n");
        }
    } while (op != 0); // Repete até que o usuário escolha sair.

    // Libera a memória alocada dinamicamente antes de encerrar.
    free(Laboratorios);
    free(Datas);
    free(Horarios);

    return 0; // Retorna 0 indicando sucesso.
}