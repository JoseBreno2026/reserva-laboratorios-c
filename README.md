# Sistema de Gerenciamento e Reserva de Laboratórios 🧪

Software de terminal desenvolvido em **C** para gerenciar a reserva de laboratórios acadêmicos. O foco do projeto é a eliminação de conflitos de horários e de duplicidade de agendamentos, usando **alocação dinâmica de memória**, **structs**, **ordenação com `qsort`** e **persistência em arquivo binário**.

Projeto da disciplina de Introdução à Programação (UFAPE, 2026).

## 👥 Autores

- Jose Breno Da Silva Almeida ([@JoseBreno2026](https://github.com/JoseBreno2026))
- Jose Almeida de Freitas Neto ([@josealmeidatech](https://github.com/josealmeidatech))

## 🚀 Funcionalidades

O programa abre um menu com as opções abaixo:

| Opção | O que faz |
|-------|-----------|
| 1 - Adicionar Laboratório | Cadastra nome, capacidade, equipamentos, data de disponibilidade e horário de funcionamento |
| 2 - Listar Todos | Lista os laboratórios ativos em ordem de ID, com as reservas de cada um |
| 3 - Buscar por Laboratório | Mostra os dados completos de um laboratório a partir do ID |
| 4 - Remover Laboratório | Remove o laboratório (remoção lógica) junto com as suas reservas |
| 5 - Atualizar Laboratório | Altera nome, capacidade, equipamentos, data e horário de funcionamento |
| 6 - Reservar Laboratório | Mostra os horários livres e registra uma reserva |
| 7 - Cancelar Reserva | Lista as reservas do laboratório e cancela a escolhida |
| 8 - Listar APENAS Disponíveis | Lista só os laboratórios que ainda têm horário livre, mostrando as janelas |
| 9 - Listar APENAS Ocupados | Lista só os laboratórios que possuem reservas |
| 0 - Sair | Encerra o programa |

### Exemplo de uso

Cadastrando um laboratório, reservando das 09:00 às 11:00 e depois listando os horários livres:

```
--- RESERVA DE LABORATORIO ---
Nome: Lab de Redes
Capacidade Maxima: 20 pessoas
Data Disponivel: 10/03/2026

Janelas de Horarios Livres:
 [ 08:00 ate 18:00 ]

Reserva para Ana Souza confirmada com sucesso!

--- LABORATORIOS COM HORARIO LIVRE ---
ID: 1 | Nome: Lab de Redes | Capacidade: 20
Equipamentos: 10 PCs e switches
Funcionamento: 08:00 as 18:00
Horarios livres:
 [ 08:00 ate 09:00 ]
 [ 11:00 ate 18:00 ]
```

### Como as reservas funcionam

- A reserva pede o **ID do laboratório**, o **nome do solicitante**, a **quantidade de pessoas** e o **horário de início**.
- A duração padrão é de **2 horas**. Se o próximo horário já reservado (ou o fechamento do laboratório) vier antes, a reserva é **ajustada automaticamente** e o programa avisa a duração final.
- O sistema **recusa** a reserva quando o horário de início está fora do funcionamento, cai dentro de uma reserva existente, a quantidade de pessoas passa da capacidade ou o laboratório já tem 10 reservas.
- Cada laboratório tem **uma única data de disponibilidade** (informada no cadastro). As reservas valem para essa data.
- Ao **atualizar** um laboratório, o sistema recusa mudanças que deixariam uma reserva existente fora do novo horário de funcionamento ou acima da nova capacidade. Nesse caso, nada é alterado.

### Validações

- Data: mês de 1 a 12 e dia compatível com o mês (fevereiro com 28 dias)
- Horário: hora de 0 a 23 e minuto de 0 a 59
- Capacidade e quantidade de pessoas maiores que zero
- Fechamento sempre posterior à abertura
- Nome do solicitante não pode ser vazio
- Entradas não numéricas nos campos numéricos são tratadas sem travar o programa
- Nomes acima de 63 caracteres e equipamentos acima de 255 caracteres são cortados no limite, sem afetar as perguntas seguintes

### Dados salvos

- Os dados são gravados automaticamente em `dados_projeto.dat` (arquivo binário, na pasta onde o programa é executado) a cada alteração. Ao abrir de novo, o programa recarrega tudo.
- Se o arquivo não existir, estiver vazio ou corrompido, o programa começa sem dados e avisa.
- O arquivo é criado na primeira alteração e **não faz parte do repositório** (está no `.gitignore`). Por ser binário, ele não é portável entre sistemas ou compiladores diferentes.
- Ao remover um laboratório, o seu ID pode ser reaproveitado por um laboratório cadastrado depois.

## 🛠️ Tecnologias e Conceitos Aplicados

- **Linguagem:** C
- **Ferramentas:** Visual Studio Code, GCC Compiler

| Conceito | Onde aparece |
|----------|--------------|
| Alocação dinâmica (`malloc`, `realloc`, `free`) | Os vetores de laboratórios, datas e horários começam com 10 posições e crescem de 10 em 10 |
| Ponteiro para ponteiro | `adicionarLab` e `carregarDados` recebem `Laboratorio **` para poder realocar o vetor da função chamadora |
| `struct`, `enum` e `typedef` | `Laboratorio`, `ReservaLab`, `Data`, `Horario` e `StatusLab` |
| Ordenação com `qsort` | A agenda de cada laboratório é mantida ordenada por horário, e a listagem geral por ID |
| Arquivos binários | `fwrite` e `fread` em `salvarDados` e `carregarDados` |
| Modularização | Interface em `laboratorio.h`, implementação em `laboratorio.c` e menu em `main.c` |
| Tratamento de falhas | Verificação de `malloc`/`realloc`, de leitura incompleta ou inconsistente do arquivo e de entradas inválidas |

## 📁 Estrutura

```
reserva-laboratorios-c/
├── main.c           # menu principal e leitura dos dados
├── laboratorio.c    # implementação das funções
├── laboratorio.h    # structs, enums e protótipos
├── tests/
│   └── testar.sh    # testes de regressão com entradas simuladas
├── .gitignore       # ignora executáveis e o arquivo de dados
└── README.md
```

## ⚙️ Como executar o projeto na sua máquina

Pré-requisito: compilador **GCC** (no Windows, MinGW ou o GCC do MSYS2).

1. Clone o repositório:

```bash
git clone https://github.com/JoseBreno2026/reserva-laboratorios-c.git
cd reserva-laboratorios-c
```

2. Compile o código utilizando o GCC:

```bash
gcc main.c laboratorio.c -o sistema_reservas
```

3. Execute o programa gerado:

```bash
./sistema_reservas          # Linux/Mac
./sistema_reservas.exe      # Windows (terminal ou Git Bash)
```

## 🧪 Testes

O script `tests/testar.sh` roda o programa com entradas simuladas e faz 30 verificações: cadastro, reservas, conflitos, cancelamento, atualização, remoção, persistência, crescimento do vetor, textos longos e arquivos de dados inválidos. Cada teste usa uma pasta temporária, então o seu `dados_projeto.dat` não é alterado.

```bash
bash tests/testar.sh ./sistema_reservas          # Linux/Mac
bash tests/testar.sh ./sistema_reservas.exe      # Windows (Git Bash)
```

Para checar também erros de memória (Linux/Mac, GCC ou Clang):

```bash
gcc -fsanitize=address,undefined -g main.c laboratorio.c -o sistema_asan
bash tests/testar.sh ./sistema_asan
```

## ⚠️ Limitações conhecidas

- Cada laboratório aceita no máximo **10 reservas** (`MAX_RESERVAS`).
- Cada laboratório tem **um único dia** de disponibilidade, e o ano é fixo (**2026**), com fevereiro sempre de 28 dias.
- A duração padrão da reserva é fixa em 2 horas.
- Os campos numéricos usam `scanf`: digitar algo como `5abc` em um campo numérico aceita o `5` e deixa `abc` para a pergunta seguinte.
- Interface apenas em linha de comando.
