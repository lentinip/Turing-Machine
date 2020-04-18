#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SBSIZE 600
#define ASIZE 5

int testMode = 0;

//DEFINIZIONE STRUTTURE ARCHI E STATI

typedef struct arc {
  //ciò che leggo sul nastro
  char read;
  //ciò che scrivo sul nastro
  char write;
  //L,R,S
  char move;
  //numero dello stato successivo
  int nextState;
  //Lista archi semplicemente concatenata
  //arco successivo
  struct arc* nextArc;
}arc;

typedef struct state {
  //numero identificativo dello stato
  int number;
  //lista con gli archi che legge (con transizione completa)
  arc* arcList;
  //identificativo se lo stato è finale
  int isFinal;
}state;

//DEFINIZIONE STRUTTURE BLOCCHI NASTRO, ARRAY DI BLOCCHI E CONFIGURAZIONI

typedef struct tapeBlock {
  char* block;
  int isUsed;
} tapeBlock;

typedef struct arrayOfTapeBlocks{
  //array che contiene in ordine i blocchi in cui è divisa la stringa del nastro
  int size;
  tapeBlock** data;
  int isUsed;
}arrayOfTapeBlocks;

typedef struct configurationMT {
  //Configurazione MT con stato attuale,posizione sul nastro, nastro, numero di passi attuali ed arco "da seguire".
  state* actualState;
  int blockPosition;
  char* block;
  int blockNumber; //numero del blocco rispetto alla divisione della stringa intera
  int actualSteps;
  arrayOfTapeBlocks* blocksArray;
  arc* interestedArc;
  //puntatore per far funzionare la coda
  struct configurationMT* prevConfigurationMT;
} configurationMT;

void arrayTapeDelete(arrayOfTapeBlocks* array);
void printArray(arrayOfTapeBlocks* array);

//FUNZIONI DI SUPPORTO PER ARCHI E STATI

void insertArc (state* actualState, arc** toInsertArc){
  if (actualState->arcList == NULL) {
    //se non c'è nessun arco nella lista imposta l'arco come primo della lista
    actualState->arcList=*toInsertArc;
    return;
  }

  else {
    //altrimenti mette l'arco in cima alla lista e lo collega alla testa della lista precedente
    arc* tempArc = actualState->arcList;
    actualState->arcList = *toInsertArc;
    actualState->arcList->nextArc = tempArc;
    return;
  }
}

//DEFINIZIONE ARRAY DI STATI

typedef struct arrayStructState {
  //Implementazione array dinamico di stati
  //dimensione
  int size;
  //puntatore ai dati
  state* data;
} arrayStructState;

void arrayStructStateFill (arrayStructState* array, int start, int toFill) {
  //funzione che aggiunge stati vuoti in un array di stati
  for (int i = start; i<=toFill; i++) {
    state newState;
    newState.number = 0;
    newState.arcList = NULL;
    newState.isFinal = 0;
    array->data[i] = newState;
  }
  return;
}

void arrayStructStateIncreaseSize (arrayStructState* array, int needed) {
  //aumenta il numero di stati nell'array al numero dello stato necessario
  int prevSize = array->size;
  array->size = needed+1;
  array->data = realloc(array->data, (sizeof(struct state))*(array->size));
  //aggiunge stati vuoti all'array
  arrayStructStateFill(array, prevSize, needed);
  return;
}

//DEFINIZIONE CODA CONFIGURAZIONE MT

typedef struct configurationQueque {
  //Coda di configurazioni
  int size;
  configurationMT* head;
  configurationMT* tail;
} configurationQueque;

void enqueque(configurationQueque* queque, configurationMT* configuration){
  //Aggiunge un elemento alla coda queque di tipo configurationQueque
  if (queque->size == 0) {
    //se la coda Ã¨ vuota
    queque->head = configuration;
    queque->tail = configuration;
  }
  else {
    queque->tail->prevConfigurationMT = configuration;
    queque->tail = configuration;
  }
  //aumento le dimensioni della coda
  queque->size = queque->size + 1;
  return;
}

