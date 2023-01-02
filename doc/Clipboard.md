# Clipboard

**Data de entrega: 6 de Janeiro 2023**

## Contexto

Um grupo de amigos combinou um jantar num restaurante que apenas têm um empregado de mesa e cozinheiro.

**Regras:**

- 1º a chegar faz o pedido, depois de toda a gente chegar

- O empregado de mesa deve levar o pedido ao cozinheiro e trazer a comida quando estiver pronta.

- No final os amigos devem abandonar a mesa depois de todos terminarem

- O ultimo a chegar têm de pagar a conta.

**Notas:**

- Os clientes, empregado de mesa e cozinheiro são processos independentes

- Sincronização feita através de semáforos e memória partilhada.

- Todos os processos são criados no inicio do programa, sendo que cada cliente chega depois de um intervalo aleatório.

- Os processos devem estar ativos apenas quando necessário.

---

## Memória partilhada

Na memoria partilhada é armazenada a seguinte estrutura:

```c
typedef struct SHARED_DATA{

          /*All states*/

          FULL_STAT fSt;

          /* semaphores ids */

          unsigned int mutex;
          unsigned int friendsArrived;
          unsigned int requestReceived;
          unsigned int foodArrived;
          unsigned int allFinished;
          unsigned int waiterRequest;
          unsigned int waitOrder;

};
```

Que pode ser separada em dois campos, um com a estrutura que contêm todos os estados relevantes ao problema, outro com os ids dos semáforos necessários para o problema.

A estrutura `fSt` terá ser actualizada ao longo da execução do programa.

```c
typedef struct FULL_STAT
{   
    STAT st;
    int tableClients;
    int tableFinishEat;
    int foodRequest;
    int foodOrder;
    int foodReady;
    int paymentRequest;
    int tableLast;
    int tableFirst;

};
```

- `tableClients`
  
  Numero de clientes na mesa

- `tableFinishEat`
  
  Numero de clientes que acabaram de comer

- `foodRequest`
  
  Variável que indica se o pedido já foi feito, pelos clientes

- `foodOrder`
  
  Variável que indica se o pedido já foi entregue ao chefe.

- `foodReady`
  
  Variável que indica se o pedido já foi cozinhado.

- `paymentRequest`
  
  Variável que indica se o pedido já foi feito, pelos clientes.

- `tableLast`
  
  id do cliente que chegou por ultimo, portanto, aquele que ira pagar a conta.

- `tableFirst`
  
  id do cliente que chegou primeiro, portanto o que ira fazer o pedido.

A estrutura `STAT` contêm os estados de todos os elementos do programa.

```c
typedef struct STAT{
    unsigned int waiterStat;
    unsigned int chefStat;
    unsigned int clientStat[TABLESIZE];

};
```

---

## Semáforos

Temos ao todo 7 semáforos para fazer a sincronização destes processos:

1. `MUTEX`
   
   Controlar o acesso a zona critica.
   
   **valor inicial: 1**

2. `FRIENDSARRIVED`
   
   **Usado pelo cliente** para esperar pelos colegas.
   
   **valor inicial: 0**

3. `REQUESTRECEIVED`
   
   **Usado pelo cliente** para esperar depois de um pedido.
   
   **valor inicial: 0**

4. `FOODARRIVED`
   
   **Usado pelo cliente** para esperar pela comida.
   
   **valor inicial: 0**

5. `ALLFINISHED`
   
   **Usado pelo cliente** para esperar que os colegas acabem.
   
   **valor inicial: 0**

6. `WAITERREQUEST`
   
   **Usado pelo empregado** para esperar pelo pedido.
   
   **valor inicial: 0**

7. `WAITORDER`
   
   **Usado pelo chefe** para esperar pelo pedido entregue pelo empregado.
   
   **valor inicial: 0**

---

## Estados

#### **Cliente**

1. `INIT`
   
   Estado inicial do cliente, ainda não chegou á mesa.

2. `WAIT_FOR_FRIENDS`
   
   Chegou ao restaurante, está a espera dos seus colegas.

3. `FOOD_REQUEST`
   
   Estado reservado para o primeiro a chegar, em que este faz o pedido.

4. `WAIT_FOR_FOOD`
   
   A espera do seu pedido.

5. `EAT`
   
   A comer, após o seu pedido ter chegado

6. `WAIT_FOR_OTHERS`
   
   Após terminada a refeição, cada um espera que todos terminem.

7. `WAIT_FOR_BILL`
   
   Estado reservado, para o ultimo a chegar em que espera pela conta.

8. `FINISHED`
   
   Cliente terminou a sua rotina

#### **Chefe**

0. `WAIT_FOR_ORDER`
   
   Chefe está a espera do pedido

1. `COOK`
   
   Este estado, só aparece uma vez, pelo que é instantâneo.

2. `REST`
   
   O Chefe terminou de cozinhar e vai descansar

#### **Empregado de mesa**

0. `WAIT_FOR_REQUEST`
   
   Espera pelo cliente que vai fazer o pedido

1. `INFORM_CHEF`
   
   Informa o chefe e volta para o estado inicial.

2. `TAKE_TO_TABLE`
   
   Entrega o prato a cada cliente

3. `RECEIVE_PAYMENT`
   
   Recebe o pagamento do cliente que vai pagar pelo jantar

## 














































