#!/bin/bash
# Testes de regressão do sistema de reservas de laboratórios.
# Roda o programa com entradas simuladas e confere as respostas.
#
# Uso (na pasta raiz do projeto, depois de compilar):
#   bash tests/testar.sh ./sistema_reservas        # Linux/macOS
#   bash tests/testar.sh ./sistema_reservas.exe    # Windows (Git Bash)
#
# Cada teste roda em uma pasta temporária, então o seu dados_projeto.dat não é tocado.

if [ -z "$1" ] || [ ! -x "$1" ]; then
    echo "Uso: bash tests/testar.sh <caminho-do-executavel>"
    exit 2
fi
BIN=$(realpath "$1")
D=$(mktemp -d); cd "$D"
run() { timeout 10 "$BIN" 2>&1; }
ok()  { echo "  [OK]   $1"; }
bad() { echo "  [FALHA] $1"; FAIL=1; }
FAIL=0
cad() { printf '1\n%s\n%s\n%s\n10 3\n%s\n%s\n' "$1" "$2" "PC" "$3" "$4"; }

echo "== Bugs que existiam no original"
timeout 3 "$BIN" </dev/null >/dev/null 2>&1; [ $? -ne 124 ] && ok "EOF no menu encerra (sem loop infinito)" || bad "EOF trava"

rm -f dados_projeto.dat
out=$( (cad LabX 5 "18 0" "8 0"; echo 2; echo 0) | run )
echo "$out" | grep -q "fechamento deve ser posterior" && ! echo "$out" | grep -q "cadastrado com sucesso" && ok "rejeita fechamento antes da abertura" || bad "aceitou fechamento invalido"

rm -f dados_projeto.dat
out=$( (cad LabNeg -5 "8 0" "18 0"; echo 2; echo 0) | run )
echo "$out" | grep -q "capacidade deve ser maior que zero" && ok "rejeita capacidade negativa" || bad "aceitou capacidade negativa"

rm -f dados_projeto.dat
out=$( (cad LabA 5 "8 0" "18 0"; printf '6\nabc\n0\n') | run )
echo "$out" | grep -q "ID invalido" && ok "reserva com ID nao numerico e tratada" || bad "ID nao numerico"

rm -f dados_projeto.dat
out=$( (cad LabB 5 "8 0" "18 0"; printf '6\n1\nAna\n2\n10 99\n9\n0\n') | run )
echo "$out" | grep -q "Horario invalido" && ! echo "$out" | grep -q "10:99" && ok "rejeita minutos invalidos (10 99)" || bad "aceitou 10:99"

rm -f dados_projeto.dat
out=$( (cad LabC 5 "8 0" "18 0"; printf '6\n1\n\n2\n9 0\n9\n0\n') | run )
echo "$out" | grep -q "Nome do solicitante invalido" && ! echo "$out" | grep -q "confirmada com sucesso" && ok "rejeita nome de solicitante vazio" || bad "aceitou nome vazio"

rm -f dados_projeto.dat
out=$( (cad LabD 5 "8 0" "18 0"; printf '6\n1\n   \n2\n9 0\n0\n') | run )
echo "$out" | grep -q "Nome do solicitante invalido" && ok "rejeita nome so com espacos" || bad "aceitou nome so com espacos"

rm -f dados_projeto.dat
out=$( (cad LabE 5 "8 0" "18 0"; printf '6\n1\nAna\n0\n9 0\n0\n') | run )
echo "$out" | grep -q "maior que zero" && ok "rejeita 0 pessoas na reserva" || bad "aceitou 0 pessoas"

echo; echo "== Regressao: fluxo normal continua funcionando"
rm -f dados_projeto.dat
out=$( (cad Lab1 6 "8 0" "12 0"; printf '6\n1\nAna\n3\n9 0\n6\n1\nBeto\n3\n10 0\n6\n1\nCaio\n2\n8 0\n2\n0\n') | run )
echo "$out" | grep -q "Reserva para Ana confirmada" && ok "reserva 1 confirmada (09:00-11:00)" || bad "reserva 1"
echo "$out" | grep -q "coincide com uma reserva existente" && ok "conflito de horario detectado" || bad "conflito nao detectado"
echo "$out" | grep -q "apenas 60 minutos" && echo "$out" | grep -q "08:00 ate 09:00 para Caio" && ok "reserva ajustada ao proximo horario ocupado (08:00-09:00)" || bad "ajuste de duracao"
echo "$out" | grep -q "09:00 ate 11:00 para Ana" && ok "listagem mostra a reserva" || bad "listagem"

rm -f dados_projeto.dat
out=$( (cad Lab1 3 "8 0" "12 0"; printf '6\n1\nAna\n9\n9 0\n0\n') | run )
echo "$out" | grep -q "Capacidade excedida" && ok "capacidade excedida bloqueia reserva" || bad "capacidade"

echo; echo "== Persistencia (fechar e abrir de novo)"
rm -f dados_projeto.dat
(cad LabP 4 "8 0" "18 0"; printf '6\n1\nAna\n2\n9 0\n0\n') | run >/dev/null
out=$( printf '2\n0\n' | run )
echo "$out" | grep -q "LabP" && echo "$out" | grep -q "09:00 ate 11:00 para Ana" && ok "dados e reserva sobrevivem ao reinicio" || bad "persistencia"

echo; echo "== Cancelamento e remocao"
out=$( printf '7\n1\n1\n2\n0\n' | run )
echo "$out" | grep -q "Reserva cancelada com sucesso" && echo "$out" | grep -q "LIVRE (Sem reservas)" && ok "cancelamento libera o horario" || bad "cancelamento"
out=$( printf '4\n1\n2\n0\n' | run )
echo "$out" | grep -q "Removido" && echo "$out" | grep -q "Nenhum laboratorio cadastrado" && ok "remocao funciona e listagem vazia nao quebra" || bad "remocao"
out=$( (cad Novo 5 "8 0" "18 0"; printf '2\n0\n') | run )
echo "$out" | grep -q "ID: 1 | Nome: Novo" && ok "slot removido e reutilizado (ID 1)" || bad "reuso de slot"