configurationMT* dequeque(configurationQueque* queque){
  //restituisce il puntatore al primo elemento della coda, lo rimuove dalla coda e modifica la dimensione della coda
  if (queque == NULL || queque->size == 0){
    //se la coda non esiste o Ã¨ vuota
    return NULL;
  }
  configurationMT* configuration;
  configuration = queque->head;
  queque->head = configuration->prevConfigurationMT;
  queque->size = queque->size - 1;
  return configuration;
}

void destroyQueque(configurationQueque* queque){
  if (testMode>1){
    printf("\n\tEntro in destroyQueque\n");
  }
  //elimina da memoria la coda
  configurationMT* configuration;
  while(queque->size != 0){
    configuration = dequeque(queque);
    arrayTapeDelete(configuration->blocksArray);
    free(configuration);
  }
return;
}

//DEFINIZIONE FUNZIONI PER BLOCCHI

tapeBlock* blankBlock;

void blockDelete(tapeBlock* toDelete) {
  if (testMode>3){
    printf("\t\t\t\tEntro in blockDelete\n");
  }
  //blockDelete non cancella il blankBlock
  if (toDelete == blankBlock){
    return;
  }

  if (toDelete == NULL) {
    printf("***Errore in blockDelete: si Ã¨ cercato di eliminare un elemento che non esiste.\n");
    exit(1);
  }

  //Diminuisco il numero degli utilizzatori del blocco;
  toDelete->isUsed = toDelete->isUsed - 1;
  if (toDelete->isUsed == 0 ) {
    if (testMode>2){
      printf("Sto per cancellare il blocco: %s\n", toDelete->block);
    }
    free(toDelete->block);
    free(toDelete);
  }
  return;
}

void arrayOfTapeBlocksResize(arrayOfTapeBlocks* array, char movement){

  int arrayNewSize = array->size + 1;
  array->size = arrayNewSize;

  if (movement == 'R'){
    array->data = (tapeBlock **) realloc(array->data, sizeof(tapeBlock *)*arrayNewSize);
    array->data[array->size - 1] = blankBlock;
  }

  if (movement == 'L'){
    //creo un nuovo array di int che contengono le chiavi
    tapeBlock** data = (tapeBlock **) malloc(sizeof(tapeBlock *)*arrayNewSize);
    //inserisco il blocco nuovo
    data[0] = blankBlock;
    for(int k = 1; k<arrayNewSize; k++){
      data[k]=array->data[k-1];
    }
    free(array->data);
    array->data = data;
  }

  return;
}

arrayOfTapeBlocks* arrayTapeGenerator(char** inputPointer) {
  //Genera l'array di blocchi che contiene gli indirizzi dei blocchi della stringa
  arrayOfTapeBlocks* array = (arrayOfTapeBlocks *) malloc(sizeof(arrayOfTapeBlocks));
  char* initialTape = *inputPointer;
  array->size = ASIZE;
  array->data = (tapeBlock **) malloc(sizeof(tapeBlock *)*array->size);
  array->isUsed = 1;
  int i=0;
  int k=0;
  int z=0;
  for(; initialTape[i]!='\0';) {
    //Creo un blocco
    tapeBlock* newBlock = (tapeBlock *) malloc(sizeof(tapeBlock));
    newBlock->isUsed = 1;

    //Creo la stringa blocco
    char* block = (char *) malloc(sizeof(char)*SBSIZE);

    for(k=0;k<SBSIZE-1;k++){
      if (initialTape[i]=='\0'){
        block[k] = '_';
      }
      else {
        block[k]=initialTape[i];
        i++;
      }
    }
    block[k]='\0';

    //Assegno la stringa al blocco
    newBlock->block = block;

    if (testMode>0){
      printf("\tBlocco generato: %s\n", block);
    }

    if (z == array->size){
      //Se sono arrivato alla fine dell'array lo ingrandisco
      array->size = array->size + 1;
      array->data = (tapeBlock **) realloc(array->data, sizeof(tapeBlock *)*array->size);
    }

    //Inserisco il blocco nell'array
    array->data[z] = newBlock;
    z++;

  }
  //inizializzo a blankBlock le altre celle dell'array se questo Ã¨ piÃ¹ piccolo di ASIZE
  for (;z<array->size;z++){
    array->data[z] = blankBlock;
  }

  return array;
}

