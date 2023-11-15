/*
  Feito por Gustavo Silveira, 2023.
  - Este Sketch lê as portas digitais e analógicas do Arduino e envia notas midi e mudanças de controle midi

  http://www.musiconerd.com
  http://www.youtube.com/musiconerd
  http://facebook.com/musiconerdmusiconerd
  http://instagram.com/musiconerd/
  http://www.gustavosilveira.net
  gustavosilveira@musiconerd.com

  Se estiver usando para algo que não seja uso pessoal, não esqueça de dar crédito.

  PS: Apenas altere o valor que possui um comentário como " // "

*/

/////////////////////////////////////////////
// Escolhendo sua placa
// Defina sua placa, escolha:
// "ATMEGA328" se estiver usando ATmega328 - Uno, Mega, Nano...
// "ATMEGA32U4" se estiver usando com ATmega32U4 - Micro, Pro Micro, Leonardo...
// "TEENSY" se estiver usando uma placa Teensy
// "DEBUG" se você apenas deseja depurar o código no monitor serial
// você não precisa comentar ou descomentar qualquer biblioteca MIDI abaixo depois de definir sua placa

#define DEBUG 1  // coloque aqui o uC que você está usando, como nas linhas acima seguido por "1", como "ATMEGA328 1", "DEBUG 1", etc.

/////////////////////////////////////////////
// Você está usando botões?
#define USING_BUTTONS 1  // comente se não estiver usando botões

/////////////////////////////////////////////
// Você está usando potenciômetros?
#define USING_POTENTIOMETERS 1  // comente se não estiver usando potenciômetros

/////////////////////////////////////////////
// BIBLIOTECAS
// -- Define a biblioteca MIDI -- //

// se estiver usando com ATmega328 - Uno, Mega, Nano...
#ifdef ATMEGA328
#include <MIDI.h>  // por Francois Best
//MIDI_CREATE_DEFAULT_INSTANCE();

// se estiver usando com ATmega32U4 - Micro, Pro Micro, Leonardo...
#elif ATMEGA32U4
#include "MIDIUSB.h"

#endif

#ifdef USING_POTENTIOMETERS
// incluir a biblioteca ResponsiveAnalogRead
#include <ResponsiveAnalogRead.h>  // [https://github.com/dxinteractive/ResponsiveAnalogRead](https://github.com/dxinteractive/ResponsiveAnalogRead)

#endif
// ---- //

/////////////////////////////////////////////
// BOTÕES
#ifdef USING_BUTTONS

const int N_BUTTONS = 3;                                // número total de botões
const int BUTTON_ARDUINO_PIN[N_BUTTONS] = { 2, 3, 4 };  // pinos de cada botão conectado diretamente ao Arduino

int buttonCState[N_BUTTONS] = {};  // armazena o valor atual do botão
int buttonPState[N_BUTTONS] = {};  // armazena o valor anterior do botão

//#define pin13 1 // descomente se estiver usando o pino 13 (pino com led), ou comente a linha se não estiver usando
byte pin13index = 12;  // coloque o índice do pino 13 do array de pinos do botãoPin[] se estiver usando, se não, comente

// debounce
unsigned long lastDebounceTime[N_BUTTONS] = { 0 };  // a última vez que o pino de saída foi alternado
unsigned long debounceDelay = 50;                   // o tempo de debounce; aumente se a saída oscilar

#endif

/////////////////////////////////////////////
// POTENCIÔMETROS
#ifdef USING_POTENTIOMETERS

const int N_POTS = 2;                            // número total de potenciômetros (slide e rotativo)
const int POT_ARDUINO_PIN[N_POTS] = { A0, A1 };  // pinos de cada potenciômetro conectado diretamente ao Arduino

int potCState[N_POTS] = { 0 };  // estado atual do potenciômetro
int potPState[N_POTS] = { 0 };  // estado anterior do potenciômetro
int potVar = 0;                 // diferença entre o estado atual e anterior do potenciômetro

int midiCState[N_POTS] = { 0 };  // estado atual do valor midi
int midiPState[N_POTS] = { 0 };  // estado anterior do valor midi

