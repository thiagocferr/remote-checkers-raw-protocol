# EP2 - MAC0352 - Redes de Computadores e Sistemas Distribuídos (2021)

## Problema e Funcionalidades (extraído do enunciado feito pelo Prof. Daniel Macêdo Batista)

### Problema

> Neste EP você deverá implementar um sistema distribuı́do que possibilite uma partida de jogo da velha em uma arquitetura hı́brida (P2P e cliente/servidor) com tolerância a algumas falhas. O sistema deve ser composto por diversas máquinas em uma rede local (a correção será feita com 3 máquinas). A invocação do primeiro código (servidor) deve ser feita recebendo como parâmetro apenas a porta (ou portas, caso seja necessário definir mais de uma porta) na qual ele irá escutar por conexões dos clientes. A invocação do segundo código (cliente) deve ser feita passando como parâmetro o endereço IP e a porta (ou portas) do servidor. A comunicação entre as máquinas pode usar UDP ou TCP. Você decide.
>
>O servidor será responsável por monitorar se os clientes estão conectados, autenticar os usuários, manter uma tabela de classificação dos jogadores (cada vitória dá 2 pontos. Cada empate dá 1 ponto) e registrar diversas ações em um arquivo de log. Note que os usuários do sistema e a tabela de classificação precisam ser persistentes. Ou seja, você vai precisar criar arquivos que mantenham essas informações para que elas sejam recuperadas na próxima vez que o servidor for executado. Os clientes executarão o jogo da velha propriamente dito, conectando-se entre si. Por isso que o jogo segue uma arquitetura hı́brida. Os clientes conversam com o servidor para algumas ações (cliente/servidor) e depois conectam- se entre si para realizarem uma partida (P2P). Durante a partida, os comandos do jogo entre os clientes **devem** ser enviados exclusivamente entre os clientes. O servidor registrará algumas informações mas ele **não** pode ter a tarefa de receber os lances do jogo de um cliente e enviar para o outro.

### Funcionalidade

**IMPORTANTE**: nem tudo o que está descrito aqui foi implementado na prática. Verificar os [slides](slides.md) que acompanha o projeto para lista do que não foi implementado