void arrayTapeIncreaseBlocks(arrayOfTapeBlocks* array){
  //Incrementa il numero di utilizzatori di 1 di ogni blocco dell'array
  tapeBlock* temp = NULL;
  for (int i=0; i<array->size; i++){
    if (array->data[i]!=blankBlock){
      //Non incrementa il numero di utilizzatori di blankBlock
      temp = array->data[i];
      temp->isUsed = temp->isUsed + 1;
    }
  }
  return;
}

void arrayTapeDelete(arrayOfTapeBlocks* array) {
  if (testMode>3){
    printf("\nEntro in arrayTapeDelete\n");
  }
  //Diminuisce il numero di utilizzatori
  array->isUsed = array->isUsed - 1;
  //Decrementa il numero di utilizzatori dei blocchi
  for (int i=0; i<array->size; i++){
    blockDelete(array->data[i]);
  }
  if (array->isUsed == 0){
    //Se l'array non è più utilizzato i blocchi saranno già  stati cancellati
    if (testMode>1){
      printf("\nSto per cancellare l'array:\n");
      printArray(array);
    }
    //Cancello ciò che rimane dell'array
    free(array->data);
    free(array);
    if (testMode>3){
      printf("\t\t*** L'array è stato cancellato!! ***\n");
    }
  }
  return;
}


//DEFINIZIONE VARIABILI GLOBALI

//numero passi massimi
int maxSteps;

//array dinamico con gli stati della MT
arrayStructState structureMT;

//Coda di configurazioni
configurationQueque cQueque;

//Info sullo stato della MT
int accepts = 0;
int notAccepts = 0;
int undefined = 0;
int undefinedConfiguration = 0;
int finish = 0;

//DEFINIZIONE FUNZIONI DI SUPPORTO A inputManager


void structureInizializer(){
  //COSTRUISCE structureMT E GLI ARCHI E GLI STATI DELLA MT

  //inizializzo la variabile globale structureMT
  structureMT.size = 20;
  structureMT.data = (state*) malloc(sizeof(struct state) * (structureMT.size));
  arrayStructStateFill(&structureMT,0, structureMT.size-1);

  //creo una stringa e leggo da input
  char inputString[5];
  memset(inputString, '\0', sizeof(inputString));
  scanf("%s", inputString);

  //leggo da input fino ad acc
  while(strncmp(inputString, "acc", 3)!=0){

    //creo uno stato temporaneo
    state tempState;
    tempState.isFinal = 0;
    tempState.arcList = NULL;

    //leggo il numero dello stato
    tempState.number = atoi(inputString);
    // printf("%d : numero stato\n", tempState.number);

    //aumento dimensione di strutureMT se necessario
    if (structureMT.size <= tempState.number) {
      arrayStructStateIncreaseSize(&structureMT, tempState.number);
    }

    //controllo se lo stato esiste giÃ , se si lo sostituisco a tempState
    if ((structureMT.data[tempState.number].arcList)!=NULL){
    tempState = structureMT.data[tempState.number];
    }

    //creo un arco temporaneo
    arc tempArc;
    tempArc.nextArc = NULL;

    //leggo il carattere da leggere sul nastro
    scanf("%s\n", &tempArc.read);

    //leggo il carattere da scrivere sul nastro
    scanf("%s\n", &tempArc.write);

    //leggo il movimento da fare
    scanf("%s\n", &tempArc.move);

    //leggo il numero dello stato successivo
    scanf("%s", inputString);
    tempArc.nextState = atoi(inputString);

    //inserisco l'arco nello stato
    arc* tempPointerArc = (arc *) malloc(sizeof(arc));
    *tempPointerArc=tempArc;
    insertArc(&tempState, &tempPointerArc);

    //aggiorno la struttura
    structureMT.data[tempState.number]=tempState;

    //aggiorno inputString
    scanf("%s", inputString);

  }

  return;
}