const int TIMEOUT = 300;              // Tempo em que o potenciômetro será lido após exceder o varThreshold
const int varThreshold = 20;          // Limiar para a variação do sinal do potenciômetro
boolean potMoving = true;             // Se o potenciômetro está se movendo
unsigned long PTime[N_POTS] = { 0 };  // Tempo previamente armazenado
unsigned long timer[N_POTS] = { 0 };  // Armazena o tempo passado desde que o temporizador foi redefinido

int reading = 0;
// Leitura Responsiva Analógica
float snapMultiplier = 0.01;                      // (0.0 - 1.0) - Aumente para uma leitura mais rápida, mas menos suave
ResponsiveAnalogRead responsivePot[N_POTS] = {};  // cria um array para os potenciômetros responsivos. Ele é preenchido no Setup.

int potMin = 10;
int potMax = 1023;

#endif

/////////////////////////////////////////////
// MIDI
byte midiCh = 0;  // Canal MIDI a ser usado - comece com 1 para a biblioteca MIDI.h ou 0 para a biblioteca MIDIUSB
byte note = 36;   // Nota mais baixa a ser usada
byte cc = 1;      // Menor CC MIDI a ser usado


/////////////////////////////////////////////
// CONFIGURAÇÃO
void setup() {

  // Taxa de baud
  // use se estiver usando com ATmega328 (uno, mega, nano...)
  // 31250 para classe de compatibilidade MIDI | 115200 para Hairless MIDI
  Serial.begin(115200);  //

#ifdef DEBUG
  Serial.println("Modo de depuração");
  Serial.println();
#endif

#ifdef USING_BUTTONS
  // Botões
  // Inicializa os botões com resistores de pull up
  for (int i = 0; i < N_BUTTONS; i++) {
    pinMode(BUTTON_ARDUINO_PIN[i], INPUT_PULLUP);
  }

#ifdef pin13  // inicializa o pino 13 como entrada
  pinMode(BUTTON_ARDUINO_PIN[pin13index], INPUT);
#endif

#endif

#ifdef USING_POTENTIOMETERS
  for (int i = 0; i < N_POTS; i++) {
    responsivePot[i] = ResponsiveAnalogRead(0, true, snapMultiplier);
    responsivePot[i].setAnalogResolution(1023);  // define a resolução
  }
#endif
}

/////////////////////////////////////////////
// LOOP
void loop() {

#ifdef USING_BUTTONS
  buttons();
#endif

#ifdef USING_POTENTIOMETERS
  potentiometers();
#endif
}

/////////////////////////////////////////////
// BOTÕES
#ifdef USING_BUTTONS

void buttons() {

  for (int i = 0; i < N_BUTTONS; i++) {

    buttonCState[i] = digitalRead(BUTTON_ARDUINO_PIN[i]);  // lê os pinos do arduino

#ifdef pin13
    if (i == pin13index) {
      buttonCState[i] = !buttonCState[i];  // inverte o pino 13 porque ele possui um resistor pull-down em vez de um pull-up
    }
#endif

    if ((millis() - lastDebounceTime[i]) > debounceDelay) {

      if (buttonPState[i] != buttonCState[i]) {
        lastDebounceTime[i] = millis();

        if (buttonCState[i] == LOW) {

          // Envie a nota MIDI ON de acordo com a placa escolhida
#ifdef ATMEGA328
          // use se estiver usando com ATmega328 (uno, mega, nano...)
          MIDI.sendNoteOn(note + i, 127, midiCh);  // nota, velocidade, canal

#elif ATMEGA32U4
          // use se estiver usando com ATmega32U4 (micro, pro micro, leonardo...)
          noteOn(midiCh, note + i, 127);  // canal, nota, velocidade
          MidiUSB.flush();

#elif TEENSY
          //faça usbMIDI.sendNoteOn se estiver usando com Teensy
          usbMIDI.sendNoteOn(note + i, 127, midiCh);  // nota, velocidade, canal

#elif DEBUG
          Serial.print(i);
          Serial.println(": botão ligado");
#endif

        } else {

          // Envie a nota MIDI OFF de acordo com a placa escolhida
#ifdef ATMEGA328
          // use se estiver usando com ATmega328 (uno, mega, nano...)
          MIDI.sendNoteOn(note + i, 0, midiCh);  // nota, velocidade, canal

#elif ATMEGA32U4
          // use se estiver usando com ATmega32U4 (micro, pro micro, leonardo...)
          noteOn(midiCh, note + i, 0);  // canal, nota, velocidade
          MidiUSB.flush();

#elif TEENSY
          //faça usbMIDI.sendNoteOn se estiver usando com Teensy
          usbMIDI.sendNoteOn(note + i, 0, midiCh);  // nota, velocidade, canal

#elif DEBUG
          Serial.print(i);
          Serial.println(": botão desligado");
#endif
        }
        buttonPState[i] = buttonCState[i];
      }
    }
  }
}