> Dois códigos precisam ser implementados: um para o cliente e um para o servidor. Quando o cliente for executado ele deve exibir o seguinte prompt para o usuário:
>
> ```bash
> JogoDaVelha>
> ```
>
> O servidor deve executar sem necessidade de interação. O ideal é que ele seja um daemon 2 , embora isso não seja obrigatório. É aceitável que ele funcione no segundo plano sendo invocado com ’&’ no shell.
>
> O seu sistema deve implementar um protocolo de rede que atenda aos seguintes requisitos:
>
> - verificação periódica, iniciada pelo servidor, de que os clientes continuam conectados. Esse meca- nismo existe em diversos sistemas e é chamado de heartbeat;
> - verificação periódica, entre clientes, da latência entre eles durante uma partida;
> - envio criptografado das credenciais de usuário e senha usando TLS ou SSL (recomenda-se a utilização das bibliotecas OpenSSL 3 ou MBed TLS 4 );
> - troca de mensagens em modo texto entre cliente e servidor e entre clientes.
>
> Com exceção das mensagens criptografadas, o protocolo criado por você deve usar comandos em ASCII para todas as ações, permitindo uma depuração fácil com o wireshark ou com o tcpdump. Comandos para as seguintes ações devem ser implementados:
>
> 1. heartbeat entre servidor e clientes
> 2. verificação de latência entre clientes numa partida
> 3. criação de um novo usuário
> 4. login
> 5. mudança de senha
> 6. logout
> 7. solicitação da lista dos usuários conectados
> 8. inı́cio de uma partida (nesse momento é necessário definir quem é ’X’ e quem é ’O’)
> 9. envio de uma jogada (qual linha e qual coluna)
> 10. encerramento de uma partida antes dela terminar, por iniciativa de um dos jogadores
> 11. recebimento do tabuleiro atualizado (toda vez que alguém faz um lance, o tabuleiro precisa aparecer atualizado para ele e para o oponente. Nesse momento o shell do jogo deve travar para quem fez a jogada e ser ’liberado’ apenas quando o outro jogador fizer o lance dele ou se a conexão com o oponente for interrompida por uma falha na rede)
> 12. envio do resultado da partida para o servidor (se o servidor ’caiu’ no meio da partida, é necessário esperar para tentar reconectar e reenviar o resultado. Essa tentativa deve esperar até 3 minutos. Se nesse intervalo não for possı́vel reconectar ao servidor, uma mensagem de erro deve ser informada para o jogador)
> 13. solicitação da classificação de todos os usuários existentes
>
> Outros comandos podem ser implementados caso você ache necessário.
>
> Os comandos 1, 2, 11 e 12 devem ocorrer sem necessidade de interação dos usuários. Eles serão enviados entre as entidades do sistema de forma periódica (1 e 2) ou quando ocorrer algum evento que faça eles serem necessários (11 e 12).
>
> Os demais comandos precisam ser invocados pelos usuários nos prompts do jogo das seguintes for- mas:
>
> - adduser <usuario> <senha>: cria um novo usuário
> - passwd <senha antiga> <senha nova>: muda a senha do usuário
> - login <usuario> <senha>: loga
> - leaders: informa a tabela de pontuação de todos os usuários registrados no sistema
> - list: lista todos os usuários conectados no momento e se estão ocupados em uma partida ou não
> - begin <oponente>: convida um oponente para jogar. Ele pode aceitar ou não
> - send <linha> <coluna>: envia a jogada
> - delay: durante uma partida, informa os 3 últimos valores de latência que foram medidos para o cliente do oponente
> - end: encerra uma partida antes da hora
> - logout: desloga
> - exit: finaliza a execução do cliente e retorna para o shell do sistema operacional
>
> A depender do resultado de um comando, alguns comandos não poderão ser usados em sequência. Por exemplo, se o usuário errar a senha, não faz sentido ele conseguir rodar o logout ou o begin já que ele não logou. Máquinas de estado 5 podem ser usadas para limitar os comandos que um usuário pode executar a depender do resultado do comando anterior.
>
> O servidor precisa manter um arquivo de log informando tudo que aconteceu durante o tempo em que o código ficou rodando. Esse arquivo de log deve informar o momento do evento e qual foi o evento. Alguns eventos que não podem deixar de serem registrados são:
>
> - Servidor iniciado (Informando se a última execução dele foi finalizada com sucesso ou se houve uma falha. Caso houve falha, e se havia alguma partida em execução, ele deve retomar o “controle” dessa partida passando a enviar os heartbeats para os clientes, caso eles ainda estejam conectados entre eles)
> - Conexão realizada por um cliente (Endereço IP do cliente);
> - Login com sucesso ou não (Nome do usuário que conseguiu, ou não, logar, e endereço IP de onde
> veio o login);
> - Desconexão realizada por um cliente (Endereço IP do cliente);
> - Inı́cio de uma partida (Endereço IP e nomes dos usuários dos jogadores);
> - Finalização de uma partida (Endereço IP, nomes dos usuários dos jogadores e nome do vencedor);
> - Desconexão inesperada de um cliente, verificada pelos heartbeats (Endereço IP do cliente);
> - Servidor finalizado
>
> O sistema deve tolerar as seguintes falhas do servidor, limitadas a um intervalo de 3 minutos. Se em 3 minutos o servidor não voltar a um estado correto de execução, as falhas devem ser informadas para algum cliente que esteja aguardando a falha ser corrigida para se comunicar com o servidor:
>
> - Processo do servidor foi finalizado por um ‘kill -9‘
> - Rede do servidor foi desconectada por um ‘ifdown‘ Recomenda-se a leitura das seções dedicadas a falhas no livro do Stevens . Essa leitura pode ajudar na implementação do tratamento a essas falhas.