void accStates() {
  //IMPOSTO GLI STATI DI ACCETTAZIONE

  //creo una stringa e leggo da input
  char inputString[5];
  memset(inputString, '\0', sizeof(inputString));
  scanf("%s", inputString);
  int temp;

  //imposto ad 1 isFinal per lo stato letto da inputString
  while (strcmp(inputString,"max")!=0) {
    temp = atoi(inputString);

    //controllo se esiste lo stato in structureMT
    if (structureMT.size <= temp) {
      //se non esiste, aumento le dimensioni dell'array e aggiungo lo stato
      arrayStructStateIncreaseSize(&structureMT, temp);
      state newState;
      newState.arcList = NULL;
      structureMT.data[temp] = newState;
    }

    structureMT.data[temp].number=temp;
    structureMT.data[temp].isFinal=1;
    //cancello la stringa in input e la rileggo
    memset(inputString, '\0', sizeof(inputString));
    scanf("%s", inputString);
  }
  return;
}

void maxStepsSetting() {
  //IMPOSTO IL NUMERO MAX DI PASSI SULLA VARIABILE GLOBALE stepMax

  //creo una stringa e leggo da input
  char inputString[7];
  memset(inputString, '\0', sizeof(inputString));
  scanf("%s", inputString);
  //converto la stringa in numero
  maxSteps = atoi(inputString);
  return;
}

void inputManager() {
  //prende l'input e lo divide per passarlo a funzioni successive

  //legge "tr" dall'input
  char tr[3];

  scanf("%s", tr);

  if (strcmp(tr, "tr")!=0) {
    printf("Problemi nel riconoscimento dell'input: impossibile leggere <tr>.\n");
    exit(0);
  }

  //costruisco gli stati e gli archi per il mio programma
  structureInizializer();

  //imposto gli stati di accettazione
  accStates();

  //imposto il numero massimo di stati
  maxStepsSetting();

  //legge "run" dall'input
  char run[4];

  scanf("%s", run);

  if (strcmp(run, "run")!=0) {
    printf("Problemi nel riconoscimento dell'input: impossibile leggere <run>.\n");
    exit(0);
  }

  //leggo \n
  char c = getchar();

  return;
}

//DEFINIZIONE FUNZIONI DI SUPPORTO A MTExecution

char* dinamicStringInput(){
  //Questa funzione salva in memoria una stringa con lunghezza variabile presa da input

  //inizializzo la stringa
  int initialLenght = 100;
  int currentLenght = initialLenght;
  char* resultString = (char*) malloc(sizeof(char)*initialLenght);

  int i = 0;
  int chunk = 100;
  int thereIsAString = 0;

  char c = getchar();

  while (c!='\n' && c!=EOF){
    //se arrivo alla fine della stringa allungo la stringa di 7
    if (i == currentLenght-1) {
      currentLenght = currentLenght + chunk;
      resultString = realloc(resultString, sizeof(char)*currentLenght);
    }
    resultString[i] = c;

    i++;
    thereIsAString = 1;
    c = getchar();
  }

  if (c == EOF && thereIsAString == 0) {
  	finish = 1;
  	free(resultString);
  	return NULL;
  }

  //imposto il terminatore
  resultString[i] = '\0';

  return resultString;
}

void printTape(arrayOfTapeBlocks* array){
  printf("\n**NASTRO:\n");
  for(int i=0; i<array->size; i++){
    printf("%s", (array->data[i])->block);
  }
  printf("\n\n");
}

void printArray(arrayOfTapeBlocks* array){
  printf("Numero di utilizzatori array: %i\n", array->isUsed);
  for(int i=0; i<array->size;i++){
    printf("\t( %s | %i )\n", (array->data[i])->block, array->data[i]->isUsed);
  }
  printf("\n");
}

