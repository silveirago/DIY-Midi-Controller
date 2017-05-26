/* Esta sketch le as portas digitais do arduino e manda notas midi se os switches sao acionados e le as portas analogicas e envia MIDI Control Change.
Feito por Gustavo Silveira, 2016.

http://www.musiconerd.com
http://www.bitcontrollers.com
silveira.go@gmail.com
*/

#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

/////////////////////////////////////////////

const int NButtons = 4; // *coloque aqui o numero de entradas digitais utilizadas 
const int button[NButtons] = {2, 3, 4, 5}; // *neste array coloque na ordem desejada os pinos das portas digitais utilizadas
int buttonCState[NButtons] = {0}; // estado atual da porta digital
int buttonPState[NButtons] = {0}; // estado previo da porta digital

/////////////////////////////////////////////

const int NPots = 3; // *coloque aqui o numero de entradas analogicas utilizadas  
const int pot[NPots] = {A0,A1,A2}; // *neste array coloque na ordem desejada os pinos das portas analogicas utilizadas
int potCState[NPots] = {0}; // estado atual da porta analogica
int potPState[NPots] = {0}; // estado previo da porta analogica
int potVar = 0; // variacao entre o valor do estado previo e o atual da porta analogica

/////////////////////////////////////////////

byte midiCh = 1; // *Canal midi a ser utilizado
byte note = 36; // *Nota mais grave que sera utilizada
byte cc = 1; // *CC mais baixo que sera utilizado

/////////////////////////////////////////////

int TIMEOUT = 300; //quantidade de tempo em que o potenciometro sera lido apos ultrapassar o varThreshold
int varThreshold = 4; //threshold para a variacao no sinal do potenciometro
boolean potMoving = true; // se o potenciometro esta se movendo
unsigned long pTime[NPots] = {0}; // tempo armazenado anteriormente
unsigned long timer[NPots] = {0}; // armazena o tempo que passou desde que o timer foi zerado

/////////////////////////////////////////////

void setup () {

  //Start midi connection
  MIDI.begin();
  //Serial Connection
  Serial.begin(115200);
  //Initialize Digital Pins as inputs
  for (int i=0; i<NButtons; i++){
    pinMode(button[i], INPUT_PULLUP);
  }
 // pinMode(button[3], INPUT); //pino 13
  
  //Initialize Analog Pins as inputs
  for (int i=0; i<NPots; i++){
    pinMode(pot[i], INPUT);
  }
  

}

void loop () {

  //Le todas as entradas digitais utilizadas
  for (int i=0; i<NButtons; i++) {
    buttonCState[i] = digitalRead(button[i]);
  }
 // buttonCState[3] = !buttonCState[3]; // Inverte o valor do pino 13

  //Le o estado dos botoes e manda NoteOns para cada um que estiver ativo
  for (int i=0; i<NButtons; i++) {

    if (buttonCState[i] != buttonPState[i]) {

      if(buttonCState[i] == LOW) {     
        MIDI.sendNoteOn(note+i, 127, midiCh); // envia NoteOn(nota, velocity, canal midi)
        buttonPState[i] = buttonCState[i];
      }
      else {
        MIDI.sendNoteOn(note+i, 0, midiCh);
        buttonPState[i] = buttonCState[i];
      }
    }
    
  }

  ////////////////////////////////////////////////////////////////////////////////////////

  for (int i=0; i<NPots; i++) { // le todas entradas analogicas utilizadas
    potCState[i] = analogRead(pot[i]);
  }

    /* para que seja feita apenas a leitura das portas analogicas quando elas sao utilizadas, sem perder resolucao, 
    ´e  preciso estabelecer um "threshold" (varThreshold),  um valor minimo que as portas tenham que ser movimentadas 
    para que se comece a leitura. Apos isso cria-se uma especie de "portao", um portao que se abre e permite
   que as porta analogicas sejam lidas sem interrupcao por determinado tempo (TIMEOUT). Quando o timer ´e menor que TIMEOUT
   significa que o potenciometro foi mexido ha muito pouco tempo, o que significa que ele provavelmente ainda esta se movendo,
   logo deve-se manter o "portao" aberto; caso o timer seja maior que TIMEOUT siginifica que ja faz um tempo que ele nao ´e movimentado,
   logo o portao deve ser fechado. Para que essa logica aconteca deve-se zerar o timer (linhas 99 e 100) a cada vez que a porta analogica
   variar mais que o varThreshold estabelecido.
    */
  for (int i=0; i<NPots; i++) {

    potVar = abs(potCState[i] - potPState[i]); // calcula a variacao da porta analogica

    if (potVar >= varThreshold) {  //sets a threshold for the variance in the pot state, if it varies more than x it sends the cc message
      pTime[i] = millis(); // armazena o tempo previo
    }
    timer[i] = millis() - pTime[i]; // reseta o timer
    if (timer[i] < TIMEOUT) { // se o timer for menor que o tempo maximo permitido significa que o potenciometro ainda esta se movendo
      potMoving = true;
    }
    else {
      potMoving = false;
    }

    if (potMoving == true) { // se o potenciometro ainda esta se movendo, mande o control change
      MIDI.sendControlChange(cc+i, map(potCState[i], 0, 1023, 0, 127), midiCh); // envia Control Change (numero do CC, valor do CC, canal midi)
      potPState[i] = potCState[i]; // armazena a leitura atual do potenciometro para comparar com a proxima
    }
  }
  
}