## Como compilar

Para compilar o EP, execute o comando "make" no diretório principal do projeto (onde se encontra este arquivo). Isso irá gerar dois arquivos: "ep_client", o programa-cliente, e "ep_server", o programa-servidor.

## Como limpar

Para remover os arquivos binários gerados pela compilação, execute o comando "make clean" no diretório principal do projeto (onde se encontra este arquivo).

## Como rodar

### Servidor

Para rodar o servidor, execute o binário "ep_server" da seguinte forma:

    $ ./ep_server [PORT]

onde "[PORT]" é o número da porta na qual o servidor escutará por conexões

### Cliente

Para rodar o cliente, execute o binário "ep_client" da seguinte forma:

    $ ./ep_client [ADDR] [PORT]

onde "[ADDR]" e "[PORT]" correspondem ao endereço IP do servidor e à porta na qual o servidor está escutando por requisições, respectivamente.

## Estrutura do projeto

O projeto consiste de 3 subdiretórios principais

### "Common"

Aqui ficam códigos utilizados tanto pelo cliente quanto pelo servidor. Inclui conjunto de funções de lida e escrita em sockets e pipes ("Communication") e de "parsing" de mensagens utilizadas no protocolo de comunicação client-servidor e cliente-cliente ("Tokenizer").

### "Server"

Aqui é criado o programa-servidor. Ele consiste de:

- main.cpp: Ponto de entrada do programa. Cria a thread principal e, para cada conexão, uma thread específica.

- Player.(cpp/hpp): Define uma classe "Player", que possui todas as informações de um jogador do sistema.

- mainThread.(cpp/hpp): Possui a lógica por trás da thread principal do servidor, que tem o papel de receber mensagens das threads secundárias (as que estão diretamente relacionadas com os clientes), processar a mensagem (de acordo com uma certa lógica) e devolver (ou não) alguma mensagem para as threads secundárias.

- clientThread.(cpp/hpp): São as threads secundárias, diretamente ligadas com os processos-clientes, servindo de intermediárias entre eles e a thread principal, com a lógica do sistema. Quando recebe mensagem do cliente, repassa para thread principal. Quando recebe mensagem da thread principal, envia para o cliente.

- Logger.(cpp/hpp): Define classe que auxilia no processo de criação de um log do sistema.

### "Client"

Aqui é criado o programa-cliente. Ele consiste de:

- main.cpp: Ponto de entrada da aplicação. Também trata de diversas lógicas da aplicação, principalmente relacionadas com o estabelecimento da conexão entre dois clientes (p2p) e no fluxo da partida.

- Client.(cpp/hpp): Define informações sobre o estado atual do cliente.

- Tictactoe.(cpp/hpp): Define operações sobre o "tabuleiro" do jogo.

- Parser(UserInput/ServerMessage/ClientMessage).(cpp/hpp): Classes que implementam as funções que interpretam mensagens vindas do usuário (pelo entrada do terminal), do servidor (pelo socket conectado a ele) ou de outro cliente (quando em uma partida).

- MessageTypes.(cpp/hpp): Define em quais estados o cliente pode executar certos comandos.


## Detalhes adicionais

O código deste EP não é completamente portátil. Para início, ele só funciona em sistemas GNU/Linux (muito do porquê tem a ver com os códigos envolvendo sockets específicos desse SO). Em teoria, ele deve ser portátil entre diferentes versões, mas há algumas restrições:

- O código utiliza a revisão C++17;
- No estágio de "linkage", além da biblioteca "pthread", para garantir maior portabilidade, a inclusão da biblioteca "stdc++fs";
- Só foi testado o código compilado sem a otimização ligada

Como informação extra, o programa foi testado após ser compilado utilizando o utilitário "g++", versão do gcc 9.3.0, com modelo de "thread" Posix.