int configurationGenerator(configurationMT* inputConfiguration){
  //Funzione che trova gli archi da processare: ritorna 1 se trova archi, 0 altrimenti

  if (testMode>3){
    printf("Entro in configurationGenerator\n\n");
  }

  int thereIsAnArc = 0;
  arc* tempArc = inputConfiguration->actualState->arcList;

  while (tempArc!=NULL){

    if (tempArc->read == inputConfiguration->block[inputConfiguration->blockPosition]) {
      //se il carattere sul nastro è uguale a quello da leggere su un arco costruisco una configurazione
      configurationMT* configuration = inputConfiguration;
      if (thereIsAnArc > 0) {
        configuration = (configurationMT*) malloc(sizeof(configurationMT));
        configuration->actualState = inputConfiguration->actualState;
        configuration->blockPosition = inputConfiguration->blockPosition;
        configuration->block = inputConfiguration->block;
        configuration->blockNumber = inputConfiguration->blockNumber;
        configuration->actualSteps = inputConfiguration->actualSteps;
        configuration->blocksArray = inputConfiguration->blocksArray;
        configuration->blocksArray->isUsed = configuration->blocksArray->isUsed + 1;
        arrayTapeIncreaseBlocks(configuration->blocksArray);
      }
      configuration->interestedArc = tempArc;
      configuration->prevConfigurationMT = NULL;
      //dopo aver creato la configurazione la aggiungo in coda
      enqueque(&cQueque, configuration);
      //segno di aver trovato un arco;
      thereIsAnArc++;
    }

    //aggiorno tempArc con l'arco successivo
    tempArc = tempArc->nextArc;
  }
  if (thereIsAnArc == 0){
    //Cancello la configurazione di input
    arrayTapeDelete(inputConfiguration->blocksArray);
    if (testMode>1){
      printf("\t\t\t\tLibero l'array di blocchi da configurationGenerator\n");
    }
    free(inputConfiguration);
    if (testMode>1){
      printf("\t\t\t\tLibero inputConfiguration da configurationGenerator\n");
    }
  }
  return thereIsAnArc;
}