#endif

/////////////////////////////////////////////
// POTENCIÔMETROS
#ifdef USING_POTENTIOMETERS

void potentiometers() {


  for (int i = 0; i < N_POTS; i++) {  // Percorre todos os potenciômetros

    reading = analogRead(POT_ARDUINO_PIN[i]);
    responsivePot[i].update(reading);
    potCState[i] = responsivePot[i].getValue();

    potCState[i] = analogRead(POT_ARDUINO_PIN[i]);  // lê os pinos do arduino

    midiCState[i] = map(potCState[i], potMin, potMax, 0, 127);  // Mapeia a leitura de potCState para um valor utilizável em midi
    //midiCState[i] = map(potCState[i], 0, 4096, 0, 127);  // Mapeia a leitura de potCState para um valor utilizável em midi - usar para ESP32

    if (midiCState[i] < 0) {
      midiCState[i] = 0;
    }
    if (midiCState[i] > 127) {
      midiCState[i] = 0;
    }

    potVar = abs(potCState[i] - potPState[i]);  // Calcula o valor absoluto entre a diferença entre o estado atual e anterior do potenciômetro
    //Serial.println(potVar);

    if (potVar > varThreshold) {  // Abre o portão se a variação do potenciômetro for maior que o limiar
      PTime[i] = millis();        // Armazena o tempo anterior
    }

    timer[i] = millis() - PTime[i];  // Redefine o temporizador 11000 - 11000 = 0ms

    if (timer[i] < TIMEOUT) {  // Se o temporizador for menor que o tempo máximo permitido, significa que o potenciômetro ainda está se movendo
      potMoving = true;
    } else {
      potMoving = false;
    }

    if (potMoving == true) {  // Se o potenciômetro ainda estiver se movendo, envie a mudança de controle
      if (midiPState[i] != midiCState[i]) {

        // Envie o CC MIDI de acordo com a placa escolhida
#ifdef ATMEGA328
        // use se estiver usando com ATmega328 (uno, mega, nano...)
        MIDI.sendControlChange(cc + i, midiCState[i], midiCh);  // número do cc, valor do cc, canal midi

#elif ATMEGA32U4
        //use se estiver usando com ATmega32U4 (micro, pro micro, leonardo...)
        controlChange(midiCh, cc + i, midiCState[i]);  //  (canal, número do CC, valor do CC)
        MidiUSB.flush();

#elif TEENSY
        //faça usbMIDI.sendControlChange se estiver usando com Teensy
        usbMIDI.sendControlChange(cc + i, midiCState[i], midiCh);  // número do cc, valor do cc, canal midi

#elif DEBUG
        Serial.print("Potenciômetro: ");
        Serial.print(i);
        Serial.print(" ");
        Serial.println(midiCState[i]);
//Serial.print("  ");
#endif

        potPState[i] = potCState[i];  // Armazena a leitura atual do potenciômetro para comparar com a próxima
        midiPState[i] = midiCState[i];
      }
    }
  }
}

#endif

/////////////////////////////////////////////
// se estiver usando com ATmega32U4 (micro, pro micro, leonardo...)
#ifdef ATMEGA32U4

// Funções MIDI Arduino (pro)micro MIDIUSB Library
void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = { 0x09, 0x90 | channel, pitch, velocity };
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = { 0x08, 0x80 | channel, pitch, velocity };
  MidiUSB.sendMIDI(noteOff);
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = { 0x0B, 0xB0 | channel, control, value };
  MidiUSB.sendMIDI(event);
}
#endif