echo; echo "== Crescimento do vetor (mais de 10 laboratorios) e reinicio"
rm -f dados_projeto.dat
( for i in $(seq 1 25); do cad "L$i" 5 "8 0" "18 0"; done; echo 0 ) | run >/dev/null
out=$( printf '2\n0\n' | run )
n=$(echo "$out" | grep -c "^ID: ")
[ "$n" -eq 25 ] && ok "25 laboratorios cadastrados e recarregados" || bad "esperado 25, veio $n"
echo "$out" | grep -q "ID: 25 | Nome: L25" && ok "ultimo laboratorio presente" || bad "ultimo lab"

echo; echo "== Arquivos de dados problematicos"
rm -f dados_projeto.dat; : > dados_projeto.dat
out=$( printf '2\n0\n' | run ); echo "$out" | grep -q "vazio ou invalido" && echo "$out" | grep -q "Nenhum laboratorio" && ok "arquivo .dat vazio (0 bytes)" || bad "dat vazio"
rm -f dados_projeto.dat; head -c 100 /dev/urandom > dados_projeto.dat
out=$( printf '2\n0\n' | run ); echo "$out" | grep -qE "invalido|corrompido" && ok "arquivo .dat com lixo aleatorio" || bad "dat lixo"
rm -f dados_projeto.dat; (cad Trunc 5 "8 0" "18 0"; echo 0) | run >/dev/null; truncate -s 300 dados_projeto.dat
out=$( printf '2\n0\n' | run ); echo "$out" | grep -q "corrompido ou incompleto" && ok "arquivo .dat truncado" || bad "dat truncado"
echo "$out" | grep -qiE "runtime error|AddressSanitizer|LeakSanitizer" && bad "sanitizer reclamou" || ok "sanitizers sem reclamacoes"

# .dat com tamanho certo, mas qtd_reservas invalida (-1) dentro do laboratorio
rm -f dados_projeto.dat; (cad Corr 5 "8 0" "18 0"; echo 0) | run >/dev/null
tam=$(wc -c < dados_projeto.dat)
off=$(( tam - 4 - 12 - 8 ))   # 4 (cabecalho) + sizeof(Laboratorio) - 4 = posicao do ultimo int (qtd_reservas)
printf '\xff\xff\xff\xff' | dd of=dados_projeto.dat bs=1 seek=$off conv=notrunc 2>/dev/null
out=$( printf '2\n0\n' | run ); echo "$out" | grep -q "reservas invalidas" && ok "arquivo .dat com qtd_reservas invalida" || bad "dat qtd_reservas invalida"
echo "$out" | grep -qiE "runtime error|AddressSanitizer|LeakSanitizer" && bad "sanitizer reclamou (qtd_reservas)" || ok "sem estouro de memoria com qtd_reservas invalida"

echo; echo "== Entradas de texto muito longo"
rm -f dados_projeto.dat
nome=$(printf 'A%.0s' $(seq 1 200))
out=$( (printf '1\n%s\n5\nPC\n10 3\n8 0\n18 0\n2\n0\n' "$nome") | run )
echo "$out" | grep -q "cadastrado com sucesso" && echo "$out" | grep -q "Nome: $(printf 'A%.0s' $(seq 1 63)) |" && ! echo "$out" | grep -q "Nome: $(printf 'A%.0s' $(seq 1 64))" && ok "nome com 200 caracteres e cortado em 63 sem bagunçar as proximas perguntas" || bad "nome muito longo"
rm -f dados_projeto.dat
out=$( (cad LabL 5 "8 0" "18 0"; printf '6\n1\n%s\n2\n9 0\n0\n' "$nome") | run )
echo "$out" | grep -q "confirmada com sucesso" && ok "solicitante com 200 caracteres nao bagunça a reserva" || bad "solicitante muito longo"

echo; echo "== Atualizacao respeita reservas ja existentes"
rm -f dados_projeto.dat
(cad LabU 10 "8 0" "18 0"; printf '6\n1\nAna\n8\n9 0\n0\n') | run >/dev/null
out=$( printf '5\n1\nLabU\n10\nPC\n10 3\n12 0\n18 0\n3\n1\n0\n' | run )
echo "$out" | grep -q "fora do novo horario de funcionamento" && echo "$out" | grep -q "Funcionamento: 08:00 as 18:00" && ok "atualizar horario que deixaria reserva fora e recusado (dados antigos mantidos)" || bad "atualizacao de horario"
out=$( printf '5\n1\nLabU\n5\nPC\n10 3\n8 0\n18 0\n3\n1\n0\n' | run )
echo "$out" | grep -q "acima da nova capacidade" && echo "$out" | grep -q "Cap: 10" && ok "atualizar capacidade menor que a reserva e recusado" || bad "atualizacao de capacidade"
out=$( printf '5\n1\nLabU2\n10\nPC novo\n11 4\n8 30\n20 0\n3\n1\n0\n' | run )
echo "$out" | grep -q "Atualizado!" && echo "$out" | grep -q "Nome: LabU2" && echo "$out" | grep -q "09:00 ate 11:00 para Ana" && ok "atualizacao compativel com a reserva e aceita" || bad "atualizacao valida"

echo; [ $FAIL -eq 0 ] && echo "RESULTADO: TODOS OS TESTES PASSARAM" || echo "RESULTADO: HOUVE FALHAS"
rm -rf "$D"