void configurationHandler(){
  //Funzione che esegue la mossa della prima configurazione in coda

  //prende la prima configurazione in coda
  configurationMT* configuration = dequeque(&cQueque);

  if (configuration==NULL){
    return;
  }

  if (testMode>0){
    printf("**************************\n\tStato attuale: %d\n", configuration->actualState->number);
    printTape(configuration->blocksArray);
  }

  //se la configurazione ha raggiunto i passi massimi allora non sappiamo se termina o meno
  if (configuration->actualSteps == maxSteps) {
    if (testMode>0){
      printf("- NUMERO DI PASSI LIMITE - numero passi: %d\n", configuration->actualSteps);
    }
    //elimino semplicemente la configurazione, posso dire che la terminazione Ã¨ incerta solo dopo aver processato tutte le configurazioni
    undefinedConfiguration = 1;
    arrayTapeDelete(configuration->blocksArray);
    free(configuration);
    return;
  }


    //gestisco il nastro/singolo blocco
    if (testMode>0){
      printf("%s: blocco precedente - ", configuration->block);
      printf("posizione sul blocco: %d\n", configuration->blockPosition);
    }

    //Se il blocco deve essere modificato
    if (configuration->block[configuration->blockPosition] != configuration->interestedArc->write){

      if (configuration->blocksArray->isUsed > 1){

        if (testMode>0){
          printf("Array precedente prima:\n");
          printArray(configuration->blocksArray);
        }

        //Creo una copia dell'array dei blocchi con isUsed = 1
        arrayOfTapeBlocks* newArray = (arrayOfTapeBlocks *) malloc(sizeof(arrayOfTapeBlocks));
        tapeBlock** data = (tapeBlock **) malloc(sizeof(tapeBlock *)*configuration->blocksArray->size);
        for (int y = 0; y<configuration->blocksArray->size; y++){
          data[y] = configuration->blocksArray->data[y];
        }
        newArray->size = configuration->blocksArray->size;
        newArray->data = data;
        newArray->isUsed = 1;

        //Creo una copia del blocco che sto utilizzando
        char* newStringBlock = (char *) malloc(sizeof(char)*SBSIZE);
        strcpy(newStringBlock, configuration->block);
        configuration->block = newStringBlock;

        //Decremento il blocco precedente se non è il blankBlock
        blockDelete(configuration->blocksArray->data[configuration->blockNumber]);

        //Creo un nuovo blocco e lo inserisco nel nuovo array e nella configurazione
        tapeBlock* newBlock = (tapeBlock *) malloc(sizeof(tapeBlock));
        newBlock->isUsed = 1;
        newBlock->block = newStringBlock;

        newArray->data[configuration->blockNumber] = newBlock;

        //Decremento il numero di utilizzatori dell'array precedente
        configuration->blocksArray->isUsed = configuration->blocksArray->isUsed - 1;

        if (testMode>0){
          printf("Array precedente dopo:\n");
          printArray(configuration->blocksArray);
        }

        //Imposto il nuovo array nella configurazione
        configuration->blocksArray = newArray;

        if (testMode>0){
          printf("Array nuovo:");
          printArray(configuration->blocksArray);
        }
      }

      if (configuration->blocksArray->data[configuration->blockNumber]->isUsed>1 || configuration->blocksArray->data[configuration->blockNumber]==blankBlock){
        //Se l'array ha isUsed == 1 ma il blocco è condiviso
        if (testMode>0){
          printf("*L'array rimane lo stesso!*\n");
          printf("Array prima:\n");
          printArray(configuration->blocksArray);
        }

        //Creo una copia del blocco che sto utilizzando
        char* newStringBlock = (char *) malloc(sizeof(char)*SBSIZE);
        strcpy(newStringBlock, configuration->block);
        configuration->block = newStringBlock;

        //Decremento il blocco precedente se non è il blankBlock
        blockDelete(configuration->blocksArray->data[configuration->blockNumber]);

        //Creo un nuovo blocco e lo inserisco nel nuovo array e nella configurazione
        tapeBlock* newBlock = (tapeBlock *) malloc(sizeof(tapeBlock));
        newBlock->isUsed = 1;
        newBlock->block = newStringBlock;
        configuration->blocksArray->data[configuration->blockNumber] = newBlock;

        if (testMode>0){
          printf("Array dopo:\n");
          printArray(configuration->blocksArray);
        }
      }

      if(testMode>4){
        printf("stringa assegnata: %s\n", configuration->block);
      }

      //Modifico il blocco
      configuration->block[configuration->blockPosition] = configuration->interestedArc->write;

      if (testMode>0){
        printf("** %s: blocco modificato **\n", configuration->block);
      }
    }

    //sposto il nastro
    if (configuration->interestedArc->move == 'R') {
      configuration->blockPosition = configuration->blockPosition + 1;
      //se ho raggiunto il limite destro del blocco
      if (configuration->blockPosition == (SBSIZE - 1)){
        //Modifico la posizione nel blocco
        configuration->blockPosition = 0;
        //Modifico il numero del blocco

        configuration->blockNumber = configuration->blockNumber + 1;

        if (configuration->blockNumber == configuration->blocksArray->size) {
          //Se il blocco non esiste lo aggiungo all'array
          arrayOfTapeBlocksResize(configuration->blocksArray, 'R');
        }
        //Modifico il puntatore alla stringa del blocco
        configuration->block = ((configuration->blocksArray->data[configuration->blockNumber]))->block;
        if(testMode>4){
          printf("blocco dopo lo spostamento: %s\n", configuration->block);
        }
      }
    }

    if (configuration->interestedArc->move == 'L') {
      configuration->blockPosition = configuration->blockPosition - 1;
      //se ho raggiunto il limite sinistro del blocco
      if (configuration->blockPosition == -1){
        //Modifico la posizione nel blocco
        configuration->blockPosition = SBSIZE - 2;
        //Modifico il numero del blocco
        configuration->blockNumber = configuration->blockNumber - 1;

        if (configuration->blockNumber == -1) {
          //Se il blocco non esiste lo aggiungo all'array
          arrayOfTapeBlocksResize(configuration->blocksArray, 'L');
          configuration->blockNumber = 0;
        }
        //Modifico il puntatore alla stringa del blocco
        configuration->block = ((configuration->blocksArray->data[configuration->blockNumber]))->block;
       }
    }
    if(testMode>4){
      printTape(configuration->blocksArray);
    }
    //aumento il numero dei passi
    configuration->actualSteps = configuration->actualSteps + 1;
    if (testMode>0){
      printf("%i: stato di arrivo - ", configuration->interestedArc->nextState);
      printf("isFinal = a %i\n\n", structureMT.data[configuration->interestedArc->nextState].isFinal);
    }

    //verifico se lo stato successivo è in structureMT, se non c'è vuol dire che è uno stato di non accettazione isolato
    if (configuration->interestedArc->nextState >= structureMT.size){
      //cancello la configurazione
      arrayTapeDelete(configuration->blocksArray);
      free(configuration);
      return;
    }

    //verifico se il nuovo stato è uno stato finale
    if ((structureMT.data[configuration->interestedArc->nextState].isFinal) == 1){
      arrayTapeDelete(configuration->blocksArray);
      free(configuration);
      accepts = 1;
      return;
    }
    if ((structureMT.data[configuration->interestedArc->nextState].isFinal) == 0){
      configuration->actualState = &structureMT.data[configuration->interestedArc->nextState];
      configurationGenerator(configuration);
    }
    return;
  }

//FUNZIONE PRINCIPALE CHE SIMULA LA MT

void MTExecution(){
  //Funzione principale che simula la MT

  //Se lo stato iniziale è finale accetta sempre
  if (structureMT.data[0].isFinal){
    accepts = 1;
    return;
  }

  //Leggo il nastro da input
  char* initialTapeStringInput = dinamicStringInput();

  if (initialTapeStringInput == NULL) {
  	return;
  }

  arrayOfTapeBlocks* initialTapeArray = arrayTapeGenerator(&initialTapeStringInput);

  //Creo la prima configurazione
  configurationMT* initialConfiguration = (configurationMT *) malloc(sizeof(configurationMT));

  initialConfiguration->actualState=&structureMT.data[0];
  initialConfiguration->blockPosition = 0;
  initialConfiguration->block = (initialTapeArray->data[0])->block;
  initialConfiguration->blockNumber = 0;
  initialConfiguration->actualSteps = 0;
  initialConfiguration->blocksArray = initialTapeArray;
  initialConfiguration->interestedArc = NULL;
  initialConfiguration->prevConfigurationMT = NULL;

  if (testMode>0){
    printf("\n\n\n *****NUOVA STRINGA*****\n stringa in ingresso: %s\n\n\n", initialTapeStringInput);
  }
  free(initialTapeStringInput);

  //se non trovo archi vuol dire che l'esecuzione termina
  if (configurationGenerator(initialConfiguration)==0){
    notAccepts = 1;
  }

  //eseguo
  while (accepts == 0 && notAccepts == 0 && undefined == 0){
    configurationHandler();

    if (cQueque.size == 0 && accepts == 0 && undefinedConfiguration == 0) {
      notAccepts = 1;
    }

    if (cQueque.size == 0 && accepts == 0 && undefinedConfiguration == 1) {
      undefined = 1;
      undefinedConfiguration = 0;
    }
  }

  if (accepts == 1) {
    printf("1\n");
  }
  if (notAccepts == 1) {
    printf("0\n");
  }
  if (undefined == 1) {
    printf("U\n");
  }

return;
}

int main () {
  inputManager();

  //Creo un blocco "blank" per quando un nastro dovrà  essere allungato
  char* blankBlockString = (char *) malloc(sizeof(char)*SBSIZE);
  int i = 0;
  for (; i<(SBSIZE-1); i++){
    blankBlockString[i]='_';
  }
  blankBlockString[i]='\0';

  blankBlock = (tapeBlock *) malloc(sizeof(tapeBlock));
  blankBlock->isUsed = 1;
  blankBlock->block = blankBlockString;

while (finish == 0){
  // printf("Entro nel ciclo while del main\n");
  MTExecution();
  //Ripulisco la coda per una nuova esecuzione di MTExecution
  destroyQueque(&cQueque);
  accepts = 0;
  notAccepts = 0;
  undefined = 0;
  if (testMode>0){
    printf("___________________________________________\n");
  }
}

//cancello il blankBlock
free(blankBlock->block);
free(blankBlock);

//cancello stati ed archi
arc* tempArc;
arc* tempArc2;
for (int k = 0; k<structureMT.size; k++){
  tempArc = structureMT.data[k].arcList;
  while(tempArc!=NULL){
    tempArc2 = tempArc->nextArc;
    free(tempArc);
    tempArc = tempArc2;
  }
}
free(structureMT.data);

  return 0;
}
